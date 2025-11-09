// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <stdint.h>

void *memset(void *destination, int32_t c, uint64_t length)
{
	uint8_t chr = (uint8_t)c;
	char *dst = (char *)destination;

	while (length--)
		dst[length] = chr;

	return destination;
}

void *memcpy(void *destination, const void *source, uint64_t length)
{
	/*
	 * memcpy does not support overlapping buffers, so always do it
	 * forwards. (Don't change this without adjusting memmove.)
	 *
	 * For speedy copying, optimize the common case where both pointers
	 * and the length are word-aligned, and copy word-at-a-time instead
	 * of byte-at-a-time. Otherwise, copy by bytes.
	 *
	 * The alignment logic below should be portable. We rely on
	 * the compiler to be reasonably intelligent about optimizing
	 * the divides and modulos out. Fortunately, it is.
	 */
	uint64_t i;

	if ((uint64_t)destination % sizeof(uint32_t) == 0 &&
		(uint64_t)source % sizeof(uint32_t) == 0 &&
		length % sizeof(uint32_t) == 0)
	{
		uint32_t *d = (uint32_t *)destination;
		const uint32_t *s = (const uint32_t *)source;

		for (i = 0; i < length / sizeof(uint32_t); i++)
			d[i] = s[i];
	}
	else
	{
		uint8_t *d = (uint8_t *)destination;
		const uint8_t *s = (const uint8_t *)source;

		for (i = 0; i < length; i++)
			d[i] = s[i];
	}

	return destination;
}

void charcpy(char *dest, char *src, int length)
{
	for (int i = 0; i < length; i++)
		dest[i] = src[i];
}

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

int strcmp(const char *str1, const char *str2)
{
	while (*str1 && (*str1 == *str2))
	{
		str1++;
		str2++;
	}

	return *(unsigned char *)str1 - *(unsigned char *)str2;
}

char *strncpy(char *dest, const char *src, unsigned int n)
{
	unsigned int i;
	for (i = 0; i < n && src[i] != '\0'; i++)
		dest[i] = src[i];

	// Rellenar con '\0' si src es más corta que n
	for (; i < n; i++)
		dest[i] = '\0';

	return dest;
}