
void sleepPrepare(voidFuncPtr callback, voidFuncPtr callback2, voidFuncPtr callback3)
{
    byte interruptPin = digitalPinToInterrupt(PIN_BUTTON);
    byte interruptPin2 = digitalPinToInterrupt(PIN_MENU_BUTTON);
    byte interruptPin3 = digitalPinToInterrupt(PIN_ALARM);
    attachInterrupt(interruptPin, callback, FALLING);
    attachInterrupt(interruptPin2, callback2, FALLING);
    attachInterrupt(interruptPin3, callback3, FALLING);

    // enable EIC clock
    GCLK->CLKCTRL.bit.CLKEN = 0; //disable GCLK module
    while (GCLK->STATUS.bit.SYNCBUSY)
        ;

    GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK6 | GCLK_CLKCTRL_ID(GCM_EIC)); //EIC clock switched on GCLK6
    while (GCLK->STATUS.bit.SYNCBUSY)
        ;

    GCLK->GENCTRL.reg = (GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_OSCULP32K | GCLK_GENCTRL_ID(6)); //source for GCLK6 is OSCULP32K
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY)
        ;

    GCLK->GENCTRL.bit.RUNSTDBY = 1; //GCLK6 run standby
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY)
        ;

    // Enable wakeup capability on pin in case being used during sleep
    EIC->WAKEUP.reg |= (1 << interruptPin);
    EIC->WAKEUP.reg |= (1 << interruptPin2);
    EIC->WAKEUP.reg |= (1 << interruptPin3);
//    PM->SLEEP.reg |= PM_SLEEP_IDLE_APB;
}


void sleepStart()
{
    USBDevice.detach();
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    __DSB();
    __WFI();
    USBDevice.attach();
}
