#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "md5.h"

#include "OTA.h"
#include "slot_handler.h"
#include "error_handler.h"

uint8_t update_slot = 0;
uint8_t state_flags = 0;

uint16_t num_update_blocks = 0;
uint16_t received_par_crcs = 0;

struct DynamicArray {
    uint16_t num;
    uint16_t* arr;
    uint16_t arr_size;
} missed_blocks;

uint8_t* start_OTA(uint8_t slot_number);

uint8_t* receive_metadata(uint8_t* metadata);
uint8_t* send_metadata(uint8_t slot_number);

uint8_t* receive_partial_crcs(uint8_t* crc_block, uint8_t num_bytes);
uint8_t* receive_block(uint8_t* data_block, uint16_t block_offset);
bool check_partial_crc(uint8_t* data_block, uint16_t block_offset);

uint8_t* check_md5(uint8_t slot_number);

uint8_t* stop_OTA();

uint8_t* erase_slot(uint8_t param);

/*  Description: Takes in commands and handles them accordingly.
    Simultates the transmission between ground station and the satellite.

    Command pattern: CMD - SIZE - DATA(32 bytes) */
uint8_t* command_handler(uint8_t* command) {
    uint8_t* response = malloc(MAX_COMMAND_SIZE);
    response[COMMAND_DESTINATION] = command[COMMAND_SOURCE];
    response[COMMAND_SOURCE] = command[COMMAND_DESTINATION];
    response[COMMAND_SERVICE] = 0;
    response[COMMAND_SIZE] = 7;
    response[COMMAND_STATE] = COMMAND_REPLY;
    response[COMMAND_METHOD] = command[COMMAND_METHOD];

    uint8_t* data = NULL;

    if((state_flags & ERASE_FLAG) > 0 && command[COMMAND_METHOD] != ERASE_SLOT) state_flags &= ~ERASE_FLAG;

    switch (command[COMMAND_METHOD])
    {
    case START_OTA:
        if(command[COMMAND_PARAMETER_SIZE] == 1) {
            if(command[COMMAND_PARAMETER] == 1 || command[COMMAND_PARAMETER] == 2) {
                data = start_OTA(command[COMMAND_PARAMETER] - 1);
                response[COMMAND_PARAMETER_SIZE] = 0;
                if(*data != NO_ERROR) set_error(response, *data);

            } else set_error(response, SLOT_OUT_OF_RANGE);
        } else set_error(response, PARAMETER_MISMATCH);
        break;

    case SEND_METADATA:
        if(command[COMMAND_PARAMETER_SIZE] == METADATA_SIZE - 1) {
            data = receive_metadata(&command[COMMAND_PARAMETER]);
            response[COMMAND_PARAMETER_SIZE] = 0;
            if(*data != NO_ERROR) set_error(response, *data);

        } else set_error(response, PARAMETER_MISMATCH);
        break;

    case RECEIVE_METADATA:
        if(command[COMMAND_PARAMETER_SIZE] == 1) {
            if(command[COMMAND_PARAMETER] == 1 || command[COMMAND_PARAMETER] == 2) {
                data = send_metadata(command[COMMAND_PARAMETER] - 1);
                if(*data == NO_ERROR) {
                    memcpy(&response[COMMAND_PARAMETER_SIZE], data + 1, data[1] + 1);
                    response[COMMAND_SIZE] += data[1];

                } else set_error(response, *data);
            } else set_error(response, SLOT_OUT_OF_RANGE);
        } else set_error(response, PARAMETER_MISMATCH);
        break;

    case SEND_PARTIAL_CRCS:
        if(command[COMMAND_PARAMETER_SIZE] <= BLOCK_SIZE) {
            data = receive_partial_crcs(&command[COMMAND_PARAMETER], command[COMMAND_PARAMETER_SIZE]);
            response[COMMAND_PARAMETER_SIZE] = 0;
            if(*data != NO_ERROR) set_error(response, *data);

        } else set_error(response, PARAMETER_MISMATCH);
        break;

    case SEND_BLOCK:
        if(command[COMMAND_PARAMETER_SIZE] <= BLOCK_SIZE + 2) {
            data = receive_block(&command[COMMAND_PARAMETER + 2], command[COMMAND_PARAMETER] | (command[COMMAND_PARAMETER + 1] << 8));
            response[COMMAND_PARAMETER_SIZE] = 0;
            if(*data != NO_ERROR) set_error(response, *data);

        } else set_error(response, PARAMETER_MISMATCH);
        break;

    case CHECK_MD5:
        if(command[COMMAND_PARAMETER_SIZE] == 1) {
            if(command[COMMAND_PARAMETER] == 1 || command[COMMAND_PARAMETER] == 2) {
                data = check_md5(command[COMMAND_PARAMETER] - 1);
                if(*data == NO_ERROR) {
                    memcpy(&response[COMMAND_PARAMETER_SIZE], data + 1, data[1] + 1);
                    response[COMMAND_SIZE] += data[1];

                } else set_error(response, *data);
            } else set_error(response, SLOT_OUT_OF_RANGE);
        } else set_error(response, PARAMETER_MISMATCH);
        break;

    case STOP_OTA:
        if(command[COMMAND_PARAMETER_SIZE] == 0) {
            data = stop_OTA();
            response[COMMAND_PARAMETER_SIZE] = 0;
            if(*data != NO_ERROR) set_error(response, *data);
        } else set_error(response, PARAMETER_MISMATCH);
        break;

    case ERASE_SLOT:
        
        break;

    default:
        break;
    }

    if(data != NULL) free(data);
    return response;
}

