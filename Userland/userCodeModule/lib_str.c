// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <stddef.h>
#include "include/lib_str.h"

char *strchr(const char *str, int c)
{
    while (*str != '\0')
    {
        if (*str == (char)c)
        {
            return (char *)str;
        }
        str++;
    }
    if (c == '\0')
    {
        return (char *)str;
    }
    return NULL;
}

void remove_extra_spaces(char *str)
{
    int i = 0, j = 0;
    int in_space = 1;

    while (str[i])
    {
        char c = str[i++];
        if (c == ' ' || c == '\t' || c == '\n')
        {
            if (!in_space)
            {
                in_space = 1;
                str[j++] = ' ';
            }
        }
        else
        {
            in_space = 0;
            str[j++] = c;
        }
    }

    if (j > 0 && str[j - 1] == ' ')
        j--;

    str[j] = '\0';
}

int is_digit_str(char *str)
{
    if (str == 0 || *str == '\0')
        return 0;

    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] < '0' || str[i] > '9')
            return 0;
    }

    return 1;
}