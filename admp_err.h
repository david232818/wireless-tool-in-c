#ifndef __ADMP_ERR_H__
#define __ADMP_ERR_H__

/* error types for admp */

typedef enum {
	ADMP_SUCCESS = 0,
	ADMP_FAIL = 1,
	ADMP_ERR_INVALID_ARGS = 2,
	ADMP_ERR_SOCKET,
	ADMP_ERR_IWCMD,
	ADMP_ERR_IFCMD,
	ADMP_ERR_CLOSE,
	ADMP_ERR_INVALID_FRAME,
	ADMP_ERR_COUNT
} admp_res_t;

void admp_printerr(admp_res_t, const char *);

#endif
