void digitsPrepare(bool prepareAlarm)
{
    DateTime now;
    if (!prepareAlarm)
    {
        now = rtc.now();
    }
    else
    {
        now = rtc.getAlarmDateTime(1);
    }
    int hh = now.hour();
    int mm = now.minute();
    digits[0] = (byte)(hh / 10);
    digits[1] = (byte)(hh % 10);
    digits[2] = (byte)(mm / 10);
    digits[3] = (byte)(mm % 10);
}

void digitsSaveAsClock()
{
    clockSet(digits[0] * 10 + digits[1], digits[2] * 10 + digits[3]);
}

void digitsSaveAsAlarm()
{
    clockAlarmSet(digits[0] * 10 + digits[1], digits[2] * 10 + digits[3]);
}

void digitsNext(){
    currentDigit++;
    if(digits[0] > 1 && digits[1] > 3){
        digits[1] = 0;
    }
}

void digitsIncrease()
{
    digits[currentDigit]++;
    if (digits[currentDigit] > 9)
    {
        digits[currentDigit] = 0;
    }
    if (currentDigit == 0 && digits[0] > 2)
    {
        digits[0] = 0;
    }
    if (currentDigit == 1 && digits[0] == 2 && digits[1] > 3)
    {
        digits[1] = 0;
    }
    if (currentDigit == 2 && digits[2] > 5)
    {
        digits[2] = 0;
    }
}