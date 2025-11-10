// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <stdint.h>
#include "../include/lib_str.h"

void charcpy(char *dest, char *src, int length)
{
	for (int i = 0; i < length; i++)
		dest[i] = src[i];
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