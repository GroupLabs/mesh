#ifndef STRING_H_H
#define STRING_H_H

#include <string.h>

// Some compilers don't have strlcpy() so our own version is provided:
size_t copy_str(char *dst, const char *src, size_t dstsize);

#endif