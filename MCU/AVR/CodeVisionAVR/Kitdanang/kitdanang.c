#include <mega8.h>
#include <delay.h>
#include <config.h>
#include <string.h>
#include <crc.h>
#include "kernel.h"
#define NOIC    1
#define ICPIC   2
#define ICAVR   4
#define IC8051  8

#define MIN1DAY    120
#define EXPIREDAY  22

volatile unsigned char ic_current = NOIC; 
unsigned char SENSOR_DETECT_8051  = 1;
eeprom unsigned char eep_expire[8];
unsigned char ram_expire[8];
void *vptr;

unsigned int copy_ram_to_eeprom(eeprom char * epr, char * rm, unsigned char length)
{
    unsigned char i;
    for(i = 0; i < length; i++)
    {
        *epr++ = *rm++;
    }
}

unsigned int copy_eeprom_to_ram(char * rm, eeprom  char * epr, unsigned char length)
{
    unsigned char i;
    for(i = 0; i < length; i++)
    {
         *rm = *epr;
         rm++;  
         epr++;
    }
}

void vBlinkLed(unsigned int x,unsigned long y){
    y=y/2; 
    LED_PIC=0;  
    LED_AVR=1;     
    LED_8051=0;    
    while(x--){
         LED_PIC=~LED_PIC;
         LED_AVR=~LED_AVR;
         LED_8051=~LED_8051; 
         delay_ms(y);  
         LED_PIC=~LED_PIC;
         LED_AVR=~LED_AVR;
         LED_8051=~LED_8051; 
         delay_ms(y);
    } 
    LED_PIC=0;  
    LED_AVR=0;     
    LED_8051=0;
}

void vDisableAllModules(){        
    CTRL_ADC_EN=1;
    CTRL_UART_EN=1;
    CTRL_I2C_EN=1; 
    CTRL_PROG_EN=0;
    VRESET=0;
  //UseDevice=4;
}
void vEnableAllModules(){
    CTRL_ADC_EN=0;   
    CTRL_UART_EN=0;
    CTRL_I2C_EN=0;  
    CTRL_PROG_EN=0;
}
void vEnablePIC(){

//    vDisableAllModules();
    CTRL_ADC_0=0; 
    CTRL_ADC_1=0; 
 
    CTRL_UART_0=0; 
    CTRL_UART_1=0;   

    CTRL_I2C_0=0;  
    CTRL_I2C_1=0;  
    
    CTRL_PROG_0=0;
    CTRL_PROG_1=0;
    
    VRESET=1;
    
    vEnableAllModules();
}
void vEnableAVR(){
    
//    vDisableAllModules();
    CTRL_ADC_0=1; 
    CTRL_ADC_1=0; 
 
    CTRL_UART_0=1; 
    CTRL_UART_1=0;   

    CTRL_I2C_0=1;  
    CTRL_I2C_1=0; 

    CTRL_PROG_0=1;
    CTRL_PROG_1=0;
    
    VRESET=1;
    vEnableAllModules();
}

void vEnable8051(){
    
//    vDisableAllModules();
    CTRL_ADC_0=0; 
    CTRL_ADC_1=1; 
 
    CTRL_UART_0=0; 
    CTRL_UART_1=1;   

    CTRL_I2C_0=0;  
    CTRL_I2C_1=1;
    
    CTRL_PROG_0=0;
    CTRL_PROG_1=1;
    
    VRESET=0;
     
    vEnableAllModules();
}

// External Interrupt 1 service routine
interrupt [EXT_INT1] void ext_int1_isr(void)
{
    //Place your code here

}

// Timer1 compare match interrupt service routine
interrupt [TIM1_COMPA] void timer1_compa_isr(void)
{ 
    SENSOR_DETECT_8051 = 0;; 
}

void vTaskIDLE(){
    while(1){  
       if(!PIND.5)  
//    vBlinkLed(2,1000);
    ;
    }
}

void vTaskReset(){
    while(1)
    {
        if(!BUT_RESET)
        {
            if((ic_current & ICPIC) || (ic_current & ICAVR))
            {   
                #asm ("CLI") 
                VRESET = 0;                                                       
                vBlinkLed(1,800);
                VRESET = 1; 
                #asm ("SEI")   
            }      
            else if(ic_current & IC8051)  
            {
                #asm ("CLI") 
                VRESET = 1;    
                vBlinkLed(1,800);
                #asm ("SEI") 
                VRESET = 0;
            }
            
        }
    }
}

