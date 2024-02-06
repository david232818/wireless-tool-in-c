#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_link.h>
#include <linux/wireless.h>

#include "admp_ieee80211_if.h"
#include "admp_err.h"
#include "admp_putils.h"

int main(int argc, char *argv[])
{
    admp_res_t res;
    struct admp_ieee80211_if_info iface;
    struct admp_ieee80211_ap_info ap;
    
    if (argc < 2 || argc > 3) {
	fprintf(stderr, "Usage: %s [-c|-s] <interface name>\n", argv[0]);
	return -1;
    }

    res = admp_ieee80211_if_init(argv[argc - 1], &iface);
    if (res != ADMP_SUCCESS) {
	fprintf(stderr,
		"Interface setup failed.. (%d)\n",
		res);
	return -1;
    }

    /* system("iwconfig"); */

    /* simple option parsing */
    switch (*(argv[1] + 1)) {
    case 'c':
	admp_ieee80211_pcap(&iface, &ap);
	break;
    case 's':
	break;
    default:
	break;
    }

    res = admp_ieee80211_if_destroy(&iface);
    return 0;
}
