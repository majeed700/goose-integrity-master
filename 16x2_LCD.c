#include <LPC214x.h>
#include "16x2_LCD.h"
#include<math.h>


void lcd_1ms(unsigned int ms)
{
	T0PR=15000-1;
	T0TC=0;
	T0TCR=1;
	while(T0TC!=ms);
	T0TCR=2;


}

void t1_1us(unsigned int us)
{
	T1PR=15-1;
	T1TC=0;
	T1TCR=1;
	while(T1TC!=us);
	T1TCR=2;


}

void lcd_data(unsigned char data)
{
 IO0PIN&=	(~(1<<17));							    //R/W P0.17
 IO1PIN = 	(unsigned int)data<<16;				    //data P1.16-p1.23
 IO0PIN|=(1<<16);  										//Rs P0.16
 IO0PIN|=	(1<<18);lcd_1ms(1);				   //En=1;
 IO0PIN&=	(~(1<<18));	//lcd_1ms(1);	
}

void lcd_command(unsigned char data)
{
 IO0PIN&=	(~(1<<17));							    
 IO1PIN = (unsigned int)data<<16;				    
 IO0PIN&= (~(1<<16));  
 IO0PIN|=	(1<<18);lcd_1ms(1);				  
 IO0PIN&=	(~(1<<18));	//lcd_1ms(1);	
}

void lcd_display_string(char *str)
{
	while(*str)
		lcd_data(*str++);
}	  

void lcd_clear(void)
{
	lcd_command(0x01);//lcd_1ms(100);	
}


void lcd_int(void)
{
	 lcd_1ms(100);																	  
	 lcd_command(0x38);lcd_1ms(10);
	 lcd_command(0x06);lcd_1ms(10);
	 lcd_command(0x0C);lcd_1ms(10);
	 lcd_command(0x01);//lcd_1ms(10);

}
void itoa_lcd(unsigned int ch)
{
	lcd_data((ch/100)+48);
	ch = ch%100;	
	lcd_data((ch/10)+48); 
	lcd_data((ch%10)+48); 
}

// reverses a string 'str' of length 'len'
void reverse(char *str, int len)
{
    int i=0, j=len-1, temp;
    while (i<j)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++; j--;
    }
}
int intToStr(int x, char str[], int d)
{
    int i = 0;
	
	  if(x<1) str[i++] = '0';
	
    while (x)
    {
        str[i++] = (x%10) + '0';
        x = x/10;
    }
 
    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';
 
    reverse(str, i);
    str[i] = '\0';
    return i;
}
// Converts a floating point number to string.
void ftoa(float n, char *res, int afterpoint)
{
    // Extract integer part
    int ipart = (int)n;
 
    // Extract floating part
    float fpart = n - (float)ipart;
 
    // convert integer part to string
    int i = intToStr(ipart, res, 0);
 
    // check for display option after point
    if (afterpoint != 0)
    {
        res[i] = '.';  // add dot
 
        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter is needed
        // to handle cases like 233.007
        fpart = fpart * pow(10, afterpoint);
 
        intToStr((int)fpart, res + i + 1, afterpoint);
    }
}
