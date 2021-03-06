/***********************************************************************
MODULE		:   UART	 
Chuc nang	:  	Khoi tao ngoai vi va toc do Baud cho UART giua MCU va PC
Tac gia		: 	Bui Trung Hieu - Webmaster: http://www.khvt.com
Su dung		:   Co the su dung voi moi muc dich ma khong can xin phep 
				tac gia ve tac quyen, tac gia khong chiu moi trach nhiem ve 
				cac tac hai gay ra va khong doi hoi cac loi ich co the co
Khoi tao	:	Ngay 31 thang 1 nam 2006 luc 9h15'PM
Hoan thanh	:	Ngay 31 thang 1 nam 2006 luc 10h15'PM

Ghi chu them:	-Modul nay duoc tao ra de tich hop voi ham printf("") san co.
				-Ham ngat dat o bank 0 va khong co uu tien ngat				
***********************************************************************/

// Khai bao thu vien thanh ghi cua P89V51RD2 va cac ham dung trong uart
#include <P89V51Rx2.h>
#include "uart.h"
						

/***********************************************************************
Chuc nang	:    Khoi tao UART o mode 1, 
				-Su dung dao dong Osc 12Mhz
				-BAUDRATE:	 19200	        
Gia tri tra ve:		Khong co
Luu y		:	De dung ket hop voi ngat uart, phai dat EA=1
************************************************************************/
#define xtal 12000000L
#define        baud 38400
#define               SMOD_  0x80
void uart_init(void)
{
 unsigned int bu;
  TR1 = 0; 	        // Configure timer 1 as a baud rate generator
  TCLK=0;RCLK=0;//tao baud thu fat deu dung time 1 mac dinh la 00 luon
 // PCON |=SMOD_; 			         // bo che do double baud; 0x80=SMOD: set serial baudrate doubler

 // TMOD=TMOD &(~T1_GATE_)&(~T1_CT_)|(T1_M1_)&(~T1_M0_);//tuong duong lenh duoi, lenh nay ro hon ve cau truc fan cung
  TMOD |= 0x20; 			         // put timer 1 into MODE 2
 // if(PCON&SMOD_)baud/=2;
  bu=(unsigned char) (-(xtal/384 )/ baud);
  TH1 =bu; 
  TR1 = 1; 				// start timer 1
  //khoi lenh nay tuong duong lenh duoi nhung ro net hon
/* {
  RI   = 1;//set khi co ngat thu phai set de co the thu dc ban dau
  TI   = 1;//set khi co ngat phat phai set de co the phat duoc ban dau
  RB8  = 0;//ko cho thu bit 8 chi set khi multi
  TB8  = 0;//ko cho truyen bit 8 chi set khi multi
  REN  = 1;//cho phep thu
  SM2  = 0;//ko phai che do multy
  SM1  = 1;
  SM0  = 0;  
 }*/
  SCON=0x53;// serial
} //Ket thuc ham khoi tao UART

void uart_isr(void) interrupt 4 using 0
{
}
//Ham ngat khong lam gi ca() :)