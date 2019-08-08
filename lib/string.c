/*
 * libc string functions
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License version 2.
 */

#include "libcflat.h"

unsigned long strlen(const char *buf)
{
    unsigned long len = 0;

    while (*buf++)
	++len;
    return len;
}

char *strcat(char *dest, const char *src)
{
    char *p = dest;

    while (*p)
	++p;
    while ((*p++ = *src++) != 0)
	;
    return dest;
}

char *strcpy(char *dest, const char *src)
{
    *dest = 0;
    return strcat(dest, src);
}

int strncmp(const char *a, const char *b, size_t n)
{
    for (; n--; ++a, ++b)
        if (*a != *b || *a == '\0')
            return *a - *b;

    return 0;
}

int strcmp(const char *a, const char *b)
{
    return strncmp(a, b, SIZE_MAX);
}

char *strchr(const char *s, int c)
{
    while (*s != (char)c)
	if (*s++ == '\0')
	    return NULL;
    return (char *)s;
}

char *strstr(const char *s1, const char *s2)
{
    size_t l1, l2;

    l2 = strlen(s2);
    if (!l2)
	return (char *)s1;
    l1 = strlen(s1);
    while (l1 >= l2) {
	l1--;
	if (!memcmp(s1, s2, l2))
	    return (char *)s1;
	s1++;
    }
    return NULL;
}

void *memset(void *s, int c, size_t n)
{
    size_t i;
    char *a = s;

    for (i = 0; i < n; ++i)
        a[i] = c;

    return s;
}

void *memcpy(void *dest, const void *src, size_t n)
{
    size_t i;
    char *a = dest;
    const char *b = src;

    for (i = 0; i < n; ++i)
        a[i] = b[i];

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *a = s1, *b = s2;
    int ret = 0;

    while (n--) {
	ret = *a - *b;
	if (ret)
	    break;
	++a, ++b;
    }
    return ret;
}

void *memmove(void *dest, const void *src, size_t n)
{
    const unsigned char *s = src;
    unsigned char *d = dest;

    if (d <= s) {
	while (n--)
	    *d++ = *s++;
    } else {
	d += n, s += n;
	while (n--)
	    *--d = *--s;
    }
    return dest;
}

void *memchr(const void *s, int c, size_t n)
{
    const unsigned char *str = s, chr = (unsigned char)c;

    while (n--)
	if (*str++ == chr)
	    return (void *)(str - 1);
    return NULL;
}

long atol(const char *ptr)
{
    long acc = 0;
    const char *s = ptr;
    int neg, c;

    while (*s == ' ' || *s == '\t')
        s++;
    if (*s == '-'){
        neg = 1;
        s++;
    } else {
        neg = 0;
        if (*s == '+')
            s++;
    }

    while (*s) {
        if (*s < '0' || *s > '9')
            break;
        c = *s - '0';
        acc = acc * 10 + c;
        s++;
    }

    if (neg)
        acc = -acc;

    return acc;
}

extern char **environ;

char *getenv(const char *name)
{
    char **envp = environ, *delim;

    while (*envp) {
        delim = strchr(*envp, '=');
        if (delim && strncmp(name, *envp, delim - *envp) == 0)
            return delim + 1;
        ++envp;
    }
    return NULL;
}

/* Very simple glob matching. Allows '*' at beginning and end of pattern. */
bool simple_glob(const char *text, const char *pattern)
{
	bool star_start = false;
	bool star_end = false;
	size_t n = strlen(pattern);
	char copy[n + 1];

	if (pattern[0] == '*') {
		pattern += 1;
		n -= 1;
		star_start = true;
	}

	strcpy(copy, pattern);

	if (n > 0 && pattern[n - 1] == '*') {
		n -= 1;
		copy[n] = '\0';
		star_end = true;
	}

	if (star_start && star_end)
		return strstr(text, copy);

	if (star_end)
		return strstr(text, copy) == text;

	if (star_start) {
		size_t text_len = strlen(text);
		const char *suffix;

		if (n > text_len)
			return false;
		suffix = text + text_len - n;
		return !strcmp(suffix, copy);
	}

	return !strcmp(text, copy);
}
