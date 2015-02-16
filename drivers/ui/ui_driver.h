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

#ifndef __UI_DRIVER_H
#define __UI_DRIVER_H

#include "mchf_board.h"

// SI570 startup value (receive freq * 4)
//#define 	STARTUP_FREQ					112000000
#define 	STARTUP_FREQ					56000000

#define 	T_STEP_1HZ						1
#define 	T_STEP_10HZ						10
#define 	T_STEP_100HZ					100
#define 	T_STEP_1KHZ						1000
#define 	T_STEP_10KHZ					10000
#define 	T_STEP_100KHZ					100000

#define 	MAX_STEPS						6

// S meter
#define 	S_METER_V_POS					55
#define 	S_METER_SIZE					9
#define 	S_VERTI_SIZE					10
#define 	S_LEFT_SHIFT					20
#define 	S_BLOCK_GAP		 				2
#define 	S_COLOR_INCR					1
#define 	S_HEIGHT_INCR					12

// Button Definitions
#define 	BUTTON_NONE						0
#define 	BUTTON_HOLD_TIME				50000
#define 	AUTO_BLINK_TIME					200000

typedef struct KeypadState
{
	// Actual time the button is pressed
	ulong	debounce_time;

	// ID of button as GPIO bit
	ulong	button_id;

	// Flag to indicate click event
	uchar	button_pressed;

	// Flag to indicate release event
	uchar	button_released;

	// Flag to indicate under process
	uchar	button_processed;

	uchar	dummy;

} KeypadState;

#define S_METER_MAX							34

#define SWR_SAMPLES_SKP						1	//5000
#define SWR_SAMPLES_CNT						4

// SWR meter public
typedef struct SWRMeter
{
	ulong	skip;

	ushort	pwr_aver;
	ushort	swr_aver;
	uchar	p_curr;

} SWRMeter;

// Power levels ranges
#define POWER_1W_MIN						0
#define POWER_1W_MAX						1400

#define POWER_2W_MIN						1400
#define POWER_2W_MAX						1700

#define POWER_3W_MIN						1700
#define POWER_3W_MAX						2000

#define POWER_4W_MIN						2000
#define POWER_4W_MAX						2400

#define POWER_5W_MIN						2400
#define POWER_5W_MAX						2700

#define POWER_6W_MIN						2700
#define POWER_6W_MAX						2900

#define POWER_7W_MIN						2900
#define POWER_7W_MAX						3100

#define POWER_8W_MIN						3100
#define POWER_8W_MAX						3400

#define POWER_9W_MIN						3400
#define POWER_9W_MAX						3600

#define POWER_10W_MIN						3600
#define POWER_10W_MAX						4000

#define POWER_SAMPLES_SKP					10	//1500
#define POWER_SAMPLES_CNT					32

// Power supply
typedef struct PowerMeter
{
	ulong	skip;

	ulong	pwr_aver;
	uchar	p_curr;

	uchar	v10;
	uchar	v100;
	uchar	v1000;
	uchar	v10000;

} PowerMeter;

#define LO_COMP_SKP							50		//50000

// LO temperature compensation
typedef struct LoTcxo
{
	ulong	skip;

	// Current compensation value
	// loaded to LO
	int		comp;

	uchar	sensor_present;

	uchar	v1000;
	uchar	v10000;
	uchar	v100000;
	uchar	v1000000;

} LoTcxo;

// Once every 25s - 0xFFFFF
#define EEPROM_SAVE_SKP						0xFFFFF

// Eeprom saving routine
typedef struct EepromSave
{
	ulong	skip;

} EepromSave;

// --------------------------------------------------------------------------
// Controls positions
// --------------------
#define SMALL_FONT_WIDTH					8
#define LARGE_FONT_WIDTH					16

// Frequency display control
#define POS_TUNE_FREQ_X						116
#define POS_TUNE_FREQ_Y						100

// Second frequency display control
#define POS_TUNE_SFREQ_X					(POS_TUNE_FREQ_X + 120)
#define POS_TUNE_SFREQ_Y					(POS_TUNE_FREQ_Y - 20)

// Band selection control
#define POS_BAND_MODE_X						(POS_TUNE_FREQ_X + 175)
#define POS_BAND_MODE_Y						(POS_TUNE_FREQ_Y + 7)
#define POS_BAND_MODE_MASK_X				(POS_BAND_MODE_X - 1)
#define POS_BAND_MODE_MASK_Y				(POS_BAND_MODE_Y - 1)
#define POS_BAND_MODE_MASK_H				13
#define POS_BAND_MODE_MASK_W				33

