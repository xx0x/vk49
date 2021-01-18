// vk49 talking watch
// based on ATSAMD21G18 (Arduino Zero, Adafruit Feather M0)
// use with "Arduino Zero (Native USB Port core)"
// author: Vaclav Mach (vaclavmach@hotmail.cz, info@xx0x.cz)
// project homepage: www.xx0x.cz

#include "RTClib.h" // modified
#include <Wire.h>
#include <TimeLib.h>
#include "Adafruit_ZeroI2S.h"
#include <SPIMemory.h> // modified
#include <AS1115.h>

#include "./pins.h"

// Debug
#define Serial SERIAL_PORT_USBVIRTUAL

// Settings
#define DISPLAY_INTENSITY 15 // 0-15
#define SAMPLERATE_HZ 22050
#define MAX_SAMPLES 160

// Battery measure
#define BAT_TOPLIMIT 1678
#define BAT_LOWLIMIT 1620
#define BAT_MEASURES 10
#define BAT_DELAY 100

// Create audio, RTC, display and flash objects
// LedControlSAMD21 lc = LedControlSAMD21(PIN_LED_DATA, PIN_LED_CLOCK, PIN_LED_LATCH, 1);
RTC_DS3231 rtc;
Adafruit_ZeroI2S i2s(PIN_I2S_FS, PIN_I2S_SCK, PIN_I2S_TX, PIN_I2S_RX);
SPIFlash flash(PIN_FLASH_CS);
AS1115 as = AS1115(0x00);

// Flashing stuff
#define BUFFER_SIZE 8600
byte buffer[BUFFER_SIZE];
#define SERIAL_MODE_NONE 0
#define SERIAL_MODE_FLASH '@'
byte serialMode = SERIAL_MODE_NONE;
#define MAX_HEADER_SIZE 1279 // 4 + 255*5

// Samples
uint32_t samplesLenghts[MAX_SAMPLES];
uint32_t samplesOffsets[MAX_SAMPLES];

// Time setting
int8_t currentDigit = -1;
byte digits[4] = {0, 0, 0, 0};
#define DIGITS_ACTIVE (currentDigit != -1)

// Flow
bool goToSleep = true;
bool menuActive = false;
bool stopPlaying = false;
bool isPlaying = false;

// Buttons and timings
#define DEBOUNCE_TIME 250
bool buttonPressed = false;
bool menuButtonPressed = false;
unsigned long lastTimeButton = 0;
unsigned long lastTimeMenuButton = 0;
#define MENU_TIMEOUT 30000

// Menu
int8_t currentMenuItem = -1;
#define IS_MENU_ACTIVE (currentMenuItem != -1)
bool alarmHasPlayed = false;
#define MENU_ITEMS 6
#define MENU_VOLUME 0   // a
#define MENU_SOUND 1    // b
#define MENU_ALARM 2    // c
#define MENU_ALARMSET 3 // d
#define MENU_TIMESET 4  // e
#define MENU_BATTERY 5  // f

// Volume
#define VOLUMES_COUNT 8
byte currentVolume = 3;
uint32_t volumes[VOLUMES_COUNT] = {0, 2000, 4000, 8000, 12000, 16000, 26000, 34000};

// Alarm
#define ALARMS_COUNT alarmsCount
#define ALARM_MAX_LOOPS 20
byte currentAlarm = 0;
byte alarmsCount = 0;
byte alarmTriggered = false;

#include "./samples.h"
#include "./clock.h"
#include "./display.h"
#include "./flash.h"
#include "./say.h"
#include "./sleep.h"
#include "./digits.h"

void setup()
{
    Wire.begin();
    analogReadResolution(12);
    pinMode(PIN_ENABLE, OUTPUT);
    digitalWrite(PIN_ENABLE, HIGH);
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    pinMode(PIN_MENU_BUTTON, INPUT_PULLUP);
    pinMode(PIN_BATSENSE, INPUT);
    delay(1000);
    displaySetup();

    Serial.begin(115200);
    Serial.println("VK49 startup\n");

    flash.begin();

    while (!flashSetup())
    {
        displayEmpty();
        delay(1000);
        displayClear();
        serialLoop();
        delay(1000);
    }

    clockSetup();
    showTime(false);
    delay(500);
    displayClear();

    saySetup();
    sleepPrepare(buttonPressedCallback, menuButtonPressedCallback, alarmCallback);
    Serial.println("VK49 ready\n");
}

void buttonPressedCallback()
{
    if (millis() - lastTimeButton < DEBOUNCE_TIME)
    {
        return;
    }
    lastTimeButton = millis();
    buttonPressed = true;
    if (isPlaying && IS_MENU_ACTIVE)
    {
        stopPlaying = true;
    }
    if (alarmTriggered)
    {
        stopPlaying = true;
        buttonPressed = false;
    }
}

