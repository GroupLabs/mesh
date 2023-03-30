#include <string.h>

size_t copy_str(char *dst, const char *src, size_t dstsize)
{
  size_t len = strlen(src);
  if(dstsize) {
    size_t bl = (len < dstsize-1 ? len : dstsize-1);
    ((char*)memcpy(dst, src, bl))[bl] = 0;
  }
  return len;
}