void vTaskIndicatorLed0(){
    while(1)
    { 
        unsigned char count;               
        count=0;
        if(!SENSOR_DETECT_PIC){ic_current |= ICPIC;count++;}
        else (ic_current &= (~ICPIC)); 
            
        if(!SENSOR_DETECT_AVR){ic_current |= ICAVR;count++;} 
        else (ic_current &= (~ICAVR)); 
        
        if(!SENSOR_DETECT_8051){ic_current |= IC8051, SENSOR_DETECT_8051 = 1;count++;} 
        else (ic_current &= (~IC8051));  
        
        if((count>1)||(!count))
        ic_current = NOIC;
        else ic_current &= (~NOIC); 

       if(ic_current & ICPIC)
       {           
           vEnablePIC();
           LED_PIC = ~LED_PIC; 
       } else 
       LED_PIC= 0;

       if(ic_current & ICAVR)
       {                    
           vEnableAVR(); 
           LED_AVR = ~LED_AVR;             
       } else 
       LED_AVR= 0;     

      if(ic_current & IC8051)
       {          
          vEnable8051();
          LED_8051 = ~ LED_8051;  
       } else 
       LED_8051= 0;
              
       if(ic_current & NOIC){vDisableAllModules();VRESET=0;} else
       _task_delay(50);      
    }
}
void vTaskCheckExpire(){
    while(1){
         unsigned char i = 22;
         vptr = ram_expire;
         if(!((unsigned int)*((unsigned int*)ram_expire))){ // is expired
            _set_taskn_status(2, TASK_SUSPEND);
            vDisableAllModules();
            i == 22;
            while(i){
                if(BUT_ATMEGA8 || BUT_RESET)i = 22;else i--;
                _task_delay(100); 
            }
            *(unsigned int *)vptr = MIN1DAY*EXPIREDAY;
            *(unsigned int *)((unsigned char *)vptr + sizeof(ram_expire)/2 -2 ) = generate_16bit_crc(ram_expire, sizeof(ram_expire)/2 - 2, 0XA001); 
            memcpy(ram_expire + sizeof(ram_expire)/2, ram_expire, sizeof(ram_expire)/2);
            _set_taskn_status(2, TASK_READY);
         };

         (*(unsigned int*)vptr)--;
         *(unsigned int *)((unsigned char *)vptr + sizeof(ram_expire)/2 -2 ) = generate_16bit_crc(ram_expire, sizeof(ram_expire)/2 - 2, 0XA001); 
         
         copy_ram_to_eeprom(eep_expire, ram_expire, sizeof(ram_expire)/2); 
         _task_delay(500);
         copy_ram_to_eeprom(eep_expire + sizeof(ram_expire)/2, ram_expire, sizeof(ram_expire)/2); 
         _task_delay(500);
    }
}

void init_board(){ //initialization
    PORTB=0x00;
    DDRB=0xFF;
    PORTC=0x30;
    DDRC=0x0F;
    PORTD=0x28;
    DDRD=0x57; 
    
    DDRB.3=0;  //INPUT ERR
    PORTB.3=1;
              
    
    DDRD.3=0;
    PORTD.3=1;     
    
    DDRD.5=0;//input
    PORTD.5=1;//pullup resitor
    
    PORTD.7=1;   
    PORTC.4=1;
    PORTC.5=1;
          

// Timer/Counter 1 initialization
// Clock source: T1 pin Falling Edge
// Mode: CTC top=OCR1A
// OC1A output: Discon.
// OC1B output: Discon.
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer1 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: On
// Compare B Match Interrupt: Off
    TCCR1A=0x00;
    TCCR1B=0x0E;
    TCNT1H=0x00;
    TCNT1L=0x00;
    ICR1H=0x00;
    ICR1L=0x00;
    OCR1AH=0x00;
    OCR1AL=0xFF;
    OCR1BH=0x00;
    OCR1BL=0x00;  
    
// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: 7.813 kHz
    TCCR0=0x05;
    TCNT0=-0x4E;  
    
    TIMSK|=0x11;

    #asm("CLI");       
    CTRL_VMCU=0;   
//    vBlinkLed(5,500);    
    vDisableAllModules(); 
    copy_eeprom_to_ram(ram_expire, eep_expire, sizeof(ram_expire)); 
    
    if(!generate_16bit_crc(ram_expire, sizeof(ram_expire)/2, 0XA001))
    memcpy(ram_expire + sizeof(ram_expire)/2, ram_expire, sizeof(ram_expire)/2);
    else if(!generate_16bit_crc(ram_expire + sizeof(ram_expire)/2, sizeof(ram_expire)/2, 0XA001))
    memcpy(ram_expire, ram_expire + sizeof(ram_expire)/2, sizeof(ram_expire)/2);
    else 
    {
         vptr = ram_expire;
         *(unsigned int *)vptr = MIN1DAY*EXPIREDAY;
         *(unsigned int *)((unsigned char *)vptr + sizeof(ram_expire)/2 -2 ) = generate_16bit_crc(ram_expire, sizeof(ram_expire)/2 - 2, 0XA001); 
         memcpy(ram_expire + sizeof(ram_expire)/2, ram_expire, sizeof(ram_expire)/2);
    }
    
    copy_ram_to_eeprom(eep_expire, ram_expire, sizeof(ram_expire)); 
}


// Declare your global variables here

void main(void)
{
    
    init_board();   
    _init_rtos();
    _create_task(0, vTaskIDLE, 16, 20, TASK_READY);
    _create_task(1, vTaskReset, 16, 30, TASK_READY);
    _create_task(2, vTaskIndicatorLed0, 16, 30, TASK_READY);
    _create_task(3, vTaskCheckExpire, 16, 30, TASK_READY);
//  create_task(5, vTaskIndicatorLed2, 16, 20, TASK_READY);
    _run(); 
  
}