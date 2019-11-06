#include <msp430g2553.h>
#include <stdlib.h>
#define DR P1OUT = P1OUT | BIT5 // define RS high
#define CWR P1OUT = P1OUT & (~BIT5) // define RS low
#define READ P1OUT = P1OUT | BIT6 // define Read signal R/W = 1 for reading
#define WRITE P1OUT = P1OUT & (~BIT6) // define Write signal R/W = 0 for writing
#define ENABLE_HIGH P1OUT = P1OUT | BIT7 // define Enable high signal
#define ENABLE_LOW P1OUT = P1OUT & (~BIT7) // define Enable Low signal
unsigned int i;
unsigned int j;
char positionString[6];
void delay(unsigned int k)
{
for(j=0;j<=k;j++)
{
for(i=0;i<900;i++);
}
}
void data_write(void)
{
ENABLE_HIGH;
delay(2);
ENABLE_LOW;
}
void data_read(void)
{
ENABLE_LOW;
delay(2);
ENABLE_HIGH;
}
void check_busy(void)
{
P2DIR &= ~(BIT3); // make P1.3 as input
while((P2IN&BIT3)==1)
{
data_read();
}
P2DIR |= BIT3; // make P1.3 as output
}
void send_command(unsigned char cmd)
{
check_busy();
WRITE;
CWR;
P2OUT = (P2OUT & 0xF0)|((cmd>>4) & 0x0F); // send higher nibble
data_write(); // give enable trigger
P2OUT = (P2OUT & 0xF0)|(cmd & 0x0F); // send lower nibble
data_write(); // give enable trigger
}
void send_data(unsigned char data)
{
check_busy();
WRITE;
DR;
P2OUT = (P2OUT & 0xF0)|((data>>4) & 0x0F); // send higher nibble
data_write(); // give enable trigger
P2OUT = (P2OUT & 0xF0)|(data & 0x0F); // send lower nibble
data_write(); // give enable trigger
}
void send_string(char *s)
{
while(*s)
{
send_data(*s);
s++;
}
}
//mine
void send_int(int num)
{
 itoa(num, positionString, 10);
 send_string(positionString);
}
void lcd_init(void)
{
P2DIR |= 0x0F;
P1DIR |= (BIT5|BIT6|BIT7);
P2OUT &= 0xF0;
P1OUT &= ~(BIT5|BIT6|BIT7);
send_command(0x33);
send_command(0x32); //LCD_clear
send_command(0x28); // 4 bit mode
send_command(0x0E); // clear the screen
send_command(0x01); // display on cursor on
send_command(0x06); // increment cursor
send_command(0x80); // row 1 column 1
}
void new_line(void)
{
 send_command(0xC0);
}
void LCD_CLR(void)
{
 send_command(0x01);
}
char *itoa(int num, char *str, int radix) {
 char sign = 0;
 char temp[17]; //an int can only be 16 bits long
 //at radix 2 (binary) the string
//is at most 16 + 1 null long.
 int temp_loc = 0;
 int digit;
 int str_loc = 0;
 //save sign for radix 10 conversion
 if (radix == 10 && num < 0) {
 sign = 1;
 num = -num;
 }
 //construct a backward string of the number.
 do {
 digit = (unsigned int)num % radix;
 if (digit < 10)
 temp[temp_loc++] = digit + '0';
 else
 temp[temp_loc++] = digit - 10 + 'A';
 num = (((unsigned int)num) / radix);
 } while ((unsigned int)num > 0);
 //now add the sign for radix 10
 if (radix == 10 && sign) {
 temp[temp_loc] = '-';
 } else {
 temp_loc--;
 }
 //now reverse the string.
 while ( temp_loc >=0 ) {// while there are still chars
 str[str_loc++] = temp[temp_loc--];
 }
 str[str_loc] = 0; // add null termination.
 return str;
}
