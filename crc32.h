#include <stdint.h>

/* Библиотека для вычисления хеша CRC32 из непрерывного потока байтов или из буфера */

// Сброс соятояния CRC32-автомата
void crc32_reset(void);
	
// Подать символ в CRC32-автомат
void crc32_put(const uint8_t data);
	
// Получить результат вычисления CRC-32
uint32_t crc32_get(void);

// Вычислить хеш из буфера
uint32_t calc_crc32(uint8_t *data, uint32_t length);
