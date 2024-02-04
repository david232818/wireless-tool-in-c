#ifndef __ADMP_UTILS_H__
#define __ADMP_UTILS_H__

#include <stdint.h>
#include <stddef.h>

#define ADMP_IS_X_IN_RANGE(min, x, max) (((x) > (min)) && ((x) < (max)))

void admp_hexdump(const void *, size_t);

#endif
