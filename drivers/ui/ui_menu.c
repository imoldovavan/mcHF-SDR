/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                              C Turner - KA7OEI 2014                             **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:    	ui_menu.c                                                      **
**  Description:    main user interface configuration/adjustment menu system       **
**  Last Modified:                                                                 **
**  Licence:		For radio amateurs experimentation, non-commercial use only!   **
************************************************************************************/

// Common
//
#include "mchf_board.h"
#include "ui_menu.h"

#include <stdio.h>
#include "arm_math.h"
#include "math.h"
#include "codec.h"
//
//
// LCD
#include "ui_lcd_hy28.h"

// Encoders
#include "ui_rotary.h"
//
// Codec control
#include "codec.h"
#include "softdds.h"

#include "audio_driver.h"

#include "ui_driver.h"
//#include "usbh_usr.h"

#include "cat_driver.h"

// Virtual eeprom
#include "eeprom.h"
//
// CW generation
#include "cw_gen.h"
//
static void UiDriverUpdateMenuLines(uchar index, uchar mode);
static void UiDriverUpdateConfigMenuLines(uchar index, uchar mode);
//
//
// Public data structures
//
// ------------------------------------------------
// Transceiver state public structure
extern __IO TransceiverState 	ts;

// ------------------------------------------------
// Frequency public
__IO DialFrequency 				df;

// ------------------------------------------------
// Encoder one public
__IO EncoderOneSelection		eos;

// ------------------------------------------------
// Encoder two public
__IO EncoderTwoSelection		ews;

// ------------------------------------------------
// Encoder three public
__IO EncoderThreeSelection		ets;

// ------------------------------------------------
// Keypad state
__IO KeypadState				ks;

// ------------------------------------------------
// SWR/Power meter
__IO SWRMeter					swrm;

// ------------------------------------------------
// Power supply meter
__IO PowerMeter					pwmt;

// ------------------------------------------------
// LO Tcxo
__IO LoTcxo						lo;

// ------------------------------------------------
// Spectrum display
extern __IO	SpectrumDisplay		sd;

// Public Audio
extern __IO		AudioDriverState	ads;
//
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateMenu
//* Object              : Display and change menu items
//* Input Parameters    : mode:  0=update all, 1=update current item, 2=go to next screen, 3=restore default setting for selected item
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
void UiDriverUpdateMenu(uchar mode)
{
	uchar var;
	bool  update_vars;
	static uchar screen_disp = 1;	// used to detect screen display switching and prevent unnecessary blanking
	ulong	m_clr, c_clr;

	m_clr = Yellow;
	c_clr = Cyan;

	update_vars = 0;

	if(ts.menu_item < 6)	{	// display first screen of items
		if(screen_disp != 1)	// redraw if this screen wasn't already displayed
			UiDriverClearSpectrumDisplay();
		screen_disp = 1;
		update_vars = 1;
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+0,"10-DSP NR Strength",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+12,"20-300Hz Center Freq.",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+24,"21-500HZ Center Freq.",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+36,"22-1.8k Center Freq.",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+48,"23-2.6k Center Freq.",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+60,"24-3.6k Filter",m_clr,Black,0);
	}
	else if(ts.menu_item < 12)	{	// display second screen of items
		if(screen_disp !=2)
			UiDriverClearSpectrumDisplay();
		screen_disp = 2;
		update_vars = 1;
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+0,"25-10k Filter",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+12,"26-Wide Filt in CW mode",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+24,"27-CW Filt in SSB mode",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+36,"28-AM mode disable",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+48,"29-LSB/USB Auto Select",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+60,"30-AGC Mode",m_clr,Black,0);
	}
	else if(ts.menu_item < 18)	{	// display third screen of items
		if(screen_disp !=3)
			UiDriverClearSpectrumDisplay();
		screen_disp = 3;
		update_vars = 1;
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+0,"31-RF Gain",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+12,"32-Cust AGC (+=Slower)", m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+24,"33-RX Codec Gain",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+36,"34-RX NB Setting",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+48,"35-RX/TX Freq Xlate",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+60,"40-Mic/Line Select",m_clr,Black,0);
	}
	else if(ts.menu_item < 24)	{	// display fourth screen of items
		if(screen_disp !=4)
			UiDriverClearSpectrumDisplay();
		screen_disp = 4;
		update_vars = 1;
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+0,"41-Mic Input Gain",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+12,"42-Line Input Gain",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+24,"43-ALC Release Time",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+36,"44-TX PRE ALC Gain",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+48,"45-TX Audio Compress",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+60,"50-CW Keyer Mode",m_clr,Black,0);
	}
	else if(ts.menu_item < 30)	{	// display fifth screen of items
		if(screen_disp !=5)
			UiDriverClearSpectrumDisplay();
		screen_disp = 5;
		update_vars = 1;
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+0,"51-CW Keyer Speed",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+12,"52-CW Sidetone Gain",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+24,"53-CW Side/Off Freq",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+36,"54-CW Paddle Reverse",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+48,"55-CW TX->RX Delay",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+60,"56-CW Freq. Offset",m_clr,Black,0);
	}
	else if(ts.menu_item < 36)	{	// display sixth screen of items
		if(screen_disp !=6)
			UiDriverClearSpectrumDisplay();
		screen_disp = 6;
		update_vars = 1;
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+0,"70-TCXO Off/On/Stop",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+12,"71-TCXO Temp. (C/F)",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+24,"80-Spec. Scope Speed",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+36,"81-Spec. Scope Filter",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+48,"82-Spec. Trace Colour",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+60,"83-Spec. Grid Colour",m_clr,Black,0);
	}
	else if(ts.menu_item < MAX_MENU_ITEM)	{	// display seventh screen of items
		if(screen_disp !=7)
			UiDriverClearSpectrumDisplay();
		screen_disp = 7;
		update_vars = 1;
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+0,"84-Spec. Scale Colour",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+12,"85-Spec. 2x Magnify",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+24,"86-Spec. AGC Adj.",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+36,"87-Spec. Rescale Adj.",m_clr,Black,0);
//		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+48,"86-Spec. AGC Adj.",m_clr,Black,0);
		UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+60,"00-Adjustment Menu",c_clr,Black,0);
	}
	//
	// ****************   Radio Calibration Menu  ***************
	//
	else if(ts.menu_item >= MAX_MENU_ITEM)	{		// Is this part of the radio configuration menu?
		if((ts.menu_item - MAX_MENU_ITEM) < 6)	{	// yes - display the first screen
			if(screen_disp != 51)
				UiDriverClearSpectrumDisplay();
			screen_disp = 51;
			update_vars = 1;
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+0,"200-Step Size Marker",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+12,"201-Step Button Swap",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+24,"202-Band+/- Button Swap",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+36,"203-Transmit Disable",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+48,"204-O/S Menu SW on TX",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+60,"205-Mute Line Out TX",c_clr,Black,0);
		}
		else if((ts.menu_item - MAX_MENU_ITEM) < 12)	{	// yes - display the second screen
			if(screen_disp != 52)
				UiDriverClearSpectrumDisplay();
			screen_disp = 52;
			update_vars = 1;
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+0,"210-Max Volume",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+12,"211-Max RX Gain (0=Max)",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+24,"220-CAT mode",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+36,"230-Freq. Calibrate",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+48,"240-LSB RX IQ Bal.",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+60,"241-LSB RX IQ Phase",c_clr,Black,0);
		}
		else if((ts.menu_item - MAX_MENU_ITEM) < 18)	{	// yes - display the third screen
			if(screen_disp != 53)
				UiDriverClearSpectrumDisplay();
			screen_disp = 53;
			update_vars = 1;
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+0,"242-USB/CW RX IQ Bal.",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+12,"243-USB RX IQ Phase",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+24,"244-AM RX IQ Bal.",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+36,"250-LSB TX IQ Bal.",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+48,"251-LSB TX IQ Phase",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+60,"252-USB/CW TX IQ Bal.",c_clr,Black,0);
		}
		else if((ts.menu_item - MAX_MENU_ITEM) < 24)	{	// yes - display the fourth screen
			if(screen_disp != 54)
				UiDriverClearSpectrumDisplay();
			screen_disp = 54;
			update_vars = 1;
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+0,"253-USB TX IQ Phase",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+12,"260-CW PA Bias (If >0 )",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+24,"261-PA Bias",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+36,"270-FWD PWR Cal. Factor",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+48,"280-XVTR Offs/Mult",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+60,"281-XVTR Offset (Hz)",c_clr,Black,0);
		}
		else if((ts.menu_item - MAX_MENU_ITEM) < 30)	{	// yes - display the fifth screen
			if(screen_disp != 55)
				UiDriverClearSpectrumDisplay();
			screen_disp = 55;
			update_vars = 1;
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+0,"290-80m 5W PWR Adjust",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+12,"291-60m 5W PWR Adjust",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+24,"292-40m 5W PWR Adjust",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+36,"293-30m 5W PWR Adjust",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+48,"294-20m 5W PWR Adjust",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+60,"295-17m 5W PWR Adjust",c_clr,Black,0);
		}
		else if((ts.menu_item - MAX_MENU_ITEM) < 36)	{	// yes - display the sixth screen
			if(screen_disp != 56)
				UiDriverClearSpectrumDisplay();
			screen_disp = 56;
			update_vars = 1;
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+0,"296-15m 5W PWR Adjust",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+12,"297-12m 5W PWR Adjust",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+24,"298-10m 5W PWR Adjust",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+36,"300-80m Full PWR Adjust",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+48,"301-60m Full PWR Adjust",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+60,"302-40m Full PWR Adjust",c_clr,Black,0);
		}
		else if((ts.menu_item - MAX_MENU_ITEM) < 42)	{	// yes - display the seventh screen
			if(screen_disp != 57)
				UiDriverClearSpectrumDisplay();
			screen_disp = 57;
			update_vars = 1;
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+0,"303-30m Full PWR Adjust",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+12,"304-20m Full PWR Adjust",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+24,"305-17m Full PWR Adjust",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+36,"306-15m Full PWR Adjust",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+48,"307-12m Full PWR Adjust",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+60,"308-10m Full PWR Adjust",c_clr,Black,0);
		}
		else if((ts.menu_item - MAX_MENU_ITEM) < 48)	{	// yes - display the eighth screen
			if(screen_disp != 58)
				UiDriverClearSpectrumDisplay();
			screen_disp = 58;
			update_vars = 1;
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+0,"310-DSP NR BufLen",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+12,"312-DSP NR FFT NumTaps",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+24,"311-DSP NR Post-AGC",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+36,"313-DSP Notch ConvRate",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+48,"314-DSP Notch BufLen",c_clr,Black,0);
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+60,"315-NB AGC T/C (<=Slow)",c_clr,Black,0);
		}
		else if((ts.menu_item - MAX_MENU_ITEM) < 54)	{	// yes - display the ninth screen
			if(screen_disp != 59)
				UiDriverClearSpectrumDisplay();
			screen_disp = 59;
			update_vars = 1;
			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+0,"320-AM TX Audio Filter",c_clr,Black,0);
