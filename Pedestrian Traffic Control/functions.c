#include "functions.h"
extern int state[2];
int sec;
int currenttime;
int log=0;
void Traffic_gpios_init(){
    /*
     * Traffic1
     *      carGreen,carYellow,carRed --> PB3,PB2,PB1
     *      pedestrianGreen,pedestrianRed --> PB4,PB5
     *      pedestrianButton --> PB0
     *
     * Traffic2
     *      carGreen,carYellow,carRed --> PD3,PD2,PD1
     *      pedestrianGreen,pedestrianRed --> PD6,PD7
     *      pedestrianButton --> PD0
     */

    SYSCTL_RCGCGPIO_R |= PORTB_CLK + PORTD_CLK;             // Enable clock to port F and port E
    while(SYSCTL_PRGPIO_R & (PORTB_CLK + PORTD_CLK) != PORTB_CLK + PORTD_CLK);

    GPIO_PORTB_LOCK_R = 0x4C4F434B;                         // Unlock GPIOCR register
    GPIO_PORTB_CR_R |= PIN_0+PIN_1+PIN_2+PIN_3+PIN_4+PIN_5; // Enable commit for GPIOPUR,GPIODEN
    GPIO_PORTB_PUR_R |= PIN_0;                              // Enable Pull Up resistor for pin 0
    GPIO_PORTB_DIR_R |= PIN_1+PIN_2+PIN_3+PIN_4+PIN_5;      // Set pins [1:5] as output
    GPIO_PORTB_DIR_R &= ~PIN_0;                             // Set pin 0 as input
    GPIO_PORTB_DEN_R |= PIN_0+PIN_1+PIN_2+PIN_3+PIN_4+PIN_5;// Enable pins [0:5] as digital GPIO pins

    GPIO_PORTD_LOCK_R = 0x4C4F434B;                         // Unlock GPIOCR register
    GPIO_PORTD_CR_R |= PIN_0+PIN_1+PIN_2+PIN_3+PIN_6+PIN_7; // Enable commit for GPIOPUR,GPIODEN
    GPIO_PORTD_PUR_R |= PIN_0;                              // Enable Pull Up resistor for pin 0
    GPIO_PORTD_DIR_R |= PIN_1+PIN_2+PIN_3+PIN_6+PIN_7;      // Set pins [1:3,6,7] as output
    GPIO_PORTD_DIR_R &= ~PIN_0;                             // Set pin 0 as input
    GPIO_PORTD_DEN_R |= PIN_0+PIN_1+PIN_2+PIN_3+PIN_6+PIN_7;// Enable pins [0:3,6,7] as digital GPIO pins


    // Initialize the car's traffic lights to be red and the pedestrian's car traffic to be green
    Car_Led_ON(0,RED);
    Ped_Led_ON(0,PEDGREEN);
    Car_Led_ON(1,RED);
    Ped_Led_ON(1,PEDGREEN);
}

void Interrupts_config(){
    /*
     * Configure Button1 to falling edge trigger interrupt
     * Configure Button2 to falling edge trigger interrupt
     */

    // Configure Port B pin 0 to falling edge trigger interrupt
    GPIO_PORTB_IS_R &= ~PIN_0;  // Set pin 0 edge sensitive
    GPIO_PORTB_IBE_R &= ~PIN_0; // Set trigger to be controlled by IEV
    GPIO_PORTB_IEV_R &= ~PIN_0; // Set trigger to be falling edge trigger
    GPIO_PORTB_ICR_R |= PIN_0;  // Clear any prior interrupt
    GPIO_PORTB_IM_R |= PIN_0;   // Unmask interrupt

    // Configure Port D pin 0 to falling edge trigger interrupt
    GPIO_PORTD_IS_R &= ~PIN_0;  // Set pin 0 edge sensitive
    GPIO_PORTD_IBE_R &= ~PIN_0; // Set trigger to be controlled by IEV
    GPIO_PORTD_IEV_R &= ~PIN_0; // Set trigger to be falling edge trigger
    GPIO_PORTD_ICR_R |= PIN_0;  // Clear any prior interrupt
    GPIO_PORTD_IM_R |= PIN_0;   // Unmask interrupt

    // Enable interrupt in NVIC
    NVIC_EN0_R |= PORTB_INTERRUPT_NUMBER+PORTD_INTERRUPT_NUMBER;    // Enable IRQ1 and IRQ3
}


