/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:                                                                     **
**  Description:                                                                   **
**  Last Modified:                                                                 **
**  Licence:		For radio amateurs experimentation, non-commercial use only!   **
************************************************************************************/

#ifndef __UI_LCD_HY28_H
#define __UI_LCD_HY28_H

#include "arm_math.h"
#include "math.h"
#include "ui_driver.h"

#define MAX_X  320
#define MAX_Y  320

#define SPI_START   (0x70)              /* Start byte for SPI transfer        */
#define SPI_RD      (0x01)              /* WR bit 1 within start              */
#define SPI_WR      (0x00)              /* WR bit 0 within start              */
#define SPI_DATA    (0x02)              /* RS bit 1 within start byte         */
#define SPI_INDEX   (0x00)              /* RS bit 0 within start byte         */

#define RGB(red,green,blue)(uint16_t)(((red>>3)<<11)|((green>>2)<<5)|(blue>> 3))

// Colors definitions, go to http://www.color-hex.com/
// choose a new one and declare here
//
#define White          		0xFFFF
#define Black          		0x0000
#define Grey           		0xBDF5
#define Blue           		0x001F
#define Blue2          		0x051F
#define Red            		0xF800
#define Magenta        		0xF81F
#define Green          		0x07E0
#define Cyan           		0x7FFF
#define Yellow         		0xFFE0

#define Orange				RGB(0xF6,0xA0,0x1A)
#define Cream				RGB(0xED,0xE7,0xD7)

#define Grey1				RGB(0x80,0x80,0x80)
#define Grey2				RGB(0xC0,0xC0,0xC0)
#define Grey3				RGB(0xA6,0xA8,0xAD)

#define Grid				RGB(COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD)

#define LCD_DIR_HORIZONTAL	0x0000
#define LCD_DIR_VERTICAL	0x0001

#define GRADIENT_STEP			8

// ----------------------------------------------------------
// Spectrum draw params
//
// Little bit right of control border
#define SPECTRUM_START_X		(POS_SPECTRUM_IND_X + 2)
//
// Shift of whole spectrum in vertical direction
#define SPECTRUM_START_Y		(POS_SPECTRUM_IND_Y - 10)
//
// Spectrum hight is bit lower that the whole control
#define SPECTRUM_HEIGHT			(POS_SPECTRUM_IND_H - 10)
//
// Dependent on FFT samples,but should be less than control width!
#define SPECTRUM_WIDTH			256
// ----------------------------------------------------------

#define LCD_REG      (*((volatile unsigned short *) 0x60000000))
#define LCD_RAM      (*((volatile unsigned short *) 0x60020000))

// ----------------------------------------------------------
// Dual purpose pins (parallel + serial)
#define LCD_D11 				LCD_CS
#define LCD_D11_SOURCE			LCD_CS_SOURCE
#define LCD_D11_PIO         	LCD_CS_PIO

// ----------------------------------------------------------

void 	UiLcdHy28_LcdClear(ushort Color);
void 	UiLcdHy28_PrintText(ushort Xpos, ushort Ypos, char *str,ushort Color, ushort bkColor,uchar font);

void 	UiLcdHy28_DrawStraightLine(ushort Xpos, ushort Ypos, ushort Length, uchar Direction,ushort color);
void 	UiLcdHy28_DrawHorizLineWithGrad(ushort Xpos, ushort Ypos, ushort Length,ushort gradient_start);

void 	UiLcdHy28_DrawEmptyRect(ushort Xpos, ushort Ypos, ushort Height, ushort Width, ushort color);
void 	UiLcdHy28_DrawBottomButton(ushort Xpos, ushort Ypos, ushort Height, ushort Width,ushort color);
void 	UiLcdHy28_DrawFullRect (ushort Xpos, ushort Ypos, ushort Height, ushort Width, ushort color);

void 	UiLcdHy28_DrawSpectrum(q15_t *fft,ushort color,ushort shift);

void 	UiLcdHy28_Test(void);
uchar 	UiLcdHy28_Init(void);

void 	UiLcdHy28_ShowStartUpScreen(ulong hold_time);

#endif
