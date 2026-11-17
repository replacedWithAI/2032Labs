#include "MKL43Z4.h"

void disableInterrupts() {
    __disable_irq();
    NVIC_DisableIRQ(PORTC_PORTD_IRQn);
    NVIC_DisableIRQ(PORTA_IRQn);
}

void enableGPIO() {
    PORTD->PCR[3] = 0x100;
    PORTA->PCR[4] = 0x100;
    PORTD->PCR[5] = 0x100;
    PORTE->PCR[31] = 0x100;
}

void setGPIODirection() {
    PTD->PDDR &= ~(1 << 3);
    PTA->PDDR &= ~(1 << 4);

    PTD->PDDR |= (1 << 5);
    PTE->PDDR |= (1 << 31);
}

void setInitialLEDValues() {
    GPIOD->PSOR = (1 << 5);
    GPIOE->PSOR = (1 << 31);
}

void setupFallingEdgeInterrupts() {
    PORTD->PCR[3] &= ~0xF0000;
    PORTD->PCR[3] |= 0xA0000;

    PORTA->PCR[4] &= ~0xF0000;
    PORTA->PCR[4] |= 0xA0000;
}

void setInterruptPriorities() {
    NVIC_SetPriority(PORTC_PORTD_IRQn, 2);
    NVIC_ClearPendingIRQ(PORTC_PORTD_IRQn);

    NVIC_SetPriority(PORTA_IRQn, 3);
    NVIC_ClearPendingIRQ(PORTA_IRQn);
}

void enableInterrupts() {
    NVIC_EnableIRQ(PORTC_PORTD_IRQn);
    NVIC_EnableIRQ(PORTA_IRQn);
    __enable_irq();
}

void PORTC_PORTD_IRQHandler(void) {
    switch(PORTD->ISFR & (1 << 3)) {
        case((1 << 3)):
            PORTD->ISFR = (1 << 3);
            GPIOD->PTOR = (1 << 5);
            break;
        default:
            // PORTD->ISFR |= 0xFFFFFFFF;
            break;
    }
}

void PORTA_IRQHandler(void) {
    switch(PORTA->ISFR & (1 << 4)) {
        case((1 << 4)):
            PORTA->ISFR = (1 << 4);
            GPIOE->PTOR = (1 << 31);
            break;
        default:
            // PORTA->ISFR |= 0xFFFFFFFF;
            break;
    }
}


int main() {
    SIM->SCGC5 |= (1 << 9) | (1 << 12) | (1 << 13);
    disableInterrupts();
    enableGPIO();
    setGPIODirection();
    setInitialLEDValues();
    setupFallingEdgeInterrupts();
    setInterruptPriorities();
    enableInterrupts();

    while (1) {
        __WFI(); // sleeping until interrupts
    }
}