# pwm-mockup
A pwm mockup driver with sysfs support

This driver was created so that programs that
require pwm functionality, can not crash when
a real pwm does not exist.

Operation is a bit verbose because I am using
the serial output for testing validation. I use
this driver with QEMU :)
