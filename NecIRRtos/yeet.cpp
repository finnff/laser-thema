


























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

  // Check 4.5ms space (remote control sends logic low)
  while (!IRpin.read() && (count < 90)) {
    count++;
    hwlib::wait_us(40);
    // hwlib::cout << "w2" << hwlib::endl;
  };



  int b1[32] = {};

  int datastate = 0;
  uint_fast64_t temptime;
  switch (datastate) {
    case 0:
      if (IRpin.read()) {
        temptime = hwlib::now_us();
        datastate = 1;
      } else {
        hwlib::wait_us(30);
        datastate = 0;
        break;
      }
      break;
    case 1:
    if (!IRpin.read()) {
        emptime = hwlib::now_us();
      } else {
        hwlib::wait_us(30);
      }
      return 0;
  
  }}

  // // Read code message (32-bit)
  // for (i = 0; i < 32; i++) {
  //   if (IRpin.read()) {
  //     if (!IRpin.read())
  //       uint_fast64_t temptime = hwlib::now_us;
  //     if (IRpin.read()) {
  //       uint_fast64_t nowtime = hwlib::now_us;
  //     }
  //   }

    // if (count > 20) {                // If space width > 1ms
    //   ir_code |= 1ul << (31 - i);    // Write 1 to bit (31 - i)
    // } else {                         // If space width < 1ms
    //   ir_code &= ~(1ul << (31 - i)); // Write 0 to bit (31 - i)
    // }
    // hwlib::cout << (ir_code) << '\n';
  }
  return ir_code;
}












































#include "hwlib.hpp"

short nec_ok = 0;
unsigned int nec_state = 0, command, inv_command, i;
unsigned int address;
unsigned long nec_code;
uint_fast64_t pulsetime;
uint_fast64_t tempendtime;
uint_fast64_t pulsetime;
uint_fast64_t case2time;
hwlib::target::pin_in IRpin;

void init(const hwlib::target::pin_in input) { IRpin = input; }

void ext_isr(void) {
  if (nec_state != 0) {
    uint_fast64_t starttime = hwlib::now_us(); // Store Timer1 value
  }
  switch (nec_state) {

  case 0: // Start receiving IR data (we're at the beginning of 9ms pulse)
    if (IRpin.read()) {
      uint_fast64_t temptime = hwlib::now_us();
      if (!IRpin.read()) {
        tempendtime = hwlib::now_us();
        pulsetime = tempendtime - temptime;
      }
    }
    nec_state = 1; // Next state: end of 9ms pulse (start of 4.5ms space)
    i = 0;
    return;

  case 1: // End of 9ms pulse
    if ((pulsetime > 9500) ||
        (pulsetime < 8500)) { // Invalid interval ==> stop decoding and reset
      nec_state = 0;          // Reset decoding process
    } else
      nec_state = 2; // Next state: end of 4.5ms space (start of 562µs pulse)
    ;                // Toggle external interrupt edge
    return;

  case 2: // End of 4.5ms space
    if (IRpin.read()) {
      case2time = hwlib::now_us();
      uint_fast64_t yeettime = case2time - tempendtime;
      if ((yeettime > 5000) ||
          (yeettime < 4000)) { // Invalid interval ==> stop decoding and reset
        nec_state = 0;         // Reset decoding process
        return;
      } else {
        nec_state = 3; // Next state: end of 562µs pulse (start of 562µs or
                       // 1687µs space)
        return;
      }
    }

  case 3: // End of 562µs pulse
    if ((time > 700) ||
        (time < 400)) { // Invalid interval ==> stop decoding and reset
      nec_state = 0;    // Reset decoding process
    } else
      nec_state = 4; // Next state: end of 562µs or 1687µs space
    return;

  case 4: // End of 562µs or 1687µs space
    if ((time > 1800) ||
        (time < 400)) { // Invalid interval ==> stop decoding and reset
      nec_state = 0;    // Reset decoding process
      return;
    }
    if (time > 1000)                 // If space width > 1ms (short space)
      bit_set(nec_code, (31 - i));   // Write 1 to bit (31 - i)
    else                             // If space width < 1ms (long space)
      bit_clear(nec_code, (31 - i)); // Write 0 to bit (31 - i)
    i++;
    if (i > 31) { // If all bits are received
      nec_ok = 1; // Decoding process OK
    }
    nec_state =
        3; // Next state: end of 562µs pulse (start of 562µs or 1687µs space)
  }
}

#INT_TIMER1 // Timer1 interrupt (used for time out)
void timer1_isr(void) {
  nec_state = 0;               // Reset decoding process
  ext_int_edge(H_TO_L);        // External interrupt edge from high to low
  setup_timer_1(T1_DISABLED);  // Disable Timer1 module
  clear_interrupt(INT_TIMER1); // Clear Timer1 interrupt flag bit
}
void main() {
  clear_interrupt(INT_TIMER1);   // Clear Timer1 interrupt flag bit
  enable_interrupts(INT_TIMER1); // Enable Timer1 interrupt
  printf(lcd_putc, "Address:0x0000");
  printf(lcd_putc, "Com:0x00 In:0x00");
  if (nec_ok) { // If the mcu successfully receives NEC protocol message
    nec_ok = 0; // Reset decoding process
    nec_state = 0;
    setup_timer_1(T1_DISABLED); // Disable Timer1 module
    address = nec_code >> 16;
    command = nec_code >> 8;
    inv_command = nec_code;
  }