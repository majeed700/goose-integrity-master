#ifndef _16X2_LCD_H_
#define _16X2_LCD_H_


void lcd_1ms(unsigned int lcd_tt);
void lcd_data(unsigned char data);
void lcd_command(unsigned char data);
void lcd_display_string(char *str);
void lcd_clear(void);
void lcd_int(void);
void itoa_lcd(unsigned int ch);

void t1_1us(unsigned int us);
void ftoa(float n, char *res, int afterpoint);
int intToStr(int x, char str[], int d);

#endif
