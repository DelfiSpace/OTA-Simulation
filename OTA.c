/*
 * SoftwareUpdateService.cpp
 *
 *  Created on: 27 Jul 2019
 *      Author: stefanosperett
 */

#include "OTA.h"
#include "error_handler.h"
#include "md5.h"
#include "slot_handler.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

uint8_t state_flags = 0;
uint8_t update_slot;
uint16_t num_update_blocks = 0;
uint16_t received_par_crcs = 0;

uint8_t payload_data[64] = {0};
uint8_t payload_size;

void start_OTA(unsigned char slot_number);

void receive_metadata(unsigned char* metadata);
void send_metadata(unsigned char slot_number);

void receive_partial_crcs(unsigned char* crc_block, unsigned char num_bytes);
void receive_block(unsigned char* data_block, unsigned short block_offset);
bool check_partial_crc(unsigned char* data_block, unsigned short block_offset);

unsigned char* send_missed_partials(unsigned char offset);

void check_md5(unsigned char slot_number);

void stop_OTA();

void erase_slot(unsigned char param);

void throw_error(unsigned char error);

uint8_t* command_handler(uint8_t* command) {
    uint8_t* workingBuffer = malloc(64);
    if (command[COMMAND_SERVICE] == SOFTWAREUPDATE_SERVICE) {
        // prepare response frame
        workingBuffer[COMMAND_DESTINATION] = command[COMMAND_SOURCE];
        workingBuffer[COMMAND_SOURCE] = command[COMMAND_DESTINATION];
        workingBuffer[COMMAND_SIZE] = PAYLOAD_SIZE_OFFSET;
        workingBuffer[COMMAND_SERVICE] = SOFTWAREUPDATE_SERVICE;
        workingBuffer[COMMAND_RESPONSE] = COMMAND_RESPONSE;

        payload_size = PAYLOAD_SIZE_OFFSET;

        switch (command[COMMAND_METHOD]) {
        case START_OTA:
            if(command[COMMAND_SIZE] == PAYLOAD_SIZE_OFFSET + 1) {
                if(command[COMMAND_DATA] == 1 || command[COMMAND_DATA] == 2) {
                    start_OTA(command[COMMAND_DATA] - 1);
                    if(payload_data[COMMAND_RESPONSE] == COMMAND_ERROR) throw_error(payload_data[COMMAND_DATA]);

                } else throw_error(SLOT_OUT_OF_RANGE);
            } else throw_error(PARAMETER_MISMATCH);
            break;

        case SEND_METADATA:
            if(command[COMMAND_SIZE] == METADATA_SIZE - 1 + PAYLOAD_SIZE_OFFSET) {
                receive_metadata(&(command[COMMAND_DATA]));
                if(payload_data[COMMAND_RESPONSE] == COMMAND_ERROR) throw_error(payload_data[COMMAND_DATA]);

            } else throw_error(PARAMETER_MISMATCH);
            break;

        case RECEIVE_METADATA:
            if(command[COMMAND_SIZE] == PAYLOAD_SIZE_OFFSET + 1) {
                if(command[COMMAND_DATA] == 1 || command[COMMAND_DATA] == 2) {
                    send_metadata(command[COMMAND_DATA] - 1);
                    if(payload_data[COMMAND_RESPONSE] == COMMAND_ERROR) throw_error(payload_data[COMMAND_DATA]);
                } else throw_error(SLOT_OUT_OF_RANGE);
            } else throw_error(PARAMETER_MISMATCH);
            break;

        case SEND_PARTIAL_CRCS:
            if(command[COMMAND_SIZE] <= BLOCK_SIZE + PAYLOAD_SIZE_OFFSET) {
                receive_partial_crcs(&(command[COMMAND_DATA]), command[COMMAND_SIZE] - PAYLOAD_SIZE_OFFSET);
                if(payload_data[COMMAND_RESPONSE] == COMMAND_ERROR) throw_error(payload_data[COMMAND_DATA]);
            } else throw_error(PARAMETER_MISMATCH);
            break;

        case RECEIVE_MISSED_PARTIALS:
//            if(command[COMMAND_PARAMETER_SIZE] == 1) {
//                data = send_missed_partials(command[COMMAND_PARAMETER]);
//                response[COMMAND_PARAMETER_SIZE] = 0;
//                if(*data != NO_ERROR) throw_error(response, *data);
//
//            } else throw_error(PARAMETER_MISMATCH);
            break;

        case SEND_BLOCK:
            if(command[COMMAND_SIZE] <= BLOCK_SIZE + 2 + PAYLOAD_SIZE_OFFSET) {
                receive_block(&(command[COMMAND_DATA + 2]), command[COMMAND_DATA] | (command[COMMAND_DATA + 1] << 8));
                if(payload_data[COMMAND_RESPONSE] == COMMAND_ERROR) throw_error(payload_data[COMMAND_DATA]);
            } else throw_error(PARAMETER_MISMATCH);
            break;

        case CHECK_MD5:
            if(command[COMMAND_SIZE] == PAYLOAD_SIZE_OFFSET + 1) {
                if(command[COMMAND_DATA] == 1 || command[COMMAND_DATA] == 2) {
                    check_md5(command[COMMAND_DATA] - 1);
                    if(payload_data[COMMAND_RESPONSE] == COMMAND_ERROR) throw_error(payload_data[COMMAND_DATA]);
                } else throw_error(SLOT_OUT_OF_RANGE);
            } else throw_error(PARAMETER_MISMATCH);
            break;

        case STOP_OTA:
            if(command[COMMAND_SIZE] == PAYLOAD_SIZE_OFFSET) {
                stop_OTA();
                if(payload_data[COMMAND_RESPONSE] == COMMAND_ERROR) throw_error(payload_data[COMMAND_DATA]);
            } else throw_error(PARAMETER_MISMATCH);
            break;

        case ERASE_SLOT:

            break;

        default:
            break;
        }

        workingBuffer[COMMAND_SIZE] = payload_size;
        memcpy(&workingBuffer[COMMAND_DATA], &payload_data[COMMAND_DATA], 64 - NUM_BLOCKS_OFFSET + 3);

        // command processed
        return workingBuffer;
    } else {
        // this command is related to another service,
        // report the command was not processed
        return NULL;
    }
}

