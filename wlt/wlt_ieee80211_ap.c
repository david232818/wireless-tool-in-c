#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "wlt_ieee80211_ap.h"

int wlt_ieee80211_ap_search(aplist_t *aplist,
			    struct wlt_ieee80211_ap *target)
{
    struct ll_node *curr, *next;
    struct wlt_ieee80211_ap *ap;

    LL_NODE_FOREACH(aplist, curr, next) {
	ap = curr->obj;
	if (memcmp(ap->bssid, target->bssid, 6) == 0)
	    return WLT_IEEE80211_AP_FOUND;
    }
    return WLT_IEEE80211_AP_NOT_FOUND;
}

void wlt_ieee80211_ap_add_tail(aplist_t *aplist,
			       struct wlt_ieee80211_ap *ap)
{
    ll_node_add_tail(aplist, &(ap->node));
}

struct wlt_ieee80211_ap *wlt_ieee80211_ap_init(void)
{
    struct wlt_ieee80211_ap *ap;

    ap = malloc(sizeof(*ap));
    if (ap == NULL) {
	perror("malloc");
	return NULL;
    }
    ll_node_init(&(ap->node), ap);
    return ap;
}

void wlt_ieee80211_ap_destroy(void *ap)
{
    free(ap);
}

void wlt_ieee80211_ap_list_clear(aplist_t *aplist)
{
    ll_node_clear(aplist, wlt_ieee80211_ap_destroy);
}