void test_all(){
    /*
     * If Button1 is pushed, Set Traffic1(carGreen,carYellow,carRed,pedestrianGreen,pedestrianRed) HIGH
     * If Button2 is pushed, Set Traffic2(carGreen,carYellow,carRed,pedestrianGreen,pedestrianRed) HIGH
     */
    unsigned int state;
    state = GPIO_PORTB_DATA_R & PIN_0;
    if(state==0){
        GPIO_PORTB_DATA_R |= PIN_1+PIN_2+PIN_3+PIN_4+PIN_5;
        //GPIO_PORTB_DATA_R &= ~(PIN_2+PIN_3+PIN_4+PIN_5);
    }
    else{
        //GPIO_PORTB_DATA_R |= PIN_3;
        GPIO_PORTB_DATA_R &= ~(PIN_1+PIN_2+PIN_3+PIN_4+PIN_5);
    }
    state = GPIO_PORTD_DATA_R & PIN_0;
    if(state==0){
        GPIO_PORTD_DATA_R |= PIN_1+PIN_2+PIN_3+PIN_6+PIN_7;
        //GPIO_PORTD_DATA_R &= ~(PIN_2+PIN_3+PIN_6+PIN_7);
    }
    else{
        //GPIO_PORTD_DATA_R |= PIN_3;
        GPIO_PORTD_DATA_R &= ~(PIN_1+PIN_2+PIN_3+PIN_6+PIN_7);
    }
}
void GPIOBHandler(void){
    /*
     * If the pedestrian's traffic light isn't already green,
     * set the car's traffic light to red and the pedestrian's traffic light to green
     * then wait 2 seconds then resume the previous state
     */

    int current=NVIC_ST_CURRENT_R;  // Save the current Systick value
    int currentsec=sec;             // and the current delay second
                                    // to reinitialize the timer for traffic light
                                    // and resume the remaining time from when it was interrupted

    if(GPIO_PORTB_MIS_R & PIN_0 != 0){ // Checking if the interrupt was caused by pin 0
        GPIO_PORTB_ICR_R |= PIN_0;     // Clear the interrupt flag of pin 0

        // if the pedestrian's traffic light is already green, do nothing
        // and if the car's traffic light is yellow, do nothing
        // (Pedestrians will wait for the traffic light to be red)
        // If you want the pedestrian button to work when the traffic light is yellow,
        // remove this condition "&& state[0]!=YELLOW"
        if((GPIO_PORTB_DATA_R>>5) & 1 && state[0]!=YELLOW){
            Car_Led_ON(0,RED);      // Set car traffic 1 to red
            Ped_Led_ON(0,PEDGREEN); // Set pedestrian traffic 1 to green
            log=1;                  // log flag to send current states to UART
            Systick_Delay(2);       // Wait 2 seconds
            Car_Led_ON(0,state[0]); // Set car traffic 1 to it's previous state
            Ped_Led_ON(0,PEDRED);   // Set pedestrian traffic 1 to red
            log=1;
        }
    }

    Systick_Reinit(current,currentsec); // Reinitialize the timer to resume the previous state

}
void GPIODHandler(void){

    /*
     * See GPIOBHandler
     */

    int current=NVIC_ST_CURRENT_R;
    int currentsec=sec;
    if(GPIO_PORTD_MIS_R & PIN_0 != 0){
        GPIO_PORTD_ICR_R |= PIN_0;
        if((GPIO_PORTD_DATA_R>>7) & 1 && state[1]!=YELLOW){
            Car_Led_ON(1,RED);
            Ped_Led_ON(1,PEDGREEN);
            log=1;
            Systick_Delay(2);
            Car_Led_ON(1,state[1]);
            Ped_Led_ON(1,PEDRED);
            log=1;
        }
    }
    Systick_Reinit(current,currentsec);
}

void Systick_Delay_One_Second(){
    /*
     * Delay for 1 second
     */
    NVIC_ST_CTRL_R = 0;                     // Disable Systick
    NVIC_ST_RELOAD_R = 16000000-1;          // Set the load value to 16000000-1 (1 second)
    NVIC_ST_CTRL_R |= 0x5;                  // Enable Systick and begin counting
    while((NVIC_ST_CTRL_R & (1<<16))==0){   // Wait the timer to finish(Check the count flag)
        if(log==1){
            send_State();                       // Send the current state to UART if log==1
            log=0;
        }
    }
    NVIC_ST_CTRL_R =0;                      // Disable Systick
}
void Systick_Reinit(unsigned long load,int newsec){
    /*
     * Reinitialize the timer with a new load value "current" and the new second to begin counting from
     */
    NVIC_ST_CTRL_R = 0;         // Disable Systick
    NVIC_ST_RELOAD_R = load;    // Load the value
    sec=newsec;                 // Load the second
    NVIC_ST_CTRL_R |= 0x5;      // Enable Systick and begin counting
}
void Systick_Delay(unsigned int seconds){
    /*
     * Delay for x seconds
     * --seconds must be positive integer
     */
    for(sec=0;sec<seconds;sec++){
        Systick_Delay_One_Second();
    }
}

