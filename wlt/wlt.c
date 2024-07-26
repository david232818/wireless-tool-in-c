#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <linux/wireless.h>
#include "wlt.h"
#include "wlt_ioctl.h"

struct wlt_dev *wlt_init(char *name)
{
    struct wlt_dev *wdev;

    if (name == NULL)
	return NULL;

    wdev = malloc(sizeof(*wdev));
    if (wdev == NULL) {
	perror("malloc");
	return NULL;
    }

    wdev->sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (wdev->sockfd == -1) {
	perror("socket");
	free(wdev);
	return NULL;
    }

    strncpy(wdev->name, name, IFNAMSIZ);

    if (wlt_getindex(wdev) == -1) {
	close(wdev->sockfd);
	free(wdev);
	return NULL;
    }

    if (wlt_getmode(wdev) == -1) {
	close(wdev->sockfd);
	free(wdev);
	return NULL;
    }

    /*
     * In initially plugged state, ioctl prints error while getting
     * channel.
     */
    wdev->chann = 1;
    return wdev;
}

void wlt_destroy(struct wlt_dev *wdev)
{
    close(wdev->sockfd);
    memset(wdev, 0x00, sizeof(*wdev));
    free(wdev);
}