uint8_t* start_OTA(uint8_t slot_number) {
    uint8_t error;
    uint8_t* data = malloc(3);
    data[0] = 0;
    data[1] = 0;

    if((state_flags & UPDATE_FLAG) == 0) {
        state_flags |= UPDATE_FLAG;
        state_flags &= ~METADATA_FLAG;
        update_slot = slot_number;
        uint8_t temp = PARTIAL;
        if((error = fram_write_bytes((METADATA_SIZE + PAR_CRC_SIZE) * slot_number, &temp, 1)) != NO_ERROR) return throw_error(data, error);
        missed_blocks.arr = NULL;
        missed_blocks.arr_size = 0;
        missed_blocks.num = 0;
    } else return throw_error(data, UPDATE_ALREADY_STARTED);

    return data;
}

uint8_t* receive_metadata(uint8_t* metadata) {
    int error;
    uint8_t* data = malloc(3);
    data[0] = 0;
    data[1] = 0;

    if((state_flags & UPDATE_FLAG) > 0) {
        if((state_flags & METADATA_FLAG) == 0) {
            if((error = fram_write_bytes((METADATA_SIZE + PAR_CRC_SIZE) * update_slot + 1, metadata, METADATA_SIZE - 1)) != NO_ERROR) return throw_error(data, error);
            num_update_blocks = metadata[NUM_BLOCKS_OFFSET - 1] | (metadata[NUM_BLOCKS_OFFSET] << 8);
            state_flags |= METADATA_FLAG;
        } else return throw_error(data, METADATA_ALREADY_RECEIVED);
    } else return throw_error(data, UPDATE_NOT_STARTED);

    return data;
}

uint8_t* send_metadata(uint8_t slot_number) {
    int error;
    uint8_t* data = malloc(METADATA_SIZE + 2);
    data[0] = 0;
    data[1] = METADATA_SIZE;

    if((error = fram_read_bytes((METADATA_SIZE + PAR_CRC_SIZE) * slot_number, &data[2], METADATA_SIZE)) != NO_ERROR) return throw_error(data, error);

    return data;
}

uint8_t* receive_partial_crcs(uint8_t* crc_block, uint8_t num_bytes) {
    int error;
    uint8_t* data = malloc(3);
    data[0] = 0;
    data[1] = 0;

    if((state_flags & UPDATE_FLAG) > 0) {
        if((state_flags & METADATA_FLAG) > 0) {
            if((state_flags & PARTIAL_CRC_FLAG) == 0) {
                if(received_par_crcs < num_update_blocks * BLOCK_SIZE) {
                    if((error = fram_write_bytes((METADATA_SIZE + PAR_CRC_SIZE) * update_slot + METADATA_SIZE + received_par_crcs, crc_block, num_bytes)) != NO_ERROR) return throw_error(data, error);

                    received_par_crcs += num_bytes;
                } else return throw_error(data, PARAMETER_MISMATCH);
            } else return throw_error(data, PARTIAL_ALREADY_RECEIVED);    
        } else return throw_error(data, METADATA_NOT_RECEIVED);
    } else return throw_error(data, UPDATE_NOT_STARTED);

    return data;
}

