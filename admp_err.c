#include <stdio.h>

#include "admp_err.h"

#define ADMP_ERR_OFFSET ADMP_ERR_INVALID_ARGS

/* Error message table */
static char *errmsg[] = {
    "Invalid arguments (e.g., NULL, Out-Of-Range)",
    "socket() error",
    "IW command of ioctl() error",
    "IF command of ioctl() error",
    "close() error",
    "Unknown frame"
}; 

/* admp_printerr: print error message */
void admp_printerr(const admp_res_t res, const char *err_func)
{
    if (res < ADMP_ERR_OFFSET || res >= ADMP_ERR_COUNT)
	return ;

    if (err_func == NULL)
	return ;

    perror(err_func);
    fprintf(stderr,
	    "\n%s in %s\n",
	    errmsg[res - ADMP_ERR_OFFSET], err_func);
}
