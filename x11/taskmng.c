#include "compiler.h"

#include "np2.h"
#include "toolkit.h"

#include "taskmng.h"

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_UNISTD_H
#include <unistd.h>
#endif

void
taskmng_initialize(void)
{

	np2running = 1;
}

BOOL
taskmng_sleep(UINT32 tick)
{
	UINT32 base;
	UINT32 now;
	UINT32 msec;
	struct timeval tv;
	int ret;

	base = GETTICK();
	while (taskmng_isavail() && (((now = GETTICK()) - base) < tick)) {
		toolkit_event_process();
		now = GETTICK();
		msec = (tick - (now - base) / 2);
		if (msec >= 0x80000000) /* minus? */
			break;
                tv.tv_usec = (msec % 1000) * 1000;
                tv.tv_sec = msec / 1000;
		do {
			ret = select(1, NULL, NULL, NULL, &tv);
		} while ((ret == -1) && errno == EINTR);
	}
	return taskmng_isavail();
}

void
taskmng_exit(void)
{

	np2running = 0;
}
