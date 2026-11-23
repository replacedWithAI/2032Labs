#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL43Z4.h"
#include "fsl_debug_console.h"

volatile bool StartTimerFlag = false;
volatile uint32_t HundredCounter = 0;


int main() {
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    BOARD_InitDebugConsole();
#endif

    SIM->SCGC5 |= (1 << 9) | (1 << 12) | (1 << 13);

    PORTD->PCR[3] = 0x100; // sensors
    PORTA->PCR[4] = 0x100;

    PORTD->PCR[5] = 0x100; //LEDs
    PORTE->PCR[31] = 0x100;


    PTD->PDDR &= ~(1u << 3u); // directions
    PTA->PDDR &= ~(1u << 4u);

    PTD->PDDR |= (1u << 5u);
    PTE->PDDR |= (1u << 31u);


    GPIOD->PCOR = (1u << 5u);// initial values
    GPIOE->PCOR = (1u << 31u);

    __disable_irq(); // I still don't know why this is necessary, but why not
    NVIC_DisableIRQ(PORTC_PORTD_IRQn);
    PORTD->PCR[3] &= ~0xF0000;
    PORTD->PCR[3] |= 0xA0000;
    NVIC_SetPriority(PORTC_PORTD_IRQn, 192);
    NVIC_ClearPendingIRQ(PORTC_PORTD_IRQn);
    NVIC_EnableIRQ(PORTC_PORTD_IRQn);
    __enable_irq();

    __disable_irq();
    NVIC_DisableIRQ(PORTA_IRQn);
    PORTA->PCR[4] &= ~0xF0000;
    PORTA->PCR[4] |= 0xA0000;
    NVIC_SetPriority(PORTA_IRQn, 192);
    NVIC_ClearPendingIRQ(PORTA_IRQn);
    NVIC_EnableIRQ(PORTA_IRQn);
    __enable_irq();


    __disable_irq();
    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
    SysTick->LOAD = 0x493E00; // timer intervals
    SysTick->CTRL |= 1 << 1 | 1 << 2; // use system clock and interrupt. No enable yet
    __enable_irq();
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;


    while (1) {
        __asm volatile ("nop");
    }
}


void PORTC_PORTD_IRQHandler(void) {
    switch(PORTD->ISFR) {
        case((1 << 3)):
            PORTD->PCR[3] |= (1 << 3);

            if (!StartTimerFlag) {
                PTD->PTOR = 1 << 5;
                PTE->PDOR |= (1 << 31); // turn stop LED off
                
                StartTimerFlag = true;
                HundredCounter = 0;

                SysTick->LOAD = 0x493E00;
                SysTick->CTRL |= 1 << 0 |1 << 1 | 1 << 2;
            }

            SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
            break;
        default:
            PORTD->ISFR |= 0xFFFFFFFF;
            break;
    }
}


void PORTA_IRQHandler(void) {
    switch(PORTA->ISFR) {
        case((1 << 4)):
            PORTA->PCR[4] |= (1 << 4);

            if (StartTimerFlag) {
                StartTimerFlag = false;
                HundredCounter = 0;

                SysTick->LOAD = 0x493E00;
                SysTick->CTRL &= ~(1 << 0); // stop the timer
            }

            break;
        default:
            PORTA->ISFR |= 0xFFFFFFFF;
            break;
    }
}


void SysTick_Handler(void) {
    HundredCounter++;
    PTD->PTOR = 1 << 5;
}