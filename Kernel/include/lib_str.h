#ifndef LIB_STR_H
#define LIB_STR_H

void charcpy(char *dest, char *src, int length);
int strcmp(const char *str1, const char *str2);
void strcpy(char *dest, const char *src);
void strncpy(char *dest, const char *src, unsigned int n);
int strlen(const char *s);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, unsigned int n);
void reverse(char s[]);
void itoa(int n, char s[], int base);

#endif