void menuButtonPressedCallback()
{
    if (millis() - lastTimeMenuButton < DEBOUNCE_TIME)
    {
        return;
    }
    lastTimeMenuButton = millis();
    menuButtonPressed = true;
    if (isPlaying)
    {
        stopPlaying = true;
    }
    if (alarmTriggered)
    {
        stopPlaying = true;
        menuButtonPressed = false;
    }
}

void alarmCallback()
{
    alarmTriggered = true;
    rtc.clearAlarm(1);
}

void smartDelay(unsigned int d)
{
    delay(10);
    for (unsigned int j = 0; j < d / 10; j++)
    {
        delay(10);
        if (buttonPressed || menuButtonPressed)
        {
            break;
        }
    }
}

void turnOff()
{
    displayOff();
    flash.powerDown();
    delay(10);
    digitalWrite(PIN_ENABLE, LOW);
    lastTimeButton = 0;
    lastTimeMenuButton = 0;
    sleepStart();
}

void turnOn()
{
    digitalWrite(PIN_ENABLE, HIGH);
    delay(10);
    displayOn();
    flash.powerUp();
}

void loop()
{
    serialLoop();
    alarmLoop();
    if (!alarmTriggered)
    {
        timeLoop();
        menuLoop();
        if (goToSleep)
        {
            goToSleep = false;
            turnOff();
            turnOn();
        }
    }
}

void timeLoop()
{
    if (buttonPressed && !IS_MENU_ACTIVE)
    {
        showTime(true);
        buttonPressed = false;
        goToSleep = true;
    }
}

void alarmLoop()
{
    if (alarmTriggered)
    {
        DateTime alarm = rtc.getAlarmDateTime(1);
        for (byte i = 0; i < ALARM_MAX_LOOPS; i++)
        {
            delay(20);
            displayTime(alarm.hour(), alarm.minute(), 00);
            saySample(SAMPLE_ALARM_BASE + currentAlarm);
            if (stopPlaying)
            {
                break;
            }
            displayClear();
            delay(500);
        }
        alarmTriggered = false;
        menuButtonPressed = false;
        buttonPressed = false;
        goToSleep = true;
        menuExit(); //  if alarm triggered in menu, close it
        delay(1000);
    }
}

byte menuFrame = 0;

void menuExit()
{
    buttonPressed = false;
    menuButtonPressed = false;
    goToSleep = true;
    alarmHasPlayed = false;
    currentMenuItem = -1;
    currentDigit = -1;
}

byte previousBatteryState = 0;

