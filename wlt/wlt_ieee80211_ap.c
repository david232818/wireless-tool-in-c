#include <stdio.h>
#include <stdlib.h>
#include "wlt_ieee80211_ap.h"

struct wlt_ieee80211_ap *wlt_ieee80211_ap_init(void)
{
    struct wlt_ieee80211_ap *ap;

    ap = malloc(sizeof(*ap));
    if (ap == NULL) {
	perror("malloc");
	return NULL;
    }
    ap->link.next = NULL;
    return ap;
}

void wlt_ieee80211_ap_destroy(struct wlt_ieee80211_ap *ap)
{
    free(ap);
}
