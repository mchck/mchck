#include <mchck.h>
#include <usb/usb.h>

static void init_my_hid(int config);

#include "desc.h"

static void
init_my_hid(int config)
{
  // hid_init();
}

void
main(void)
{
  usb_init(&hid_dev);
  sys_yield_for_frogs();
}