//			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+12,"312-DSP NR FFT NumTaps",c_clr,Black,0);
//			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+24,"311-DSP NR Post-AGC",c_clr,Black,0);
//			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+36,"313-DSP Notch ConvRate",c_clr,Black,0);
//			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+48,"314-DSP Notch BufLen",c_clr,Black,0);
//			UiLcdHy28_PrintText(POS_MENU_IND_X, POS_MENU_IND_Y+60,"315-NB AGC T/C (<=Slow)",c_clr,Black,0);
		}
	}


	if(ts.menu_var_changed)		// show warning if variable has changed
		c_clr = Orange;
	else						// erase warning by using the same color as the background
		c_clr = Black;
	//
	UiLcdHy28_PrintText(POS_SPECTRUM_IND_X - 2, POS_SPECTRUM_IND_Y + 60, " Save settings using POWER OFF!  ", c_clr, Black, 0);

	//
	// These functions are used to scan the individual menu items and display the items.
	// In each of the FOR loops below, make CERTAIN that the precise number of items are included for each menu!
	//
	if((mode == 0) || update_vars)	{		// display all items and their current settings
		update_vars = 0;
		if(ts.menu_item < 6)	{	// first screen of items
			for(var = 0; var < 6; var++)
				UiDriverUpdateMenuLines(var, 0);
		}
		else if(ts.menu_item < 12)	{	// second screen of items
			for(var = 6; var < 12; var++)
				UiDriverUpdateMenuLines(var, 0);
		}
		else if(ts.menu_item < 18)	{	// third screen of items
			for(var = 12; var < 18; var++)
				UiDriverUpdateMenuLines(var, 0);
		}
		else if(ts.menu_item < 24)	{	// fourth screen of items
			for(var = 18; var < 24; var++)
				UiDriverUpdateMenuLines(var, 0);
		}
		else if(ts.menu_item < 30)	{	// fifth screen of items
			for(var = 24; var < 30; var++)
				UiDriverUpdateMenuLines(var, 0);
		}
		else if(ts.menu_item < 36)	{	// sixth screen of items
			for(var = 30; var < 36; var++)
				UiDriverUpdateMenuLines(var, 0);
		}
		else if(ts.menu_item < MAX_MENU_ITEM)	{	// seventh screen of items
			for(var = 36; var < MAX_MENU_ITEM; var++)
				UiDriverUpdateMenuLines(var, 0);
		}
		//
		// *** ADJUSTMENT MENU ***
		//
		else if(ts.menu_item >= MAX_MENU_ITEM)	{	// Is this one of the radio configuration items?
			if(ts.menu_item < MAX_MENU_ITEM + 6)
				for(var = MAX_MENU_ITEM; var < (MAX_MENU_ITEM + 6); var++)			// first screen of configuration items
					UiDriverUpdateConfigMenuLines(var-MAX_MENU_ITEM, 0);
			else if(ts.menu_item < MAX_MENU_ITEM + 12)
				for(var = MAX_MENU_ITEM + 6; var < (MAX_MENU_ITEM + 12); var++)		// second screen of configuration items
					UiDriverUpdateConfigMenuLines(var-MAX_MENU_ITEM, 0);
			else if(ts.menu_item < MAX_MENU_ITEM + 18)
				for(var = MAX_MENU_ITEM + 12; var < (MAX_MENU_ITEM + 18); var++)		// third screen of configuration items
					UiDriverUpdateConfigMenuLines(var-MAX_MENU_ITEM, 0);
			else if(ts.menu_item < MAX_MENU_ITEM + 24)
				for(var = MAX_MENU_ITEM + 18; var < (MAX_MENU_ITEM + 24); var++)		// fourth screen of configuration items
					UiDriverUpdateConfigMenuLines(var-MAX_MENU_ITEM, 0);
			else if(ts.menu_item < MAX_MENU_ITEM + 30)
				for(var = MAX_MENU_ITEM + 24; var < (MAX_MENU_ITEM + 30); var++)		// fifth screen of configuration items
					UiDriverUpdateConfigMenuLines(var-MAX_MENU_ITEM, 0);
			else if(ts.menu_item < MAX_MENU_ITEM + 36)
				for(var = MAX_MENU_ITEM + 30; var < (MAX_MENU_ITEM + 36); var++)	// sixth screen of configuration items
					UiDriverUpdateConfigMenuLines(var-MAX_MENU_ITEM, 0);
			else if(ts.menu_item < MAX_MENU_ITEM + 42)
				for(var = MAX_MENU_ITEM + 36; var < (MAX_MENU_ITEM + 42); var++)	// seventh screen of configuration items
					UiDriverUpdateConfigMenuLines(var-MAX_MENU_ITEM, 0);
			else if(ts.menu_item < MAX_MENU_ITEM + 48)
				for(var = MAX_MENU_ITEM + 42; var < (MAX_MENU_ITEM + 48); var++)	// eighth screen of configuration items
					UiDriverUpdateConfigMenuLines(var-MAX_MENU_ITEM, 0);
			else if(ts.menu_item < MAX_MENU_ITEM + MAX_RADIO_CONFIG_ITEMS)
				for(var = MAX_MENU_ITEM + 48; var < (MAX_MENU_ITEM + MAX_RADIO_CONFIG_ITEMS); var++)	// ninth screen of configuration items
					UiDriverUpdateConfigMenuLines(var-MAX_MENU_ITEM, 0);
		}
	}
	//
	if(mode == 1)	{	// individual item selected/changed
		if(ts.menu_item < MAX_MENU_ITEM)					// main menu item
			UiDriverUpdateMenuLines(ts.menu_item, 1);
		else												// "adjustment" menu item
			UiDriverUpdateConfigMenuLines(ts.menu_item-MAX_MENU_ITEM, 1);
	}
	else if(mode == 3)	{	// restore default setting for individual item
		if(ts.menu_item < MAX_MENU_ITEM)					// main menu item
			UiDriverUpdateMenuLines(ts.menu_item, 3);
		else												// "adjustment" menu item
			UiDriverUpdateConfigMenuLines(ts.menu_item-MAX_MENU_ITEM,3);
	}
}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateMenuLines
//* Object              : Display and and change line items
//* Input Parameters    : index:  Line to display  mode:  0=display/update 1=change item 3=set default
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
static void UiDriverUpdateMenuLines(uchar index, uchar mode)
{
	char options[32];
	ulong opt_pos;					// y position of option cursor
	static ulong opt_oldpos = 999;	// y position of option cursor, previous
	uchar select;
	ulong	clr;
	int var;
	float tcalc;
	bool	fchange = 0;
	uchar	temp_sel;		// used as temporary holder during selection
	bool	disp_shift = 0;	// TRUE if option display is to be shifted to the left to allow more options

	clr = White;		// color used it display of adjusted options

	if(mode == 0)	{	// are we in update/display mode?
		select = index;	// use index passed from calling function
		var = 0;		// prevent any change of variable
	}
	else	{			// this is "change" mode
		select = ts.menu_item;	// item selected from encoder
		var = ts.menu_var;		// change from encoder
		ts.menu_var = 0;		// clear encoder change detect
	}
	strcpy(options, "ERROR");	// pre-load to catch error condition
	//
	switch(select)	{		//  DSP_NR_STRENGTH_MAX
	case MENU_DSP_NR_STRENGTH:	// DSP Noise reduction strength
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.dsp_nr_strength++;
			fchange = 1;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
			if(ts.dsp_nr_strength)
				ts.dsp_nr_strength--;
		}
		//
		if(ts.dsp_nr_strength > DSP_NR_STRENGTH_MAX)
			ts.dsp_nr_strength = DSP_NR_STRENGTH_MAX;
		//
		if(mode == 3)	{
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.dsp_nr_strength = DSP_NR_STRENGTH_DEFAULT;
			fchange = 1;
		}
		//
		if(fchange)	{		// did it change?
			if(ts.dsp_active & 1)	// only change if DSP active
				audio_driver_set_rx_audio_filter();
		}
		//
		if(!(ts.dsp_active & 1))	// make red if DSP not active
			clr = Orange;
		else	{
			if(ts.dsp_nr_strength >= DSP_STRENGTH_RED)
				clr = Red;
			else if(ts.dsp_nr_strength >= DSP_STRENGTH_ORANGE)
				clr = Orange;
			else if(ts.dsp_nr_strength >= DSP_STRENGTH_YELLOW)
				clr = Yellow;
		}
		//
		sprintf(options, " %u ", ts.dsp_nr_strength);
		opt_pos = MENU_DSP_NR_STRENGTH % MENUSIZE;	// Y position of this menu item
		break;
	//
	case MENU_300HZ_SEL:	// 300 Hz filter select
		if(var >= 1)	{	// did the selection increase?
			ts.filter_300Hz_select++;	// yes - increase
			fchange = 1;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
		}
		else if(var <= -1)	{		// did the setting decrease?
			fchange = 1;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			if(ts.filter_300Hz_select)		// yes, reduce the setting if not at minimum
				ts.filter_300Hz_select--;
		}
		//
		if(mode == 3)	{		// load default setting
			ts.filter_300Hz_select = FILTER_300HZ_DEFAULT;
			fchange = 1;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
		}
		//
		if(ts.filter_300Hz_select > MAX_300HZ_FILTER)
			ts.filter_300Hz_select = MAX_300HZ_FILTER;
		//
		if(ts.filter_id != AUDIO_300HZ)
			clr = Orange;
		//
		switch(ts.filter_300Hz_select)	{
		case 0:
			strcpy(options, "Off  ");
			clr = Red;
			break;
		case 1:
			strcpy(options, "500Hz");
			break;
		case 2:
			strcpy(options, "550Hz");
			break;
		case 3:
			strcpy(options, "600Hz");
			break;
		case 4:
			strcpy(options, "650Hz");
			break;
		case 5:
			strcpy(options, "700Hz");
			break;
		case 6:
			strcpy(options, "750Hz");
			break;
		case 7:
			strcpy(options, "800Hz");
			break;
		case 8:
			strcpy(options, "850Hz");
			break;
		case 9:
			strcpy(options, "900Hz");
			break;
		}
		//
		if((ts.txrx_mode == TRX_MODE_RX) && (fchange))	{		// set filter if changed
			audio_driver_set_rx_audio_filter();
			if(ts.filter_id == AUDIO_300HZ)	{
				//UiDriverProcessActiveFilterScan();	// find next active filter
				UiDriverChangeFilter(0);
			}
		}
		//
		opt_pos = MENU_300HZ_SEL % MENUSIZE;
		break;
	case MENU_500HZ_SEL:	// 500 Hz filter select
		if(var >= 1)	{	// did the selection increase?
			fchange = 1;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.filter_500Hz_select++;	// yes - increase
		}
		else if(var <= -1)	{		// did the setting decrease?
			fchange = 1;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			if(ts.filter_500Hz_select)		// yes, reduce the setting if not at minimum
				ts.filter_500Hz_select--;
		}
		//
		if(mode == 3)	{		// load default setting
			ts.filter_500Hz_select = FILTER_500HZ_DEFAULT;
			fchange = 1;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
		}
		//
		if(ts.filter_500Hz_select > MAX_500HZ_FILTER)
			ts.filter_500Hz_select = MAX_500HZ_FILTER;
		//
		if(ts.filter_id != AUDIO_500HZ)
			clr = Orange;
		//
		switch(ts.filter_500Hz_select)	{
		case 0:
			strcpy(options, "Off  ");
			clr = Red;
			break;
		case 1:
			strcpy(options, "550Hz");
			break;
		case 2:
			strcpy(options, "650Hz");
			break;
		case 3:
			strcpy(options, "750Hz");
			break;
		case 4:
			strcpy(options, "850Hz");
			break;
		case 5:
			strcpy(options, "950Hz");
			break;
		}
		//
		if((ts.txrx_mode == TRX_MODE_RX) && (fchange))	{		// set filter if changed
			audio_driver_set_rx_audio_filter();
			if(ts.filter_id == AUDIO_500HZ)	{
				//UiDriverProcessActiveFilterScan();	// find next active filter
				UiDriverChangeFilter(0);
			}
		}
		//
		opt_pos = MENU_500HZ_SEL % MENUSIZE;
		break;
	case MENU_1K8_SEL:	// 1.8 kHz filter select
		if(var >= 1)	{	// did the selection increase?
			fchange = 1;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.filter_1k8_select++;	// yes - increase
		}
		else if(var <= -1)	{		// did the setting decrease?
			fchange = 1;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			if(ts.filter_1k8_select)		// yes, reduce the setting if not at minimum
				ts.filter_1k8_select--;
		}
		//
		if(mode == 3)	{	// load default setting
			fchange = 1;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.filter_1k8_select = FILTER_1K8_DEFAULT;
		}
		//
		if(ts.filter_1k8_select > MAX_1K8_FILTER)
			ts.filter_1k8_select = MAX_1K8_FILTER;
		//
		if(ts.filter_id != AUDIO_1P8KHZ)
			clr = Orange;
		//
		switch(ts.filter_1k8_select)	{
		case 0:
			strcpy(options, "Off   ");
			clr = Red;
			break;
		case 1:
			strcpy(options, "1125Hz");
			break;
		case 2:
			strcpy(options, "1275Hz");
			break;
		case 3:
			strcpy(options, "1427Hz");
			break;
		case 4:
			strcpy(options, "1575Hz");
			break;
		case 5:
			strcpy(options, "1725Hz");
			break;
		}
		//
		if((ts.txrx_mode == TRX_MODE_RX) && (fchange))	{	// set filter if changed
			audio_driver_set_rx_audio_filter();
			if(ts.filter_id == AUDIO_1P8KHZ)	{
				//UiDriverProcessActiveFilterScan();	// find next active filter
				UiDriverChangeFilter(0);
			}
		}
		//
		opt_pos = MENU_1K8_SEL % MENUSIZE;
		break;
	case MENU_2k3_SEL: // 2.3 kHz filter select
		if(var >= 1)	{	// did the selection increase?
			fchange = 1;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.filter_2k3_select++;	// yes - increase
		}
		else if(var <= -1)	{		// did the setting decrease?
			fchange = 1;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			if(ts.filter_2k3_select)		// yes, reduce the setting if not at minimum
				ts.filter_2k3_select--;
		}
		//
		if(mode == 3)	{	// load default setting
			fchange = 1;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.filter_2k3_select = FILTER_2K3_DEFAULT;
		}
		//
		if(ts.filter_2k3_select <= MIN_FILTER_SELECT_VAL)
			ts.filter_2k3_select = 1;
		else if(ts.filter_2k3_select > MAX_2K3_FILTER)
			ts.filter_2k3_select = MAX_2K3_FILTER;
		//
		if(ts.filter_id != AUDIO_2P3KHZ)
			clr = Orange;
		//
		switch(ts.filter_2k3_select)	{
		case 1:
			strcpy(options, "1262Hz");
			break;
		case 2:
			strcpy(options, "1412Hz");
			break;
		case 3:
			strcpy(options, "1562Hz");
			break;
		case 4:
			strcpy(options, "1712Hz");
			break;
		}
		//
		if((ts.txrx_mode == TRX_MODE_RX) && (fchange))		// set filter if changed
			audio_driver_set_rx_audio_filter();
		//
		opt_pos = MENU_2k3_SEL % MENUSIZE;
		break;
	case MENU_3K6_SEL: // 3.6 kHz filter select
		if(var >= 1)	{	// setting increase?
			ts.filter_3k6_select = 1;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.filter_3k6_select= 0;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
		}
		//
		if(mode == 3)	{
			ts.filter_3k6_select = 1;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
		}
		//
		if(ts.filter_id != AUDIO_3P6KHZ)
			clr = Orange;
		//
		if(ts.filter_3k6_select)
			strcpy(options, " ON ");
		else	{
			strcpy(options, " OFF");
			clr = Red;
		}
		//
		if((ts.filter_id == AUDIO_3P6KHZ) && fchange)	{
			//UiDriverProcessActiveFilterScan();	// find next active filter
			UiDriverChangeFilter(0);
		}
		//
		opt_pos = MENU_3K6_SEL % MENUSIZE;
		break;
	case MENU_10K_SEL: // 10 kHz filter select
		if(var >= 1)	{	// setting increase?
			ts.filter_10k_select = 1;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.filter_10k_select= 0;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
		}
		//
		if(mode == 3)	{
			ts.filter_10k_select = 1;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
		}
		//
		if(ts.filter_id != AUDIO_10KHZ)
			clr = Orange;
		//
		if(ts.filter_10k_select)
			strcpy(options, " ON ");
		else	{
			strcpy(options, " OFF");
			clr = Red;
		}
		//
		if((ts.filter_id == AUDIO_10KHZ) && fchange)	{
			//UiDriverProcessActiveFilterScan();	// find next active filter
			UiDriverChangeFilter(0);
		}
		//
		opt_pos = MENU_10K_SEL % MENUSIZE;
		break;
	case MENU_CW_WIDE_FILT: // CW mode wide filter enable/disable
		if(var >= 1)	{	// setting increase?
			ts.filter_cw_wide_disable = 1;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
		}
		else if(var <= -1)	{	// setting decrease?
			ts.filter_cw_wide_disable = 0;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
		}
		//
		if(mode == 3)	{
			ts.filter_cw_wide_disable = 0;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
		}
		//
		if(!ts.filter_cw_wide_disable)	// sense is inverted since this is a DISABLE flag
			strcpy(options, " ON ");
		else	{
			strcpy(options, " OFF");
			clr = Orange;
		}
		//
		if(ts.dmod_mode != DEMOD_CW)	// if not in CW mode, make orange
			clr = Orange;
		//
		opt_pos = MENU_CW_WIDE_FILT % MENUSIZE;
		break;
		//
	case MENU_SSB_NARROW_FILT: // SSW mode narrow filter enable/disable
		if(var >= 1)	{	// setting increase?
			ts.filter_ssb_narrow_disable = 1;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
		}
		else if(var <= -1)	{	// setting decrease?
			ts.filter_ssb_narrow_disable = 0;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
		}
		//
		if(mode == 3)	{
			ts.filter_ssb_narrow_disable = 0;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
		}
		//
		if(!ts.filter_ssb_narrow_disable)	// sense is inverted since this is a DISABLE flag
			strcpy(options, " ON ");

		else	{
			strcpy(options, " OFF");
			clr = Orange;
		}
		//
		if(ts.dmod_mode == DEMOD_CW)	// if in voice mode, make orange
			clr = Orange;
		//
		opt_pos = MENU_SSB_NARROW_FILT % MENUSIZE;
		break;
		//
	case MENU_AM_DISABLE: // AM mode enable/disable
		if(var >= 1)	{	// setting increase?
			ts.am_mode_disable = 1;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
		}
		else if(var <= -1)	{	// setting decrease?
			ts.am_mode_disable = 0;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
		}
		//
		if(mode == 3)	{
			ts.am_mode_disable = 0;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
		}
		//
		if(ts.am_mode_disable)	{	// sense is inverted since this is a DISABLE flag
			strcpy(options, " ON ");
			clr = Orange;
		}
		else
			strcpy(options, " OFF");
		//
		opt_pos = MENU_AM_DISABLE % MENUSIZE;
		break;
		//
	case MENU_SSB_AUTO_MODE_SELECT:		// Enable/Disable auto LSB/USB select
		if(var >= 1)	{	// did the selection increase?
			ts.lsb_usb_auto_select++;	// yes - increase
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
		}
		else if(var <= -1)	{		// did the setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
			if(ts.lsb_usb_auto_select)		// yes, reduce the setting if not at minimum
				ts.lsb_usb_auto_select--;
		}
		//
		if(mode == 3)	{		// load default setting
			fchange = 1;
			ts.lsb_usb_auto_select = AUTO_LSB_USB_OFF;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
		}
		//
		if(ts.lsb_usb_auto_select > AUTO_LSB_USB_MAX)	// limit to maximum setting
			ts.lsb_usb_auto_select = AUTO_LSB_USB_MAX;
		//
		if(ts.lsb_usb_auto_select == AUTO_LSB_USB_ON)		// LSB on bands < 10 MHz
			strcpy(options, "   ON  ");		// yes
		else if(ts.lsb_usb_auto_select == AUTO_LSB_USB_60M)	// USB on 60 meters?
			strcpy(options, "USB 60M");		// yes
		else
			strcpy(options, "  OFF  ");		// no (obviously!)
		//
		opt_pos = MENU_SSB_AUTO_MODE_SELECT % MENUSIZE;	// Y position of this menu item
		break;
	//
	case MENU_AGC_MODE:	// AGC mode
		if(var >= 1)	{	// did the selection increase?
			ts.agc_mode++;	// yes - increase
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
		}
		else if(var <= -1)	{		// did the setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
			if(ts.agc_mode > AGC_SLOW)		// yes, reduce the setting if not at minimum
				ts.agc_mode--;
		}
		//
		if(mode == 3)	{		// load default setting
			fchange = 1;
			ts.agc_mode = AGC_DEFAULT;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
		}
		//
		if(ts.agc_mode > AGC_MAX_MODE)		// limit selection ranges for this mode
			ts.agc_mode = AGC_MAX_MODE;
		//
		if(ts.agc_mode == AGC_SLOW)
			strcpy(options, "SLOW  ");
		else if(ts.agc_mode == AGC_MED)
			strcpy(options, "MED   ");
		else if(ts.agc_mode == AGC_FAST)
			strcpy(options, "FAST  ");
		else if(ts.agc_mode == AGC_OFF)	{
			strcpy(options, "MANUAL");
			clr = Red;
		}
		else if(ts.agc_mode == AGC_CUSTOM)
			strcpy(options, "CUSTOM");
		//
		if(fchange)	{
			// now set the AGC
			if(ts.agc_mode == AGC_SLOW)
				ads.agc_decay = AGC_SLOW_DECAY;
			else if(ts.agc_mode == AGC_FAST)
				ads.agc_decay = AGC_FAST_DECAY;
			else if(ts.agc_mode == AGC_CUSTOM)	{
				tcalc = (float)ts.agc_custom_decay;	// use temp var "tcalc" as audio function
				tcalc += 30;			// can be called mid-calculation!
				tcalc /= 10;
				tcalc *= -1;
				tcalc = powf(10, tcalc);
				ads.agc_decay = tcalc;
			}
			else
				ads.agc_decay = AGC_MED_DECAY;
		}
		//
		if(ts.txrx_mode == TRX_MODE_TX)	// Orange if in TX mode
			clr = Orange;
		//
		opt_pos = MENU_AGC_MODE % MENUSIZE;	// Y position of this menu item
		break;
	//
	case MENU_RF_GAIN_ADJ:		// RF gain control adjust
		if(var >= 1)	{	// setting increase?
			ts.rf_gain++;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
			if(ts.rf_gain)
				ts.rf_gain--;
		}
		//
		if(mode == 3)	{
			ts.rf_gain = DEFAULT_RF_GAIN;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
		}
		//
		if(ts.rf_gain > MAX_RF_GAIN)
			ts.rf_gain = MAX_RF_GAIN;
		//
		// calculate RF gain setting
		//
		if(fchange)	{
			tcalc = (float)ts.rf_gain;	// use temp var as the resulting
			tcalc -= 20;				// variable may be used during
			tcalc /= 10;				// the actual calculation!
			tcalc = powf(10, tcalc);
			ads.agc_rf_gain = tcalc;
		}
		//
		if(ts.rf_gain < 20)
			clr = Red;
		else if(ts.rf_gain < 30)
			clr = Orange;
		else if(ts.rf_gain < 40)
			clr = Yellow;
		else
			clr = White;
		//
		if(fchange)		// did RFGain get changed?
			UiDriverChangeRfGain(0);	// yes, change on-screen RF gain setting
		//
		sprintf(options, " %d ", ts.rf_gain);
		opt_pos = MENU_RF_GAIN_ADJ % MENUSIZE;
		break;
		// RX Codec gain adjust
	case MENU_CUSTOM_AGC:		// Custom AGC adjust
		if(var >= 1)	{	// setting increase?
			ts.agc_custom_decay++;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
			if(ts.agc_custom_decay)
				ts.agc_custom_decay--;
		}
		//
		if(mode == 3)	{
			ts.agc_custom_decay = AGC_CUSTOM_DEFAULT;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
		}
		//
		if(fchange)	{
			if(ts.agc_custom_decay > AGC_CUSTOM_MAX)
				ts.agc_custom_decay = AGC_CUSTOM_MAX;
			// now set the custom AGC - if in custom mode
			if(ts.agc_mode == AGC_CUSTOM)	{
				tcalc = (float)ts.agc_custom_decay;	// use temp var "tcalc" as audio function
				tcalc += 30;			// can be called mid-calculation!
				tcalc /= 10;
				tcalc *= -1;
				tcalc = powf(10, tcalc);
				ads.agc_decay = tcalc;
			}
		}
		//
		if((ts.txrx_mode == TRX_MODE_TX) || (ts.agc_mode != AGC_CUSTOM))	// Orange if in TX mode
			clr = Orange;
		else if(ts.agc_custom_decay <= AGC_CUSTOM_FAST_WARNING)				// Display in red if setting may be too fast
			clr = Red;
		//
		sprintf(options, " %d ", ts.agc_custom_decay);
		opt_pos = MENU_CUSTOM_AGC % MENUSIZE;
		break;
	// A/D Codec Gain/Mode setting/adjust
	case MENU_CODEC_GAIN_MODE:
		if(var >= 1)	{	// setting increase?
			ts.rf_codec_gain++;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			if(ts.rf_codec_gain)
				ts.rf_codec_gain--;
		}
		//
		if(mode == 3)	{
			ts.rf_codec_gain = DEFAULT_RF_CODEC_GAIN_VAL;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
		}
		//
		if(ts.rf_codec_gain > MAX_RF_CODEC_GAIN_VAL)
			ts.rf_codec_gain = MAX_RF_CODEC_GAIN_VAL;
		//
		if(ts.rf_codec_gain == 9)
			strcpy(options, "AUTO ");
		else	{	// if anything other than "Auto" give a warning in RED
			sprintf(options,"> %u <", ts.rf_codec_gain);
			clr = Red;
		}
		opt_pos = MENU_CODEC_GAIN_MODE % MENUSIZE;
		break;
		//
	case MENU_NOISE_BLANKER_SETTING:
		if(var >= 1)	{	// setting increase?
			ts.nb_setting++;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
			if(ts.nb_setting)
				ts.nb_setting--;
		}
		//
		if(mode == 3)	{		// default
			ts.nb_setting = 0;	// off
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
		}
		//
		if(ts.nb_setting > MAX_NB_SETTING)
			ts.nb_setting = MAX_NB_SETTING;
		//
		if(ts.nb_setting >= NB_WARNING3_SETTING)
			clr = Red;		// above this value, make it red
		else if(ts.nb_setting >= NB_WARNING2_SETTING)
			clr = Orange;		// above this value, make it orange
		else if(ts.nb_setting >= NB_WARNING1_SETTING)
			clr = Yellow;		// above this value, make it yellow
		//
		sprintf(options," %u  ", ts.nb_setting);
		//
		opt_pos = MENU_NOISE_BLANKER_SETTING % MENUSIZE;
		break;
	//
	case MENU_RX_FREQ_CONV:		// Enable/Disable receive frequency conversion
		if(var >= 1)	{	// setting increase?
			ts.iq_freq_mode++;
			fchange = 1;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
			if(ts.iq_freq_mode)
				ts.iq_freq_mode--;
		}
		//
		if(mode == 3)	{		// default
			ts.iq_freq_mode = FREQ_IQ_CONV_MODE_DEFAULT;	// off
			fchange = 1;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
		}
		//
		if(ts.iq_freq_mode > FREQ_IQ_CONV_MODE_MAX)
			ts.iq_freq_mode = FREQ_IQ_CONV_MODE_MAX;
		//
		disp_shift = 1;
		if(!ts.iq_freq_mode)
			sprintf(options,"    OFF   ");
		else if(ts.iq_freq_mode == 1)	{
			sprintf(options,"RX LO HIGH");
			sd.magnify = 0;		// force spectrum scope magnify to OFF mode
		}
		else if(ts.iq_freq_mode == 2)	{
			sprintf(options,"RX LO LOW ");
			sd.magnify = 0;		// force spectrum scope magnify to OFF mode
		}
		//
		//
		if(fchange)	{	// update parameters if changed
			UiDriverUpdateFrequency(1);	// update frequency display without checking encoder
		}
		//
		opt_pos = MENU_RX_FREQ_CONV % MENUSIZE;	// Y position of this menu item
		break;
	//
	case MENU_MIC_LINE_MODE:	// Mic/Line mode
		if(var >= 1)	{	// setting increase?
			ts.tx_audio_source++;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
			if(ts.tx_audio_source)
				ts.tx_audio_source--;
		}
		//
		if(ts.tx_audio_source >= TX_AUDIO_MAX_ITEMS)
			ts.tx_audio_source = TX_AUDIO_MAX_ITEMS -1;
		//
		if(mode == 3)	{
			ts.tx_audio_source = TX_AUDIO_MIC;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
		}
		//
		if(ts.tx_audio_source == TX_AUDIO_MIC)
			strcpy(options, "MIC ");
		else if(ts.tx_audio_source == TX_AUDIO_LINEIN)
			strcpy(options, "LINE");
		//
		if(fchange)	{		// if there was a change, do update of on-screen information
			if(ts.dmod_mode == DEMOD_CW)
				UiDriverChangeKeyerSpeed(0);
			else
				UIDriverChangeAudioGain(0);
		}
		///
		opt_pos = MENU_MIC_LINE_MODE % MENUSIZE;	// Y position of this menu item
		break;
	//
	case MENU_MIC_GAIN:	// Mic Gain setting
		if(ts.tx_audio_source == TX_AUDIO_MIC)	{	// Allow adjustment only if in MIC mode
			if(var >= 1)	{	// setting increase?
				ts.tx_mic_gain++;
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				fchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				fchange = 1;
				if(ts.tx_mic_gain)
					ts.tx_mic_gain--;
			}
			//
			if(mode == 3)	{
				ts.tx_mic_gain = MIC_GAIN_DEFAULT;
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				fchange = 1;
			}
			//
			if(ts.tx_mic_gain < MIC_GAIN_MIN)
				ts.tx_mic_gain = MIC_GAIN_MIN;
			//
			if(ts.tx_mic_gain > MIC_GAIN_MAX)
				ts.tx_mic_gain = MIC_GAIN_MAX;
		}
		//
		if((ts.txrx_mode == TRX_MODE_TX) && (fchange))	{		// only adjust the hardware if in TX mode (it will kill RX otherwise!)
			if(ts.tx_mic_gain > 50)	{		// actively adjust microphone gain and microphone boost
				ts.mic_boost = 1;
				Codec_WriteRegister(W8731_ANLG_AU_PATH_CNTR,0x0015);	// mic boost on
				ts.tx_mic_gain_mult = (ts.tx_mic_gain - 35)/3;			// above 50, rescale software amplification
			}
			else	{
				ts.mic_boost = 0;
				Codec_WriteRegister(W8731_ANLG_AU_PATH_CNTR,0x0014);	// mic boost off
				ts.tx_mic_gain_mult = ts.tx_mic_gain;
			}
		}
		//
		if(fchange)	{		// update on-screen info if there was a change
			if(ts.dmod_mode == DEMOD_CW)
				UiDriverChangeKeyerSpeed(0);
			else
				UIDriverChangeAudioGain(0);
		}
		//
		if(ts.tx_audio_source != TX_AUDIO_MIC)	// Orange if not in MIC-IN mode
			clr = Orange;
		//
		sprintf(options, " %u  ", ts.tx_mic_gain);
		opt_pos = MENU_MIC_GAIN % MENUSIZE;	// Y position of this menu item
		break;
	//
	case MENU_LINE_GAIN:	// Line Gain setting
		if(ts.tx_audio_source == TX_AUDIO_LINEIN)	{	// Allow adjustment only if in line-in mode
			if(var >= 1)	{	// setting increase?
				ts.tx_line_gain++;
				fchange = 1;
				ts.menu_var_changed = 1;	// indicate that a change has occurred
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				fchange = 1;
				if(ts.tx_line_gain)
					ts.tx_line_gain--;
			}
			//
			if(ts.tx_line_gain < LINE_GAIN_MIN)
				ts.tx_line_gain = LINE_GAIN_MIN;
			//
			if(ts.tx_line_gain > LINE_GAIN_MAX)
				ts.tx_line_gain = LINE_GAIN_MAX;
			//
			if(mode == 3)	{
				ts.tx_line_gain = LINE_GAIN_DEFAULT;
				fchange = 1;
				ts.menu_var_changed = 1;	// indicate that a change has occurred
			}
		}
		//
		if(fchange)	{		// update on-screen info and codec if there was a change
			if(ts.dmod_mode == DEMOD_CW)
				UiDriverChangeKeyerSpeed(0);
			else	{		// in voice mode
				UIDriverChangeAudioGain(0);
				if(ts.txrx_mode == TRX_MODE_TX)		// in transmit mode?
					Codec_Line_Gain_Adj(ts.tx_line_gain);		// change codec gain
			}
		}
		//
		if(ts.tx_audio_source != TX_AUDIO_LINEIN)	// Orange if not in LINE-IN mode
			clr = Orange;
		//
		sprintf(options, " %u ", ts.tx_line_gain);
		opt_pos = MENU_LINE_GAIN % MENUSIZE;	// Y position of this menu item
		break;
	//
	case MENU_ALC_RELEASE:		// ALC Release adjust
		if(ts.tx_comp_level == TX_AUDIO_COMPRESSION_MAX)	{
			if(var >= 1)	{	// setting increase?
				ts.alc_decay++;
				fchange = 1;
				ts.menu_var_changed = 1;	// indicate that a change has occurred
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				fchange = 1;
				if(ts.alc_decay)
					ts.alc_decay--;
			}
			//
			if(mode == 3)	{
				ts.alc_decay = ALC_DECAY_DEFAULT;
				fchange = 1;
				ts.menu_var_changed = 1;	// indicate that a change has occurred
			}
			//
			if(ts.alc_decay > ALC_DECAY_MAX)
				ts.alc_decay = ALC_DECAY_MAX;
				//
			if(fchange)	{		// value changed?  Recalculate
				tcalc = (float)ts.alc_decay;	// use temp var "tcalc" as audio function
				tcalc += 35;			// can be called mid-calculation!
				tcalc /= 10;
				tcalc *= -1;
				tcalc = powf(10, tcalc);
				ads.alc_decay = tcalc;
			}
		}
		else			// indicate RED if "Compression Level" below was nonzero
			clr = Red;
		//
		sprintf(options, " %d ", ts.alc_decay);
		opt_pos = MENU_ALC_RELEASE % MENUSIZE;
		break;
	//
	case MENU_ALC_POSTFILT_GAIN:		// ALC TX Post-filter gain (Compressor level)
		if(ts.tx_comp_level == TX_AUDIO_COMPRESSION_MAX)	{
			if(var >= 1)	{	// setting increase?
				ts.alc_tx_postfilt_gain++;
				fchange = 1;
				ts.menu_var_changed = 1;	// indicate that a change has occurred
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				fchange = 1;
				if(ts.alc_tx_postfilt_gain)
					ts.alc_tx_postfilt_gain--;
			}
			//
			if(mode == 3)	{
				ts.alc_tx_postfilt_gain = ALC_POSTFILT_GAIN_DEFAULT;
				fchange = 1;
				ts.menu_var_changed = 1;	// indicate that a change has occurred
			}
			//
			if(ts.alc_tx_postfilt_gain > ALC_POSTFILT_GAIN_MAX)
				ts.alc_tx_postfilt_gain = ALC_POSTFILT_GAIN_MAX;
			else if(ts.alc_tx_postfilt_gain < ALC_POSTFILT_GAIN_MIN)
				ts.alc_tx_postfilt_gain = ALC_POSTFILT_GAIN_MIN;
			//
			if(fchange)	{
				if(ts.dmod_mode != DEMOD_CW)	// In voice mode?
					UiDriverChangeCmpLevel(0);	// update on-screen display of compression level
			}
		}
		else			// indicate RED if "Compression Level" below was nonzero
			clr = Red;
		//
		sprintf(options, " %d ", ts.alc_tx_postfilt_gain);
		opt_pos = MENU_ALC_POSTFILT_GAIN % MENUSIZE;
		break;
	case MENU_TX_COMPRESSION_LEVEL:		// ALC TX Post-filter gain (Compressor level)
		if(var >= 1)	{	// setting increase?
			ts.tx_comp_level++;
			fchange = 1;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
			if(ts.tx_comp_level)
				ts.tx_comp_level--;
		}
		//
		if(mode == 3)	{
			ts.tx_comp_level = TX_AUDIO_COMPRESSION_DEFAULT;
			fchange = 1;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
		}
		//
		if(ts.tx_comp_level > TX_AUDIO_COMPRESSION_MAX)
			ts.tx_comp_level = TX_AUDIO_COMPRESSION_MAX;
		//
		if(fchange)	{
			UiCalcTxCompLevel();			// calculate parameters for selected amount of compression
			//
			if(ts.dmod_mode != DEMOD_CW)	// In voice mode?
				UiDriverChangeCmpLevel(0);	// update on-screen display of compression level
		}
		//
		if(ts.tx_comp_level < TX_AUDIO_COMPRESSION_MAX)	// 	display numbers for all but the highest value
			sprintf(options," %d ",ts.tx_comp_level);
		else					// show "SV" (Stored Value) for highest value
			strcpy(options, " SV");
//		sprintf(options, " %d ", ts.tx_comp_level);
		opt_pos = MENU_TX_COMPRESSION_LEVEL % MENUSIZE;
		break;
	case MENU_KEYER_MODE:	// Keyer mode
		if(var >= 1)	{	// setting increase?
			ts.keyer_mode++;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			if(ts.keyer_mode)
				ts.keyer_mode--;
		}
		//
		if(ts.keyer_mode >= CW_MAX_MODE)
			ts.keyer_mode = CW_MAX_MODE - 1;
		//
		if(mode == 3)	{
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.keyer_mode = CW_MODE_IAM_B;
		}
		//
		if(ts.keyer_mode == CW_MODE_IAM_B)
			strcpy(options, "IAM_B");
		else if(ts.keyer_mode == CW_MODE_IAM_A)
			strcpy(options, "IAM_A");
		else if(ts.keyer_mode == CW_MODE_STRAIGHT)
			strcpy(options, "STR_K");
		opt_pos = MENU_KEYER_MODE % MENUSIZE;	// Y position of this menu item
		break;
	//
	case MENU_KEYER_SPEED:	// keyer speed
		if(var >= 1)	{	// setting increase?
			ts.keyer_speed++;
			fchange = 1;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			if(ts.keyer_speed > MAX_KEYER_SPEED)
				ts.keyer_speed = MAX_KEYER_SPEED;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
			if(ts.keyer_speed)	{
				ts.keyer_speed--;
				if(ts.keyer_speed < MIN_KEYER_SPEED)
					ts.keyer_speed = MIN_KEYER_SPEED;
			}
		}
		//
		if(mode == 3)	{
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.keyer_speed = DEFAULT_KEYER_SPEED;
			fchange = 1;
		}
		//
		if(fchange)	{		// did it get changed?
			if(ts.dmod_mode == DEMOD_CW)		// yes, update on-screen info
				UiDriverChangeKeyerSpeed(0);
			else
				UIDriverChangeAudioGain(0);
		}
		//
		sprintf(options, " %u ", ts.keyer_speed);
		opt_pos = MENU_KEYER_SPEED % MENUSIZE;	// Y position of this menu item
		break;
	//
	case MENU_SIDETONE_GAIN:	// sidetone gain
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.st_gain++;
			fchange = 1;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
			if(ts.st_gain)
				ts.st_gain--;
		}
		//
		if(ts.st_gain > SIDETONE_MAX_GAIN)
			ts.st_gain = SIDETONE_MAX_GAIN;
		//
		if(mode == 3)	{
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.st_gain = DEFAULT_SIDETONE_GAIN;
			fchange = 1;
		}
		//
		if(fchange)	{		// did it change?
			if(ts.dmod_mode == DEMOD_CW)
				UiDriverChangeStGain(0);		// update on-screen display of sidetone gain
			else
				UiDriverChangeCmpLevel(0);
			//
		}
		//
		sprintf(options, " %u ", ts.st_gain);
		opt_pos = MENU_SIDETONE_GAIN % MENUSIZE;	// Y position of this menu item
		break;
	//
	case MENU_SIDETONE_FREQUENCY:	// sidetone frequency
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.sidetone_freq+= 10;
			fchange = 1;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
			if(ts.sidetone_freq > CW_SIDETONE_FREQ_MIN)	{
				ts.sidetone_freq-= 10;
			}
		}
		if(ts.sidetone_freq > CW_SIDETONE_FREQ_MAX)
			ts.sidetone_freq = CW_SIDETONE_FREQ_MAX;
		//
		if(ts.sidetone_freq <= CW_SIDETONE_FREQ_MIN)
			ts.sidetone_freq = CW_SIDETONE_FREQ_MIN;
		//
		// Set the operational sidetone frequency
		//
		if(mode == 3)	{
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.sidetone_freq = CW_SIDETONE_FREQ_DEFAULT;
			fchange = 1;
		}
		//
		if((ts.dmod_mode == DEMOD_CW) && (fchange))	{
			softdds_setfreq((float)ts.sidetone_freq,ts.samp_rate,0);
			UiDriverUpdateFrequency(1);	// update frequency display without checking encoder
		}
		//
		sprintf(options, " %u ", (uint)ts.sidetone_freq);
		opt_pos = MENU_SIDETONE_FREQUENCY % MENUSIZE;	// Y position of this menu item
		break;
	//
	case MENU_PADDLE_REVERSE:	// CW Paddle reverse
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.paddle_reverse = 1;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.paddle_reverse= 0;
		}
		//
		if(mode == 3)	{
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.paddle_reverse = 0;
		}
		//
		if(ts.paddle_reverse)
			strcpy(options, "ON ");
		else
			strcpy(options, "OFF");
		//
		opt_pos = MENU_PADDLE_REVERSE % MENUSIZE;	// Y position of this menu item
		break;
		//
	case MENU_CW_TX_RX_DELAY:	// CW TX->RX delay
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.cw_rx_delay++;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			if(ts.cw_rx_delay)
				ts.cw_rx_delay--;
		}
		//
		if(ts.cw_rx_delay > CW_RX_DELAY_MAX)
			ts.cw_rx_delay = CW_RX_DELAY_MAX;
		//
		if(mode == 3)	{
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.cw_rx_delay = CW_RX_DELAY_DEFAULT;
		}
		//
		sprintf(options, " %u ", ts.cw_rx_delay);
		opt_pos = MENU_CW_TX_RX_DELAY % MENUSIZE;	// Y position of this menu item
		break;
	//
	case MENU_CW_OFFSET_MODE:	// CW offset mode (e.g. USB, LSB, etc.)
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.cw_offset_mode++;
			fchange = 1;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
			if(ts.cw_offset_mode)
				ts.cw_offset_mode--;
		}
		//
		if(ts.cw_offset_mode > CW_OFFSET_MAX)
			ts.cw_offset_mode = CW_OFFSET_MAX;
		//
		if(mode == 3)	{
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.cw_offset_mode = CW_OFFSET_MODE_DEFAULT;
		}
		//
		switch(ts.cw_offset_mode)	{
			case CW_OFFSET_USB_TX:
				sprintf(options, "     USB   ");
				break;
			case CW_OFFSET_LSB_TX:
				sprintf(options, "     LSB   ");
				break;
			case CW_OFFSET_USB_RX:
				sprintf(options, "  USB DISP ");
				break;
			case CW_OFFSET_LSB_RX:
				sprintf(options, "  LSB DISP ");
				break;
			case CW_OFFSET_USB_SHIFT:
				sprintf(options, " USB SHIFT ");
				break;
			case CW_OFFSET_LSB_SHIFT:
				sprintf(options, " LSB SHIFT ");
				break;
			case CW_OFFSET_AUTO_TX:
				sprintf(options, "AUT USB/LSB");
				break;
			case CW_OFFSET_AUTO_RX:
				sprintf(options, " AUTO DISP ");
				break;
			case CW_OFFSET_AUTO_SHIFT:
				sprintf(options, " AUTO SHIFT");
				break;
			default:
				sprintf(options, "  ERROR!    ");
				break;
		}
		//
		if(fchange)	{	// update parameters if changed
			UiCWSidebandMode();
			UiDriverShowMode();
			UiDriverUpdateFrequency(1);	// update frequency display without checking encoder
		}
		//
		disp_shift = 1;	// shift left to allow more room
		//
		opt_pos = MENU_CW_OFFSET_MODE % MENUSIZE;	// Y position of this menu item
		break;
	//
	case MENU_TCXO_MODE:	// TCXO On/Off
		temp_sel = (df.temp_enabled & 0x0f);		// get current setting without upper nybble
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			temp_sel++;
			fchange = 1;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			fchange = 1;
			if(temp_sel)
				temp_sel--;
		}
		//
		if(mode == 3)	{
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			temp_sel = TCXO_OFF;
			fchange = 1;
		}
		//
		if(temp_sel > TCXO_TEMP_STATE_MAX)	{
			temp_sel = TCXO_TEMP_STATE_MAX;	// add back in setting
		}
		//
		if(lo.sensor_present)			// no sensor present
			temp_sel = TCXO_OFF;	// force TCXO disabled
		//
		df.temp_enabled = temp_sel | (df.temp_enabled & 0xf0);	// overlay new temperature setting with old status of upper nybble
		//
		if(temp_sel == TCXO_OFF)	{
			strcpy(options, "OFF ");
			if(fchange)
				UiDriverCreateTemperatureDisplay(0,1);
		}
		else if(temp_sel == TCXO_ON)	{
			strcpy(options, "ON  ");
			if(fchange)	{
				ui_si570_init_temp_sensor();
				UiDriverCreateTemperatureDisplay(1,1);
			}
		}
		else if(temp_sel == TCXO_STOP)	{
			strcpy(options, "STOP");
			if(fchange)
				UiDriverCreateTemperatureDisplay(0,1);
		}
		//
		opt_pos = MENU_TCXO_MODE % MENUSIZE;	// Y position of this menu item
		break;
		//
	case MENU_TCXO_C_F:	// TCXO display C/F mode
		if(df.temp_enabled & 0xf0)	// Yes - Is Fahrenheit mode enabled?
			temp_sel = 1;	// yes - set to 1
		else
			temp_sel = 0;	// no - Celsius
		//
		if((df.temp_enabled & 0x0f) != TCXO_STOP)	{	// is temperature display enabled at all?
			if(df.temp_enabled & 0xf0)	// Yes - Is Fahrenheit mode enabled?
				temp_sel = 1;	// yes - set to 1
			else
				temp_sel = 0;	// no - Celsius
			//
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				temp_sel++;
				fchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				fchange = 1;
				if(temp_sel)
					temp_sel--;
			}
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				temp_sel = 0;		// default is 0 (Celsius)
				fchange = 1;
			}
			//
			if(temp_sel > 1)	{
				temp_sel = 1;
			}
			//
			if(temp_sel)					// Fahrenheit mode?
				df.temp_enabled |= 0xf0;	// set upper nybble
			else							// Celsius mode?
				df.temp_enabled &= 0x0f;	// clear upper nybble
		}
		else
			clr = Orange;
		//
		if(fchange)		// update screen if a change was made
			UiDriverCreateTemperatureDisplay(1,1);
		//
		if(temp_sel == 0)	{			// Celsius display
			strcpy(options, " C ");
		}
		else if(temp_sel == 1)	{
			strcpy(options, " F ");
		}
		//
		opt_pos = MENU_TCXO_C_F % MENUSIZE;	// Y position of this menu item
		break;
		//
	case MENU_SPEC_SCOPE_SPEED:	// spectrum scope speed
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.scope_speed++;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			if(ts.scope_speed)
				ts.scope_speed--;
		}
		//
		if(ts.scope_speed > SPECTRUM_SCOPE_SPEED_MAX)
			ts.scope_speed = SPECTRUM_SCOPE_SPEED_MAX;
		//
		if(mode == 3)	{
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.scope_speed = SPECTRUM_SCOPE_SPEED_DEFAULT;
		}
		//
		if(ts.scope_speed)
			sprintf(options, " %u ", ts.scope_speed);
		else
			strcpy(options, "Off");
		//
		opt_pos = MENU_SPEC_SCOPE_SPEED % MENUSIZE;	// Y position of this menu item
		break;
		//
	case MENU_SCOPE_FILTER_STRENGTH:	// spectrum filter strength
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.scope_filter++;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			if(ts.scope_filter > SPECTRUM_SCOPE_FILTER_MIN)
				ts.scope_filter--;
		}
		//
		if(ts.scope_filter < SPECTRUM_SCOPE_FILTER_MIN)
			ts.scope_filter = SPECTRUM_SCOPE_FILTER_MIN;
		//
		if(ts.scope_filter > SPECTRUM_SCOPE_FILTER_MAX)
			ts.scope_filter = SPECTRUM_SCOPE_FILTER_MAX;
		//
		if(mode == 3)	{
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.scope_filter = SPECTRUM_SCOPE_FILTER_DEFAULT;
		}
		//
		sprintf(options, " %u ", ts.scope_filter);
		opt_pos = MENU_SCOPE_FILTER_STRENGTH % MENUSIZE;	// Y position of this menu item
		break;
	//
	case MENU_SCOPE_TRACE_COLOUR:	// spectrum scope trace colour
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.scope_trace_colour++;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			if(ts.scope_trace_colour)
				ts.scope_trace_colour--;
		}
		//
		//
		if(ts.scope_trace_colour > SPEC_MAX_COLOUR-1)	// exclude SPEC_BLACK
			ts.scope_trace_colour = SPEC_MAX_COLOUR-1;	//
		//
		if(mode == 3)	{
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.scope_trace_colour = SPEC_COLOUR_TRACE_DEFAULT;
		}
		//
		if(ts.scope_trace_colour == SPEC_GREY)	{
			clr = Grey;
			strcpy(options, "Gry");
		}
		else if(ts.scope_trace_colour == SPEC_BLUE)	{
			clr = Blue;
			strcpy(options, "Blu");
		}
		else if(ts.scope_trace_colour == SPEC_RED)	{
			clr = Red;
			strcpy(options, "Red");
		}
		else if(ts.scope_trace_colour == SPEC_MAGENTA)	{
			clr = Magenta;
			strcpy(options, "Mag");
		}
		else if(ts.scope_trace_colour == SPEC_GREEN)	{
			clr = Green;
			strcpy(options, "Grn");
		}
		else if(ts.scope_trace_colour == SPEC_CYAN)	{
			clr = Cyan;
			strcpy(options, "Cyn");
		}
		else if(ts.scope_trace_colour == SPEC_YELLOW)	{
			clr = Yellow;
			strcpy(options, "Yel");
		}
		else if(ts.scope_trace_colour == SPEC_ORANGE)	{
			clr = Orange;
			strcpy(options, "Org");
		}
		else	{
			clr = White;
			strcpy(options, "Wht");
		}
		opt_pos = MENU_SCOPE_TRACE_COLOUR % MENUSIZE;	// Y position of this menu item
		break;
		//
	case MENU_SCOPE_GRID_COLOUR:	// spectrum scope grid colour
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.scope_grid_colour++;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			if(ts.scope_grid_colour)
				ts.scope_grid_colour--;
		}
		//
		if(ts.scope_grid_colour > SPEC_MAX_COLOUR)
			ts.scope_grid_colour = SPEC_MAX_COLOUR;
		//
		if(mode == 3)	{
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.scope_grid_colour = SPEC_COLOUR_GRID_DEFAULT;
		}
		//
		if(ts.scope_grid_colour == SPEC_WHITE)	{
			clr = White;
			strcpy(options, "Wht");
		}
		else if(ts.scope_grid_colour == SPEC_BLUE)	{
			clr = Blue;
			strcpy(options, "Blu");
		}
		else if(ts.scope_grid_colour == SPEC_RED)	{
			clr = Red;
			strcpy(options, "Red");
		}
		else if(ts.scope_grid_colour == SPEC_MAGENTA)	{
			clr = Magenta;
			strcpy(options, "Mag");
		}
		else if(ts.scope_grid_colour == SPEC_GREEN)	{
			clr = Green;
			strcpy(options, "Grn");
		}
		else if(ts.scope_grid_colour == SPEC_CYAN)	{
			clr = Cyan;
			strcpy(options, "Cyn");
		}
		else if(ts.scope_grid_colour == SPEC_YELLOW)	{
			clr = Yellow;
			strcpy(options, "Yel");
		}
		else if(ts.scope_grid_colour == SPEC_BLACK)	{
			clr = Grid;
			strcpy(options, "Blk");
		}
		else if(ts.scope_grid_colour == SPEC_ORANGE)	{
			clr = Orange;
			strcpy(options, "Org");
		}
		else	{
			clr = Grey;
			strcpy(options, "Gry");
		}
		opt_pos = MENU_SCOPE_GRID_COLOUR % MENUSIZE;	// Y position of this menu item
		break;
		//
	case MENU_SCOPE_SCALE_COLOUR:	// spectrum scope scale colour
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.scope_scale_colour++;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			if(ts.scope_scale_colour)
				ts.scope_scale_colour--;
		}
		//
		if(ts.scope_scale_colour > SPEC_MAX_COLOUR)
			ts.scope_scale_colour = SPEC_MAX_COLOUR;
		//
		if(mode == 3)	{
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.scope_scale_colour = SPEC_COLOUR_SCALE_DEFAULT;
		}
		//
		if(ts.scope_scale_colour == SPEC_WHITE)	{
			clr = White;
			strcpy(options, "Wht");
		}
		else if(ts.scope_scale_colour == SPEC_BLUE)	{
			clr = Blue;
			strcpy(options, "Blu");
		}
		else if(ts.scope_scale_colour == SPEC_RED)	{
			clr = Red;
			strcpy(options, "Red");
		}
		else if(ts.scope_scale_colour == SPEC_MAGENTA)	{
			clr = Magenta;
			strcpy(options, "Mag");
		}
		else if(ts.scope_scale_colour == SPEC_GREEN)	{
			clr = Green;
			strcpy(options, "Grn");
		}
		else if(ts.scope_scale_colour == SPEC_CYAN)	{
			clr = Cyan;
			strcpy(options, "Cyn");
		}
		else if(ts.scope_scale_colour == SPEC_YELLOW)	{
			clr = Yellow;
			strcpy(options, "Yel");
		}
		else if(ts.scope_scale_colour == SPEC_BLACK)	{
			clr = Grid;
			strcpy(options, "Blk");
		}
		else if(ts.scope_scale_colour == SPEC_ORANGE)	{
			clr = Orange;
			strcpy(options, "Org");
		}
		else	{
			clr = Grey;
			strcpy(options, "Gry");
		}
		opt_pos = MENU_SCOPE_SCALE_COLOUR % MENUSIZE;	// Y position of this menu item
		break;
		//
	case MENU_SCOPE_MAGNIFY:	// Spectrum 2x magnify mode on/off
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			sd.magnify = 1;		// 2x magnify on
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			sd.magnify =  0;	// magnify off
		}
		//
		if(mode == 3)	{
			sd.magnify = 0;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
		}
		//
		if(ts.iq_freq_mode)	{	// is translate mode on?
			sd.magnify = 0;		// yes - force magnify off!
			clr = Red;			// make it RED to indicate that it is locked out!
		}
		//
		if(sd.magnify)	{
			strcpy(options, "ON ");
			clr = Orange;
		}
		else
			strcpy(options, "OFF");

		opt_pos = MENU_SCOPE_MAGNIFY % MENUSIZE;	// Y position of this menu item
		break;
		//
	case MENU_SCOPE_ALC_ADJUST:	// Spectrum scope AGC adjust
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.scope_agc_rate++;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			if(ts.scope_agc_rate)
				ts.scope_agc_rate--;
		}
		//
		if(ts.scope_agc_rate < SPECTRUM_SCOPE_AGC_MIN)
			ts.scope_agc_rate = SPECTRUM_SCOPE_AGC_MIN;
		//
		if(ts.scope_agc_rate > SPECTRUM_SCOPE_AGC_MAX)
			ts.scope_agc_rate = SPECTRUM_SCOPE_AGC_MAX;
		//
		if(mode == 3)	{
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.scope_agc_rate = SPECTRUM_SCOPE_AGC_DEFAULT;
		}
		//
		sd.agc_rate = (float)ts.scope_agc_rate;		// recalculate "live" agc rate
		sd.agc_rate = 1/(sd.agc_rate/5);
		//
		sprintf(options, " %u ", ts.scope_agc_rate);
		opt_pos = MENU_SCOPE_ALC_ADJUST % MENUSIZE;	// Y position of this menu item
		break;
	//
	case MENU_SCOPE_RESCALE_ADJUST:	// Spectrum scope Rescale adjust
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.scope_rescale_rate++;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			if(ts.scope_rescale_rate)
				ts.scope_rescale_rate--;
		}
		//
		if(ts.scope_rescale_rate < SPECTRUM_SCOPE_RESCALE_MIN)
			ts.scope_rescale_rate = SPECTRUM_SCOPE_RESCALE_MIN;
		//
		if(ts.scope_rescale_rate > SPECTRUM_SCOPE_RESCALE_MAX)
			ts.scope_rescale_rate = SPECTRUM_SCOPE_RESCALE_MAX;
		//
		if(mode == 3)	{
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.scope_rescale_rate = SPECTRUM_SCOPE_RESCALE_DEFAULT;
		}
		//
		sd.rescale_rate = (float)ts.scope_rescale_rate;	// recalculate "live" rescale rate
		sd.rescale_rate = 1/sd.rescale_rate;
		//
		sprintf(options, " %u ", ts.scope_rescale_rate);
		opt_pos = MENU_SCOPE_RESCALE_ADJUST % MENUSIZE;	// Y position of this menu item
		break;
	//
