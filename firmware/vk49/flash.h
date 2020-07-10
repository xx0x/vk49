
uint32_t currentFlashCount = 0;
uint32_t flashAddress = 0;
bool flashDisplayActive = false;

uint32_t flashReadInt(uint32_t address)
{
    byte buff[4];
    flash.readByteArray(address, buff, 4);
    return buff[0] << 24 | buff[1] << 16 | buff[2] << 8 | buff[3];
}

bool flashIsEmpty()
{
    return false;
    // return (flash.readByte(0) == 0);
}
void flashSetup(bool startup)
{

    Serial.print("VK49 | Flash manufacturer: ");
    Serial.println(flash.getManID());
    Serial.print("VK49 | Flash JEDEC ID: ");
    Serial.println(flash.getJEDECID());
    Serial.print("VK49 | Flash capacity: ");
    Serial.println(flash.getCapacity());
    Serial.println("");

    bool empty = flashIsEmpty();

    Serial.print("VK49 | Flash empty: ");
    Serial.println(empty);
    Serial.println("");

    if (empty)
    {
        return;
    }

    uint32_t flashHeaderLength = flashReadInt(0);
    uint32_t prevAddress = flashHeaderLength;

    // clear old stuff
    for (byte i = 0; i < MAX_SAMPLES; i++)
    {
        samplesOffsets[i] = 0;
        samplesLenghts[i] = 0;
    }

    // clear old alarms
    alarmsCount = 0;

    // read new data
    for (byte i = 4; i < flashHeaderLength; i += 5)
    {
        byte num = flash.readByte(i);
        samplesOffsets[num] = prevAddress;
        samplesLenghts[num] = flashReadInt(i + 1);
        prevAddress += samplesLenghts[num];
        if (num >= SAMPLE_ALARM_BASE)
        {
            alarmsCount++;
        }
    }

    // check alarm, whether available
    if (currentAlarm >= alarmsCount)
    {
        currentAlarm = 0;
    }

    // display new data
    for (byte i = 0; i < MAX_SAMPLES; i++)
    {
        if (samplesLenghts[i] != 0)
        {
            Serial.print("VK49 | Sample ");
            if (i < 10)
            {
                Serial.print(" ");
            }
            Serial.print(i);
            Serial.print(": offset ");
            Serial.print(samplesOffsets[i]);
            Serial.print(", length ");
            Serial.println(samplesLenghts[i]);
        }
    }
    Serial.print("VK49 | Found alarms: ");
    Serial.println(alarmsCount);
}

void flashProcessByte(byte data)
{
    buffer[currentFlashCount] = data;
    if (currentFlashCount >= BUFFER_SIZE)
    {
        flash.writeByteArray(flashAddress - currentFlashCount, buffer, currentFlashCount);
        Serial.print(".");
        currentFlashCount = 0;
        flashDisplayActive = !flashDisplayActive;
        if (flashDisplayActive)
        {
            displayFlash((uint16_t) (flashAddress / 10486));
        }
        else
        {
            displayClear();
        }
    }
    else
    {
        currentFlashCount++;
    }
    flashAddress++;
}

void flashStart()
{
    flashAddress = 0;
    currentFlashCount = 0;
    Serial.println("VK49 | Erasing chip.");
    flash.eraseChip();
    Serial.println("VK49 | Chip erased.");
    Serial.println("VK49 | Starting flashing.");
}

void flashEnd()
{
    Serial.println("");
    Serial.println("VK49 | Flashing ended.");
    if (flashAddress > 0)
    {
        flash.writeByteArray(flashAddress - currentFlashCount, buffer, currentFlashCount); // write remainings
        Serial.print("VK49 | ");
        Serial.print(flashAddress);
        Serial.println(" bytes written.");
        displayClear();
        flashSetup(false);
    }
}
