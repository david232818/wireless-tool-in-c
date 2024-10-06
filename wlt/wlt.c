#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "wlt.h"
#include "wlt_wnetdev.h"

struct wlt_wdev *wlt_wdev_init(char *namep)
{
    struct wlt_wdev *wdevp;

    if (namep == NULL)
	goto ERRINVAL;

    wdevp = malloc(sizeof(*wdevp));
    if (wdevp == NULL) {
	perror("malloc");
	goto ERRMEM;
    }

    if (wlt_wdev_open(wdevp) == -1)
	goto ERRSOCK;

    strncpy(wdevp->name, namep, sizeof(wdevp->name) - 1);

    if (wlt_getindex(wdevp) == -1)
	goto ERRIOCTL;

    if (wlt_getmode(wdevp) == -1)
	goto ERRIOCTL;

    /*
     * In initially plugged state, ioctl prints error while getting
     * channel.
     */
    wdevp->chann = 1;
    return wdevp;

ERRIOCTL:
    wlt_wdev_close(wdevp);

ERRSOCK:
    free(wdevp);

ERRINVAL:
ERRMEM:
    return NULL;
}

void wlt_wdev_destroy(struct wlt_wdev *wdevp)
{
    wlt_wdev_close(wdevp);
    memset(wdevp, 0x00, sizeof(*wdevp));
    free(wdevp);
}
