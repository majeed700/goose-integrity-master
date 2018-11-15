

#ifndef _UART0_UART1_H_
#define _UART0_UART1_H_

#define CR     0x0D

#define STA 0xaa
#define STO 0xcc

void init_serial (void);
int getchar (void);
int putchar (int ch);
void putstr(char *str);

void init_serial0 (void);
int getchar0 (void);
int putchar0 (int ch);
void putstr0(char *str);

void itoa(int ch);
												  
void init_uart0_intr(void);
void init_uart1_intr(void);
void UART0int_ISR(void)__irq;
void UART1int_ISR(void)__irq;

#endif


