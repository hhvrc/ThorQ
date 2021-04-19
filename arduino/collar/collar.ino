#include <digitalWriteFast.h>

#define RF_PIN 12

// 281.69014 microseconds
#define DelayShort  282

// 845.07042 microseconds
#define DelayLong  845

// 1,690.14084 microseconds
#define DelaySync 1690

void writeLow()
{
    digitalWriteFast(RF_PIN, HIGH);
    delayMicroseconds(DelayShort);
    digitalWriteFast(RF_PIN, LOW);
    delayMicroseconds(DelayLong);
}
void writeHigh()
{
    digitalWriteFast(RF_PIN, HIGH);
    delayMicroseconds(DelayLong);
    digitalWriteFast(RF_PIN, LOW);
    delayMicroseconds(DelayShort);
}
void writeSync()
{
    digitalWriteFast(RF_PIN, HIGH);
    delayMicroseconds(DelaySync);
    digitalWriteFast(RF_PIN, LOW);
    delayMicroseconds(DelayLong);
}

// Write [00000000]
void writeValue0()
{
    writeLow();
    writeLow();
    writeLow();
    writeLow();
    writeLow();
    writeLow();
    writeLow();
    writeLow();
}
// Write [00001110]
void writeValue1()
{
    writeLow();
    writeLow();
    writeLow();
    writeLow();
    writeHigh();
    writeHigh();
    writeHigh();
    writeLow();
}
// Write [00011100]
void writeValue2()
{
    writeLow();
    writeLow();
    writeLow();
    writeHigh();
    writeHigh();
    writeHigh();
    writeLow();
    writeLow();
}
// Write [00101010]
void writeValue3()
{
    writeLow();
    writeLow();
    writeHigh();
    writeLow();
    writeHigh();
    writeLow();
    writeHigh();
    writeLow();
}
// Write [00111000]
void writeValue4()
{
    writeLow();
    writeLow();
    writeHigh();
    writeHigh();
    writeHigh();
    writeLow();
    writeLow();
    writeLow();
}
// Write [01000110]
void writeValue5()
{
    writeLow();
    writeHigh();
    writeLow();
    writeLow();
    writeLow();
    writeHigh();
    writeHigh();
    writeLow();
}
// Write [01010100]
void writeValue6()
{
    writeLow();
    writeHigh();
    writeLow();
    writeHigh();
    writeLow();
    writeHigh();
    writeLow();
    writeLow();
}
// Write [01100010]
void writeValue7()
{
    writeLow();
    writeHigh();
    writeHigh();
    writeLow();
    writeLow();
    writeLow();
    writeHigh();
    writeLow();
}

// Write [0001]
void writeActionShock()
{
    writeLow();
    writeLow();
    writeLow();
    writeHigh();
}
// Write [0010]
void writeActionVibrate()
{
    writeLow();
    writeLow();
    writeHigh();
    writeLow();
}
// Write [0100]
void writeActionBeep()
{
    writeLow();
    writeHigh();
    writeLow();
    writeLow();
}
// Write [0101]
void writeActionAuto()
{
    writeLow();
    writeHigh();
    writeLow();
    writeHigh();
}
// Write [1111]
void writeActionManual()
{
    writeHigh();
    writeHigh();
    writeHigh();
    writeHigh();
}

// Write [1000]
void writeChannel1()
{
    writeHigh();
    writeLow();
    writeLow();
    writeLow();
}
// Write [1111]
void writeChannel2()
{
    writeHigh();
    writeHigh();
    writeHigh();
    writeHigh();
}

// Write [1010101010101010]
void writeID()
{
    writeHigh();
    writeLow();
    writeHigh();
    writeLow();
    writeHigh();
    writeLow();
    writeHigh();
    writeLow();
    writeHigh();
    writeLow();
    writeHigh();
    writeLow();
    writeHigh();
    writeLow();
    writeHigh();
    writeLow();
}

enum Command
{
    Shock   = 0,
    Vibrate = 1,
    Beep    = 2,
    Auto    = 3,
    Manual  = 4
};

typedef void(*Func)();

Func commands[4];
Func values[8];

void writeMessageChannel1(int cmd, int v1, int v2, int v3, int v4)
{
    Func action = commands[cmd];

    writeSync();
    writeChannel1();
    (*action)();
    writeID();
    (*values[v1])();
    (*values[v2])();
    (*values[v3])();
    (*values[v4])();
    writeChannel1();
    (*action)();
    writeLow(); // End Sync
}
void writeMessageChannel2(int cmd, int v1, int v2, int v3, int v4)
{
    Func action = commands[cmd];

    writeSync();
    writeChannel2();
    (*action)();
    writeID();
    (*values[v1])();
    (*values[v2])();
    (*values[v3])();
    (*values[v4])();
    writeChannel2();
    (*action)();
    writeLow(); // End Sync
}

void setup()
{
    Serial.begin(115200);
    
    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);
  
    pinModeFast(RF_PIN, OUTPUT);

    // assign function pointers
    commands[Command::Shock]   = writeActionShock;
    commands[Command::Vibrate] = writeActionVibrate;
    commands[Command::Beep]    = writeActionBeep;
    commands[Command::Auto]    = writeActionAuto;
    commands[Command::Manual]  = writeActionManual;

    values[0] = writeValue0;
    values[1] = writeValue1;
    values[2] = writeValue2;
    values[3] = writeValue3;
    values[4] = writeValue4;
    values[5] = writeValue5;
    values[6] = writeValue6;
    values[7] = writeValue7;
}

int pos = 0;
int buf[3];

int prevShock = 0;
int prevVibrate = 0;
int prevBeep = 0;

void loop()
{
  if (Serial.available() > 0)
  {
    char c = Serial.read();
    if (c == '>') // Clear
    {
      Serial.write("clear");
      Serial.flush();
      pos = 0;
    }
    else if (c == '<') // Execute
    {
      int command  = buf[0];
      int channel  = buf[1];
      
      int activeVal  = buf[2];
      int shockVal   = 0;
      int vibrateVal = 0;
      int beepVal    = 0;

      switch (command)
      {
      case Command::Shock:
          shockVal = prevShock = activeVal;
          Serial.write("shock");
          Serial.flush();
          break;
      case Command::Vibrate:
          vibrateVal = prevVibrate = activeVal;
          Serial.write("vibrate");
          Serial.flush();
          break;
      case Command::Beep:
          beepVal = prevBeep = activeVal;
          Serial.write("beep");
          Serial.flush();
          break;
      case Command::Auto:
          shockVal = prevShock;
          vibrateVal = prevVibrate;
          beepVal = prevBeep;
          Serial.write("auto");
          Serial.flush();
          break;
      case Command::Manual:
      default:
        break;
      }

      if (channel == 1)
      {
          writeMessageChannel1(command, activeVal, shockVal, vibrateVal, beepVal);
      }
      else if (channel == 2)
      {
          writeMessageChannel2(command, activeVal, shockVal, vibrateVal, beepVal);
      }
      
      pos = 0;
    }
    else if (c == 'V')
    {
      Serial.write("ThorQ-1.0");
      Serial.flush();
      pos = 0;
    }
    else
    {
      buf[pos++] = c - '0';

      if (pos >= 3) {
        pos = 0;
      }
    }
  }
}
