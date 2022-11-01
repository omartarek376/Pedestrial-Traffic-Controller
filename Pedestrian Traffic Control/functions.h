/*
 * functions.h
 *
 *  Created on: Nov 3, 2021
 *      Author: ronid
 */

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_
#include "tm4c123gh6pm.h"

#define PORTF_CLK 0x20
#define PORTE_CLK 0x10
#define PORTD_CLK 0x08
#define PORTC_CLK 0x04
#define PORTB_CLK 0x02
#define PORTA_CLK 0x01

#define PIN_0 0x01
#define PIN_1 0x02
#define PIN_2 0x04
#define PIN_3 0x08
#define PIN_4 0x10
#define PIN_5 0x20
#define PIN_6 0x40
#define PIN_7 0x80

#define RED (1<<1)
#define BLUE (1<<2)
#define YELLOW (1<<2)
#define GREEN (1<<3)
#define PEDGREEN (1<<4)
#define PEDRED (1<<5)


#define PORTA_INTERRUPT_NUMBER (1<<0)
#define PORTB_INTERRUPT_NUMBER (1<<1)
#define PORTC_INTERRUPT_NUMBER (1<<2)
#define PORTD_INTERRUPT_NUMBER (1<<3)
#define PORTE_INTERRUPT_NUMBER (1<<4)
#define PORTF_INTERRUPT_NUMBER (1<<30)

#define TIMER_0 (1<<0)
#define TIMER_1 (1<<1)
#define TIMER_2 (1<<2)
#define TIMER_3 (1<<3)
#define TIMER_4 (1<<4)
#define TIMER_5 (1<<5)

void Car_Led_ON(int port,int color);
void Ped_Led_ON(int port,int color);
void Traffic_gpios_init();
void Interrupts_config();
void test_all();
void car_traffic_init(char port);
void ped_traffic_init(char port,int firstpin);
void Systick_Delay_One_Second();
void Systick_Delay(unsigned int seconds);
void Systick_Reinit(unsigned long current,int currentsec);
void uart_Init();
void UART5_Transmit(char data);
void send_String(char string[]);
void send_State();

#endif /* FUNCTIONS_H_ */
