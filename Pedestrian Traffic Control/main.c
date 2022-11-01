#include "functions.h"

/**
 * main.c
 */
int state[2];
extern int log;
void main(void)
{
    Traffic_gpios_init();       // Initialize GPIOS
    Interrupts_config();        // Configure Interrupts
    uart_Init();
    UART5_Transmit('H');
    send_String("test hi \n");
    send_String("test hi \n");
    int counter=0;
    int which;
    while(1){
        //test_all();             // If button is pushed set LEDS to HIGH
        which=counter%2;
        state[which]=GREEN;
        Car_Led_ON(which,state[which]);
        Ped_Led_ON(which,PEDRED);
        log=1;
        Systick_Delay(5);
        state[which]=YELLOW;
        Car_Led_ON(which,state[which]);
        Systick_Delay(2);
        state[which]=RED;
        Car_Led_ON(which,state[which]);
        Ped_Led_ON(which,PEDGREEN);
        log=1;
        Systick_Delay(1);
        counter++;
    }
}
