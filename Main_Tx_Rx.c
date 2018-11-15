#include <LPC214X.H>
#include "UART0_UART1.h"
#include "16X2_LCD.h"

#define PKT_DELAY 5000//us
#define STA 0xaa
#define STO 0xcc
#define Total_PKT 100

extern unsigned char uart0_receive_flag,uart1_receive_flag,ping_flag,new_packet_flag;
extern char uart0_buf[17],uart1_buf[17];
char data_buf[15];
unsigned int timeout = 0; 
unsigned int rece = 0;
char tx_packets[103][13];
unsigned int repet_count= 0;
float per=0.0,ber=0.0;
unsigned int total_rx_pkt =0,total_tx_pkt=0,total_loss_pkt=0;
char packet_data[11][13]=
{
"1234567890",
"abcdefghij",
"~!@#%^&()_",
"234.677990",
"8643245467",
"EGYUdgdfht",
"+-/CXV2434",		
"Hellofdffg",
"XFGFD  DFF",
"<?:fHKHG12",
};



void fill_packet()
{
	unsigned char a = 0,b=0;
	// tx array - define loop
	for(a = 0; a<(Total_PKT/10) ; a++)
	{  //packet number - first byte 
		//to add packet number(to convert in ascii we add +48) in first byte of packet
		tx_packets[a][0] = a+1+48;
		for(b = 1; b<11 ; b++)
		{		
			tx_packets[a][b] = packet_data[a][b-1];      				
		}
		tx_packets[a][11] = 0;
	}

}



void tx_bulk_packet()
{
	unsigned char a = 0,loop=0;
	
	fill_packet();
	//10 times loop repeat for the characters
	for(loop = 0; loop<10 ; loop++)
	{
			for(a = 0; a<(Total_PKT/10) ; a++)
			{		
					putchar(STA);
					putstr(&tx_packets[a][0]);
					putchar(STO);
				
					//lcd_1ms(PKT_DELAY);
					t1_1us(PKT_DELAY);
			}
	}

}

int main()
{
	unsigned char loop=0,match = 1;
	char disp[6];
	
	PINSEL0=0x00050f05;
  PINSEL1=0x00000000;
  PINSEL2=0x00000000;
   
  IO0DIR=0xfdbf6dcd;      
  IO1DIR=0xfdffffff;
		
  init_serial0(); //uart0
  init_serial();  //uart1
		
	putstr0("\r\nMY TechnoCare\r\nRF Communication");
	putstr("RF Communication");
	
	lcd_int();
	lcd_command(0x80);lcd_display_string("MY TechnoCare");lcd_1ms(3000);
	lcd_command(0xc0);lcd_display_string("RF Commu. Test");
		
	init_uart0_intr();
	init_uart1_intr();
	
	uart0_receive_flag = uart1_receive_flag = 0;	
	lcd_clear();lcd_command(0x80);	
	rece = 0;
	while(1)
	{			
		if(uart0_receive_flag==1)
		{
			if(ping_flag == 0)
			{
				lcd_clear();
				lcd_command(0x80);lcd_display_string(uart0_buf);
			}
			if(ping_flag == 1)
			{				
				lcd_clear();
				lcd_command(0x80);lcd_display_string("PING Test");	
			 
				tx_bulk_packet();
				ping_flag = 0;
			}	
			uart0_receive_flag=0;		
		}
		
		if(uart1_receive_flag==1)
			
		{		//10 bytes comparing 	
			 match = 1;
			 /*for(loop = 0; loop<11 ; loop++)
			 {
					 if(uart1_buf[loop+1] != packet_data[uart1_buf[0]-1-48][loop]) match = 0;
			 }	 
			 if(match)*/ rece++;	
       	repet_count = rece;
        total_loss_pkt = 100-repet_count;
	      per = (total_loss_pkt)/(total_tx_pkt);
	      ber = (total_loss_pkt)/(total_tx_pkt*13.0*8.0);			
			 intToStr(rece,disp,4);		 
			 lcd_command(0x80);	lcd_display_string(disp);					
			 uart1_receive_flag=0;

			
		}		
	}	
}//main


