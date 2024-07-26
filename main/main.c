#include <stdio.h>
#include "wlt.h"

int main(int argc, char *argv[])
{
    struct wlt_dev *wdev;

    if (argc < 3) {
	fputs("Usage: %s [-c] <interface name>\n", stderr);
	return -1;
    }
    
    wdev = wlt_init(argv[2]);
    if (wdev == NULL)
	return -1;

    wlt_ieee80211_pcap(wdev);

    puts("destroying wlt..");
    wlt_destroy(wdev);
    return 0;
}