void menuLoop()
{
    if (millis() - lastTimeButton > MENU_TIMEOUT && millis() - lastTimeMenuButton > MENU_TIMEOUT)
    {
        displayClear();
        delay(50);
        displayShowOff();
        delay(500);
        displayClear();
        menuExit();
        return;
    }
    if (menuButtonPressed)
    {
        menuButtonPressed = false;
        if (!DIGITS_ACTIVE)
        {
            menuFrame = 0;
            currentMenuItem++;
        }
        else
        {
            digitsNext();
            if (currentDigit >= 4)
            {
                if (currentMenuItem == MENU_ALARMSET)
                {
                    digitsSaveAsAlarm();
                    DateTime alarm = rtc.getAlarmDateTime(1);
                    displayTime(alarm.hour(), alarm.minute(), 00);
                    if (saySample(SAMPLE_INTRO))
                    {
                        if (saySample(SAMPLE_ALARM_SET))
                        {
                            delay(300);
                            if (saySample(SAMPLE_ALARM_CURRENT))
                            {
                                sayTime(alarm.hour(), alarm.minute(), 00, false, true);
                            }
                        }
                    }
                }
                else if (currentMenuItem == MENU_TIMESET)
                {
                    digitsSaveAsClock();
                    DateTime now = rtc.now();
                    displayTime(now.hour(), now.minute(), now.second());
                    if (saySample(SAMPLE_INTRO))
                    {
                        if (saySample(SAMPLE_TIME_SET))
                        {
                            delay(300);
                            if (saySample(SAMPLE_TIME_CURRENT))
                            {
                                sayTime(now.hour(), now.minute(), now.second(), false, true);
                            }
                        }
                    }
                }
                displayClear();
                menuExit();
                return;
            }
        }
        goToSleep = false;
        displayClear();
        if (currentMenuItem >= MENU_ITEMS)
        {
            menuExit();
        }
    }
    if (!IS_MENU_ACTIVE)
    {
        return;
    }

    bool alarmEnabled;

    switch (currentMenuItem)
    {
    case MENU_VOLUME: // A
        if (buttonPressed)
        {
            buttonPressed = false;
            currentVolume++;
            if (currentVolume >= VOLUMES_COUNT)
            {
                currentVolume = 0;
            }
        }
        displayMenuItemNumber(currentMenuItem, currentVolume);
        break;
    case MENU_SOUND: // B
        if (buttonPressed)
        {
            buttonPressed = false;
            if (alarmHasPlayed)
            {
                currentAlarm++;
            }
            alarmHasPlayed = true;
            if (currentAlarm >= ALARMS_COUNT)
            {
                currentAlarm = 0;
            }
            delay(30);
            displayMenuItemNumber(currentMenuItem, currentAlarm + 1);
            delay(30);
            saySample(currentAlarm + SAMPLE_ALARM_BASE);
        }
        else
        {
            displayMenuItemNumber(currentMenuItem, currentAlarm + 1);
        }
        break;
    case MENU_ALARM: // C
        alarmEnabled = clockIsAlarmEnabled();
        if (buttonPressed)
        {
            buttonPressed = false;
            alarmEnabled = !alarmEnabled;
            if (alarmEnabled)
            {
                clockEnableAlarm();
            }
            else
            {
                clockDisableAlarm();
            }
        }
        displayMenuItemBoolean(currentMenuItem, alarmEnabled);
        break;

    case MENU_ALARMSET: // D
    case MENU_TIMESET:  // E
        // SET ALARM / TIME
        if (buttonPressed && currentDigit == -1)
        {
            buttonPressed = false;
            currentDigit = 0;
            digitsPrepare(currentMenuItem == MENU_ALARMSET);
        }
        else
        {
            if (currentDigit == -1)
            {
                DateTime time;
                if (currentMenuItem == MENU_ALARMSET)
                {
                    time = rtc.getAlarmDateTime(1);
                }
                else
                {
                    time = rtc.now();
                }
                if (menuFrame == 1)
                {
                    displayMenuItemNumber(currentMenuItem, time.hour());
                }
                else if (menuFrame == 2)
                {
                    displayMenuItemNumber(currentMenuItem, time.minute());
                }
                else
                {
                    displayMenuItem(currentMenuItem);
                }
                smartDelay(500);
            }
            else if (currentDigit >= 0 && currentDigit <= 3)
            {
                if (buttonPressed)
                {
                    buttonPressed = false;
                    digitsIncrease();
                }
                displayClear();
                for (byte i = 0; i < 4; i++)
                {
                    if (i != currentDigit || menuFrame < 2)
                    {
                        displayDigit(i, digits[i]);
                    }
                }
            }
        }

        break;
    case MENU_BATTERY: // F
        delay(50);
        if (previousBatteryState > 0)
        {
            displayMenuItemNumber(currentMenuItem, previousBatteryState);
        }
        else
        {
            displayMenuItem(currentMenuItem);
        }
        delay(50);
        previousBatteryState = readBattery();
        smartDelay(200);
        break;
    default:
        break;
    }
    menuFrame++;
    if (menuFrame >= 4)
    {
        menuFrame = 0;
    }
    smartDelay(100);
}

void showTime(bool say)
{
    DateTime now = rtc.now();
    int hh = now.hour();
    int mm = now.minute();
    int ss = now.second();
    Serial.print("Now is: ");
    Serial.print(hh);
    Serial.print(":");
    Serial.print(mm);
    Serial.print(":");
    Serial.print(ss);
    Serial.println("");
    displayTime(hh, mm, ss);
    if (say)
    {
        if (volumes[currentVolume] > 0)
        {
            sayTime(hh, mm, ss);
        }
        else
        {
            delay(2000);
        }
        Serial.println("Saying finished");
    }
}

/* FLASH STUFF */
void serialLoop()
{
    while (Serial.available() > 0)
    {
        byte inByte = Serial.read();
        switch (serialMode)
        {
        case SERIAL_MODE_FLASH:
            flashProcessByte(inByte);
            break;
        default:
            serialMode = inByte;
            if (serialMode == SERIAL_MODE_FLASH)
            {
                flashStart();
            }
        }
    }
    if (serialMode == SERIAL_MODE_FLASH)
    {
        flashEnd();
    }
    serialMode = SERIAL_MODE_NONE;
}

byte readBattery()
{
    long measuredvbat = 0;
    byte num = 0;
    for (byte i = 0; i < BAT_MEASURES; i++)
    {
        measuredvbat += analogRead(PIN_BATSENSE);
        num++;
        delay(BAT_DELAY);
        if (buttonPressed || menuButtonPressed)
        {
            break;
        }
    }
    measuredvbat = constrain(measuredvbat / num, BAT_LOWLIMIT, BAT_TOPLIMIT);
    byte batcap = map(measuredvbat, BAT_LOWLIMIT, BAT_TOPLIMIT, 0, 99);
    return batcap;
}