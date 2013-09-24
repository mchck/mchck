#include <stdio.h>
#include <usb/usb.h>

static void init_my_hid(int config);

#include "desc.h"

static void
init_my_hid(int config)
{
  // hid_init();
}

int
main(void)
{
  usb_init(&hid_dev);
  vusb_main_loop();
}