void start_OTA(unsigned char slot_number) {
    unsigned char error;
    payload_size = PAYLOAD_SIZE_OFFSET;

    if((state_flags & UPDATE_FLAG) == 0) {
        unsigned char state;
        if((error = fram_read_bytes((METADATA_SIZE + PAR_CRC_SIZE) * slot_number, &state, 1)) != NO_ERROR) return throw_error(error);

        if(state == EMPTY) {
            state_flags |= UPDATE_FLAG;
            state_flags &= ~METADATA_FLAG;
            update_slot = slot_number;
            state = PARTIAL;
            if((error = fram_write_bytes((METADATA_SIZE + PAR_CRC_SIZE) * slot_number, &state, 1)) != NO_ERROR) return throw_error(error);
//            missed_blocks.arr = nullptr;
//            missed_blocks.arr_size = 0;
//            missed_blocks.num = 0;

        } else return throw_error(SLOT_NOT_EMPTY);
    } else return throw_error(UPDATE_ALREADY_STARTED);
}

void receive_metadata(unsigned char* metadata) {
    unsigned char error;
    payload_size = PAYLOAD_SIZE_OFFSET;

    if((state_flags & UPDATE_FLAG) > 0) {
        if((state_flags & METADATA_FLAG) == 0) {
            if((error = fram_write_bytes((METADATA_SIZE + PAR_CRC_SIZE) * update_slot + 1, metadata, METADATA_SIZE - 1)) != NO_ERROR) return throw_error(error);
            num_update_blocks = metadata[NUM_BLOCKS_OFFSET - 1] | (metadata[NUM_BLOCKS_OFFSET] << 8);
            state_flags |= METADATA_FLAG;
        } else return throw_error(METADATA_ALREADY_RECEIVED);
    } else return throw_error(UPDATE_NOT_STARTED);
}

