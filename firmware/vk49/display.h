
void displayClear()
{
    as.clear();
}

void displayOn()
{
    as.resume(true);
    as.setIntensity(DISPLAY_INTENSITY);
    displayClear();
}

void displayOff()
{
    as.shutdown(true);
}

void displaySetup()
{
    as.init(4, DISPLAY_INTENSITY);
    displayClear();
    delay(100);
    as.display("4  9");
    delay(500);
    displayClear();
    as.display(" 49 ");
    delay(500);
    displayClear();
}

void displayDigits(byte h1, byte h2, byte m1, byte m2)
{
    char txt[5];
    sprintf(txt, "%d%d.%d%d", h1, h2, m1, m2);
    as.display(txt);
}

void displayDigit(byte digit, byte value)
{
    as.displayNumber(digit, value);
}

void displayTime(int hh, int mm, int ss)
{
    char txt[5];
    sprintf(txt, "%d%d.%d%d", (hh / 10) % 10, hh % 10, (mm / 10) % 10, mm % 10);
    as.display(txt);
}

void displayEmpty()
{
    as.display("EEEE");
}

void displayShowOff()
{
    as.display(" off");
}

void displayFlash()
{
    as.display("FL");
}
void displayFlash(uint16_t num)
{
    char txt[4];
    sprintf(txt, "F%d%d%d", (num / 100) % 10, (num / 10) % 10, num % 10);
    as.display(txt);
}

void displayBattery(byte val)
{
    char txt[4];
    sprintf(txt, " %d%d%d", (val / 100) % 10, (val / 10) % 10, val % 10);
    as.display(txt);
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
    char txt[4];
    sprintf(txt, "%c", displayGetChar(item));
    as.display(txt);
}

void displayMenuItem(byte item, byte val)
{
    char txt[4];
    sprintf(txt, "%c %d%d", displayGetChar(item), (val / 10) % 10, val % 10);
    as.display(txt);
}

void displayMenuItem(byte item, bool val)
{
    char txt[4];
    if (val)
    {
        sprintf(txt, "%c on", displayGetChar(item));
    }
    else
    {
        sprintf(txt, "%c of", displayGetChar(item));
    }
    as.display(txt);
}
