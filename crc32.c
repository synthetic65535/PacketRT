#include <stdint.h>
#include "crc32.h"


// Состояние CRC32-автомата
uint32_t crc32_state;


// Lookup table для CRC32-автомата
static uint32_t crc32_table(uint8_t index)
{
    switch (index)
    {
        case 0: return 0x00000000;
        case 1: return 0x1db71064;
        case 2: return 0x3b6e20c8;
        case 3: return 0x26d930ac;
        
        case 4: return 0x76dc4190;
        case 5: return 0x6b6b51f4;
        case 6: return 0x4db26158;
        case 7: return 0x5005713c;
        
        case 8: return 0xedb88320;
        case 9: return 0xf00f9344;
        case 10: return 0xd6d6a3e8;
        case 11: return 0xcb61b38c;
        
        case 12: return 0x9b64c2b0;
        case 13: return 0x86d3d2d4;
        case 14: return 0xa00ae278;
        case 15: return 0xbdbdf21c;
        
        default: return 0x00000000;
    }
}


// Сброс соятояния CRC32-автомата
void crc32_reset(void)
{
    crc32_state = ~0L;
}


// Подать символ в CRC32-автомат
void crc32_put(const uint8_t data)
{
    uint8_t table_index = 0;
    table_index = crc32_state ^ (data >> (0 * 4));
    crc32_state = crc32_table(table_index & 0x0f) ^ (crc32_state >> 4);
    table_index = crc32_state ^ (data >> (1 * 4));
    crc32_state = crc32_table(table_index & 0x0f) ^ (crc32_state >> 4);
}


// Получить результат вычисления CRC-32
uint32_t crc32_get(void)
{
    return ~crc32_state;
}


uint32_t calc_crc32(uint8_t *data, uint32_t length)
{
	crc32_reset();
	for (uint32_t i=0; i<length; i++) crc32_put(data[i]);
	return crc32_get();
}