// Demodulator mode control
#define POS_DEMOD_MODE_X					(POS_TUNE_FREQ_X + 1)
#define POS_DEMOD_MODE_Y					(POS_TUNE_FREQ_Y - 20)
#define POS_DEMOD_MODE_MASK_X				(POS_DEMOD_MODE_X - 1)
#define POS_DEMOD_MODE_MASK_Y				(POS_DEMOD_MODE_Y - 1)
#define POS_DEMOD_MODE_MASK_H				13
#define POS_DEMOD_MODE_MASK_W				41

// Tunning step control
#define POS_TUNE_STEP_X						(POS_TUNE_FREQ_X + 50)
#define POS_TUNE_STEP_Y						(POS_TUNE_FREQ_Y - 20)
#define POS_TUNE_STEP_MASK_X				(POS_TUNE_STEP_X - 1)
#define POS_TUNE_STEP_MASK_Y				(POS_TUNE_STEP_Y - 1)
#define POS_TUNE_STEP_MASK_H				17
#define POS_TUNE_STEP_MASK_W				49

#define POS_RADIO_MODE_X					4
#define POS_RADIO_MODE_Y					5

// Bottom bar
#define POS_BOTTOM_BAR_X					0
#define POS_BOTTOM_BAR_Y					228
#define POS_BOTTOM_BAR_BUTTON_W				62
#define POS_BOTTOM_BAR_BUTTON_H				16

// Virtual Button 1
#define POS_BOTTOM_BAR_F1_X					(POS_BOTTOM_BAR_X +                              2)
#define POS_BOTTOM_BAR_F1_Y					POS_BOTTOM_BAR_Y

// Virtual Button 2
#define POS_BOTTOM_BAR_F2_X					(POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*1 +  4)
#define POS_BOTTOM_BAR_F2_Y					POS_BOTTOM_BAR_Y

// Virtual Button 3
#define POS_BOTTOM_BAR_F3_X					(POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*2 +  6)
#define POS_BOTTOM_BAR_F3_Y					POS_BOTTOM_BAR_Y

// Virtual Button 4
#define POS_BOTTOM_BAR_F4_X					(POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*3 +  8)
#define POS_BOTTOM_BAR_F4_Y					POS_BOTTOM_BAR_Y

// Virual Button 5
#define POS_BOTTOM_BAR_F5_X					(POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*4 + 10)
#define POS_BOTTOM_BAR_F5_Y					POS_BOTTOM_BAR_Y

// --------------------------------------------------
// Encoder one controls indicator
// audio gain
#define POS_AG_IND_X						0
#define POS_AG_IND_Y						27
// sidetone gain
#define POS_SG_IND_X						60
#define POS_SG_IND_Y						27

// --------------------------------------------------
// Encoder two controls indicator
// RF Gain indicator
#define POS_RF_IND_X						0
#define POS_RF_IND_Y						43
// RF attenuator
#define POS_RA_IND_X						60
#define POS_RA_IND_Y						43

// --------------------------------------------------
// Encoder three controls indicator
// RIT indicator
#define POS_RIT_IND_X						0
#define POS_RIT_IND_Y						59
// keyer speed
#define POS_KS_IND_X						60
#define POS_KS_IND_Y						59

// --------------------------------------------------
// Calibration mode
//
// PA bias
#define POS_PB_IND_X						0
#define POS_PB_IND_Y						78
// IQ gain balance
#define POS_BG_IND_X						0
#define POS_BG_IND_Y						94
// IQ phase balance
#define POS_BP_IND_X						0
#define POS_BP_IND_Y						110

// --------------------------------------------------
// Standalone controls
//
// Keyer mode
#define POS_KM_IND_X						0
#define POS_KM_IND_Y						131
// Power level
#define POS_PW_IND_X						0
#define POS_PW_IND_Y						147
// Filter indicator
#define POS_FIR_IND_X						0
#define POS_FIR_IND_Y						163

// Spectrum display
#define POS_SPECTRUM_IND_X					60
#define POS_SPECTRUM_IND_Y					150
#define POS_SPECTRUM_IND_H					80
#define POS_SPECTRUM_IND_W					258
#define COL_SPECTRUM_GRAD					0x40

// ETH position
#define POS_ETH_IND_X						220
#define POS_ETH_IND_Y						2

// USB Keyboard position
#define POS_KBD_IND_X						220
#define POS_KBD_IND_Y						18

// S meter position
#define POS_SM_IND_X						116
#define POS_SM_IND_Y						0

// Supply Voltage indicator
#define POS_PWRN_IND_X						0
#define POS_PWRN_IND_Y						193
#define POS_PWR_IND_X						5
#define POS_PWR_IND_Y						(POS_PWRN_IND_Y + 15)
#define COL_PWR_IND							Grey

#define POS_TEMP_IND_X						0
#define POS_TEMP_IND_Y						0

// --------------------------------------------------------------------------
// Exports
void ui_driver_init(void);
void ui_driver_thread(void);
void ui_driver_irq(void);
void ui_driver_toggle_tx(void);

#endif
