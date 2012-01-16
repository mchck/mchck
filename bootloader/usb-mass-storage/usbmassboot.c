#include <stdint.h>

#include "msc.h"

int
main(void)
{
	/* just to link them */
	msc_init(NULL);
	msc_bulk_in(NULL);
	msc_bulk_out(NULL, 0, NULL);
	msc_control(NULL, NULL);
}
