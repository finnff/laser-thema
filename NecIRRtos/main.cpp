

// simple IR signal detector


#include "hwlib.hpp"

unsigned long nec_remote_read(hwlib::target::pin_in IRpin);

int main(void) {

  namespace target = hwlib::target;

  auto tsop_signal = target::pin_in(target::pins::d8);

  // auto led = target::pin_out(target::pins::led);

  for (;;) {
    uint_fast64_t temp = nec_remote_read(tsop_signal);
    if (temp > 0) {
      hwlib::cout << temp << hwlib::endl;
    }
  }
}

unsigned long nec_remote_read(hwlib::target::pin_in IRpin) {
  unsigned long ir_code;
  unsigned long count = 0, i;
  // Check 9ms pulse (remote control sends logic high)

  // hwlib::cout << "w0" << hwlib::endl;
  while (IRpin.read() && (count < 180)) {
    /// mischien  inverteren van pin.Read()? als niet werkt.
    count++;
    hwlib::wait_us(40);
    // hwlib::cout << "w1" << hwlib::endl;
  }

  count = 0;
  // Check 4.5ms space (remote control sends logic low)
  while (!IRpin.read() && (count < 90)) {
    count++;
    hwlib::wait_us(40);
    // hwlib::cout << "w2" << hwlib::endl;
  }

  // Read code message (32-bit)
  for (i = 0; i < 32; i++) {
    count = 0;
    while ((!IRpin.read()) && (count < 15)) {
      count++;
      hwlib::wait_us(40);
      // hwlib::cout << "w3" << hwlib::endl;
    }
    if ((count > 14) || (count < 2)) { // NEC protocol?
      return 0;
    }

    count = 0;
    while (IRpin.read() && (count < 30)) {
      count++;
      hwlib::wait_us(40);
    }
    if ((count > 29)){
      // NEC protocol?
      hwlib::cout << "29+" << hwlib::endl;
      return 0;
    }
    if ((count < 4)) {
      hwlib::cout << "4-" << hwlib::endl;
      return 0;
    }

    if (count > 20) {                // If space width > 1ms
      ir_code |= 1ul << (31 - i);    // Write 1 to bit (31 - i)
    } else {                         // If space width < 1ms
      ir_code &= ~(1ul << (31 - i)); // Write 0 to bit (31 - i)
    }
  }
  return ir_code;
}