//
// ******************  Make sure that this menu item is ALWAYS the last of the main menu items!
//
	case MENU_CONFIG_ENABLE:	// Radio Config Menu Enable - not saved in EEPROM, does not trigger "save" indicator
		if(var >= 1)	{	// setting increase?
			ts.radio_config_menu_enable = 1;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.radio_config_menu_enable = 0;
		}
		//
		if(mode == 3)
			ts.radio_config_menu_enable = 0;
		//
		if(ts.radio_config_menu_enable)	{
			strcpy(options, "ON ");
			clr = Orange;
		}
		else
			strcpy(options, "OFF");

		opt_pos = 5;	// Y position of this menu item
		break;
	default:						// Move to this location if we get to the bottom of the table!
		strcpy(options, "ERROR!");
		opt_pos = 5;
		break;
	}
	//
	if(!disp_shift)
		UiLcdHy28_PrintText(POS_MENU_CHANGE_X, POS_MENU_IND_Y + (opt_pos * 12), options, clr, Black, 0);
	else	// shift left to accommodate large display
		UiLcdHy28_PrintText(POS_MENU_CHANGE_X-24 , POS_MENU_IND_Y + (opt_pos * 12), options, clr, Black, 0);
	//
	if(mode == 1)	{
		if(opt_oldpos != 999)		// was the position of a previous cursor stored?
			UiLcdHy28_PrintText(POS_MENU_CURSOR_X, POS_MENU_IND_Y + (opt_oldpos * 12), " ", Black, Black, 0);	// yes - erase it
		//
		opt_oldpos = opt_pos;	// save position of new "old" cursor position
		UiLcdHy28_PrintText(POS_MENU_CURSOR_X, POS_MENU_IND_Y + (opt_pos * 12), "<", Green, Black, 0);	// place cursor at active position
	}
	//
	return;
}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateConfigMenuLines
//* Object              : Display and and change line items related to the radio hardware configuration
//* Input Parameters    : index:  Line to display  mode:  0=display/update 1=change item 3=set default
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
static void UiDriverUpdateConfigMenuLines(uchar index, uchar mode)
{
	char options[32], temp[32];
	ulong opt_pos;					// y position of option
	static ulong opt_oldpos = 999;	// y position of option
	uchar select;
	ulong	clr;
	ulong	calc_var;
	uchar	temp_var;
	int var;
	bool tchange = 0;		// used to indicate a parameter change
	bool disp_shift = 0;	// used to cause display to be shifted to the left for large amounts of data (e.g. frequency displays)

	clr = White;		// color used it display of adjusted options

	if(mode == 0)	{	// are we in update/display mode?
		select = index;	// use index passed from calling function
		var = 0;		// prevent any change of variable
	}
	else	{			// this is "change" mode
		select = ts.menu_item-MAX_MENU_ITEM;	// item selected from encoder
		var = ts.menu_var;		// change from encoder
		ts.menu_var = 0;		// clear encoder change detect
	}
	strcpy(options, "ERROR");	// pre-load to catch error condition
	//
	switch(select)	{		//
	//
	case CONFIG_FREQ_STEP_MARKER_LINE:	// Frequency step marker line on/off
		temp_var = ts.freq_step_config & 0x0f;
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			temp_var = 1;		// step size marker line on
			tchange = 1;				// indicate change of parameter
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			temp_var =  0;	// step size marker line off
			tchange = 1;				// indicate change of parameter
		}
		//
		if(mode == 3)	{
			temp_var = 0;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			tchange = 1;				// indicate change of parameter
		}
		//
		if(tchange)	{		// something changed?
			if(temp_var)		// yes, is line to be enabled?
				ts.freq_step_config |= 0x0f;	// yes, set lower nybble
			else			// line disabled?
				ts.freq_step_config &= 0xf0;	// no, clear lower nybble
			//
			UiDriverShowStep(df.tuning_step);	// update screen
		}
		//
		if(ts.freq_step_config & 0x0f)	{	// marker line enabled?
			strcpy(options, "  ON ");		// yes!
		}
		else
			strcpy(options, "  OFF");

		opt_pos = CONFIG_FREQ_STEP_MARKER_LINE % MENUSIZE;	// Y position of this menu item
		break;
		//
	case CONFIG_STEP_SIZE_BUTTON_SWAP:	// Step size button swap on/off
		temp_var = ts.freq_step_config & 0xf0;
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			temp_var = 1;		// step button swap on
			tchange = 1;				// indicate change of parameter
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			temp_var =  0;	// step button swap off
			tchange = 1;				// indicate change of parameter
		}
		//
		if(mode == 3)	{
			temp_var = 0;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			tchange = 1;				// indicate change of parameter
		}
		//
		if(tchange)	{
			if(temp_var)	// is button to be swapped?
				ts.freq_step_config |= 0xf0;	// set upper nybble
			else			// line disabled?
				ts.freq_step_config &= 0x0f;	// clear upper nybble
		}
		//
		if(ts.freq_step_config & 0xf0)	{	// STEP button swap enabled?
			strcpy(options, "  ON ");		// yes
		}
		else
			strcpy(options, "  OFF");
		//
		opt_pos = CONFIG_STEP_SIZE_BUTTON_SWAP % MENUSIZE;	// Y position of this menu item
		break;
		//
	case CONFIG_BAND_BUTTON_SWAP:	// Swap position of Band+ and Band- buttons
		temp_var = ts.misc_flags1 & 2;
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			temp_var = 1;				// band up/down swap to be enabled
			tchange = 1;				// indicate change of parameter
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			temp_var =  0;				// band up/down swap is to be disabled
			tchange = 1;				// indicate change of parameter
		}
		//
		if(mode == 3)	{
			temp_var = 0;				// turn it off by default
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			tchange = 1;				// indicate change of parameter
		}
		//
		if(tchange)	{
			if(temp_var)	// band up/down swap is to be enabled
				ts.misc_flags1 |= 2;		// set LSB
			else			// band up/down swap is to be disabled
				ts.misc_flags1 &= 0xfd;		// clear LSB
		}
		//
		if(ts.misc_flags1 & 2)				// band up/down swap enabled?
			strcpy(options, "  ON ");		// yes
		else
			strcpy(options, "  OFF");		// no (obviously!)
		//
		opt_pos = CONFIG_BAND_BUTTON_SWAP % MENUSIZE;	// Y position of this menu item
		break;
		//
	case CONFIG_TX_DISABLE:	// Step size button swap on/off
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.tx_disable = 1;
			tchange = 1;				// indicate change of parameter
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.tx_disable = 0;
			tchange = 1;				// indicate change of parameter
		}
		//
		if(mode == 3)	{
			ts.tx_disable = 0;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
		}
		//
		if(ts.tx_disable)	{			// Transmit disabled?
			strcpy(options, "  ON ");		// yes
			UiLcdHy28_PrintText(POS_BOTTOM_BAR_F5_X,POS_BOTTOM_BAR_F5_Y,"  TUNE",Grey1,Black,0);	// Make TUNE button Grey
		}
		else	{
			strcpy(options, "  OFF");
			UiLcdHy28_PrintText(POS_BOTTOM_BAR_F5_X,POS_BOTTOM_BAR_F5_Y,"  TUNE",White,Black,0);	// Make TUNE button White
		}
		//
		opt_pos = CONFIG_TX_DISABLE % MENUSIZE;	// Y position of this menu item
		break;
		//
	case CONFIG_AUDIO_MAIN_SCREEN_MENU_SWITCH:	// AFG/(STG/CMP) and RIT/(WPM/MIC/LIN) are to change automatically with TX/RX
		temp_var = ts.misc_flags1 & 1;
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			temp_var = 1;				// change-on-tx is to be disabled
			tchange = 1;				// indicate change of parameter
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			temp_var =  0;				// change-on-tx is to be enabled
			tchange = 1;				// indicate change of parameter
		}
		//
		if(mode == 3)	{
			temp_var = 0;				// turn it on by default
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			tchange = 1;				// indicate change of parameter
		}
		//
		if(tchange)	{
			if(temp_var)	// change-on-tx is to be disabled
				ts.misc_flags1 |= 1;		// set LSB
			else			// change-on-tx is to be enabled
				ts.misc_flags1 &= 0xfe;		// clear LSB
		}
		//
		if(ts.misc_flags1 & 1)				// change-on-TX status
			strcpy(options, "  OFF");
		else
			strcpy(options, "  ON ");
		//
		opt_pos = CONFIG_AUDIO_MAIN_SCREEN_MENU_SWITCH % MENUSIZE;	// Y position of this menu item
		break;
		//
	case CONFIG_MUTE_LINE_OUT_TX:	// Enable/disable MUTE of TX audio on LINE OUT
		temp_var = ts.misc_flags1 & 4;
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			temp_var = 1;				// MUTE of TX audio on LINE OUT is enabled
			tchange = 1;				// indicate change of parameter
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			temp_var =  0;				// MUTE of TX audio on LINE OUT is disabled
			tchange = 1;				// indicate change of parameter
		}
		//
		if(mode == 3)	{
			temp_var = 0;				// turn it off by default
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			tchange = 1;				// indicate change of parameter
		}
		//
		if((tchange) && (!ts.iq_freq_mode))		{	// did the status change and is translate mode NOT active?
			if(temp_var)	// Yes - MUTE of TX audio on LINE OUT is enabled
				ts.misc_flags1 |= 4;		// set LSB
			else			// MUTE of TX audio on LINE OUT is disabled
				ts.misc_flags1 &= 0xfb;		// clear LSB
		}
		//
		if(ts.misc_flags1 & 4)				// MUTE of TX audio on LINE OUT disabled
			strcpy(options, "  ON ");
		else
			strcpy(options, "  OFF");
		//
		if(ts.iq_freq_mode)	// Mark RED if translate mode is active
			clr = Red;
		//
		opt_pos = CONFIG_MUTE_LINE_OUT_TX % MENUSIZE;	// Y position of this menu item
		break;
		//
	case CONFIG_MAX_VOLUME:	// maximum audio volume
		if(var >= 1)	{	// did the selection increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.audio_max_volume++;	// yes - increase
		}
		else if(var <= -1)	{		// did the setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			if(ts.audio_max_volume)		// yes, reduce the setting if not at minimum
				ts.audio_max_volume--;
		}
		//
		if(mode == 3)	{		// load default setting
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.audio_max_volume = MAX_VOLUME_DEFAULT;
		}
		//
		if(ts.audio_max_volume > MAX_VOLUME_MAX)		// limit selection ranges for this mode
			ts.audio_max_volume = MAX_VOLUME_MAX;
		//
		if(ts.audio_max_volume < MAX_VOLUME_MIN)		// limit selection ranges for this mode
			ts.audio_max_volume = MAX_VOLUME_MIN;
		//
		if(ts.audio_gain > ts.audio_max_volume)	{			// is the volume currently higher than the new setting?
			ts.audio_gain = ts.audio_max_volume;		// yes - force the volume to the new value
			//
			sprintf(temp,"%02d",ts.audio_gain);			// Update screen indicator
			UiLcdHy28_PrintText((POS_AG_IND_X + 38),(POS_AG_IND_Y + 1), temp,White,Black,0);
		}
		sprintf(options, "    %u ", ts.audio_max_volume);
		//
		if(ts.audio_max_volume <= MAX_VOL_RED_THRESH)			// Indicate that gain has been reduced by changing color
			clr = Red;
		else if(ts.audio_max_volume <= MAX_VOLT_YELLOW_THRESH)
			clr = Orange;
		//
		opt_pos = CONFIG_MAX_VOLUME % MENUSIZE;	// Y position of this menu item
		break;
	//
	case CONFIG_MAX_RX_GAIN:	// maximum RX gain setting
		if(var >= 1)	{	// did the selection increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.max_rf_gain++;	// yes - increase
			tchange = 1;
		}
		else if(var <= -1)	{		// did the setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			tchange = 1;
			if(ts.max_rf_gain)		// yes, reduce the setting if not at minimum
				ts.max_rf_gain--;
		}
		//
		if(mode == 3)	{		// load default setting
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.max_rf_gain = MAX_RF_GAIN_DEFAULT;
			tchange = 1;
		}
		//
		if(tchange)	{
			ads.agc_knee = AGC_KNEE_REF * (float)(ts.max_rf_gain + 1);
			ads.agc_val_max = AGC_VAL_MAX_REF / ((float)(ts.max_rf_gain + 1));
			ads.post_agc_gain = POST_AGC_GAIN_SCALING_REF / (float)(ts.max_rf_gain + 1);
		}
		//
		if(ts.max_rf_gain > MAX_RF_GAIN_MAX)		// limit selection ranges for this mode
			ts.max_rf_gain = MAX_RF_GAIN_MAX;
		//
		sprintf(options, "    %u ", ts.max_rf_gain);
		//
		opt_pos = CONFIG_MAX_RX_GAIN % MENUSIZE;	// Y position of this menu item
		break;
	//
	//
	// *****************  WARNING *********************
	// If you change CAT mode, THINGS MAY GET "BROKEN" - for example, you may not be able to reliably save to EEPROM!
	// This needs to be investigated!
	//
	case CONFIG_CAT_ENABLE:	// CAT mode	 - not saved in EEPROM, does not trigger "save" indicator
		if(var >= 1)	{	// setting increase?
			ts.cat_mode_active = 1;
			tchange = 1;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.cat_mode_active = 0;
			tchange = 1;
		}
		//
		if(mode == 3)
			ts.cat_mode_active = 0;
		//
		if(ts.cat_mode_active)	{
			strcpy(options, "  ON ");
			if(tchange)
				cat_driver_init();
		}
		else	{
			strcpy(options, "  OFF");
			if(tchange)
				cat_driver_stop();
		}
		opt_pos = CONFIG_CAT_ENABLE % MENUSIZE;	// Y position of this menu item
		break;
		//
	case CONFIG_FREQUENCY_CALIBRATE:		// Frequency Calibration
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.freq_cal += df.tuning_step;
			tchange = 1;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.freq_cal -= df.tuning_step;
			tchange = 1;
		}
		if(ts.freq_cal < MIN_FREQ_CAL)
			ts.freq_cal = MIN_FREQ_CAL;
		else if(ts.freq_cal > MAX_FREQ_CAL)
			ts.freq_cal = MAX_FREQ_CAL;
		//
		if(mode == 3)	{
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.freq_cal = 0;
			tchange = 1;
		}
		//
		if(tchange)
			UiDriverUpdateFrequency(1);	// Update LO frequency without checking encoder
		//
		disp_shift = 1;
		sprintf(options, "   %d    ", ts.freq_cal);
		opt_pos = CONFIG_FREQUENCY_CALIBRATE % MENUSIZE;
		break;
		//
	case CONFIG_LSB_RX_IQ_GAIN_BAL:		// LSB RX IQ Gain balance
		if((ts.dmod_mode == DEMOD_LSB) && (ts.txrx_mode == TRX_MODE_RX)) 	{	// only allow adjustment if in LSB mode
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.rx_iq_lsb_gain_balance++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.rx_iq_lsb_gain_balance--;
				tchange = 1;
			}
			if(ts.rx_iq_lsb_gain_balance < MIN_RX_IQ_GAIN_BALANCE)
				ts.rx_iq_lsb_gain_balance  = MIN_RX_IQ_GAIN_BALANCE;
			else if(ts.rx_iq_lsb_gain_balance > MAX_RX_IQ_GAIN_BALANCE)
				ts.rx_iq_lsb_gain_balance  = MAX_RX_IQ_GAIN_BALANCE;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.rx_iq_lsb_gain_balance = 0;
				tchange = 1;
			}
			//
			if(tchange)
				UiCalcRxIqGainAdj();
		}
		else		// Orange if not in RX and/or correct mode
			clr = Orange;
		//
		sprintf(options, "   %d  ", ts.rx_iq_lsb_gain_balance);
		opt_pos = CONFIG_LSB_RX_IQ_GAIN_BAL % MENUSIZE;
		break;
		//
	case CONFIG_LSB_RX_IQ_PHASE_BAL:		// LSB RX IQ Phase balance
		if((ts.dmod_mode == DEMOD_LSB) && (ts.txrx_mode == TRX_MODE_RX))	{
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.rx_iq_lsb_phase_balance++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.rx_iq_lsb_phase_balance--;
				tchange = 1;
			}
			if(ts.rx_iq_lsb_phase_balance < MIN_RX_IQ_PHASE_BALANCE)
				ts.rx_iq_lsb_phase_balance  = MIN_RX_IQ_PHASE_BALANCE;
			//
			if(ts.rx_iq_lsb_phase_balance > MAX_RX_IQ_PHASE_BALANCE)
				ts.rx_iq_lsb_phase_balance  = MAX_RX_IQ_PHASE_BALANCE;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.rx_iq_lsb_phase_balance = 0;
				tchange = 1;
			}
			//
			if(tchange)
				UiCalcRxPhaseAdj();
		}
		else		// Orange if not in RX and/or correct mode
			clr = Orange;
		//
		sprintf(options, "   %d  ", ts.rx_iq_lsb_phase_balance);
		opt_pos = CONFIG_LSB_RX_IQ_PHASE_BAL % MENUSIZE;
		break;
		//
	case CONFIG_USB_RX_IQ_GAIN_BAL:		// USB/CW RX IQ Gain balance
		if(((ts.dmod_mode == DEMOD_USB) || (ts.dmod_mode == DEMOD_CW))  && (ts.txrx_mode == TRX_MODE_RX))	{
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.rx_iq_usb_gain_balance++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.rx_iq_usb_gain_balance--;
				tchange = 1;
			}
			if(ts.rx_iq_usb_gain_balance < MIN_RX_IQ_GAIN_BALANCE)
				ts.rx_iq_usb_gain_balance  = MIN_RX_IQ_GAIN_BALANCE;
			else if(ts.rx_iq_usb_gain_balance > MAX_RX_IQ_GAIN_BALANCE)
				ts.rx_iq_usb_gain_balance  = MAX_RX_IQ_GAIN_BALANCE;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.rx_iq_usb_gain_balance = 0;
				tchange = 1;
			}
			//
			if(tchange)
				UiCalcRxIqGainAdj();
		}
		else		// Orange if not in RX and/or correct mode
			clr = Orange;
		//
		sprintf(options, "   %d  ", ts.rx_iq_usb_gain_balance);
		opt_pos = CONFIG_USB_RX_IQ_GAIN_BAL % MENUSIZE;
		break;
		//
	case CONFIG_USB_RX_IQ_PHASE_BAL:		// USB RX IQ Phase balance
		if((ts.dmod_mode == DEMOD_USB)  && (ts.txrx_mode == TRX_MODE_RX))	{
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.rx_iq_usb_phase_balance++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.rx_iq_usb_phase_balance--;
				tchange = 1;
			}
			if(ts.rx_iq_usb_phase_balance < MIN_RX_IQ_PHASE_BALANCE)
				ts.rx_iq_usb_phase_balance  = MIN_RX_IQ_PHASE_BALANCE;
			//
			if(ts.rx_iq_usb_phase_balance > MAX_RX_IQ_PHASE_BALANCE)
				ts.rx_iq_usb_phase_balance  = MAX_RX_IQ_PHASE_BALANCE;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.rx_iq_usb_phase_balance = 0;
				tchange = 1;
			}
			//
			if(tchange)
				UiCalcRxPhaseAdj();
		}
		else		// Orange if not in RX and/or correct mode
			clr = Orange;
		//
		sprintf(options, "   %d  ", ts.rx_iq_usb_phase_balance);
		opt_pos = CONFIG_USB_RX_IQ_PHASE_BAL % MENUSIZE;
		break;
		//
	case 	CONFIG_AM_RX_GAIN_BAL:		// AM RX IQ Phase balance
		if((ts.dmod_mode == DEMOD_AM)  && (ts.txrx_mode == TRX_MODE_RX))	{
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.rx_iq_am_gain_balance++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.rx_iq_am_gain_balance--;
				tchange = 1;
			}
			if(ts.rx_iq_am_gain_balance < MIN_RX_IQ_GAIN_BALANCE)
				ts.rx_iq_am_gain_balance  = MIN_RX_IQ_GAIN_BALANCE;
			//
			if(ts.rx_iq_am_gain_balance > MAX_RX_IQ_GAIN_BALANCE)
				ts.rx_iq_am_gain_balance  = MAX_RX_IQ_GAIN_BALANCE;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.rx_iq_am_gain_balance = 0;
				tchange = 1;
			}
			//
			if(tchange)
				UiCalcRxIqGainAdj();
		}
		else		// Orange if not in RX and/or correct mode
			clr = Orange;
		//
		sprintf(options, "   %d  ", ts.rx_iq_am_gain_balance);
		opt_pos = 	CONFIG_AM_RX_GAIN_BAL % MENUSIZE;
		break;
		//
	case CONFIG_LSB_TX_IQ_GAIN_BAL:		// LSB TX IQ Gain balance
		if((ts.dmod_mode == DEMOD_LSB) && (ts.txrx_mode == TRX_MODE_TX))	{
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.tx_iq_lsb_gain_balance++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.tx_iq_lsb_gain_balance--;
				tchange = 1;
			}
			if(ts.tx_iq_lsb_gain_balance < MIN_TX_IQ_GAIN_BALANCE)
				ts.tx_iq_lsb_gain_balance  = MIN_TX_IQ_GAIN_BALANCE;
			//
			if(ts.tx_iq_lsb_gain_balance > MAX_TX_IQ_GAIN_BALANCE)
				ts.tx_iq_lsb_gain_balance  = MAX_TX_IQ_GAIN_BALANCE;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.tx_iq_lsb_gain_balance = 0;
				tchange = 1;
			}
			//
			if(tchange)
				UiCalcTxIqGainAdj();
		}
		else		// Orange if not in TX and/or correct mode
			clr = Orange;
		//
		sprintf(options, "   %d  ", ts.tx_iq_lsb_gain_balance);
		opt_pos = CONFIG_LSB_TX_IQ_GAIN_BAL % MENUSIZE;
		break;
		//
	case CONFIG_LSB_TX_IQ_PHASE_BAL:		// LSB TX IQ Phase balance
		if((ts.dmod_mode == DEMOD_LSB) && (ts.txrx_mode == TRX_MODE_TX))	{
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.tx_iq_lsb_phase_balance++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.tx_iq_lsb_phase_balance--;
				tchange = 1;
			}
			if(ts.tx_iq_lsb_phase_balance < MIN_TX_IQ_PHASE_BALANCE)
				ts.tx_iq_lsb_phase_balance  = MIN_TX_IQ_PHASE_BALANCE;
			//
			if(ts.tx_iq_lsb_phase_balance > MAX_TX_IQ_PHASE_BALANCE)
				ts.tx_iq_lsb_phase_balance  = MAX_TX_IQ_PHASE_BALANCE;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.tx_iq_lsb_phase_balance = 0;
				tchange = 1;
			}
			//
			if(tchange)
				UiCalcTxPhaseAdj();
		}
		else		// Orange if not in TX and/or correct mode
			clr = Orange;
		//
		sprintf(options, "   %d  ", ts.tx_iq_lsb_phase_balance);
		opt_pos = CONFIG_LSB_TX_IQ_PHASE_BAL % MENUSIZE;
		break;
		//
	case CONFIG_USB_TX_IQ_GAIN_BAL:		// USB/CW TX IQ Gain balance
		if(((ts.dmod_mode == DEMOD_USB) || (ts.dmod_mode == DEMOD_CW)) && (ts.txrx_mode == TRX_MODE_TX))	{
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.tx_iq_usb_gain_balance++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.tx_iq_usb_gain_balance--;
				tchange = 1;
			}
			if(ts.tx_iq_usb_gain_balance < MIN_TX_IQ_GAIN_BALANCE)
				ts.tx_iq_usb_gain_balance  = MIN_TX_IQ_GAIN_BALANCE;
			//
			if(ts.tx_iq_usb_gain_balance > MAX_TX_IQ_GAIN_BALANCE)
				ts.tx_iq_usb_gain_balance  = MAX_TX_IQ_GAIN_BALANCE;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.tx_iq_usb_gain_balance = 0;
				tchange = 1;
			}
			//
			if(tchange)
				UiCalcTxIqGainAdj();
		}
		else		// Orange if not in TX and/or correct mode
			clr = Orange;
		//
		sprintf(options, "   %d  ", ts.tx_iq_usb_gain_balance);
		opt_pos = CONFIG_USB_TX_IQ_GAIN_BAL % MENUSIZE;
		break;
		//
	case CONFIG_USB_TX_IQ_PHASE_BAL:		// USB TX IQ Phase balance
		if((ts.dmod_mode == DEMOD_USB) && (ts.txrx_mode == TRX_MODE_TX))	{
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.tx_iq_usb_phase_balance++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.tx_iq_usb_phase_balance--;
				tchange = 1;
			}
			if(ts.tx_iq_usb_phase_balance < MIN_TX_IQ_PHASE_BALANCE)
				ts.tx_iq_usb_phase_balance  = MIN_TX_IQ_PHASE_BALANCE;
			//
			if(ts.tx_iq_usb_phase_balance > MAX_TX_IQ_PHASE_BALANCE)
				ts.tx_iq_usb_phase_balance  = MAX_TX_IQ_PHASE_BALANCE;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.tx_iq_usb_phase_balance = 0;
				tchange = 1;
			}
			//
			if(tchange)
				UiCalcTxPhaseAdj();
		}
		else		// Orange if not in TX and/or correct mode
			clr = Orange;
		//
		sprintf(options, "   %d  ", ts.tx_iq_usb_phase_balance);
		opt_pos = CONFIG_USB_TX_IQ_PHASE_BAL % MENUSIZE;
		break;
		//
	case CONFIG_CW_PA_BIAS:		// CW PA Bias adjust
		if((ts.tune) || (ts.txrx_mode == TRX_MODE_TX))	{	// enable only in TUNE mode
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pa_cw_bias++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				if(ts.pa_cw_bias)
					ts.pa_cw_bias--;
				tchange = 1;
			}
			//
			if(ts.pa_bias > MAX_PA_BIAS)
				ts.pa_cw_bias  = MAX_PA_BIAS;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pa_cw_bias = 0;
				tchange = 1;
			}
			//
			if(tchange)	{
				if((ts.dmod_mode == DEMOD_CW) && (ts.pa_cw_bias))	{	// in CW mode and bias NONZERO?
					calc_var = BIAS_OFFSET + (ts.pa_cw_bias * 2);
					if(calc_var > 255)
						calc_var = 255;
					//
							DAC_SetChannel2Data(DAC_Align_8b_R,calc_var);	// Set DAC Channel 1 DHR12L register
				}
				else	{
					calc_var = BIAS_OFFSET + (ts.pa_bias * 2);	// if it is zero, use the "other" value
					if(calc_var > 255)
						calc_var = 255;
					//
							DAC_SetChannel2Data(DAC_Align_8b_R,calc_var);	// Set DAC Channel 1 DHR12L register
				}
			}
		}
		else		// Orange if not in TUNE or TX mode
			clr = Orange;
		//
		sprintf(options, "   %u ", ts.pa_cw_bias);
		opt_pos = CONFIG_CW_PA_BIAS % MENUSIZE;
		break;
		//
	case CONFIG_PA_BIAS:		// PA Bias adjust (Including CW if CW bias == 0)
		if((ts.tune) || (ts.txrx_mode == TRX_MODE_TX))	{	// enable only in TUNE mode
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pa_bias++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				if(ts.pa_bias)
					ts.pa_bias--;
				tchange = 1;
			}
			//
			if(ts.pa_bias > MAX_PA_BIAS)
				ts.pa_bias  = MAX_PA_BIAS;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pa_bias = 0;
				tchange = 1;
			}
			//
			if(tchange)	{
				if((ts.dmod_mode != DEMOD_CW) || ((ts.dmod_mode == DEMOD_CW) && !ts.pa_cw_bias))	{	// is it NOT in CW mode, or is it in CW mode and the CW bias set to zero?
					calc_var = BIAS_OFFSET + (ts.pa_bias * 2);
					if(calc_var > 255)
						calc_var = 255;
					//
					// Set DAC Channel 1 DHR12L register
					DAC_SetChannel2Data(DAC_Align_8b_R,calc_var);
				}
			}
		}
		else		// Orange if not in TUNE or TX mode
			clr = Orange;
		//
		sprintf(options, "   %u ", ts.pa_bias);
		opt_pos = CONFIG_PA_BIAS % MENUSIZE;
		break;
		//
	case CONFIG_RF_FWD_PWR_CALIBRATE:		// RF power FWD power meter calibrate
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			swrm.fwd_cal++;
			tchange = 1;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			swrm.fwd_cal--;
			tchange = 1;
		}
		//
		if(swrm.fwd_cal > SWR_CAL_MAX)
			swrm.fwd_cal  = SWR_CAL_MAX;
		else if(swrm.fwd_cal < SWR_CAL_MIN)
			swrm.fwd_cal  = SWR_CAL_MIN;
		//
		if(mode == 3)	{
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			swrm.fwd_cal = SWR_CAL_DEFAULT;
			tchange = 1;
		}
		//
		if(ts.txrx_mode != TRX_MODE_TX)	// Orange if not in TX mode
			clr = Orange;
		//
		sprintf(options, "  %u ", swrm.fwd_cal);
		opt_pos = CONFIG_RF_FWD_PWR_CALIBRATE % MENUSIZE;
		break;
		//
	case CONFIG_XVTR_OFFSET_MULT:	// Transverter Frequency Display Offset/Multiplier Mode On/Off
		if(var >= 1)	{	// setting increase?
			tchange = 1;
			ts.xverter_mode++;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
		}
		else if(var <= -1)	{	// setting decrease?
			tchange = 1;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			if(ts.xverter_mode)
				ts.xverter_mode--;
		}
		//
		if(ts.xverter_mode > XVERTER_MULT_MAX)
			ts.xverter_mode = XVERTER_MULT_MAX;
		//
		if(mode == 3)	{
			tchange = 1;
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.xverter_mode = 0;
		}
		//
		if(tchange)	{		// change?
			ts.refresh_freq_disp = 1;	// cause frequency display to be completely refreshed
			UiDriverUpdateFrequency(1);	// Yes - update frequency
			ts.refresh_freq_disp = 0;
		}
		//
		if(ts.xverter_mode)	{
			sprintf(options, " ON x%u ", ts.xverter_mode);	// Display on/multiplication factor
			clr = Red;
		}
		else
			strcpy(options, "  OFF   ");
		//
		opt_pos = CONFIG_XVTR_OFFSET_MULT % MENUSIZE;	// Y position of this menu item
		break;
	case CONFIG_XVTR_FREQUENCY_OFFSET:		// Adjust transverter Frequency offset
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.xverter_offset += df.tuning_step;
			tchange = 1;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			if(ts.xverter_offset >= df.tuning_step)	// subtract only if we have room to do so
				ts.xverter_offset -= df.tuning_step;
			else
				ts.xverter_offset = 0;				// else set to zero
			//
			tchange = 1;
		}
		//
		if(ts.xverter_offset > XVERTER_OFFSET_MAX)
			ts.xverter_offset  = XVERTER_OFFSET_MAX;
		//
		if(mode == 3)	{
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.xverter_offset = 0;		// default for this option is to zero it out
			tchange = 1;
		}
		//
		if(tchange)	{		// change?
			ts.refresh_freq_disp = 1;	// cause frequency display to be completely refreshed
			UiDriverUpdateFrequency(1);	// Yes - update frequency
			ts.refresh_freq_disp = 0;
			tchange = 1;
		}
		//
		if(ts.xverter_mode)	// transvert mode active?
			clr = Red;		// make number red to alert user of this!
		//
		disp_shift = 1;		// cause display to be shifted to the left so that it will fit
		sprintf(options, " %09u", (uint)ts.xverter_offset);	// print with nine digits
		//
		opt_pos = CONFIG_XVTR_FREQUENCY_OFFSET % MENUSIZE;
		break;
		//
	case CONFIG_80M_5W_ADJUST:		// 80m 5 watt adjust
		if((ts.band == BAND_MODE_80) && (ts.power_level == PA_LEVEL_5W))	{
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_80m_5w_adj++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_80m_5w_adj--;
				tchange = 1;
			}
			//
			if(ts.pwr_80m_5w_adj > TX_POWER_FACTOR_MAX)
				ts.pwr_80m_5w_adj  = TX_POWER_FACTOR_MAX;
			else if(ts.pwr_80m_5w_adj < TX_POWER_FACTOR_MIN)
				ts.pwr_80m_5w_adj = TX_POWER_FACTOR_MIN;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_80m_5w_adj = TX_POWER_FACTOR_80_DEFAULT;
				tchange = 1;
			}
			//
			if(tchange)	{		// did something change?
				UiDriverSetBandPowerFactor(ts.band);	// yes, update the power factor
				Codec_SidetoneSetgain();				// adjust the sidetone gain
			}
		}
		else	// not enabled
			clr = Orange;
		//
		sprintf(options, "  %u ", ts.pwr_80m_5w_adj);
		opt_pos = CONFIG_80M_5W_ADJUST % MENUSIZE;
		break;
		//
	case CONFIG_60M_5W_ADJUST:		// 60m 5 watt adjust
		if((ts.band == BAND_MODE_60) && (ts.power_level == PA_LEVEL_5W))	{
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_60m_5w_adj++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_60m_5w_adj--;
				tchange = 1;
			}
			//
			if(ts.pwr_60m_5w_adj > TX_POWER_FACTOR_MAX)
				ts.pwr_60m_5w_adj = TX_POWER_FACTOR_MAX;
			else if(ts.pwr_60m_5w_adj < TX_POWER_FACTOR_MIN)
				ts.pwr_60m_5w_adj = TX_POWER_FACTOR_MIN;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_60m_5w_adj = TX_POWER_FACTOR_60_DEFAULT;
				tchange = 1;
			}
			//
			if(tchange)		// did something change?
				UiDriverSetBandPowerFactor(ts.band);	// yes, update the power factor
		}
		else	// not enabled
			clr = Orange;
		//
		sprintf(options, "  %u ", ts.pwr_60m_5w_adj);
		opt_pos = CONFIG_60M_5W_ADJUST % MENUSIZE;
		break;
		//
	case CONFIG_40M_5W_ADJUST:		// 40m 5 watt adjust
		if((ts.band == BAND_MODE_40) && (ts.power_level == PA_LEVEL_5W))	{
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_40m_5w_adj++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_40m_5w_adj--;
				tchange = 1;
			}
			//
			if(ts.pwr_40m_5w_adj > TX_POWER_FACTOR_MAX)
				ts.pwr_40m_5w_adj = TX_POWER_FACTOR_MAX;
			else if(ts.pwr_40m_5w_adj < TX_POWER_FACTOR_MIN)
				ts.pwr_40m_5w_adj = TX_POWER_FACTOR_MIN;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_40m_5w_adj = TX_POWER_FACTOR_40_DEFAULT;
				tchange = 1;
			}
			if(tchange)	{	// did something change?
				UiDriverSetBandPowerFactor(ts.band);	// yes, update the power factor
				Codec_SidetoneSetgain();				// adjust the sidetone gain
			}
		}
		else	// not enabled
			clr = Orange;
		//
		sprintf(options, "  %u ", ts.pwr_40m_5w_adj);
		opt_pos = CONFIG_40M_5W_ADJUST % MENUSIZE;
		break;
		//
	case CONFIG_30M_5W_ADJUST:		// 30m 5 watt adjust
		if((ts.band == BAND_MODE_30) && (ts.power_level == PA_LEVEL_5W))	{
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_30m_5w_adj++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_30m_5w_adj--;
				tchange = 1;
			}
			//
			if(ts.pwr_30m_5w_adj > TX_POWER_FACTOR_MAX)
				ts.pwr_30m_5w_adj = TX_POWER_FACTOR_MAX;
			else if(ts.pwr_30m_5w_adj < TX_POWER_FACTOR_MIN)
				ts.pwr_30m_5w_adj = TX_POWER_FACTOR_MIN;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_30m_5w_adj = TX_POWER_FACTOR_30_DEFAULT;
				tchange = 1;
			}
			if(tchange)	{	// did something change?
				UiDriverSetBandPowerFactor(ts.band);	// yes, update the power factor
				Codec_SidetoneSetgain();				// adjust the sidetone gain
			}
		}
		else	// not enabled
			clr = Orange;
		//
		sprintf(options, "  %u ", ts.pwr_30m_5w_adj);
		opt_pos = CONFIG_30M_5W_ADJUST % MENUSIZE;
		break;
		//
	case CONFIG_20M_5W_ADJUST:		// 20m 5 watt adjust
		if((ts.band == BAND_MODE_20) && (ts.power_level == PA_LEVEL_5W))	{
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_20m_5w_adj++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_20m_5w_adj--;
				tchange = 1;
			}
			//
			if(ts.pwr_20m_5w_adj > TX_POWER_FACTOR_MAX)
				ts.pwr_20m_5w_adj = TX_POWER_FACTOR_MAX;
			else if(ts.pwr_20m_5w_adj < TX_POWER_FACTOR_MIN)
				ts.pwr_20m_5w_adj = TX_POWER_FACTOR_MIN;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_20m_5w_adj = TX_POWER_FACTOR_20_DEFAULT;
				tchange = 1;
			}
			if(tchange)	{	// did something change?
				UiDriverSetBandPowerFactor(ts.band);	// yes, update the power factor
				Codec_SidetoneSetgain();				// adjust the sidetone gain
			}
		}
		else	// not enabled
			clr = Orange;
		//
		sprintf(options, "  %u ", ts.pwr_20m_5w_adj);
		opt_pos = CONFIG_20M_5W_ADJUST % MENUSIZE;
		break;
		//
	case CONFIG_17M_5W_ADJUST:		// 17m 5 watt adjust
		if((ts.band == BAND_MODE_17) && (ts.power_level == PA_LEVEL_5W))	{
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_17m_5w_adj++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_17m_5w_adj--;
				tchange = 1;
			}
			//
			if(ts.pwr_17m_5w_adj > TX_POWER_FACTOR_MAX)
				ts.pwr_17m_5w_adj = TX_POWER_FACTOR_MAX;
			else if(ts.pwr_17m_5w_adj < TX_POWER_FACTOR_MIN)
				ts.pwr_17m_5w_adj = TX_POWER_FACTOR_MIN;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_17m_5w_adj = TX_POWER_FACTOR_17_DEFAULT;
				tchange = 1;
			}
			if(tchange)	{	// did something change?
				UiDriverSetBandPowerFactor(ts.band);	// yes, update the power factor
				Codec_SidetoneSetgain();				// adjust the sidetone gain
			}
		}
		else	// not enabled
			clr = Orange;
		//
		sprintf(options, "  %u ", ts.pwr_17m_5w_adj);
		opt_pos = CONFIG_17M_5W_ADJUST % MENUSIZE;
		break;
		//
	case CONFIG_15M_5W_ADJUST:		// 15m 5 watt adjust
		if((ts.band == BAND_MODE_15) && (ts.power_level == PA_LEVEL_5W))	{
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_15m_5w_adj++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_15m_5w_adj--;
				tchange = 1;
			}
			//
			if(ts.pwr_15m_5w_adj > TX_POWER_FACTOR_MAX)
				ts.pwr_15m_5w_adj  = TX_POWER_FACTOR_MAX;
			else if(ts.pwr_15m_5w_adj < TX_POWER_FACTOR_MIN)
				ts.pwr_15m_5w_adj = TX_POWER_FACTOR_MIN;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_15m_5w_adj = TX_POWER_FACTOR_15_DEFAULT;
				tchange = 1;
			}
			if(tchange)	{	// did something change?
				UiDriverSetBandPowerFactor(ts.band);	// yes, update the power factor
				Codec_SidetoneSetgain();				// adjust the sidetone gain
			}
		}
		else	// not enabled
			clr = Orange;
		//
		sprintf(options, "  %u ", ts.pwr_15m_5w_adj);
		opt_pos = CONFIG_15M_5W_ADJUST % MENUSIZE;
		break;
		//
	case CONFIG_12M_5W_ADJUST:		// 12m 5 watt adjust
		if((ts.band == BAND_MODE_12) && (ts.power_level == PA_LEVEL_5W))	{
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_12m_5w_adj++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_12m_5w_adj--;
				tchange = 1;
			}
			//
			if(ts.pwr_12m_5w_adj > TX_POWER_FACTOR_MAX)
				ts.pwr_12m_5w_adj = TX_POWER_FACTOR_MAX;
			else if(ts.pwr_12m_5w_adj < TX_POWER_FACTOR_MIN)
				ts.pwr_12m_5w_adj = TX_POWER_FACTOR_MIN;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_12m_5w_adj = TX_POWER_FACTOR_12_DEFAULT;
				tchange = 1;
			}
			if(tchange)	{	// did something change?
				UiDriverSetBandPowerFactor(ts.band);	// yes, update the power factor
				Codec_SidetoneSetgain();				// adjust the sidetone gain
			}
		}
		else	// not enabled
			clr = Orange;
		//
		sprintf(options, "  %u ", ts.pwr_12m_5w_adj);
		opt_pos = CONFIG_12M_5W_ADJUST % MENUSIZE;
		break;
		//
	case CONFIG_10M_5W_ADJUST:		// 10m 5 watt adjust
		if((ts.band == BAND_MODE_10) && (ts.power_level == PA_LEVEL_5W))	{
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_10m_5w_adj++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_10m_5w_adj--;
				tchange = 1;
			}
			//
			if(ts.pwr_10m_5w_adj > TX_POWER_FACTOR_MAX)
				ts.pwr_10m_5w_adj  = TX_POWER_FACTOR_MAX;
			else if(ts.pwr_10m_5w_adj < TX_POWER_FACTOR_MIN)
				ts.pwr_10m_5w_adj  = TX_POWER_FACTOR_MIN;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_10m_5w_adj = TX_POWER_FACTOR_10_DEFAULT;
				tchange = 1;
			}
			if(tchange)	{	// did something change?
				UiDriverSetBandPowerFactor(ts.band);	// yes, update the power factor
				Codec_SidetoneSetgain();				// adjust the sidetone gain
			}
		}
		else	// not enabled
			clr = Orange;
		//
		sprintf(options, "  %u ", ts.pwr_10m_5w_adj);
		opt_pos = CONFIG_10M_5W_ADJUST % MENUSIZE;
		break;
		//
	case CONFIG_80M_FULL_POWER_ADJUST:		// 80m full power adjust
		if((ts.band == BAND_MODE_80) && (ts.power_level == PA_LEVEL_FULL))	{
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_80m_full_adj++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_80m_full_adj--;
				tchange = 1;
			}
			//
			if(ts.pwr_80m_full_adj > TX_POWER_FACTOR_MAX)
				ts.pwr_80m_full_adj  = TX_POWER_FACTOR_MAX;
			else if(ts.pwr_80m_full_adj < TX_POWER_FACTOR_MIN)
				ts.pwr_80m_full_adj = TX_POWER_FACTOR_MIN;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_80m_full_adj = TX_POWER_FACTOR_80_DEFAULT;
				tchange = 1;
			}
			//
			if(tchange)	{		// did something change?
				UiDriverSetBandPowerFactor(ts.band);	// yes, update the power factor
				Codec_SidetoneSetgain();				// adjust the sidetone gain
			}
		}
		else	// not enabled
			clr = Orange;
		//
		sprintf(options, "  %u ", ts.pwr_80m_full_adj);
		opt_pos = CONFIG_80M_FULL_POWER_ADJUST % MENUSIZE;
		break;
		//
	case CONFIG_60M_FULL_POWER_ADJUST:		// 60m full power adjust
		if((ts.band == BAND_MODE_60) && (ts.power_level == PA_LEVEL_FULL))	{
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_60m_full_adj++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_60m_full_adj--;
				tchange = 1;
			}
			//
			if(ts.pwr_60m_full_adj > TX_POWER_FACTOR_MAX)
				ts.pwr_60m_full_adj = TX_POWER_FACTOR_MAX;
			else if(ts.pwr_60m_full_adj < TX_POWER_FACTOR_MIN)
				ts.pwr_60m_full_adj = TX_POWER_FACTOR_MIN;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_60m_full_adj = TX_POWER_FACTOR_60_DEFAULT;
				tchange = 1;
			}
			//
			if(tchange)		// did something change?
				UiDriverSetBandPowerFactor(ts.band);	// yes, update the power factor
		}
		else	// not enabled
			clr = Orange;
		//
		sprintf(options, "  %u ", ts.pwr_60m_full_adj);
		opt_pos = CONFIG_60M_FULL_POWER_ADJUST % MENUSIZE;
		break;
		//
	case CONFIG_40M_FULL_POWER_ADJUST:		// 40m full power adjust
		if((ts.band == BAND_MODE_40) && (ts.power_level == PA_LEVEL_FULL))	{
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_40m_full_adj++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_40m_full_adj--;
				tchange = 1;
			}
			//
			if(ts.pwr_40m_full_adj > TX_POWER_FACTOR_MAX)
				ts.pwr_40m_full_adj = TX_POWER_FACTOR_MAX;
			else if(ts.pwr_40m_full_adj < TX_POWER_FACTOR_MIN)
				ts.pwr_40m_full_adj = TX_POWER_FACTOR_MIN;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_40m_full_adj = TX_POWER_FACTOR_40_DEFAULT;
				tchange = 1;
			}
			if(tchange)	{	// did something change?
				UiDriverSetBandPowerFactor(ts.band);	// yes, update the power factor
				Codec_SidetoneSetgain();				// adjust the sidetone gain
			}
		}
		else	// not enabled
			clr = Orange;
		//
		sprintf(options, "  %u ", ts.pwr_40m_full_adj);
		opt_pos = CONFIG_40M_FULL_POWER_ADJUST % MENUSIZE;
		break;
		//
	case CONFIG_30M_FULL_POWER_ADJUST:		// 30m full power adjust
		if((ts.band == BAND_MODE_30) && (ts.power_level == PA_LEVEL_FULL))	{
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_30m_full_adj++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_30m_full_adj--;
				tchange = 1;
			}
			//
			if(ts.pwr_30m_full_adj > TX_POWER_FACTOR_MAX)
				ts.pwr_30m_full_adj = TX_POWER_FACTOR_MAX;
			else if(ts.pwr_30m_full_adj < TX_POWER_FACTOR_MIN)
				ts.pwr_30m_full_adj = TX_POWER_FACTOR_MIN;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_30m_full_adj = TX_POWER_FACTOR_30_DEFAULT;
				tchange = 1;
			}
			if(tchange)	{	// did something change?
				UiDriverSetBandPowerFactor(ts.band);	// yes, update the power factor
				Codec_SidetoneSetgain();				// adjust the sidetone gain
			}
		}
		else	// not enabled
			clr = Orange;
		//
		sprintf(options, "  %u ", ts.pwr_30m_full_adj);
		opt_pos = CONFIG_30M_FULL_POWER_ADJUST % MENUSIZE;
		break;
		//
	case CONFIG_20M_FULL_POWER_ADJUST:		// 20m full power adjust
		if((ts.band == BAND_MODE_20) && (ts.power_level == PA_LEVEL_FULL))	{
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_20m_full_adj++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_20m_full_adj--;
				tchange = 1;
			}
			//
			if(ts.pwr_20m_full_adj > TX_POWER_FACTOR_MAX)
				ts.pwr_20m_full_adj = TX_POWER_FACTOR_MAX;
			else if(ts.pwr_20m_full_adj < TX_POWER_FACTOR_MIN)
				ts.pwr_20m_full_adj = TX_POWER_FACTOR_MIN;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_20m_full_adj = TX_POWER_FACTOR_20_DEFAULT;
				tchange = 1;
			}
			if(tchange)	{	// did something change?
				UiDriverSetBandPowerFactor(ts.band);	// yes, update the power factor
				Codec_SidetoneSetgain();				// adjust the sidetone gain
			}
		}
		else	// not enabled
			clr = Orange;
		//
		sprintf(options, "  %u ", ts.pwr_20m_full_adj);
		opt_pos = CONFIG_20M_FULL_POWER_ADJUST % MENUSIZE;
		break;
		//
	case CONFIG_17M_FULL_POWER_ADJUST:		// 17m full power adjust
		if((ts.band == BAND_MODE_17) && (ts.power_level == PA_LEVEL_FULL))	{
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_17m_full_adj++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_17m_full_adj--;
				tchange = 1;
			}
			//
			if(ts.pwr_17m_full_adj > TX_POWER_FACTOR_MAX)
				ts.pwr_17m_full_adj = TX_POWER_FACTOR_MAX;
			else if(ts.pwr_17m_full_adj < TX_POWER_FACTOR_MIN)
				ts.pwr_17m_full_adj = TX_POWER_FACTOR_MIN;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_17m_full_adj = TX_POWER_FACTOR_17_DEFAULT;
				tchange = 1;
			}
			if(tchange)	{	// did something change?
				UiDriverSetBandPowerFactor(ts.band);	// yes, update the power factor
				Codec_SidetoneSetgain();				// adjust the sidetone gain
			}
		}
		else	// not enabled
			clr = Orange;
		//
		sprintf(options, "  %u ", ts.pwr_17m_full_adj);
		opt_pos = CONFIG_17M_FULL_POWER_ADJUST % MENUSIZE;
		break;
		//
	case CONFIG_15M_FULL_POWER_ADJUST:		// 15m full power adjust
		if((ts.band == BAND_MODE_15) && (ts.power_level == PA_LEVEL_FULL))	{
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_15m_full_adj++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_15m_full_adj--;
				tchange = 1;
			}
			//
			if(ts.pwr_15m_full_adj > TX_POWER_FACTOR_MAX)
				ts.pwr_15m_full_adj  = TX_POWER_FACTOR_MAX;
			else if(ts.pwr_15m_full_adj < TX_POWER_FACTOR_MIN)
				ts.pwr_15m_full_adj = TX_POWER_FACTOR_MIN;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_15m_full_adj = TX_POWER_FACTOR_15_DEFAULT;
				tchange = 1;
			}
			if(tchange)	{	// did something change?
				UiDriverSetBandPowerFactor(ts.band);	// yes, update the power factor
				Codec_SidetoneSetgain();				// adjust the sidetone gain
			}
		}
		else	// not enabled
			clr = Orange;
		//
		sprintf(options, "  %u ", ts.pwr_15m_full_adj);
		opt_pos = CONFIG_15M_FULL_POWER_ADJUST % MENUSIZE;
		break;
		//
	case CONFIG_12M_FULL_POWER_ADJUST:		// 12m full power adjust
		if((ts.band == BAND_MODE_12) && (ts.power_level == PA_LEVEL_FULL))	{
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_12m_full_adj++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_12m_full_adj--;
				tchange = 1;
			}
			//
			if(ts.pwr_12m_full_adj > TX_POWER_FACTOR_MAX)
				ts.pwr_12m_full_adj = TX_POWER_FACTOR_MAX;
			else if(ts.pwr_12m_full_adj < TX_POWER_FACTOR_MIN)
				ts.pwr_12m_full_adj = TX_POWER_FACTOR_MIN;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_12m_full_adj = TX_POWER_FACTOR_12_DEFAULT;
				tchange = 1;
			}
			if(tchange)	{	// did something change?
				UiDriverSetBandPowerFactor(ts.band);	// yes, update the power factor
				Codec_SidetoneSetgain();				// adjust the sidetone gain
			}
		}
		else	// not enabled
			clr = Orange;
		//
		sprintf(options, "  %u ", ts.pwr_12m_full_adj);
		opt_pos = CONFIG_12M_FULL_POWER_ADJUST % MENUSIZE;
		break;
		//
	case CONFIG_10M_FULL_POWER_ADJUST:		// 10m full power adjust
		if((ts.band == BAND_MODE_10) && (ts.power_level == PA_LEVEL_FULL))	{
			if(var >= 1)	{	// setting increase?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_10m_full_adj++;
				tchange = 1;
			}
			else if(var <= -1)	{	// setting decrease?
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_10m_full_adj--;
				tchange = 1;
			}
			//
			if(ts.pwr_10m_full_adj > TX_POWER_FACTOR_MAX)
				ts.pwr_10m_full_adj  = TX_POWER_FACTOR_MAX;
			else if(ts.pwr_10m_full_adj < TX_POWER_FACTOR_MIN)
				ts.pwr_10m_full_adj  = TX_POWER_FACTOR_MIN;
			//
			if(mode == 3)	{
				ts.menu_var_changed = 1;	// indicate that a change has occurred
				ts.pwr_10m_full_adj = TX_POWER_FACTOR_10_DEFAULT;
				tchange = 1;
			}
			if(tchange)	{	// did something change?
				UiDriverSetBandPowerFactor(ts.band);	// yes, update the power factor
				Codec_SidetoneSetgain();				// adjust the sidetone gain
			}
		}
		else	// not enabled
			clr = Orange;
		//
		sprintf(options, "  %u ", ts.pwr_10m_full_adj);
		opt_pos = CONFIG_10M_FULL_POWER_ADJUST % MENUSIZE;
		break;
		//
	case CONFIG_DSP_NR_DECORRELATOR_BUFFER_LENGTH:		// Adjustment of DSP noise reduction de-correlation delay buffer length
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.dsp_nr_delaybuf_len+= 16;
			tchange = 1;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.dsp_nr_delaybuf_len-= 16;
			tchange = 1;
		}
		//
		ts.dsp_nr_delaybuf_len &= 0xfff0;	// mask bottom nybble to enforce 16-count boundary
		//
		if(ts.dsp_nr_delaybuf_len > DSP_NR_BUFLEN_MAX)
			ts.dsp_nr_delaybuf_len  = DSP_NR_BUFLEN_MAX;
		else if(ts.dsp_nr_delaybuf_len < DSP_NR_BUFLEN_MIN)
			ts.dsp_nr_delaybuf_len  = DSP_NR_BUFLEN_MIN;
		//
		if(mode == 3)	{
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.dsp_nr_delaybuf_len = DSP_NR_BUFLEN_DEFAULT;
			tchange = 1;
		}
		if(tchange)	{	// did something change?
			if(ts.dsp_active & 1)	// only update if DSP NR active
				audio_driver_set_rx_audio_filter();
		}
		//
		if(!(ts.dsp_active & 1))	// mark orange if DSP NR not active
			clr = Orange;
		//
		if(ts.dsp_nr_numtaps >= ts.dsp_nr_delaybuf_len)	// Warn if number of taps greater than/equal buffer length!
			clr = Red;
		//
		sprintf(options, "  %u ", (uint)ts.dsp_nr_delaybuf_len);
		//
		opt_pos = CONFIG_DSP_NR_DECORRELATOR_BUFFER_LENGTH % MENUSIZE;
		break;
		//
	case CONFIG_DSP_NR_FFT_NUMTAPS:		// Adjustment of DSP noise reduction de-correlation delay buffer length
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.dsp_nr_numtaps+= 16;
			tchange = 1;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.dsp_nr_numtaps-= 16;
			tchange = 1;
		}
		//
		ts.dsp_nr_numtaps &= 0xf0;	// mask bottom nybble to enforce 16-count boundary
		//
		if(ts.dsp_nr_numtaps > DSP_NR_NUMTAPS_MAX)
			ts.dsp_nr_numtaps  = DSP_NR_NUMTAPS_MAX;
		else if(ts.dsp_nr_numtaps < DSP_NR_NUMTAPS_MIN)
			ts.dsp_nr_numtaps  = DSP_NR_NUMTAPS_MIN;
		//
		if(mode == 3)	{
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.dsp_nr_numtaps = DSP_NR_NUMTAPS_DEFAULT;
			tchange = 1;
		}
		if(tchange)	{	// did something change?
			if(ts.dsp_active & 1)	// only update if DSP NR active
				audio_driver_set_rx_audio_filter();
		}
		//
		if(!(ts.dsp_active & 1))	// mark orange if DSP NR not active
			clr = Orange;
		//
		if(ts.dsp_nr_numtaps >= ts.dsp_nr_delaybuf_len)	// Warn if number of taps greater than/equal buffer length!
			clr = Red;
		//
		sprintf(options, "  %u ", ts.dsp_nr_numtaps);
		//
		opt_pos = CONFIG_DSP_NR_FFT_NUMTAPS % MENUSIZE;
		break;
		//
	case CONFIG_DSP_NR_POST_AGC_SELECT:		// selection of location of DSP noise reduction - pre audio filter/AGC or post AGC/filter
		if(var >= 1)	{		// setting increase?
			ts.dsp_active |= 2;				// it is on - set LSB+1 active - DSP noise reduction is AFTER AGC
			tchange = 1;
			ts.menu_var_changed = 1;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.dsp_active &= 0xfd;		// clear LSB+1	 - DSP noise reduction is BEFORE filtering/AGC
			tchange = 1;
			ts.menu_var_changed = 1;
		}
		//
		if(mode == 3)	{		// Default mode is to
			ts.dsp_active &= 0xfd;		// clear LSB+1	- DSP noise reduction is BEFORE filtering/AGC
			tchange = 1;
			ts.menu_var_changed = 1;
		}
		//
		if(!(ts.dsp_active & 1))	// mark orange if DSP NR not active
			clr = Orange;
		//
		if(tchange)	{	// did something change?
			if(ts.dsp_active & 1)	// only update if DSP NR active
				audio_driver_set_rx_audio_filter();
		}
		//
		if(ts.dsp_active & 2)	// Is it on?
			sprintf(options, "  YES");
		else
			sprintf(options, "  NO ");
		//
		opt_pos = CONFIG_DSP_NR_POST_AGC_SELECT % MENUSIZE;
		break;
		//
	case CONFIG_DSP_NOTCH_CONVERGE_RATE:		// Adjustment of DSP noise reduction de-correlation delay buffer length
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.dsp_notch_mu++;
			tchange = 1;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			if(ts.dsp_notch_mu)
				ts.dsp_notch_mu--;
			tchange = 1;
		}
		//
		ts.dsp_nr_numtaps &= 0xf0;	// mask bottom nybble to enforce 16-count boundary
		//
		if(ts.dsp_notch_mu > DSP_NOTCH_MU_MAX)
			ts.dsp_notch_mu  = DSP_NOTCH_MU_MAX;
		//
		if(mode == 3)	{
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.dsp_notch_mu = DSP_NOTCH_MU_DEFAULT;
			tchange = 1;
		}
		if(tchange)	{	// did something change?
			if(ts.dsp_active & 4)	// only update if Notch DSP is active
				audio_driver_set_rx_audio_filter();
		}
		//
		if(!(ts.dsp_active & 4))	// mark orange if Notch DSP not active
			clr = Orange;
		//
		sprintf(options, "  %u ", ts.dsp_notch_mu);
		//
		opt_pos = CONFIG_DSP_NOTCH_CONVERGE_RATE % MENUSIZE;
		break;
		//
	case CONFIG_DSP_NOTCH_DECORRELATOR_BUFFER_LENGTH:		// Adjustment of DSP noise reduction de-correlation delay buffer length
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.dsp_notch_delaybuf_len += 32;
			tchange = 1;
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.dsp_notch_delaybuf_len -= 32;
			tchange = 1;
		}
		//
		ts.dsp_notch_delaybuf_len &= 0xffe0;	// mask bottom nybble to enforce 64-count boundary
		//
		if(ts.dsp_notch_delaybuf_len > DSP_NOTCH_BUFLEN_MAX)
			ts.dsp_notch_delaybuf_len  = DSP_NOTCH_BUFLEN_MAX;
		else if(ts.dsp_notch_delaybuf_len < DSP_NOTCH_BUFLEN_MIN)
			ts.dsp_notch_delaybuf_len  = DSP_NOTCH_BUFLEN_MIN;
		//
		if(mode == 3)	{
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.dsp_notch_delaybuf_len = DSP_NOTCH_DELAYBUF_DEFAULT;
			tchange = 1;
		}
		if(tchange)	{	// did something change?
			if(ts.dsp_active & 1)	// only update if DSP Notch active
				audio_driver_set_rx_audio_filter();
		}
		//
		if(!(ts.dsp_active & 4))	// mark orange if DSP Notch not active
			clr = Orange;
		//
		//
		sprintf(options, "  %u ", (uint)ts.dsp_notch_delaybuf_len);
		//
		opt_pos = CONFIG_DSP_NOTCH_DECORRELATOR_BUFFER_LENGTH % MENUSIZE;
		break;
		//
	case CONFIG_AGC_TIME_CONSTANT:		// Adjustment of Noise Blanker AGC Time Constant
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.nb_agc_time_const++;
			tchange = 1;
		}
		else if(var <= -1)	{	// setting decrease?
			if(ts.nb_agc_time_const)	{
				ts.nb_agc_time_const--;
				tchange = 1;
				ts.menu_var_changed = 1;	// indicate that a change has occurred
			}
		}
		//
		if(ts.nb_agc_time_const > NB_MAX_AGC_SETTING)
			ts.nb_agc_time_const  = NB_MAX_AGC_SETTING;
		//
		if(mode == 3)	{
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			ts.nb_agc_time_const = NB_AGC_DEFAULT;
			tchange = 1;
		}
		//
		if(tchange)	{				// parameter changed?
			UiCalcNB_AGC();	// yes - recalculate new values for Noise Blanker AGC
		}
		//
		sprintf(options, "  %u ", ts.nb_agc_time_const);
		//
		opt_pos = CONFIG_AGC_TIME_CONSTANT % MENUSIZE;
		break;
		//
	case CONFIG_AM_TX_FILTER_ENABLE:	// Enable/disable AM TX audio filter
		temp_var = ts.misc_flags1 & 8;
		if(var >= 1)	{	// setting increase?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			temp_var = 1;				// AM TX audio filter is disabled
			tchange = 1;				// indicate change of parameter
		}
		else if(var <= -1)	{	// setting decrease?
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			temp_var =  0;				// AM TX audio filter is enabled
			tchange = 1;				// indicate change of parameter
		}
		//
		if(mode == 3)	{
			temp_var = 0;				// enable by default
			ts.menu_var_changed = 1;	// indicate that a change has occurred
			tchange = 1;				// indicate change of parameter
		}
		//
		if(tchange)		{	// did the status change and is translate mode NOT active?
			if(temp_var)	// AM TX audio filter is disabled
				ts.misc_flags1 |= 8;		// set LSB
			else			// AM TX audio filter is enabled
				ts.misc_flags1 &= 0xf7;		// clear LSB
		}
		//
		if(ts.misc_flags1 & 8)	{			// Display status of TX audio filter
			strcpy(options, "  OFF");
			clr = Orange;					// warn user that filter is off!
		}
		else
			strcpy(options, "  ON  ");
		//
		opt_pos = CONFIG_AM_TX_FILTER_ENABLE % MENUSIZE;	// Y position of this menu item
		break;
		//
	default:						// Move to this location if we get to the bottom of the table!
		strcpy(options, "ERROR!");
		opt_pos = 5;
		break;
	}
	//
	if(!disp_shift)	// in normal position?
		UiLcdHy28_PrintText(POS_MENU_CHANGE_X, POS_MENU_IND_Y + (opt_pos * 12), options, clr, Black, 0);		// yes, normal position
	else	// shift left to accommodate large display
		UiLcdHy28_PrintText(POS_MENU_CHANGE_X-24 , POS_MENU_IND_Y + (opt_pos * 12), options, clr, Black, 0);
	//
	if(mode == 1)	{
		if(opt_oldpos != 999)		// was the position of a previous cursor stored?
			UiLcdHy28_PrintText(POS_MENU_CURSOR_X, POS_MENU_IND_Y + (opt_oldpos * 12), " ", Black, Black, 0);	// yes - erase it
		//
		opt_oldpos = opt_pos;	// save position of new "old" cursor position
		UiLcdHy28_PrintText(POS_MENU_CURSOR_X, POS_MENU_IND_Y + (opt_pos * 12), "<", Green, Black, 0);	// place cursor at active position
	}
	//
	return;
}

