void saySetup()
{
    if (!i2s.begin(I2S_32_BIT, SAMPLERATE_HZ))
    {
        Serial.println("VK49 | Failed to initialize I2S.");
        while (1)
            ;
    }
    i2s.enableTx();
}

#define SHOULD_BLINK (blinkH > -1 && blinkM > -1)

uint32_t toRead = BUFFER_SIZE;
bool blink = false;
int blinkH = -1;
int blinkM = -1;
byte rec = 0;
int16_t sample = 0;
uint32_t totalRecieved = 0;
uint32_t volume = 0;

bool processSample(byte part)
{
    sample = (part << (8 * rec)) | sample;
    rec++;
    totalRecieved++;
    if (rec == 2)
    {
        int32_t sampleX = sample * volume;
        i2s.write(sampleX, sampleX);
        rec = 0;
        sample = 0;

        if (blink && totalRecieved % 16000 == 0)
        {
            if (totalRecieved % 32000 == 0)
            {
                displayTime(blinkH, blinkM, 00);
            }
            else
            {
                displayClear();
            }
        }

        if (stopPlaying)
        {
            return false;
        }
    }
    return true;
}

bool saySample(byte sample, int blinkHh, int blinkMm)
{
    uint32_t offset = samplesOffsets[sample];
    uint32_t length = samplesLenghts[sample];

    if (offset == 0 || length == 0)
    {
        return false;
    }

    volume = volumes[currentVolume];
    blinkH = blinkHh;
    blinkM = blinkMm;
    blink = SHOULD_BLINK;
    isPlaying = true;
    stopPlaying = false;
    totalRecieved = 0;

    flash.readCustom(offset, length, processSample);

    isPlaying = false;
    if (blink)
    {
        displayClear();
    }
    return true;
}

bool saySample(byte sample)
{
    return saySample(sample, -1, -1);
}

bool sayNumber(byte num)
{
    if (num < 21)
    {
        return saySample(num);
    }
    bool cont = saySample((num / 10) * 10);
    if (cont && num % 10 != 0)
    {
        return saySample(num % 10);
    }
    return cont;
}

void sayTime(int hh, int mm, int ss)
{
    bool cont = true;
    cont = saySample(SAMPLE_INTRO);
    if (!cont)
        return;
    cont = sayNumber(hh);
    if (!cont)
        return;
    if (hh % 10 == 1 && hh != 11)
    {
        cont = saySample(SAMPLE_HOURS1);
    }
    else if ((hh % 10 == 2 && hh != 12) || (hh % 10 == 3 && hh != 13) || (hh % 10 == 4 && hh != 14))
    {
        cont = saySample(SAMPLE_HOURS2_4);
    }
    else
    {
        cont = saySample(SAMPLE_HOURS);
    }
    if (!cont)
        return;
    cont = sayNumber(mm);
    if (!cont)
        return;
    if (mm % 10 == 1 && mm != 11)
    {
        cont = saySample(SAMPLE_MINUTES1);
    }
    else if ((mm % 10 == 2 && mm != 12) || (mm % 10 == 3 && mm != 13) || (mm % 10 == 4 && mm != 14))
    {
        cont = saySample(SAMPLE_MINUTES2_4);
    }
    else
    {
        cont = saySample(SAMPLE_MINUTES);
    }
    if (!cont)
        return;
    saySample(SAMPLE_OUTRO);
}

void sayTestAllSamples()
{
    for (byte i = 0; i < MAX_SAMPLES; i++)
    {
        if (saySample(i))
        {
            delay(1000);
        }
    }
}