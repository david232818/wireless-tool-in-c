#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include "io_multiplex.h"

struct io_event *io_event_init(int maxios, int maxevts, int timeout)
{
    struct io_event *io_evtp;

    io_evtp = malloc(sizeof(*io_evtp));
    if (!io_evtp) {
	perror("malloc");
	goto OUT;
    }

    io_evtp->descriptor = epoll_create(maxios);
    if (io_evtp->descriptor == -1) {
	perror("epoll_create");
	goto OUT_RELEASE_IOEVENT;
    }

    io_evtp->evtbuf = malloc(sizeof(struct epoll_event) * maxevts);
    if (io_evtp->evtbuf == NULL) {
	perror("malloc");
	goto OUT_CLOSE;
    }

    io_evtp->iosbase = malloc(sizeof(*(io_evtp->iosbase)) * maxios);
    if (io_evtp->iosbase == NULL) {
	perror("malloc");
	goto OUT_RELEASE_IOEVENT_BUF;
    }

    io_evtp->maxevts = maxevts;
    io_evtp->maxios = maxios;
    io_evtp->timeout = timeout;
    io_evtp->iosptr = io_evtp->iosbase;
    return io_evtp;

OUT_RELEASE_IOEVENT_BUF:
    free(io_evtp->evtbuf);
    
OUT_CLOSE:
    close(io_evtp->descriptor);

OUT_RELEASE_IOEVENT:
    free(io_evtp);

OUT:
    return NULL;
}

int io_event_add(struct io_event *io_evtp,
	       io_desc_t io_descriptor)
{
    struct epoll_event evt;
    ptrdiff_t iosptr_offset;

    if (!io_evtp || (io_descriptor == -1))
	goto OUT;

    iosptr_offset = (ptrdiff_t) (io_evtp->iosptr - io_evtp->iosbase);
    if (iosptr_offset > io_evtp->maxios)
	goto OUT;

    *(io_evtp->iosptr) = io_descriptor;
    
    evt.events = EPOLLIN;
    evt.data.fd = *(io_evtp->iosptr);
    if (epoll_ctl(io_evtp->descriptor,
		  EPOLL_CTL_ADD,
		  *(io_evtp->iosptr),
		  &evt) < 0) {
	perror("epoll_ctl");
	goto OUT;
    }
    (io_evtp->iosptr)++;
    return 0;

OUT:
    return -1;
}

/* io_event_cap: wait for events to be captured in specified timeout */
int io_event_cap(struct io_event *io_evtp)
{
    int res;
    
    if (!io_evtp)
	goto OUT;
    
    res = epoll_wait(io_evtp->descriptor,
		     (struct epoll_event *) io_evtp->evtbuf,
		     io_evtp->maxevts,
		     io_evtp->timeout);
    if (res < 0)
	perror("epoll_wait");
    return res;

OUT:
    return -1;
}

void io_event_destroy(struct io_event *io_evtp)
{
    close(io_evtp->descriptor);
    free(io_evtp->evtbuf);
    io_evtp->evtbuf = NULL;
    free(io_evtp->iosbase);
    io_evtp->iosbase = io_evtp->iosptr = NULL;
    free(io_evtp);
}
