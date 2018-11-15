

#include <LPC214x.h>
#include "UART0_UART1.h"

unsigned char uart0_receive_flag=0,uart1_receive_flag=0,ping_flag=0,new_packet_flag = 0;
unsigned char uart0_count=0,uart1_count=0;
char uart0_buf[17],uart1_buf[17];


void init_serial(void)  {              
  PINSEL0|=0x00050000;
  U1LCR = 0x83;
  
  U1DLM = 0x00;  
 	U1DLL = 0x61;				 //9600

  U1LCR = 0x03;                         
}

int getchar (void)
{                    
  while (!(U1LSR & 0x01));
	return (U1RBR);
}

int putchar (int ch)  {                 

  if (ch == '\n')  {
    while (!(U1LSR & 0x20));
    U1THR = CR;                          
  }
  while (!(U1LSR & 0x20));
  return (U1THR = ch);
} 

void putstr(char *str)
{
	while(*str)
		putchar(*str++);
}


//==============================================================

void init_serial0(void)  
{               
	PINSEL0|=0x00000005;
	U0LCR = 0x83;                         

	U0DLM = 0x00;
  U0DLL = 0x61;				 /* 9600 */

	U0LCR = 0x03;                         
}

int getchar0 (void)
{                    

  while (!(U0LSR & 0x01));
	return (U0RBR);
}

int putchar0 (int ch)  {                 

  if (ch == '\n')  {
    while (!(U0LSR & 0x20));
    U0THR = CR;                         
  }
  while (!(U0LSR & 0x20));
  return (U0THR = ch);
} 

void putstr0(char *str)
{
	while(*str)
		putchar0(*str++);
}


//------------------------------------------------------------------------------------------------


void init_uart0_intr(void)  
{              
		
	VICIntSelect=0x00;
  VICVectCntl1=0x26;											
  VICVectAddr1=(unsigned long)UART0int_ISR;  
	VICIntEnable=0x40;  
  U0IER=0X01;

}

void init_uart1_intr(void)  
{              
	U1IER=0X01;	
	VICIntSelect=0x00;
    VICVectCntl0=0x27;											
    VICVectAddr0=(unsigned long)UART1int_ISR;  
	VICIntEnable=0x80;  
  U1IER=0X01;

}

void UART0int_ISR()__irq
{ 	
	uart0_buf[uart0_count++]=U0RBR; uart0_buf[uart0_count]=0;
	
//	if(uart0_count>=16)  
	uart0_count=0;
	
	if(uart0_buf[0] == 'P') ping_flag=1;
          
	uart0_receive_flag=1;
			
	VICVectAddr=0x00;
}

void UART1int_ISR()__irq
{	
  uart1_buf[uart1_count++]=U1RBR;

	if(uart1_buf[uart1_count-1] == STA)	{uart1_count=0; new_packet_flag = 0; }
		
	if(uart1_buf[uart1_count-1] == STO) 
	{
		uart1_buf[uart1_count-1]= 0;
		
//		uart1_buf[uart1_count-2] = 'Q';
		
		uart1_count=0;
		
		new_packet_flag = 1;
		
	uart1_receive_flag=1;
	}
	
  if(uart1_count==120)uart1_count =0;
          
	
	
	VICVectAddr=0x00;
}

void itoa(int ch)
{
//	putchar0((ch/10)+48); 
//	putchar0((ch%10)+48); 

	putchar0((ch/100)+48);
	ch = ch%100;	
	putchar0((ch/10)+48); 
	putchar0((ch%10)+48);
}