void send_metadata(unsigned char slot_number) {
    unsigned char error;
    payload_size = PAYLOAD_SIZE_OFFSET + METADATA_SIZE ;

    if((error = fram_read_bytes((METADATA_SIZE + PAR_CRC_SIZE) * slot_number, &payload_data[COMMAND_DATA], METADATA_SIZE)) != NO_ERROR) return throw_error(error);
}

void receive_partial_crcs(unsigned char* crc_block, unsigned char num_bytes) {
    unsigned char error;
    payload_size = PAYLOAD_SIZE_OFFSET;

    if((state_flags & UPDATE_FLAG) > 0) {
        if((state_flags & METADATA_FLAG) > 0) {
            if((state_flags & PARTIAL_CRC_FLAG) == 0) {
                if(received_par_crcs < num_update_blocks * BLOCK_SIZE) {
                    if((error = fram_write_bytes((METADATA_SIZE + PAR_CRC_SIZE) * update_slot + METADATA_SIZE + received_par_crcs, crc_block, num_bytes)) != NO_ERROR) return throw_error(error);

                    received_par_crcs += num_bytes;
                } else return throw_error(PARAMETER_MISMATCH);
            } else return throw_error(PARTIAL_ALREADY_RECEIVED);
        } else return throw_error(METADATA_NOT_RECEIVED);
    } else return throw_error(UPDATE_NOT_STARTED);
}

unsigned char* send_missed_partials(unsigned char offset) {
//    int error;
//    unsigned char* data = (unsigned char*)malloc(BLOCK_SIZE + 2);
//    data[0] = 0;
//
//    if(offset * BLOCK_SIZE / 2 < missed_blocks.num) {
//        data[1] = (missed_blocks.arr_size - offset * BLOCK_SIZE / 2 > BLOCK_SIZE / 2) ? BLOCK_SIZE : (BLOCK_SIZE / 2 - (missed_blocks.arr_size - missed_blocks.num)) * 2;
//        memcpy(&data[2], &missed_blocks.arr[offset * BLOCK_SIZE / 2], data[1]);
//    } else throw_error(OFFSET_OUT_OF_RANGE);
//
//    return data;
    return NULL;
}

void receive_block(unsigned char* data_block, uint16_t block_offset) {
    unsigned char error;
    payload_size = PAYLOAD_SIZE_OFFSET;

    if((state_flags & UPDATE_FLAG) > 0) {
        if((state_flags & METADATA_FLAG) > 0) {
            if((state_flags & PARTIAL_CRC_FLAG) == 0) {
                if(received_par_crcs < num_update_blocks * BLOCK_SIZE) {
                    if(block_offset < num_update_blocks) {
                        if(check_partial_crc(data_block, block_offset)) {
                            if((error = slot_write_bytes(update_slot, block_offset * BLOCK_SIZE, data_block, BLOCK_SIZE)) != NO_ERROR) return throw_error(error);
                        } else {
//                            if(missed_blocks.arr == NULL) {
//                                missed_blocks.arr = malloc(BLOCK_SIZE / 2 * sizeof(uint16_t));
//                                missed_blocks.arr_size = BLOCK_SIZE / 2;
//                            }
//                            if(missed_blocks.num >= missed_blocks.arr_size) {
//                                missed_blocks.arr = realloc(missed_blocks.arr, (missed_blocks.arr_size + BLOCK_SIZE / 2) * sizeof(uint16_t));
//                                missed_blocks.arr_size += BLOCK_SIZE / 2;
//                            }
//                            missed_blocks.arr[missed_blocks.num] = block_offset;
//                            missed_blocks.num++;
                            return throw_error(CRC_MISMATCH);
                        }
                    } else return throw_error(OFFSET_OUT_OF_RANGE);
                } else return throw_error(PARAMETER_MISMATCH);
            } else return throw_error(PARTIAL_ALREADY_RECEIVED);
        } else return throw_error(METADATA_NOT_RECEIVED);
    } else return throw_error(UPDATE_NOT_STARTED);
}

