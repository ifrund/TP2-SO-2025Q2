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

void strcpy(char *dest, const char *src)
{
	while ((*dest++ = *src++) != '\0')
		;
}

void strncpy(char *dest, const char *src, unsigned int n)
{
	unsigned int i;
	for (i = 0; i < n && src[i] != '\0'; i++)
	{
		dest[i] = src[i];
	}
	dest[i] = '\0';
}

int strlen(const char *s)
{
	int n = 0;
	while (s[n] != '\0')
		n++;
	return n;
}

char *strcat(char *dest, const char *src)
{
	char *ret = dest;
	while (*dest)
		dest++;
	while ((*dest++ = *src++))
		;
	return ret;
}

char *strncat(char *dest, const char *src, unsigned int n)
{
	char *ret = dest;
	while (*dest)
		dest++;
	while (n-- && *src)
	{
		*dest++ = *src++;
	}
	*dest = '\0';
	return ret;
}

void reverse(char s[])
{
	int i, j;
	char c;
	for (i = 0, j = strlen(s) - 1; i < j; i++, j--)
	{
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

void itoa(int n, char s[], int base)
{
	int i = 0, sign = n;

	if (n < 0)
		n = -n;

	do
	{
		int digit = n % base;
		s[i++] = (digit < 10) ? digit + '0' : digit - 10 + 'A';
	} while ((n /= base) > 0);

	if (sign < 0)
		s[i++] = '-';

	s[i] = '\0';
	reverse(s);
}