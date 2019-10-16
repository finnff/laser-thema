#include "hwlib.hpp"

/* This is the period between timer interrupts in usec */
#define PERIOD 50
/* Threshold for error correction */
#define THRESHOLD 0.2
/* Buffer length for keeping received IR codes */
#define IRCODE_BUF_LEN 5
/* Do not change anything below */
#define START_PULSE_HIGH (9000 / PERIOD)
#define START_PULSE_LOW_DATA (4500 / PERIOD)
#define START_PULSE_LOW_REPEAT (2250 / PERIOD)
#define DATA_PULSE_HIGH (562 / PERIOD)
#define DATA_PULSE_LOW_ZERO (562 / PERIOD)
#define DATA_PULSE_LOW_ONE (1687 / PERIOD)
#define REPEAT_PULSE_HIGH (562 / PERIOD)
#define REPEAT_DISTANCE (108000 / PERIOD)

enum { IR_IDLE, IR_INIT, IR_INIT_LOW, IR_DATA_HIGH, IR_DATA_LOW, IR_REPEAT };

void init();
uint32_t getNext();

int pin;
int currentState;
size_t len;
uint32_t mask, next, received[IRCODE_BUF_LEN];
uint16_t pw, repeatpw;
bool mayRepeat;

static NECIRDecode *myInstance = NULL;

/*
 * The timer interrupt handler that uses the above trick to invoke the
 * stateMachine() method.
 */
void intWrapper()
{
    if(myInstance)
        myInstance->stateMachine();
}

/* Main constructor. */
NECIRDecode::NECIRDecode(int pin, int timer) : irTimer(HardwareTimer(timer))
{
    this->pin = pin;
    myInstance = this;
    currentState = IR_IDLE;
    mayRepeat = false;
    len = 0;
}

/* Initialization function, usually called in setup(). */
void NECIRDecode::init()
{
    pinMode(pin, INPUT);

    irTimer.pause();
    irTimer.setPeriod(PERIOD);
    irTimer.setChannel1Mode(TIMER_OUTPUTCOMPARE);
    irTimer.setCompare1(1);
    irTimer.attachCompare1Interrupt(intWrapper);
    irTimer.refresh();
    irTimer.resume();
}



void stateMachine(hwlib::target::pin_in &IRpin) {
  bool inp = (IRpin.read()) ? false : true;

  if (mayRepeat) {
    repeatpw++;
    if (repeatpw > ((1 + THRESHOLD) * REPEAT_DISTANCE))
      mayRepeat = false;
  }

  switch (currentState) {
  case IR_IDLE:
    if (inp == false)
      break;
    currentState = IR_INIT;
    pw = 0;
    repeatpw = 0;
    mayRepeat = true;
    break;
  case IR_INIT:
    if (inp == true) {
      pw++;
      break;
    }
    if ((pw > ((1 - THRESHOLD) * START_PULSE_HIGH)) &&
        (pw < ((1 + THRESHOLD) * START_PULSE_HIGH))) {
      currentState = IR_INIT_LOW;
    } else {
      currentState = IR_IDLE;
    }
    pw = 0;
    break;
  case IR_INIT_LOW:
    if (inp == false) {
      pw++;
      break;
    }
    if ((pw > ((1 - THRESHOLD) * START_PULSE_LOW_DATA)) &&
        (pw < ((1 + THRESHOLD) * START_PULSE_LOW_DATA))) {
      currentState = IR_DATA_HIGH;
      next = 0;
      mask = 0x80000000;
    } else if (mayRepeat && (pw > ((1 - THRESHOLD) * START_PULSE_LOW_REPEAT)) &&
               (pw < ((1 + THRESHOLD) * START_PULSE_LOW_REPEAT))) {
      currentState = IR_REPEAT;
    } else {
      currentState = IR_IDLE;
    }
    pw = 0;
    break;
  case IR_DATA_HIGH:
    if (inp == true) {
      pw++;
      break;
    }
    if ((pw > ((1 - THRESHOLD) * DATA_PULSE_HIGH)) &&
        (pw < ((1 + THRESHOLD) * DATA_PULSE_HIGH))) {
      currentState = IR_DATA_LOW;
    } else {
      currentState = IR_IDLE;
    }
    pw = 0;
    break;
  case IR_DATA_LOW:
    if (inp == false) {
      pw++;
      break;
    }
    if ((pw > ((1 - THRESHOLD) * DATA_PULSE_LOW_ZERO)) &&
        (pw < ((1 + THRESHOLD) * DATA_PULSE_LOW_ZERO))) {
      mask >>= 1;
      currentState = IR_DATA_HIGH;
      if (mask == 0) {
        currentState = IR_IDLE;
        received[len++] = next;
      }
    } else if ((pw > ((1 - THRESHOLD) * DATA_PULSE_LOW_ONE)) &&
               (pw < ((1 + THRESHOLD) * DATA_PULSE_LOW_ONE))) {
      next |= mask;
      mask >>= 1;
      currentState = IR_DATA_HIGH;
      if (mask == 0) {
        currentState = IR_IDLE;
        received[len++] = next;
      }
    } else {
      currentState = IR_IDLE;
    }
    pw = 0;
    break;
  case IR_REPEAT:
    if (inp == true) {
      pw++;
      break;
    }
    if ((pw > ((1 - THRESHOLD) * REPEAT_PULSE_HIGH)) &&
        (pw < ((1 + THRESHOLD) * REPEAT_PULSE_HIGH))) {
      received[len++] = 0xffffffff;
    }
    currentState = IR_IDLE;
    pw = 0;
    break;
  }
}

uint32_t getNext() {
  size_t i;
  uint32_t ret;

  if (len == 0)
    return 0;

  ret = received[0];

  for (i = 1; i < len; i++)
    received[i - 1] = received[i];
  len--;

  return ret;
}