uint8_t* receive_block(uint8_t* data_block, uint16_t block_offset) {
    int error;
    uint8_t* data = malloc(3);
    data[0] = 0;
    data[1] = 0;

    if((state_flags & UPDATE_FLAG) > 0) {
        if((state_flags & METADATA_FLAG) > 0) {
            if((state_flags & PARTIAL_CRC_FLAG) == 0) {
                if(received_par_crcs < num_update_blocks * BLOCK_SIZE) {
                    if(check_partial_crc(data_block, block_offset)) {
                        if((error = slot_write_bytes(update_slot, block_offset * BLOCK_SIZE, data_block, BLOCK_SIZE)) != NO_ERROR) return throw_error(data, error);
                    } else {
                        if(missed_blocks.arr == NULL) {
                            missed_blocks.arr = calloc(BLOCK_SIZE / 2, sizeof(uint16_t));
                            missed_blocks.arr_size = BLOCK_SIZE / 2;
                        }
                        if(missed_blocks.num >= missed_blocks.arr_size) {
                            missed_blocks.arr = realloc(missed_blocks.arr, missed_blocks.arr_size + BLOCK_SIZE / 2);
                            missed_blocks.arr_size += BLOCK_SIZE / 2;
                        }
                        missed_blocks.arr[missed_blocks.num] = block_offset;
                        missed_blocks.num++;
                        printf("\nMissed block array: ");
                        for(int i = 0; i < missed_blocks.arr_size; i++) printf("%d ", missed_blocks.arr[i]);
                        puts("\n");
                        return throw_error(data, CRC_MISMATCH);
                    } 
                } else return throw_error(data, PARAMETER_MISMATCH);
            } else return throw_error(data, PARTIAL_ALREADY_RECEIVED);    
        } else return throw_error(data, METADATA_NOT_RECEIVED);
    } else return throw_error(data, UPDATE_NOT_STARTED);

    return data;
}

bool check_partial_crc(uint8_t* data_block, uint16_t block_offset) {
    uint8_t val = 0;

	for(int i = 0; i < BLOCK_SIZE; i++) {
		val = CRC_TABLE[val ^ data_block[i]];
	}
    
    uint8_t crc;
    fram_read_bytes((METADATA_SIZE + PAR_CRC_SIZE) * update_slot + METADATA_SIZE + block_offset, &crc, 1);

    return crc == val;
}

uint8_t* check_md5(uint8_t slot_number) {
    int error;
    uint8_t* data = malloc(3);
    data[0] = 0;
    data[1] = 1;

    uint8_t digest[CRC_SIZE];

    MD5_CTX md5_c;
    MD5_Init(&md5_c);

    uint16_t num_blocks;
    if((error = fram_read_bytes((METADATA_SIZE + PAR_CRC_SIZE) * slot_number + NUM_BLOCKS_OFFSET, (uint8_t*)&num_blocks, sizeof(uint16_t))) != NO_ERROR) return throw_error(data, error);

    uint8_t meta_crc[CRC_SIZE];
    if((error = fram_read_bytes((METADATA_SIZE + PAR_CRC_SIZE) * slot_number + CRC_OFFSET, meta_crc, CRC_SIZE)) != NO_ERROR) return throw_error(data, error);

    uint8_t* buffer = malloc(num_blocks * BLOCK_SIZE * sizeof(uint8_t));
    if(buffer == NULL) return throw_error(data, MEMORY_FULL);

    if((error = slot_read_bytes(slot_number, 0, buffer, num_blocks * BLOCK_SIZE)) != NO_ERROR) {
        free(buffer);
        return throw_error(data, error);
    }

    MD5_Update(&md5_c, buffer, num_blocks * BLOCK_SIZE);
    free(buffer);

    MD5_Final(digest, &md5_c);

    if(memcmp(digest, meta_crc, CRC_SIZE) == 0) {
        data[2] = true;
    } else {
        data[2] = false;
    }
    
    return data;
}

uint8_t* stop_OTA() {
    int error;
    uint8_t* data = malloc(3);
    data[0] = 0;
    data[1] = 1;

    if((state_flags & UPDATE_FLAG) > 0) {
        data = check_md5(update_slot);
        if(data[0] != NO_ERROR) return throw_error(data, data[0]);
        if(data[2]) {
            uint8_t temp = FULL;
            if((error = fram_write_bytes((METADATA_SIZE + PAR_CRC_SIZE) * update_slot, &temp, 1)) != NO_ERROR) return throw_error(data, error);
            state_flags &= ~UPDATE_FLAG;
            update_slot = 0;
        } else return throw_error(data, MD5_MISMATCH);
    } else return throw_error(data, UPDATE_NOT_STARTED);

    return data;
}

uint8_t* erase_slot(uint8_t param) {
    uint8_t* data = malloc(3);
    data[0] = 0;
    data[1] = 1;

    if((state_flags & UPDATE_FLAG) == 0) {
        if((state_flags & ERASE_FLAG) == 0) {
            if(param == ACKNOWLEDGE) {
                //TODO: erase slot
            } else data[2] = false;
        } else data[2] = ACKNOWLEDGE;
    } else return throw_error(data, UPDATE_ALREADY_STARTED);

    return data;
}