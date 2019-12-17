#include <stdint.h>
#include <stdio.h>

#ifndef DEF_H_
#define DEF_H

#define BLOCK_SIZE 64
#define CRC_SIZE 16
#define METADATA_SIZE (CRC_SIZE + 4 + 2)

enum flags {
    ERASE_FLAG = 0x01,
    UPDATE_FLAG = 0x02
};

enum slot_status{
    EMPTY = 0x00,
    PARTIAL = 0x01,
    FULL = 0x02,
    TRANSMISSION = 0x04
};

enum update_status{
    IDLE,
    UPDATE    
} state;

struct Metadata
{
    enum slot_status status;
    uint32_t version;
    uint16_t num_blocks;
    uint8_t crc[CRC_SIZE];
    uint8_t* partial_crcs;
};

struct Slot {
    char* file;
    struct Metadata* meta;
    char* descriptor;
};
#endif // DEF_H