
void displayClear()
{
    lc.clearDisplay(0);
}

void displayOn()
{
    lc.setScanLimit(0, 4);
    lc.shutdown(0, false);
    lc.setIntensity(0, DISPLAY_INTENSITY);
    displayClear();
}

void displayOff()
{
    lc.shutdown(0, true);
}

void displaySetup()
{
    displayOn();
    delay(100);
    lc.setDigit(0, 0, 4, false);
    lc.setDigit(0, 3, 9, false);
    delay(500);
    displayClear();
    lc.setDigit(0, 1, 4, false);
    lc.setDigit(0, 2, 9, false);
    delay(500);
    displayClear();
}

void displayDigits(int h1, int h2, int m1, int m2)
{
    lc.setDigit(0, 0, h1, false);
    lc.setDigit(0, 1, h2, false);
    lc.setDigit(0, 2, m1, false);
    lc.setDigit(0, 3, m2, false);
}

void displayDigit(byte digit, byte value)
{
    lc.setDigit(0, digit, value, false);
}

void displayTime(int hh, int mm, int ss)
{
#if LED_PANEL == UNKNOWN_2481AS
    lc.setDigit(0, 0, (hh / 10) % 10, false);
    lc.setDigit(0, 1, hh % 10, ss % 2 == 0);
    lc.setDigit(0, 2, (mm / 10) % 10, false);
    lc.setDigit(0, 3, mm % 10, false);
#endif
#if LED_PANEL == ARKLED_SR420281N
    lc.setDigit(0, 0, (hh / 10) % 10, false);
    lc.setDigit(0, 1, hh % 10, true);
    lc.setDigit(0, 2, (mm / 10) % 10, false);
    lc.setDigit(0, 3, mm % 10, false);
#endif
}

void displayEmpty()
{
    lc.setChar(0, 0, 'E', false);
    lc.setChar(0, 1, 'E', false);
    lc.setChar(0, 2, 'E', false);
    lc.setChar(0, 3, 'E', false);
}

void displayShowOff()
{
    lc.setChar(0, 1, 'o', false);
    lc.setChar(0, 2, 'f', false);
    lc.setChar(0, 3, 'f', false);
}

void displayFlash()
{
    lc.setChar(0, 0, 'F', false);
    lc.setChar(0, 1, 'L', false);
}
void displayFlash(uint16_t num)
{
    lc.setChar(0, 0, 'F', false);
    lc.setDigit(0, 1, (num / 100) % 10, false);
    lc.setDigit(0, 2, (num / 10) % 10, false);
    lc.setDigit(0, 3, num % 10, false);
}

void displayBattery(byte val)
{
    lc.setChar(0, 3, (val / 100) % 10, false);
    lc.setChar(0, 2, (val / 10) % 10, false);
    lc.setChar(0, 1, val % 10, false);
}

char displayGetChar(byte val)
{
    char c = 0;
    switch (val)
    {
    case 0:
        c = 'a';
        break;
    case 1:
        c = 'b';
        break;
    case 2:
        c = 'c';
        break;
    case 3:
        c = 'd';
        break;
    case 4:
        c = 'e';
        break;
    case 5:
        c = 'f';
        break;
    case 6:
        c = 'h';
        break;
    }
    return c;
}

void displayMenuItem(byte item)
{
    lc.setChar(0, 0, displayGetChar(item), false);
}
void displayMenuItem(byte item, byte val)
{
    displayMenuItem(item);
    lc.setDigit(0, 2, (val / 10) % 10, false);
    lc.setDigit(0, 3, val % 10, false);
}
void displayMenuItem(byte item, bool val)
{
    displayMenuItem(item);
    if (val)
    {
        lc.setChar(0, 2, 'o', false);
        lc.setChar(0, 3, 'n', false);
    }
    else
    {
        lc.setChar(0, 2, 'o', false);
        lc.setChar(0, 3, 'f', false);
    }
}
