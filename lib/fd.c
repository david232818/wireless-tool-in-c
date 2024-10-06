#include <stdio.h>
#include "fd.h"
#include "io_multiplex.h"

/* file_getdesc: return file descriptor for I/O multiplexing */
io_desc_t file_getdesc(FILE *iop)
{
    if (!iop)
	return -1;

    /* Although POSIX defines fileno(), it is not in the ISO C. */
    return iop->_fileno;
}
