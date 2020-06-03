// the data rate is 3,6 kHz, so one pulse is 277,778 microseconds
// On arduino UNO call/ret operations take 4 cycles each, we will take this into consideration

#define CLOCK 16
#define CALLRET_CYCLES 4 // UNO=4 MEGA=5

#if   CLOCK == 8
// One pulse at this clock rate is 2222 cycles

// Delay for exactly 69,444 microseconds
__attribute__((naked)) void delayShort()
{
    // 0,25 pulses ==  69,444 us == 556 cycles

#if   CALLRET_CYCLES == 4
    asm volatile (
    );
#elif CALLRET_CYCLES == 5
    asm volatile (
    );
#endif
}
// Delay for exactly 208,333 microseconds
__attribute__((naked)) void delayLong()
{
    // 0,75 pulses == 208,333 us == 1666 cycles

#if   CALLRET_CYCLES == 4
    asm volatile (
    );
#elif CALLRET_CYCLES == 5
    asm volatile (
    );
#endif
}
// Delay for exactly 416,666 microseconds
__attribute__((naked)) void delaySync()
{
    // 1,50 pulses == 416,667 us == 3333 cycles

#if   CALLRET_CYCLES == 4
    asm volatile (
    );
#elif CALLRET_CYCLES == 5
    asm volatile (
    );
#endif
}
#elif CLOCK == 16
// One pulse at this clock rate is 4444 cycles

// Delay for exactly 69,444 microseconds
__attribute__((naked)) void delayShort()
{
    // 0,25 pulses ==  69,444 us == 1111 cycles

#if   CALLRET_CYCLES == 4
    // Compensate for call/ret delay (1111 - (4 * 2) cycles)
    // So wait for 1103 cycles
    asm volatile (
        "    ldi  r18, 2"	"\n"
        "    ldi  r19, 110"	"\n"
        "1:  dec  r19"	    "\n"
        "    brne 1b"	    "\n"
        "    dec  r18"	    "\n"
        "    brne 1b"	    "\n"
    );
#elif CALLRET_CYCLES == 5
    asm volatile (
    );
#endif
}
// Delay for exactly 208,333 microseconds
__attribute__((naked)) void delayLong()
{
    // 0,75 pulses == 208,333 us == 3333 cycles

#if   CALLRET_CYCLES == 4
    // Compensate for call/ret delay (3333 - (4 * 2) cycles)
    // So wait for 3325 cycles
    asm volatile (
        "    ldi  r18, 5"	"\n"
        "    ldi  r19, 80"	"\n"
        "1:  dec  r19"	    "\n"
        "    brne 1b"	    "\n"
        "    dec  r18"	    "\n"
        "    brne 1b"	    "\n"
        "    rjmp 1f"	    "\n"
        "1:"	            "\n"
    );
#elif CALLRET_CYCLES == 5
    asm volatile (
    );
#endif
}
// Delay for exactly 416,666 microseconds
__attribute__((naked)) void delaySync()
{
    // 1,50 pulses == 416,667 us == 6666 cycles

#if   CALLRET_CYCLES == 4
    // Compensate for call/ret delay (6666 - (4 * 2) cycles)
    // So wait for 6658 cycles
    asm volatile (
        "    ldi  r18, 9"	"\n"
        "    ldi  r19, 165"	"\n"
        "1:  dec  r19"	    "\n"
        "    brne 1b"	    "\n"
        "    dec  r18"	    "\n"
        "    brne 1b"	    "\n"
    );
#elif CALLRET_CYCLES == 5
    asm volatile (
    );
#endif
}
#endif

void writeLow()
{
    DDRB |= 0b00000100;
    delayShort();
    DDRB &= 0b11111011;
    delayLong();
}
void writeHigh()
{
    DDRB |= 0b00000100;
    delayLong();
    DDRB &= 0b11111011;
    delayShort();
}
void writeSync()
{
    DDRB |= 0b00000100;
    delaySync();
    DDRB &= 0b11111011;
    delayLong();
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
    Auto,
    Beep,
    Vibrate,
    Shock
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
    // Set pin mode and state
    DDRB  &= 0b11111011;
    PORTB |= 0b00000100;

    // assign function pointers
    commands[Command::Auto] = writeActionAuto;
    commands[Command::Beep] = writeActionBeep;
    commands[Command::Vibrate] = writeActionVibrate;
    commands[Command::Shock] = writeActionShock;

    values[0] = writeValue0;
    values[1] = writeValue1;
    values[2] = writeValue2;
    values[3] = writeValue3;
    values[4] = writeValue4;
    values[5] = writeValue5;
    values[6] = writeValue6;
    values[7] = writeValue7;
}

void loop()
{
    writeMessageChannel1(Command::Vibrate, 4, 0, 0, 0);
    writeMessageChannel2(Command::Shock, 7, 0, 0, 0);
}
