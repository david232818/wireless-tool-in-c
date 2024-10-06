#ifndef __FD_H__
#define __FD_H__

#include <stdio.h>
#include "io_multiplex.h"

/* file_getdesc: return file descriptor for I/O multiplexing */
io_desc_t file_getdesc(FILE *iop);

#endif
