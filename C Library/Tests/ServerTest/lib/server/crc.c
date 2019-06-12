#include "crc.h"

#define CRC8_POLY 0x91

// CCIT CRC16 calculation.
uint16_t CRC_Calculate16(uint16_t seed, uint8_t *buff, uint16_t size)
{
	uint8_t x;
	uint16_t crc = seed;

	while (size--)
	{
		x = crc >> 8 ^ *buff++;
		x ^= x >> 4;
		crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x << 5)) ^ ((uint16_t)x);
	}
	return crc;
}

uint8_t CRC_Calculate8(uint8_t seed, uint8_t *buff, uint16_t size)
{
	uint8_t crc = seed;

	for (uint16_t i = 0; i < size; i++)
	{
		crc ^= buff[i];

		for (uint8_t j = 0; j < 8; j++)
		{
			if (crc & 0x01)
			{
				crc ^= CRC8_POLY;
				crc >>= 1;
			}
		}
	}
	return crc;
}