void Car_Led_ON(int port,int color){
    /*
     * Set the car's traffic light to the chosen color
     * Set the chosen color LED pin and clear the other LEDs' pins
     * if port==0 PortB
     * if port==1 PortD
     * Traffic1
     *      carGreen,carYellow,carRed --> PB3,PB2,PB1
     * Traffic2
     *      carGreen,carYellow,carRed --> PD3,PD2,PD1
     */
    if(port==0){
        GPIO_PORTB_DATA_R &=~0x0e;
        GPIO_PORTB_DATA_R |= color;
    }else if(port==1){
        GPIO_PORTD_DATA_R &=~0x0e;
        GPIO_PORTD_DATA_R |= color;
    }
}

void Ped_Led_ON(int port,int color){
    /*
     * Set the pedestrian's traffic light to the chosen color
     * Set the chosen color LED pin and clear the other LEDs' pins
     * if port==0 PortB
     * if port==1 PortD
     * Traffic1
     *      pedestrianGreen,pedestrianRed --> PB4,PB5
     * Traffic2
     *      pedestrianGreen,pedestrianRed --> PD6,PD7
     */
    if(port==0){
        GPIO_PORTB_DATA_R &=~0x30;
        GPIO_PORTB_DATA_R |= color;
    }else if(port==1){
        GPIO_PORTD_DATA_R &=~(0x30<<2);
        GPIO_PORTD_DATA_R |= (color<<2);
    }
}

void uart_Init(){
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R5;
    SYSCTL_RCGCGPIO_R |= PORTE_CLK;
    GPIO_PORTE_DEN_R |= PIN_4+PIN_5;
    GPIO_PORTE_AFSEL_R |= PIN_4+PIN_5;
    GPIO_PORTE_PCTL_R |= GPIO_PCTL_PE4_U5RX+GPIO_PCTL_PE5_U5TX;
    UART5_CTL_R &= ~UART_CTL_UARTEN;
    int baud_rate=9600;
    int uart_system_clock=16000000;
    int clk_div=16;
    float brd = 1.0*uart_system_clock/(clk_div*baud_rate);
    int brdi=brd;
    float brdf=brd-brdi;
    UART5_IBRD_R = brdi;
    UART5_FBRD_R = (int)(brdf*64+0.5);
    UART5_LCRH_R = 0x60;//8-bit, no parity, 1-stop bit, no FIFO
    UART5_CC_R = 0;
    UART5_CTL_R |= UART_CTL_RXE+UART_CTL_TXE+UART_CTL_UARTEN;
}

void UART5_Transmit(char data){
    while((UART5_FR_R & 0x20) != 0); // wait until Tx buffer not full
    UART5_DR_R = data;               // before giving it another byte
}

void send_String(char string[]){
    int i=0;
    while(string[i]!='\0'){
        UART5_Transmit(string[i]);
        i++;
    }
}

void send_State(){
    //static char states[2][2]={{"CARS NORTH SOUTH\t","PEDESTRIAN NORTH SOUTH\t"}
    //                         ,{"CARS EAST WEST\n","PEDESTRIAN EAST WEST\n"}};
    if((GPIO_PORTB_DATA_R>>4) & 1)
        send_String("PEDESTRIAN EAST WEST\t");
    else
        send_String("CARS NORTH SOUTH\t");
    if((GPIO_PORTD_DATA_R>>6) & 1)
        send_String("PEDESTRIAN NORTH SOUTH\n");
    else
        send_String("CARS EAST WEST\n");

    //send_String(states[0][(GPIO_PORTB_DATA_R>>4) & 1]);
    //send_String(states[1][(GPIO_PORTD_DATA_R>>6) & 1]);
}

/*
void Timer_init(){
    SYSCTL_RCGCTIMER_R |= TIMER_0;
    TIMER0_CTL_R |= TIMER_CTL_TAEN;
    TIMER0_CFG_R |= 0x00000000;
    TIMER0_TAMR_R |= TIMER_TAMR_TAMR_PERIOD;
    TIMER0_TAILR_R = 16000000*5; //5seconds

}
*/
