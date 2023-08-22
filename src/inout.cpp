#include "inout.h"

#include "flags.h"
#include "main.h"

String globalinfo;

StatusLed statusLed;
Buttons buttons;

void StatusLed::setup()
{
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);

  set(STARTING, true);
  set(OFF, true);
}

bool StatusLed::hasToBlink(bool force)
{
  unsigned long now = millis();
  // forced blink
  if (force)
  {
    nextBlinkTime = now + settings.blinkTimeoutEighth;
    current = 0;
  }
  // if timeout is reached
  else if (now > nextBlinkTime)
  {
    unsigned long delta = now - nextBlinkTime;
    // too much time has passed, ignore delta correction
    if (delta > settings.blinkTimeoutEighth)
      nextBlinkTime = now + settings.blinkTimeoutEighth;
    else
      // delta correction
      nextBlinkTime = now + settings.blinkTimeoutEighth - delta;
    current = (current + 1) % 9;
  }
  else
    return false;
  return true;
}

void StatusLed::write()
{
  if (current == 8)
  {
    analogWrite(PIN_LED_G, 0);
    analogWrite(PIN_LED_R, 0);
  }
  else
  {
    uint8_t curMask = 1 << (7 - current);
    bool curG = (status & curMask) != 0;
    analogWrite(PIN_LED_G, curG ? settings.ledIntensity : 0);

    bool curR = ((status >> 8) & curMask) != 0;
    analogWrite(PIN_LED_R, curR ? settings.ledIntensity : 0);
  }
}

void StatusLed::set(Status newStatus, bool force)
{
  status = newStatus;
  handle(force || (status >> 8) == L_ON);
}

Status StatusLed::get()
{
  return status;
}

void StatusLed::handle(bool force)
{
  // in case of force, show all the pattern
  if (force)
  {
    hasToBlink(true);
    write();
    unsigned long now = millis();
    unsigned long next = now + settings.blinkTimeoutEighth * 9;
    while (now < next)
    {
      handle(false);
      now = millis();
    }
  }
  else if (hasToBlink(false))
  {
    write();
  }
}

void Buttons::setup()
{
  pinMode(PIN_BUT1, INPUT);
}

void Buttons::handle()
{
  int but1Read = digitalRead(PIN_BUT1);
  if (but1Read == BUT1_PUSHED)
  {
    if (but1 == 0)
    {
      but1 = millis();
      up();
    }
    on();
  }
  else
  {
    if (but1 != 0)
    {
      down();
      but1 = 0;
    }
  }
}

void Buttons::up() {}

void Buttons::on()
{
  ulong since = millis() - but1;
  if (since > 10000 && statusLed.get() != RESET_OR_RESTORE)
  {
    statusLed.set(RESET_OR_RESTORE, true);
  }
  else if (since > 500 && statusLed.get() != RESET && statusLed.get() != RESET_OR_RESTORE)
  {
    statusLed.set(RESET, true);
  }
}

void Buttons::down()
{
  ulong since = millis() - but1;
  if (since > 10000)
  {
    settings.resetOrRestore();
  }
  if (since > 500)
  {
    restart();
  }
}
