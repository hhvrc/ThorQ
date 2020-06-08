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
    Serial.begin(9600);
    
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

int v_shock = 0;
int v_vibrate = 0;
int v_beep = 0;

void loop()
{
    if (Serial.available() > 0)
    {
        char c = Serial.read();
        if (c == 'c')
        {
            if (pos = 3)
            {
                int v0=buf[1], v1=buf[2], v2=0, v3=0, v4=0;

                switch (v0)
                {
                case Command::Shock:
                    v2 = v_shock = v1;
                    break;
                case Command::Vibrate:
                    v3 = v_vibrate = v1;
                    break;
                case Command::Beep:
                    v4 = v_beep = v1;
                    break;
                case Command::Auto:
                    v2 = v_shock;
                    v3 = v_vibrate;
                    v4 = v_beep;
                    break;
                case Command::Manual:
                    break;
                }

                if (v0 < 4)
                {


                    if (buf[0] == 1)
                    {
                        writeMessageChannel1(v0, v1, v2, v3, v4);
                    }
                    else if (buf[0] == 2)
                    {
                        writeMessageChannel2(v0, v1, v2, v3, v4);
                    }
                }
            }
            pos = 0;
            memset(buf, 0, sizeof(buf));
        }
        else
        {
          switch (c)
          {
            case '0':
              buf[pos] = 0;
              pos++;
              break;
            case '1':
              buf[pos] = 1;
              pos++;
              break;
            case '2':
              buf[pos] = 2;
              pos++;
              break;
            case '3':
              buf[pos] = 3;
              pos++;
              break;
            case '4':
              buf[pos] = 4;
              pos++;
              break;
            case '5':
              buf[pos] = 5;
              pos++;
              break;
            case '6':
              buf[pos] = 6;
              pos++;
              break;
            case '7':
              buf[pos] = 7;
              pos++;
              break;
          }
        }
        }
}