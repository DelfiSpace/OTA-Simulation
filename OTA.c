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

uint8_t* start_OTA(uint8_t slot_number);
uint8_t* receive_metadata(uint8_t* metadata);
uint8_t* send_metadata(uint8_t slot_number);
uint8_t* receive_partial_crcs(uint8_t* crc_block, uint8_t num_bytes);
uint8_t* receive_block();
void check_partial_crc();
uint8_t* check_md5(uint8_t slot_number);
void write_to_flash();
uint8_t* stop_OTA();
uint8_t* erase_slot(uint8_t param);

/*  Description: Takes in commands and handles them accordingly.
    Simultates the transmission between ground station and the satellite.

    Command pattern: CMD - SIZE - DATA(32 bytes) */
uint8_t* command_handler(uint8_t* command) {
    uint8_t* response = malloc(MAX_COMMAND_SIZE);
    response[COMMAND_DESTINATION] = command[COMMAND_SOURCE];
    response[COMMAND_SOURCE] = command[COMMAND_DESTINATION];
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
        receive_block();
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
    uint8_t* data = malloc(3);
    data[0] = 0;
    data[1] = 0;

    if((state_flags & UPDATE_FLAG) == 0) {
        state_flags |= UPDATE_FLAG;
        state_flags &= ~METADATA_FLAG;
        update_slot = slot_number;
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
            num_update_blocks = metadata[NUM_BLOCKS_OFFSET - 1];
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
            if(received_par_crcs < num_update_blocks * BLOCK_SIZE) {
                if((error = fram_write_bytes((METADATA_SIZE + PAR_CRC_SIZE) * update_slot + METADATA_SIZE + received_par_crcs, crc_block, num_bytes)) != NO_ERROR) return throw_error(data, error);

                received_par_crcs += num_bytes;
            } else return throw_error(data, PARAMETER_MISMATCH);
        } else return throw_error(data, METADATA_NOT_RECEIVED);
    } else return throw_error(data, UPDATE_NOT_STARTED);

    return data;
}

uint8_t* receive_block() {
    return NULL;
}

void check_partial_crc() {

}

uint8_t* check_md5(uint8_t slot_number) {
    int error;
    uint8_t* data = malloc(CRC_SIZE + 1 + 2);
    data[0] = 0;
    data[1] = CRC_SIZE + 1;

    MD5_CTX md5_c;
    MD5_Init(&md5_c);

    uint16_t num_blocks;
    if((error = fram_read_bytes((METADATA_SIZE + PAR_CRC_SIZE) * slot_number + NUM_BLOCKS_OFFSET, (uint8_t*)&num_blocks, sizeof(uint16_t))) != NO_ERROR) return throw_error(data, error);

    uint8_t meta_crc[CRC_SIZE];
    if((error = fram_read_bytes((METADATA_SIZE + PAR_CRC_SIZE) * slot_number + CRC_OFFSET, meta_crc, CRC_SIZE)) != NO_ERROR) return throw_error(data, error);

    uint8_t* buffer = malloc(num_blocks * sizeof(uint8_t));
    if(buffer == NULL) return throw_error(data, MEMORY_FULL);

    if((error = slot_read_bytes(slot_number, 0, buffer, num_blocks)) != NO_ERROR) {
        free(buffer);
        return throw_error(data, error);
    }

    MD5_Update(&md5_c, buffer, num_blocks);
    free(buffer);

    MD5_Final(&data[3], &md5_c);

    if(memcmp(&data[3], meta_crc, CRC_SIZE) == 0) {
        data[2] = true;
    } else {
        data[2] = false;
    }
    
    return data;
}

void write_to_flash() {

}

uint8_t* stop_OTA() {
    uint8_t* data = malloc(3);
    data[0] = 0;
    data[1] = 0;

    if((state_flags & UPDATE_FLAG) > 0) {
        state_flags &= ~UPDATE_FLAG;
        update_slot = 0;
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