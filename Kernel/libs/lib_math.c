// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <stdint.h>
#include "../include/lib_math.h"

uint32_t uintToBase(uint64_t value, char *buffer, uint32_t base)
{
	char *p = buffer;
	char *p1, *p2;
	uint32_t digits = 0;

	do
	{
		uint32_t remainder = value % base;
		*p++ = (remainder < 10) ? remainder + '0' : remainder + 'A' - 10;
		digits++;
	} while (value /= base);

	*p = 0;

	p1 = buffer;
	p2 = p - 1;
	while (p1 < p2)
	{
		char tmp = *p1;
		*p1 = *p2;
		*p2 = tmp;
		p1++;
		p2--;
	}

	return digits;
}

int pow(int base, int exponent)
{
	double result = 1.0;

	if (exponent > 0)
	{
		for (int i = 0; i < exponent; i++)
		{
			result *= base;
		}
	}
	else if (exponent < 0)
	{
		for (int i = 0; i < -exponent; i++)
		{
			result /= base;
		}
	}

	return result;
}