bool check_partial_crc(unsigned char* data_block, uint16_t block_offset) {
    unsigned char error;
    unsigned char val = 0;

    for(int i = 0; i < BLOCK_SIZE; i++) {
        val = CRC_TABLE[val ^ data_block[i]];
    }

    unsigned char crc;
    if((error = fram_read_bytes((METADATA_SIZE + PAR_CRC_SIZE) * update_slot + METADATA_SIZE + block_offset, &crc, 1)) != NO_ERROR) {
        throw_error(error);
        return false;
    }

    return crc == val;
}

void check_md5(unsigned char slot_number) {
    unsigned char error;
    payload_size = PAYLOAD_SIZE_OFFSET + 1;

    unsigned char digest[CRC_SIZE];

    MD5_CTX md5_c;
    MD5_Init(&md5_c);

    uint16_t num_blocks;
    if((error = fram_read_bytes((METADATA_SIZE + PAR_CRC_SIZE) * slot_number + NUM_BLOCKS_OFFSET, (unsigned char*)&num_blocks, sizeof(uint16_t))) != NO_ERROR) return throw_error(error);

    unsigned char meta_crc[CRC_SIZE];
    if((error = fram_read_bytes((METADATA_SIZE + PAR_CRC_SIZE) * slot_number + CRC_OFFSET, meta_crc, CRC_SIZE)) != NO_ERROR) return throw_error(error);

    unsigned char buffer[BLOCK_SIZE];

    for(int i = 0; i < num_blocks; i++) {
        if((error = slot_read_bytes(slot_number, num_blocks * i, buffer, BLOCK_SIZE)) != NO_ERROR) return throw_error(error);
        MD5_Update(&md5_c, buffer, BLOCK_SIZE);
    }

    MD5_Final(digest, &md5_c);

    bool equal = true;
    for(int i = 0; i < CRC_SIZE; i++) {
        if(digest[i] != meta_crc[i]) {
            equal = false;
            break;
        }
    }

    payload_data[COMMAND_DATA] = equal;
}

void stop_OTA() {
    unsigned char error;
    payload_size = PAYLOAD_SIZE_OFFSET + 1;

    if((state_flags & UPDATE_FLAG) > 0) {
        check_md5(update_slot);
        if(payload_data[COMMAND_RESPONSE] == COMMAND_ERROR) return throw_error(payload_data[COMMAND_DATA]);
        if(payload_data[COMMAND_DATA]) {
           unsigned char temp = FULL;
           if((error = fram_write_bytes((METADATA_SIZE + PAR_CRC_SIZE) * update_slot, &temp, 1)) != NO_ERROR) return throw_error(error);
    //           free(missed_blocks.arr);
    //           missed_blocks.arr = NULL;
    //           missed_blocks.arr_size = 0;
    //           missed_blocks.num = 0;
           state_flags &= ~UPDATE_FLAG;
           update_slot = 0;
        } else return throw_error(MD5_MISMATCH);
    } else return throw_error(UPDATE_NOT_STARTED);
}

void erase_slot(unsigned char param) {
    payload_size = PAYLOAD_SIZE_OFFSET + 1;

    if((state_flags & UPDATE_FLAG) == 0) {
        if((state_flags & ERASE_FLAG) == 0) {
           if(param == ACKNOWLEDGE) {
               //TODO: erase slot
           } else payload_data[COMMAND_DATA] = false;
        } else payload_data[COMMAND_DATA] = ACKNOWLEDGE;
    } else return throw_error(UPDATE_ALREADY_STARTED);
}

void throw_error(unsigned char error) {
    payload_data[COMMAND_RESPONSE] = COMMAND_ERROR;
    payload_size = PAYLOAD_SIZE_OFFSET + 1;
    payload_data[COMMAND_DATA] = error;
}
