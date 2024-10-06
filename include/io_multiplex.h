#ifndef __IO_MULTIPLEX_H__
#define __IO_MULTIPLEX_H__

#include <stdint.h>
#include <sys/epoll.h>

/*
 * Note that types and functions defined in this header have to wrap
 * two OS dependent things: I/O multiplexing mechanism and internal
 * I/O or file representation.
 */

/*
 * We can use void pointer to store registered files for I/O multiplexing. But
 * user have to know which type is for internal file representation when using
 * below macro-like features. That is, it breaks the isolation of 
 * non-poratable part of a function that uses it.
 */
typedef int io_desc_t;

/* I/O event structure for multiplexing */
struct io_event {
    intptr_t descriptor;	/* descriptor of I/O multiplexer */
    int maxevts;		/* maximum events */
    int maxios;			/* maximum I/O */
    int timeout;		/* I/O event timeout */
    void *evtbuf;		/* event buffer */
    io_desc_t *iosbase;		/* registered I/O descriptors */
    io_desc_t *iosptr;		/* pointer for next I/O registeration */
};

#define IO_EVENT_GET_IOSBASE(io_evtp) ((io_evtp)->iosbase)

/* return a I/O descriptor in iosp and iosp points to the next */
#define IO_EVENT_IOSPTR_CHK_AND_ADVANCE(io_evtp, iosp) \
    (((iosp) <= (io_evtp)->iosptr) ? *(iosp)++ : *(iosp))

/* check whether idx-th event in event buffer is iosp's event */
#define IO_EVENT_GETDESC_FROM_IOSPTR_AND_ADVANCE(io_evtp, idx, iosp)	\
    (IO_EVENT_IOSPTR_CHK_AND_ADVANCE((io_evtp), (iosp))		\
     == ((*((struct epoll_event *) (io_evtp)->evtbuf + idx)).data.fd))

struct io_event *io_event_init(int maxios, int maxevts, int timeout);

int io_event_add(struct io_event *io_evtp,
		 io_desc_t io_descriptor);

/* io_event_cap: wait for events to be captured in specified timeout */
int io_event_cap(struct io_event *io_evtp);

void io_event_destroy(struct io_event *io_evtp);

#endif
