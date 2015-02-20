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

// Common
#include "mchf_board.h"

#include <stdio.h>
#include "arm_math.h"
#include "math.h"

// LCD
#include "ui_lcd_hy28.h"

// Encoders
#include "ui_rotary.h"

// SI570 control
#include "ui_si570.h"
#include "ui_soft_tcxo.h"

// Codec control
#include "codec.h"
#include "softdds.h"

#include "audio_driver.h"
#include "hamm_wnd.h"
#include "ui_driver.h"
//#include "usbh_usr.h"

#include "cat_driver.h"

// Virtual eeprom
#include "eeprom.h"

#include "cw_gen.h"

static void 	UiDriverPublicsInit(void);
static void 	UiDriverProcessKeyboard(void);
static void 	UiDriverProcessFunctionKeyClick(ulong id);

static void 	UiDriverShowMode(void);
static void 	UiDriverShowStep(ulong step);
static void 	UiDriverShowBand(uchar band);
static void 	UiDriverChangeBandFilter(uchar band,uchar bpf_only);

static void 	UiDriverCreateDesktop(void);

static void 	UiDriverCreateFunctionButtons(bool full_repaint);

static void 	UiDriverCreateSpectrumScope(void);
//static void 	UiDriverRepaintSpectrumScope(void);
//static void 	UiDriverCreateDigiPanel(void);

static void 	UiDriverCreateSMeter(void);

//static void 	UiDriverUpdateTopMeter(uchar val,uchar old);
static void 	UiDriverUpdateTopMeterA(uchar val,uchar old);
static void 	UiDriverUpdateBtmMeter(uchar val);

static void 	UiDriverInitFrequency(void);

static void 	UiDriverUpdateFrequency(char skip_encoder_check);
static void 	UiDriverUpdateFrequencyFast(void);
static void 	UiDriverUpdateLcdFreq(ulong dial_freq,ushort color);
static void 	UiDriverUpdateSecondLcdFreq(ulong dial_freq);

static void 	UiDriverChangeTunningStep(uchar is_up);

static void		UiDriverKeypadCheck(void);

static void 	UiDriverChangeDemodMode(void);
static void 	UiDriverChangeBand(uchar is_up);

static bool 	UiDriverCheckFrequencyEncoder(void);
static void 	UiDriverCheckEncoderOne(void);
static void 	UiDriverCheckEncoderTwo(void);
static void 	UiDriverCheckEncoderThree(void);

static void 	UiDriverChangeEncoderOneMode(uchar skip);
static void 	UiDriverChangeEncoderTwoMode(uchar skip);
static void 	UiDriverChangeEncoderThreeMode(uchar skip);

//static void 	UiDriverSelectBandFilter(void);

// encoder one
static void 	UiDriverChangeAfGain(uchar enabled);
static void 	UiDriverChangeStGain(uchar enabled);
static void 	UiDriverChangeKeyerSpeed(uchar enabled);
// encoder two
static void 	UiDriverChangeRfGain(uchar enabled);
static void 	UiDriverChangeRfAttenuator(uchar enabled);
static void 	UiDriverChangeIQGainBalance(uchar enabled,uchar visible);
static void 	UiDriverChangeIQPhaseBalance(uchar enabled,uchar visible);
static void 	UiDriverChangePaBias(uchar enabled,uchar visible);
// encoder three
static void 	UiDriverChangeRit(uchar enabled);
static void 	UiDriverChangeFilter(uchar ui_only_update);

static void 	UiDriverChangeKeyerMode(void);
static void 	UiDriverChangePowerLevel(void);

static void 	UiDriverInitSpectrumDisplay(void);
static void 	UiDriverReDrawSpectrumDisplay(void);

//static void 	UiDriverUpdateEthernetStatus(void);
//static void 	UiDriverUpdateUsbKeyboardStatus(void);

static void 	UiDriverPowerOffCheck(void);

static void 	UiDriverHandleSmeter(void);
static void 	UiDriverHandleSWRMeter(void);
static void 	UiDriverHandlePowerSupply(void);

// LO TCXO routines
static void 	UiDriverUpdateLoMeter(uchar val,uchar active);
static void 	UiDriverCreateTemperatureDisplay(uchar enabled,uchar create);
static void 	UiDriverRefreshTemperatureDisplay(uchar enabled,int temp);
static void 	UiDriverHandleLoTemperature(void);

//static void 	UiDriverEditMode(void);

static void 	UiDriverSwitchOffPtt(void);

static void 	UiDriverDelayedUnmute(void);

static void 	UiDriverSetBandPowerFactor(uchar band);

static void 	UiDriverLoadEepromValues(void);
static void 	UiDriverSaveEepromValues(void);

// Tuning steps
const ulong tune_steps[MAX_STEPS] = {T_STEP_1HZ,T_STEP_10HZ,T_STEP_100HZ,T_STEP_1KHZ,T_STEP_10KHZ,T_STEP_100KHZ};

// Band definitions - band base frequency value
const ulong tune_bands[MAX_BANDS] = { BAND_FREQ_80,
									  BAND_FREQ_60,
									  BAND_FREQ_40,
									  BAND_FREQ_30,
									  BAND_FREQ_20,
									  BAND_FREQ_17,
									  BAND_FREQ_15,
									  BAND_FREQ_12,
									  BAND_FREQ_10};

// Band definitions - band frequency size
const ulong size_bands[MAX_BANDS] = { BAND_SIZE_80,
									  BAND_SIZE_60,
									  BAND_SIZE_40,
									  BAND_SIZE_30,
									  BAND_SIZE_20,
									  BAND_SIZE_17,
									  BAND_SIZE_15,
									  BAND_SIZE_12,
									  BAND_SIZE_10};

// -------------------------------------------------------
// Constant declaration of the buttons map across ports
// - update if moving buttons around !!!
const ButtonMap	bm[16] =
{
		{BUTTON_M2_PIO,		BUTTON_M2},		// 0
		{BUTTON_G2_PIO,		BUTTON_G2},		// 1
		{BUTTON_G3_PIO,		BUTTON_G3},		// 2
		{BUTTON_BNDM_PIO,	BUTTON_BNDM},	// 3
		{BUTTON_G4_PIO,		BUTTON_G4},		// 4
		{BUTTON_M3_PIO,		BUTTON_M3},		// 5
		{BUTTON_STEPM_PIO,	BUTTON_STEPM},	// 6
		{BUTTON_STEPP_PIO,	BUTTON_STEPP},	// 7
		{BUTTON_M1_PIO,		BUTTON_M1},		// 8
		{BUTTON_F3_PIO,		BUTTON_F3},		// 9
		{BUTTON_F1_PIO,		BUTTON_F1},		// 10
		{BUTTON_F2_PIO,		BUTTON_F2},		// 11
		{BUTTON_F4_PIO,		BUTTON_F4},		// 12
		{BUTTON_BNDP_PIO,	BUTTON_BNDP},	// 13
		{BUTTON_F5_PIO,		BUTTON_F5},		// 14
		{BUTTON_G1_PIO,		BUTTON_G1}		// 15
};

// Bands tuning values
__IO ulong band_dial_value[MAX_BANDS];
__IO ulong band_decod_mode[MAX_BANDS];

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
// Auto mode blinking text
//__IO AutoButtonState			abst;

// ------------------------------------------------
// On screen clock
//__IO ClockState 				cs;

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
// Eeprom Saving
__IO EepromSave					es;

// ------------------------------------------------
// CAT driver state
__IO CatDriver					kd;

// move to struct ??
__IO ulong 						unmute_delay = 0;

// ------------------------------------------------
// Spectrum display
extern __IO	SpectrumDisplay		sd;

// ------------------------------------------------
// Public USB Keyboard status
//extern __IO KeyBoardStatus		kbs;

// ------------------------------------------------
// Public s meter
extern	__IO	SMeter			sm;

// ------------------------------------------------
// Eeprom items
extern uint16_t VirtAddVarTab[NB_OF_VAR];



uchar drv_state = 0;
uchar drv_init = 0;




//*----------------------------------------------------------------------------
//* Function Name       : ui_driver_init
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void ui_driver_init(void)
{
	short res;

#ifdef DEBUG_BUILD
	printf("ui driver init...\n\r");
#endif

	// Driver publics init
	UiDriverPublicsInit();

	// Init frequency publics
	UiDriverInitFrequency();

	// Init spectrum display
	UiDriverInitSpectrumDisplay();

	// Load from eeprom
	UiDriverLoadEepromValues();

	// Temp sensor setup
	lo.sensor_present = ui_si570_init_temp_sensor();

	// Read SI570 settings
	res = ui_si570_get_configuration();
	if(res != 0)
	{
		//printf("err I2C: %d\n\r",res);
	}

	// Create desktop screen
	UiDriverCreateDesktop();

	// Set SoftDDS in CW mode
	if(ts.dmod_mode == DEMOD_CW)
		softdds_setfreq(CW_SIDETONE_OFFCET,ts.samp_rate,0);
	else
		softdds_setfreq(0.0,ts.samp_rate,0);

	// Update codec volume
	//  0 - 10: via codec command
	// 10 - 20: soft gain after decoder
	Codec_Volume((ts.audio_gain*8));

	// Set TX power factor
	UiDriverSetBandPowerFactor(ts.band_mode);

	// Reset inter driver requests flag
	ts.LcdRefreshReq	= 0;
	ts.new_band 		= ts.band_mode;
	df.step_new 		= df.tunning_step;

	// Extra HW init
	mchf_board_post_init();

	// Acknowledge end of init for the main thread called
	// via irq(even before the init is done)
	// bit useless since 0.171 as IRQs are enabled in
	// mchf_board_post_init(), but still used by
	// ui_driver_toggle_tx() to prevent re-entrance
	drv_init = 1;

#ifdef DEBUG_BUILD
	printf("ui driver init ok\n\r");
#endif
}

//*----------------------------------------------------------------------------
//* Function Name       : ui_driver_thread
//* Object              : non urgent, time taking operations
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void ui_driver_thread(void)
{
	// Is there user request for power off ?
//	UiDriverPowerOffCheck();

	// Spectrum display
//	UiDriverReDrawSpectrumDisplay();

	// S meter
//	UiDriverHandleSmeter();

	// FWD/SWR meter
//	UiDriverHandleSWRMeter();

	// Display voltage
//	UiDriverHandlePowerSupply();

	// LO temperature compensation
//	UiDriverHandleLoTemperature();

	// Check keypad
//	UiDriverKeypadCheck();

	// Process encoders
//	UiDriverCheckEncoderOne();
//	UiDriverCheckEncoderTwo();
//	UiDriverCheckEncoderThree();

//	UiDriverUpdateFrequency(0);

	// USB keyboard update
	//UiDriverUpdateUsbKeyboardStatus();

	// Keyboard processor
//	UiDriverProcessKeyboard();

	// Wspr edit mode
	//UiDriverEditMode();

	// Handle return to RX
//	UiDriverSwitchOffPtt();

	// Save eeprom values
	UiDriverSaveEepromValues();
}

//*----------------------------------------------------------------------------
//* Function Name       : ui_driver_irq
//* Object              : All real time processing here
//* Object              : only fast, non blocking operations
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void ui_driver_irq(void)
{
	// Do not run the state machine
	// before the driver init is done
	if(!drv_init)
		return;

	switch(drv_state)
	{
		case 0:
			UiDriverPowerOffCheck();
			break;
		case 1:
			UiDriverReDrawSpectrumDisplay();
			break;
		case 2:
			UiDriverHandleSmeter();
			break;
		case 3:
			UiDriverHandleSWRMeter();
			break;
		case 4:
			UiDriverHandlePowerSupply();
			break;
		case 5:
			UiDriverHandleLoTemperature();
			break;
		case 6:
			UiDriverKeypadCheck();
			break;
		case 7:
			UiDriverCheckEncoderOne();
			break;
		case 8:
			UiDriverCheckEncoderTwo();
			break;
		case 9:
			UiDriverCheckEncoderThree();
			break;
		case 10:
			UiDriverUpdateFrequency(0);
			break;
		case 11:
			UiDriverProcessKeyboard();
			break;
		case 12:
			UiDriverSwitchOffPtt();
			break;
		default:
			drv_state = 0;
			return;
	}
	drv_state++;
}

//*----------------------------------------------------------------------------
//* Function Name       : ui_driver_toggle_tx
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void ui_driver_toggle_tx(void)
{
	// Disable irq processing
	drv_init = 0;

	if(ts.txrx_mode == TRX_MODE_TX)
	{
		PTT_CNTR_PIO->BSRRL  	= PTT_CNTR;		// TX on
		RED_LED_PIO->BSRRL 		= RED_LED;		// Red led on
	}
	else
	{
		PTT_CNTR_PIO->BSRRH  	= PTT_CNTR;		// TX off
		RED_LED_PIO->BSRRH 		= RED_LED;		// Red led off
	}

	// Re-set frequency
	UiDriverUpdateFrequencyFast();

	// Switch codec mode
	Codec_RX_TX();

	// Set filters - complex issue with the CMOS switches
	// logic is set in unknown state when PTT rise/fall is too
	// fast or dirty
	//--UiDriverChangeBandFilter(ts.band_mode,1); -  not working

	// Enable irq processing
	drv_init = 1;
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverPublicsInit
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//*----------------------------------------------------------------------------
static void UiDriverPublicsInit(void)
{
	// Button state structure init state
	ks.button_id			= BUTTON_NONE;
	ks.button_pressed		= 0;
	ks.button_released		= 0;
	ks.button_processed		= 0;
	ks.debounce_time		= 0;

	// Init encoder one
	eos.value_old 			= 0;
	eos.value_new			= ENCODER_ONE_RANGE;
	eos.de_detent			= 0;

	// Init encoder two
	ews.value_old 			= 0;
	ews.value_new			= ENCODER_TWO_RANGE;
	ews.de_detent			= 0;

	// Init encoder three
	ets.value_old 			= 0;
	ets.value_new			= ENCODER_THR_RANGE;
	ets.de_detent			= 0;

	// Auto button blink state
	//abst.blink_flag 		= 0;
	//abst.blink_skip 		= 0;

	// SWR meter init
	swrm.skip 				= 0;
	swrm.p_curr				= 0;
	swrm.pwr_aver 			= 0;
	swrm.swr_aver 			= 0;

	// Power supply meter
	pwmt.skip 				= 0;
	pwmt.p_curr				= 0;
	pwmt.pwr_aver 			= 0;

	// LO tcxo
	lo.skip					= 0;
	lo.comp					= 0;
	lo.v1000000				= 0;
	lo.v100000				= 0;
	lo.v10000				= 0;
	lo.v1000				= 0;
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverProcessKeyboard
//* Object              : process hardcoded buttons click and hold
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverProcessKeyboard(void)
{
	if(ks.button_processed)
	{
		//printf("button process: %02x, debounce time: %d\n\r",ks.button_id,ks.debounce_time);

		// Is it click or hold ?
		if(ks.debounce_time < BUTTON_HOLD_TIME)
		{
			// Process click
			switch(ks.button_id)
			{
				// BUTTON_M2
				case 0:
					UiDriverChangeEncoderTwoMode(0);
					break;

				// BUTTON_G2
				case 1:
				{
					ts.power_level++;

					if(ts.power_level == PA_LEVEL_MAX_ENTRY)
						ts.power_level = PA_LEVEL_FULL;

					UiDriverChangePowerLevel();

					break;
				}

				// BUTTON_G2
				case 2:
				{
					ts.keyer_mode++;

					if(ts.keyer_mode == CW_MAX_MODE)
						ts.keyer_mode = CW_MODE_IAM_B;

					// Display mode
					UiDriverChangeKeyerMode();

					break;
				}

				// BUTTON_BNDM
				case 3:
					UiDriverChangeBand(0);
					break;

				// BUTTON_G4
				case 4:
				{
					ts.filter_id++;
					if(ts.filter_id == AUDIO_MAX_FIR)
						ts.filter_id = AUDIO_FIR_1P8KHZ;

					// Change filter
					UiDriverChangeFilter(0);

					break;
				}

				// BUTTON_M3
				case 5:
					UiDriverChangeEncoderThreeMode(0);
					break;

				// BUTTON_STEPM
				case 6:
					UiDriverChangeTunningStep(0);
					break;

				// BUTTON_STEPP
				case 7:
					UiDriverChangeTunningStep(1);
					break;

				// BUTTON_M1
				case 8:
					UiDriverChangeEncoderOneMode(0);
					break;

				// BUTTON_BNDP
				case 13:
					UiDriverChangeBand(1);
					break;

				// BUTTON_G1
				case 15:
					UiDriverChangeDemodMode();
					break;

				default:
					UiDriverProcessFunctionKeyClick(ks.button_id);
					break;
			}
		}
		else
		{
			// Process hold
			switch(ks.button_id)
			{
				default:
					break;
			}
		}

		// Reset flag, allow other buttons to be checked
		ks.button_processed = 0;
		ks.debounce_time	= 0;
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverProcessFunctionKeyClick
//* Object              : process function buttons click
//* Object              : based on current demod mode
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverProcessFunctionKeyClick(ulong id)
{
	//printf("button: %02x\n\r",id);

	// --------------------------------------------
	// F1 process
	if(id == 10)
	{
		if(!ts.calib_mode)	// normal mode
		{
			// Increase
			ts.tx_audio_source++;
			if(ts.tx_audio_source == TX_AUDIO_MAX_ITEMS)
				ts.tx_audio_source = TX_AUDIO_MIC;

			// Change button caption
			if(ts.tx_audio_source == TX_AUDIO_MIC)
				UiLcdHy28_PrintText(POS_BOTTOM_BAR_F1_X,POS_BOTTOM_BAR_F1_Y,"  MIC  ",White,Black,0);
			else
				UiLcdHy28_PrintText(POS_BOTTOM_BAR_F1_X,POS_BOTTOM_BAR_F1_Y,"  LINE ",White,Black,0);
		}
		else	// calibration mode F1 function
		{
			// Toggle
			ts.mic_boost = !ts.mic_boost;

			if(!ts.mic_boost)
				UiLcdHy28_PrintText(POS_BOTTOM_BAR_F1_X,POS_BOTTOM_BAR_F1_Y," MB OFF",White,Black,0);
			else
				UiLcdHy28_PrintText(POS_BOTTOM_BAR_F1_X,POS_BOTTOM_BAR_F1_Y," MB ON ",White,Black,0);
		}
	}

	// --------------------------------------------
	// F2 process
	if(id == 11)
	{
		// No need to process this menu if no chip avail
		if(lo.sensor_present)
			return;

		// Toggle soft tcxo
		df.temp_enabled = !df.temp_enabled;

		// Change button color
		if(df.temp_enabled)
		{
			// Button caption
			UiLcdHy28_PrintText(POS_BOTTOM_BAR_F2_X,POS_BOTTOM_BAR_F2_Y,"  TCXO",Blue,Black,0);

			// Indicator on
			UiDriverCreateTemperatureDisplay(1,0);
		}
		else
		{
			// Button caption
			UiLcdHy28_PrintText(POS_BOTTOM_BAR_F2_X,POS_BOTTOM_BAR_F2_Y,"  TCXO",White,Black,0);

			// Indicator off
			UiDriverCreateTemperatureDisplay(0,0);

			// Reset temp compensation value
			df.temp_factor = 0;
			UiDriverUpdateFrequencyFast();

			// Reset to default, so when turned off routine will work as expected
			lo.comp = 0;
		}
	}

	// --------------------------------------------
	// F3 process
	if(id == 9)
	{
		// Toggle state
		kd.enabled = !kd.enabled;

		// Change button color
		if(kd.enabled)
		{
			// Button caption
			UiLcdHy28_PrintText(POS_BOTTOM_BAR_F3_X,POS_BOTTOM_BAR_F3_Y,"  CAT ",Blue,Black,0);

			// Start driver
			cat_driver_init();
		}
		else
		{
			// Button caption
			UiLcdHy28_PrintText(POS_BOTTOM_BAR_F3_X,POS_BOTTOM_BAR_F3_Y,"  CAT ",White,Black,0);

			cat_driver_stop();
		}
	}

	// --------------------------------------------
	// F4 process
	if(id == 12)
	{
		// Toggle tune
		ts.tune = !ts.tune;

		// Change button color
		if(ts.tune)
		{
			// DDS on
			softdds_setfreq(CW_SIDETONE_OFFCET,ts.samp_rate,0);

			// To TX
			ts.txrx_mode = TRX_MODE_TX;
			ui_driver_toggle_tx();				// tune

			UiLcdHy28_PrintText(POS_BOTTOM_BAR_F4_X,POS_BOTTOM_BAR_F4_Y,"  TUNE",Red,Black,0);
		}
		else
		{
			// DDS off
			softdds_setfreq(0.0,ts.samp_rate,0);

			// Back to RX
			ts.txrx_mode = TRX_MODE_RX;
			ui_driver_toggle_tx();				// tune

			UiLcdHy28_PrintText(POS_BOTTOM_BAR_F4_X,POS_BOTTOM_BAR_F4_Y,"  TUNE",White,Black,0);
		}
	}

	// --------------------------------------------
	// F5 process
	if(id == 14)
	{
		// Toggle calibration mode
		ts.calib_mode = !ts.calib_mode;

		// Change button color
		if(ts.calib_mode)
		{
			UiLcdHy28_PrintText(POS_BOTTOM_BAR_F5_X,POS_BOTTOM_BAR_F5_Y,"  CAL ",Red,Black,0);

			// Show calibration controls
			UiDriverChangePaBias		(1,1);
			UiDriverChangeIQGainBalance	(1,1);
			UiDriverChangeIQPhaseBalance(1,1);

			// Disable other pots
			UiDriverChangeEncoderOneMode(1);
			UiDriverChangeEncoderTwoMode(1);
			UiDriverChangeEncoderThreeMode(1);

			// Change F1 caption in Calibration Mode
			if(!ts.mic_boost)
				UiLcdHy28_PrintText(POS_BOTTOM_BAR_F1_X,POS_BOTTOM_BAR_F1_Y," MB OFF",White,Black,0);
			else
				UiLcdHy28_PrintText(POS_BOTTOM_BAR_F1_X,POS_BOTTOM_BAR_F1_Y," MB ON ",White,Black,0);
		}
		else
		{
			ushort value;

			UiLcdHy28_PrintText(POS_BOTTOM_BAR_F5_X,POS_BOTTOM_BAR_F5_Y,"  CAL ",White,Black,0);

			// Hide calibration controls
			UiDriverChangePaBias		(0,0);
			UiDriverChangeIQGainBalance	(0,0);
			UiDriverChangeIQPhaseBalance(0,0);

			// Reset pots
			UiDriverChangeEncoderOneMode(0);
			UiDriverChangeEncoderTwoMode(0);
			UiDriverChangeEncoderThreeMode(0);

			// Restore F1 caption in Normal Mode
			if(ts.tx_audio_source == TX_AUDIO_MIC)
				UiLcdHy28_PrintText(POS_BOTTOM_BAR_F1_X,POS_BOTTOM_BAR_F1_Y,"  MIC  ",White,Black,0);
			else
				UiLcdHy28_PrintText(POS_BOTTOM_BAR_F1_X,POS_BOTTOM_BAR_F1_Y,"  LINE ",White,Black,0);

			// Save bias if changed
			if(EE_ReadVariable(VirtAddVarTab[EEPROM_PA_BIAS], &value) == 0)
			{
				if(ts.pa_bias != value)
				{
					EE_WriteVariable(VirtAddVarTab[EEPROM_PA_BIAS],ts.pa_bias);
					//printf("-->PA BIAS saved\n\r");
				}
			}

			// Save mic boost state if changed
			if(EE_ReadVariable(VirtAddVarTab[EEPROM_MIC_BOOST], &value) == 0)
			{
				if(ts.mic_boost != value)
				{
					EE_WriteVariable(VirtAddVarTab[EEPROM_MIC_BOOST],ts.mic_boost);
					//printf("-->MIC BOOST state saved\n\r");
				}
			}
		}
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverShowMode
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverShowMode(void)
{
	// Clear control
	UiLcdHy28_DrawFullRect(POS_DEMOD_MODE_MASK_X,POS_DEMOD_MODE_MASK_Y,POS_DEMOD_MODE_MASK_H,POS_DEMOD_MODE_MASK_W,Blue);

	// Create Decode Mode (USB/LSB/AM/FM/CW)
	switch(ts.dmod_mode)
	{
		case DEMOD_USB:
			UiLcdHy28_PrintText((POS_DEMOD_MODE_X + 8),POS_DEMOD_MODE_Y,"USB",Cream,Blue,0);
			break;
		case DEMOD_LSB:
			UiLcdHy28_PrintText((POS_DEMOD_MODE_X + 8),POS_DEMOD_MODE_Y,"LSB",Cream,Blue,0);
			break;
		case DEMOD_AM:
			UiLcdHy28_PrintText((POS_DEMOD_MODE_X + 12),POS_DEMOD_MODE_Y,"AM",Cream,Blue,0);
			break;
		case DEMOD_CW:
			UiLcdHy28_PrintText((POS_DEMOD_MODE_X + 12),POS_DEMOD_MODE_Y,"CW",Cream,Blue,0);
			break;
		default:
			break;
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverShowStep
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverShowStep(ulong step)
{
	// Clear control
	UiLcdHy28_DrawFullRect(POS_TUNE_STEP_MASK_X,POS_TUNE_STEP_MASK_Y,POS_TUNE_STEP_MASK_H,POS_TUNE_STEP_MASK_W,Black);

	// Create Step Mode
	switch(df.tunning_step)
	{
		case T_STEP_1HZ:
			UiLcdHy28_PrintText((POS_TUNE_STEP_X + SMALL_FONT_WIDTH*3),POS_TUNE_STEP_Y,"1Hz",White,Black,0);
			break;
		case T_STEP_10HZ:
			UiLcdHy28_PrintText((POS_TUNE_STEP_X + SMALL_FONT_WIDTH*2),POS_TUNE_STEP_Y,"10Hz",White,Black,0);
			break;
		case T_STEP_100HZ:
			UiLcdHy28_PrintText((POS_TUNE_STEP_X + SMALL_FONT_WIDTH*1),POS_TUNE_STEP_Y,"100Hz",White,Black,0);
			break;
		case T_STEP_1KHZ:
			UiLcdHy28_PrintText((POS_TUNE_STEP_X + SMALL_FONT_WIDTH*2),POS_TUNE_STEP_Y,"1kHz",White,Black,0);
			break;
		case T_STEP_10KHZ:
			UiLcdHy28_PrintText((POS_TUNE_STEP_X + SMALL_FONT_WIDTH*1),POS_TUNE_STEP_Y,"10kHz",White,Black,0);
			break;
		case T_STEP_100KHZ:
			UiLcdHy28_PrintText((POS_TUNE_STEP_X + SMALL_FONT_WIDTH*0),POS_TUNE_STEP_Y,"100kHz",White,Black,0);
			break;
		default:
			break;
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverShowBand
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverShowBand(uchar band)
{
	// Clear control
	UiLcdHy28_DrawFullRect(POS_BAND_MODE_MASK_X,POS_BAND_MODE_MASK_Y,POS_BAND_MODE_MASK_H,POS_BAND_MODE_MASK_W,Black);

	// Create Band value
	switch(band)
	{
		case BAND_MODE_80:
			UiLcdHy28_PrintText(POS_BAND_MODE_X,POS_BAND_MODE_Y,"80m",Orange,Black,0);
			break;

		case BAND_MODE_60:
			UiLcdHy28_PrintText(POS_BAND_MODE_X,POS_BAND_MODE_Y,"60m",Orange,Black,0);
			break;

		case BAND_MODE_40:
			UiLcdHy28_PrintText(POS_BAND_MODE_X,POS_BAND_MODE_Y,"40m",Orange,Black,0);
			break;

		case BAND_MODE_30:
			UiLcdHy28_PrintText(POS_BAND_MODE_X,POS_BAND_MODE_Y,"30m",Orange,Black,0);
			break;

		case BAND_MODE_20:
			UiLcdHy28_PrintText(POS_BAND_MODE_X,POS_BAND_MODE_Y,"20m",Orange,Black,0);
			break;

		case BAND_MODE_17:
			UiLcdHy28_PrintText(POS_BAND_MODE_X,POS_BAND_MODE_Y,"17m",Orange,Black,0);
			break;

		case BAND_MODE_15:
			UiLcdHy28_PrintText(POS_BAND_MODE_X,POS_BAND_MODE_Y,"15m",Orange,Black,0);
			break;

		case BAND_MODE_12:
			UiLcdHy28_PrintText(POS_BAND_MODE_X,POS_BAND_MODE_Y,"12m",Orange,Black,0);
			break;

		case BAND_MODE_10:
			UiLcdHy28_PrintText(POS_BAND_MODE_X,POS_BAND_MODE_Y,"10m",Orange,Black,0);
			break;

		default:
			break;
	}
}

// -------------------------------------------
// 	 BAND		BAND0		BAND1		BAND2
//
//	 80m		1			1			x
//	 40m		1			0			x
//	 20/30m		0			0			x
//	 15-10m		0			1			x
//
// -------------------------------------------
//
static void UiDriverChangeBandFilter(uchar band,uchar bpf_only)
{
	if(bpf_only)
		goto do_bpf;

	// ---------------------------------------------
	// Set LPFs
	// Set relays in groups, internal first, then external group
	// state change via two pulses on BAND2 line, then idle
	switch(band)
	{
		case BAND_MODE_80:
		{
			// Internal group - Set(High/Low)
			BAND0_PIO->BSRRL = BAND0;
			BAND1_PIO->BSRRH = BAND1;

			// Pulse relays
			BAND2_PIO->BSRRH = BAND2;
			non_os_delay();
			BAND2_PIO->BSRRL = BAND2;

			// External group -Set(High/High)
			BAND0_PIO->BSRRL = BAND0;
			BAND1_PIO->BSRRL = BAND1;

			// Pulse relays
			BAND2_PIO->BSRRH = BAND2;
			non_os_delay();
			BAND2_PIO->BSRRL = BAND2;

			break;
		}

		case BAND_MODE_60:
		case BAND_MODE_40:
		{
			// Internal group - Set(High/Low)
			BAND0_PIO->BSRRL = BAND0;
			BAND1_PIO->BSRRH = BAND1;

			// Pulse relays
			BAND2_PIO->BSRRH = BAND2;
			non_os_delay();
			BAND2_PIO->BSRRL = BAND2;

			// External group - Reset(Low/High)
			BAND0_PIO->BSRRH = BAND0;
			BAND1_PIO->BSRRL = BAND1;

			// Pulse relays
			BAND2_PIO->BSRRH = BAND2;
			non_os_delay();
			BAND2_PIO->BSRRL = BAND2;

			break;
		}

		case BAND_MODE_30:
		case BAND_MODE_20:
		{
			// Internal group - Reset(Low/Low)
			BAND0_PIO->BSRRH = BAND0;
			BAND1_PIO->BSRRH = BAND1;

			// Pulse relays
			BAND2_PIO->BSRRH = BAND2;
			non_os_delay();
			BAND2_PIO->BSRRL = BAND2;

			// External group - Reset(Low/High)
			BAND0_PIO->BSRRH = BAND0;
			BAND1_PIO->BSRRL = BAND1;

			// Pulse relays
			BAND2_PIO->BSRRH = BAND2;
			non_os_delay();
			BAND2_PIO->BSRRL = BAND2;

			break;
		}

		case BAND_MODE_17:
		case BAND_MODE_15:
		case BAND_MODE_12:
		case BAND_MODE_10:
		{
			// Internal group - Reset(Low/Low)
			BAND0_PIO->BSRRH = BAND0;
			BAND1_PIO->BSRRH = BAND1;

			// Pulse relays
			BAND2_PIO->BSRRH = BAND2;
			non_os_delay();
			BAND2_PIO->BSRRL = BAND2;

			// External group - Set(High/High)
			BAND0_PIO->BSRRL = BAND0;
			BAND1_PIO->BSRRL = BAND1;

			// Pulse relays
			BAND2_PIO->BSRRH = BAND2;
			non_os_delay();
			BAND2_PIO->BSRRL = BAND2;

			break;
		}

		default:
			break;
	}

do_bpf:

	// ---------------------------------------------
	// Set BPFs
	// Constant line states for the BPF filter,
	// always last - after LPF change
	switch(band)
	{
		case BAND_MODE_80:
		{
			BAND0_PIO->BSRRL = BAND0;
			BAND1_PIO->BSRRL = BAND1;
			break;
		}

		case BAND_MODE_60:
		case BAND_MODE_40:
		{
			BAND0_PIO->BSRRL = BAND0;
			BAND1_PIO->BSRRH = BAND1;
			break;
		}

		case BAND_MODE_30:
		case BAND_MODE_20:
		{
			BAND0_PIO->BSRRH = BAND0;
			BAND1_PIO->BSRRH = BAND1;
			break;
		}

		case BAND_MODE_17:
		case BAND_MODE_15:
		case BAND_MODE_12:
		case BAND_MODE_10:
		{
			BAND0_PIO->BSRRH = BAND0;
			BAND1_PIO->BSRRL = BAND1;
			break;
		}

		default:
			break;
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCreateDesktop
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverCreateDesktop(void)
{
	//char temp[10];

	// Backlight off - hide startup logo
	LCD_BACKLIGHT_PIO->BSRRH = LCD_BACKLIGHT;

	// Clear display
	UiLcdHy28_LcdClear(Black);

	// Create Band value
	UiDriverShowBand(ts.band_mode);

	// Set filters
	UiDriverChangeBandFilter(ts.band_mode,0);

	// Create Decode Mode (USB/LSB/AM/FM/CW)
	UiDriverShowMode();

	// Create Step Mode
	UiDriverShowStep(df.tunning_step);

	// Frequency
	UiLcdHy28_PrintText(POS_TUNE_FREQ_X,POS_TUNE_FREQ_Y,"14.000.000",White,Black,1);

	// Second Frequency
	UiLcdHy28_PrintText(POS_TUNE_SFREQ_X,POS_TUNE_SFREQ_Y,"14.000.000",Grey,Black,0);

	// Function buttons
	UiDriverCreateFunctionButtons(true);

	// S-meter
	UiDriverCreateSMeter();

	// Spectrum scope
	UiDriverCreateSpectrumScope();

	// -----------------
	// Encoder one modes
	// -----------------
	//  Audio gain
	UiDriverChangeAfGain(1);
	// Sidetone gain
	UiDriverChangeStGain(0);

	// -----------------
	// Encoder two modes
	// -----------------
	// RF gain
	UiDriverChangeRfGain(1);
	// RF Attenuator
	UiDriverChangeRfAttenuator(0);

	// -----------------
	// Encoder three modes
	// -----------------
	// RIT
	UiDriverChangeRit(1);
	// Keyer speed
	UiDriverChangeKeyerSpeed(0);

	// -----------------
	// Calib mode
	// -----------------
	// IQ balance
//	UiDriverChangeIQGainBalance(0);
	// IQ balance
//	UiDriverChangeIQPhaseBalance(0);
	// PA bias
//	UiDriverChangePaBias(0);


	// Keyer mode
	UiDriverChangeKeyerMode();

	// Power level
	UiDriverChangePowerLevel();

	// FIR via keypad, not encoder mode
	UiDriverChangeFilter(1);

	// Create USB Keyboard indicator
	//UiLcdHy28_PrintText(POS_KBD_IND_X,POS_KBD_IND_Y,"KBD",Grey,Black,0);

	// Create RX/TX indicator
	//UiLcdHy28_PrintText(POS_TX_IND_X,POS_TX_IND_Y,	"RX", Green,Black,0);

	// Create voltage
	UiLcdHy28_DrawStraightLine	(POS_PWRN_IND_X,(POS_PWRN_IND_Y - 1),56,LCD_DIR_HORIZONTAL,Grey);
	UiLcdHy28_PrintText			(POS_PWRN_IND_X, POS_PWRN_IND_Y,"  VCC  ", Grey, 	Blue, 0);
	UiLcdHy28_PrintText			(POS_PWR_IND_X,POS_PWR_IND_Y,   "12.00V",  COL_PWR_IND,Black,0);

	// Create temperature
	if((lo.sensor_present == 0) && (df.temp_enabled))
		UiDriverCreateTemperatureDisplay(1,1);
	else
		UiDriverCreateTemperatureDisplay(0,1);

	// Set correct frequency
	UiDriverUpdateFrequency(1);

	// Backlight on - only when all is drawn
	LCD_BACKLIGHT_PIO->BSRRL = LCD_BACKLIGHT;
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCreateFunctionButtons
//* Object              : function keys based on decoder mode
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverCreateFunctionButtons(bool full_repaint)
{
	char cap1[20],cap2[20],cap3[20],cap4[20],cap5[20];

	// Create bottom bar
	if(full_repaint)
	{
		UiLcdHy28_DrawBottomButton((POS_BOTTOM_BAR_X +                             0),(POS_BOTTOM_BAR_Y - 4),POS_BOTTOM_BAR_BUTTON_H,POS_BOTTOM_BAR_BUTTON_W,Grey);
		UiLcdHy28_DrawBottomButton((POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*1 + 2),(POS_BOTTOM_BAR_Y - 4),POS_BOTTOM_BAR_BUTTON_H,POS_BOTTOM_BAR_BUTTON_W,Grey);
		UiLcdHy28_DrawBottomButton((POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*2 + 4),(POS_BOTTOM_BAR_Y - 4),POS_BOTTOM_BAR_BUTTON_H,POS_BOTTOM_BAR_BUTTON_W,Grey);
		UiLcdHy28_DrawBottomButton((POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*3 + 6),(POS_BOTTOM_BAR_Y - 4),POS_BOTTOM_BAR_BUTTON_H,POS_BOTTOM_BAR_BUTTON_W,Grey);
		UiLcdHy28_DrawBottomButton((POS_BOTTOM_BAR_X + POS_BOTTOM_BAR_BUTTON_W*4 + 8),(POS_BOTTOM_BAR_Y - 4),POS_BOTTOM_BAR_BUTTON_H,(POS_BOTTOM_BAR_BUTTON_W + 1),Grey);
	}

	// Change button caption
	if(ts.tx_audio_source == TX_AUDIO_MIC)
		strcpy(cap1,"  MIC ");
	else
		strcpy(cap1,"  LINE");

	strcpy(cap2,"  TCXO");
	strcpy(cap3,"  CAT ");
	strcpy(cap4,"  TUNE");
	strcpy(cap5,"  CAL ");

	// Draw buttons text
	UiLcdHy28_PrintText(POS_BOTTOM_BAR_F1_X,POS_BOTTOM_BAR_F1_Y,cap1,White,Black,0);

	if((lo.sensor_present == 0) && (df.temp_enabled))
		UiLcdHy28_PrintText(POS_BOTTOM_BAR_F2_X,POS_BOTTOM_BAR_F2_Y,cap2,Blue,Black,0); // enabled
	else
		UiLcdHy28_PrintText(POS_BOTTOM_BAR_F2_X,POS_BOTTOM_BAR_F2_Y,cap2,White,Black,0);

	UiLcdHy28_PrintText(POS_BOTTOM_BAR_F3_X,POS_BOTTOM_BAR_F3_Y,cap3,White,Black,0);
	UiLcdHy28_PrintText(POS_BOTTOM_BAR_F4_X,POS_BOTTOM_BAR_F4_Y,cap4,White,Black,0);
	UiLcdHy28_PrintText(POS_BOTTOM_BAR_F5_X,POS_BOTTOM_BAR_F5_Y,cap5,White,Black,0);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCreateSMeter
//* Object              : draw the S meter
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverCreateSMeter(void)
{
	uchar 	i,v_s;
	char	num[20];
	int		col;

	// W/H ratio ~ 3.5
	UiLcdHy28_DrawEmptyRect(POS_SM_IND_X,POS_SM_IND_Y,72,202,Grey);

	// Draw top line
	UiLcdHy28_DrawStraightLine((POS_SM_IND_X +  18),(POS_SM_IND_Y + 20),92,LCD_DIR_HORIZONTAL,White);
	UiLcdHy28_DrawStraightLine((POS_SM_IND_X +  18),(POS_SM_IND_Y + 21),92,LCD_DIR_HORIZONTAL,White);
	UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 113),(POS_SM_IND_Y + 20),75,LCD_DIR_HORIZONTAL,Green);
	UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 113),(POS_SM_IND_Y + 21),75,LCD_DIR_HORIZONTAL,Green);

	// Leading text
	UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 12),(POS_SM_IND_Y +  5),"S",  White,Black,4);
	UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 12),(POS_SM_IND_Y + 36),"P",  White,Black,4);
	UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 12),(POS_SM_IND_Y + 59),"SWR",White,Black,4);

	// Trailing text
	UiLcdHy28_PrintText((POS_SM_IND_X + 185),(POS_SM_IND_Y + 5), "dB",Green,Black,4);
	UiLcdHy28_PrintText((POS_SM_IND_X + 185),(POS_SM_IND_Y + 36)," W",White,Black,4);

	// Draw s markers on top white line
	for(i = 0; i < 10; i++)
	{
		num[0] = i + 0x30;
		num[1] = 0;

		// Draw s text, only odd numbers
		if(i%2)
		{
			UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 4 + i*10),(POS_SM_IND_Y + 5),num,White,Black,4);
			v_s = 5;
		}
		else
			v_s = 3;

		// Lines
		UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 20) - v_s),v_s,LCD_DIR_VERTICAL,White);
		UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 19) + i*10),((POS_SM_IND_Y + 20) - v_s),v_s,LCD_DIR_VERTICAL,White);
	}

	// Draw s markers on top red line
	for(i = 0; i < 4; i++)
	{
		// Prepare text
		num[0] = i*2 + 0x30;
		num[1] = 0x30;
		num[2] = 0x00;

		if(i)
		{
			// Draw text
			UiLcdHy28_PrintText(((POS_SM_IND_X + 113) - 6 + i*20),(POS_SM_IND_Y + 5),num,Green,Black,4);

			// Draw vert lines
			UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 113) + i*20),(POS_SM_IND_Y + 15),5,LCD_DIR_VERTICAL,Green);
			UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 114) + i*20),(POS_SM_IND_Y + 15),5,LCD_DIR_VERTICAL,Green);
		}
	}

	// Draw middle line
	UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 18),(POS_SM_IND_Y + 32),170,LCD_DIR_HORIZONTAL,White);
	UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 18),(POS_SM_IND_Y + 33),170,LCD_DIR_HORIZONTAL,White);

	// Draw s markers on middle white line
	for(i = 0; i < 12; i++)
	{
		if(i < 10)
		{
			num[0] = i + 0x30;
			num[1] = 0;
		}
		else
		{
			num[0] = i/10 + 0x30;
			num[1] = i%10 + 0x30;
			num[2] = 0;
		}

		// Draw s text, only odd numbers
		if(!(i%2))
		{
			// Text
			UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 3 + i*15),(POS_SM_IND_Y + 36),num,White,Black,4);

			// Lines
			if(i)
			{
				UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 18) + i*15),((POS_SM_IND_Y + 32) - 2),2,LCD_DIR_VERTICAL,White);
				UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 19) + i*15),((POS_SM_IND_Y + 32) - 2),2,LCD_DIR_VERTICAL,White);
			}
			else
			{
				UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 18) + i*15),((POS_SM_IND_Y + 32) - 7),7,LCD_DIR_VERTICAL,White);
				UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 19) + i*15),((POS_SM_IND_Y + 32) - 7),7,LCD_DIR_VERTICAL,White);
			}
		}
	}

	// Draw bottom line
	UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 18),(POS_SM_IND_Y + 55), 62,LCD_DIR_HORIZONTAL,White);
	UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 18),(POS_SM_IND_Y + 56), 62,LCD_DIR_HORIZONTAL,White);
	UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 83),(POS_SM_IND_Y + 55),105,LCD_DIR_HORIZONTAL,Red);
	UiLcdHy28_DrawStraightLine((POS_SM_IND_X + 83),(POS_SM_IND_Y + 56),105,LCD_DIR_HORIZONTAL,Red);
	col = White;

	// Draw S markers on middle white line
	for(i = 0; i < 12; i++)
	{
		if(i > 6) col = Red;

		if(!(i%2))
		{
			if(i)
			{
				num[0] = i/2 + 0x30;
				num[1] = 0;

				// Text
				UiLcdHy28_PrintText(((POS_SM_IND_X + 18) - 3 + i*10),(POS_SM_IND_Y + 59),num,White,Black,4);

				UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 55) - 2),2,LCD_DIR_VERTICAL,col);
				UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 19) + i*10),((POS_SM_IND_Y + 55) - 2),2,LCD_DIR_VERTICAL,col);
			}
			else
			{
				UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 18) + i*10),((POS_SM_IND_Y + 55) - 7),7,LCD_DIR_VERTICAL,col);
				UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 19) + i*10),((POS_SM_IND_Y + 55) - 7),7,LCD_DIR_VERTICAL,col);
			}
		}
	}

	// Draw meters
	UiDriverUpdateTopMeterA(0,0);
	UiDriverUpdateBtmMeter(0);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateTopMeter
//* Object              : redraw indicator
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
/*static void UiDriverUpdateTopMeter(uchar val,uchar old)
{
	uchar 	i,v_s;
	int		col;

	// Do not waste time redrawing if outside of the range
	if(val > S_METER_MAX)
		return;

	// Indicator height
	v_s = 3;

	if(ts.txrx_mode == TRX_MODE_TX)
	{
		col = Blue;

		// Draw first indicator
		for(i = 1; i < 34; i++)
		{
			if(val < i)
				col = Grey;

			// Lines
			UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 18) + i*5),((POS_SM_IND_Y + 28) - v_s),v_s,LCD_DIR_VERTICAL,col);
			UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 19) + i*5),((POS_SM_IND_Y + 28) - v_s),v_s,LCD_DIR_VERTICAL,col);
			UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 20) + i*5),((POS_SM_IND_Y + 28) - v_s),v_s,LCD_DIR_VERTICAL,col);
		}

		return;
	}

	if((val == 0) && (old == 0))
	{
		col = Grey;

		// Draw/Clear indicator
		for(i = 1; i < S_METER_MAX; i++)
		{
			// Lines
			UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 18) + i*5),((POS_SM_IND_Y + 28) - v_s),v_s,LCD_DIR_VERTICAL,col);
			UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 19) + i*5),((POS_SM_IND_Y + 28) - v_s),v_s,LCD_DIR_VERTICAL,col);
			UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 20) + i*5),((POS_SM_IND_Y + 28) - v_s),v_s,LCD_DIR_VERTICAL,col);
		}
	}

	// Updating in UP direction
	if(val > old)
	{
		col = Blue;

		for(i = 1; i < val; i++)
		{
			if(i > S_METER_MAX)
				return;

			// Lines
			UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 18) + i*5),((POS_SM_IND_Y + 28) - v_s),v_s,LCD_DIR_VERTICAL,col);
			UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 19) + i*5),((POS_SM_IND_Y + 28) - v_s),v_s,LCD_DIR_VERTICAL,col);
			UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 20) + i*5),((POS_SM_IND_Y + 28) - v_s),v_s,LCD_DIR_VERTICAL,col);
		}
	}

	// Updating in DOWN direction
	if(val < old)
	{
		col = Grey;

		for(i = old; i > val; i--)
		{
			if(i == 0)
				break;

			if(i > S_METER_MAX)
				return;

			// Lines
			UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 18) + i*5),((POS_SM_IND_Y + 28) - v_s),v_s,LCD_DIR_VERTICAL,col);
			UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 19) + i*5),((POS_SM_IND_Y + 28) - v_s),v_s,LCD_DIR_VERTICAL,col);
			UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 20) + i*5),((POS_SM_IND_Y + 28) - v_s),v_s,LCD_DIR_VERTICAL,col);
		}
	}
}*/

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateTopMeterA
//* Object              : redraw indicator, same like upper implementation
//* Input Parameters    : but no hold
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverUpdateTopMeterA(uchar val,uchar old)
{
	uchar 	i,v_s;
	int		col = Blue;

	// Do not waste time redrawing if outside of the range
	if(val > 34)
		return;

	// Indicator height
	v_s = 3;

	// Draw first indicator
	for(i = 1; i < 34; i++)
	{
		if(val < i)
			col = Grey;

		// Lines
		UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 18) + i*5),((POS_SM_IND_Y + 28) - v_s),v_s,LCD_DIR_VERTICAL,col);
		UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 19) + i*5),((POS_SM_IND_Y + 28) - v_s),v_s,LCD_DIR_VERTICAL,col);
		UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 20) + i*5),((POS_SM_IND_Y + 28) - v_s),v_s,LCD_DIR_VERTICAL,col);
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateBtmMeter
//* Object              : redraw indicator
//* Input Parameters    :
//* Output Parameters   : ToDo: Old value public, so to skip refresh!!!
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverUpdateBtmMeter(uchar val)
{
	uchar 	i,v_s;
	int		col = Blue;

	// Do not waste time redrawing if outside of the range
	if(val > 34)
		return;

	// Indicator height
	v_s = 3;

	// Draw first indicator
	for(i = 1; i < 34; i++)
	{
		if(val < i)
			col = Grey;

		// Lines
		UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 18) + i*5),((POS_SM_IND_Y + 51) - v_s),v_s,LCD_DIR_VERTICAL,col);
		UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 19) + i*5),((POS_SM_IND_Y + 51) - v_s),v_s,LCD_DIR_VERTICAL,col);
		UiLcdHy28_DrawStraightLine(((POS_SM_IND_X + 20) + i*5),((POS_SM_IND_Y + 51) - v_s),v_s,LCD_DIR_VERTICAL,col);
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCreateSpectrumScope
//* Object              : draw the spectrum scope control
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverCreateSpectrumScope(void)
{
	ulong i;

	// Draw top band
	for(i = 0; i < 16; i++)
		UiLcdHy28_DrawHorizLineWithGrad(POS_SPECTRUM_IND_X,(POS_SPECTRUM_IND_Y - 20 + i),POS_SPECTRUM_IND_W,COL_SPECTRUM_GRAD);

	// Top band text - middle caption
	UiLcdHy28_PrintText(			(POS_SPECTRUM_IND_X + 72),
									(POS_SPECTRUM_IND_Y - 18),
									"SPECTRUM SCOPE",
									Grey,
									RGB((COL_SPECTRUM_GRAD*2),(COL_SPECTRUM_GRAD*2),(COL_SPECTRUM_GRAD*2)),0);

	// Top band text - grid size
	//UiLcdHy28_PrintText(			(POS_SPECTRUM_IND_X +  2),
	//								(POS_SPECTRUM_IND_Y - 18),
	//								"Grid 6k",
	//								Grey,
	//								RGB((COL_SPECTRUM_GRAD),(COL_SPECTRUM_GRAD),(COL_SPECTRUM_GRAD)),4);

	// Draw control left and right border
	for(i = 0; i < 2; i++)
	{
		UiLcdHy28_DrawStraightLine(	(POS_SPECTRUM_IND_X - 2 + i),
									(POS_SPECTRUM_IND_Y - 20),
									(POS_SPECTRUM_IND_H + 12),
									LCD_DIR_VERTICAL,
									RGB(COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD));

		UiLcdHy28_DrawStraightLine(	(POS_SPECTRUM_IND_X + POS_SPECTRUM_IND_W - 2 + i),
									(POS_SPECTRUM_IND_Y - 20),
									(POS_SPECTRUM_IND_H + 12),
									LCD_DIR_VERTICAL,
									RGB(COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD));
	}

	// Frequency bar separator
	UiLcdHy28_DrawHorizLineWithGrad(POS_SPECTRUM_IND_X,(POS_SPECTRUM_IND_Y + POS_SPECTRUM_IND_H - 20),POS_SPECTRUM_IND_W,COL_SPECTRUM_GRAD);

	// Frequency bar text
	UiLcdHy28_PrintText((POS_SPECTRUM_IND_X +   1),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_IND_H - 17),"24",Grey,Black,4);
	UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 239),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_IND_H - 17),"24",Grey,Black,4);
	UiLcdHy28_PrintText((POS_SPECTRUM_IND_X +  58),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_IND_H - 17),"12",Grey,Black,4);
	UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 186),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_IND_H - 17),"12",Grey,Black,4);
	UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 126),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_IND_H - 17), "0",Grey,Black,4);

	// Horizontal grid lines
	for(i = 1; i < 4; i++)
	{
		// Save y position for repaint
		sd.horz_grid_id[i - 1] = (POS_SPECTRUM_IND_Y - 5 + i*16);

		// Draw
		UiLcdHy28_DrawStraightLine(	POS_SPECTRUM_IND_X,
									sd.horz_grid_id[i - 1],
									POS_SPECTRUM_IND_W,
									LCD_DIR_HORIZONTAL,
									RGB((COL_SPECTRUM_GRAD),(COL_SPECTRUM_GRAD),(COL_SPECTRUM_GRAD)));

		//printf("vy: %d\n\r",sd.horz_grid_id[i - 1]);
	}

	// Vertical grid lines
	for(i = 1; i < 8; i++)
	{
		// Save x position for repaint
		sd.vert_grid_id[i - 1] = (POS_SPECTRUM_IND_X + 32*i + 1);

		// Draw
		UiLcdHy28_DrawStraightLine(	sd.vert_grid_id[i - 1],
									(POS_SPECTRUM_IND_Y -  4),
									(POS_SPECTRUM_IND_H - 15),
									LCD_DIR_VERTICAL,
									RGB((COL_SPECTRUM_GRAD),(COL_SPECTRUM_GRAD),(COL_SPECTRUM_GRAD)));

		//printf("vx: %d\n\r",sd.vert_grid_id[i - 1]);
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverRepaintSpectrumScope
//* Object              : clear drawing part
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
/*static void UiDriverRepaintSpectrumScope(void)
{
	ulong i;

	UiLcdHy28_DrawFullRect(POS_SPECTRUM_IND_X,POS_SPECTRUM_IND_Y - 4,POS_SPECTRUM_IND_H - 17,POS_SPECTRUM_IND_W - 2,Black);

	// Horizontal grid lines
	for(i = 1; i < 4; i++)
	{
		// Save y position for repaint
		sd.horz_grid_id[i - 1] = (POS_SPECTRUM_IND_Y - 5 + i*16);

		// Draw
		UiLcdHy28_DrawStraightLine(	POS_SPECTRUM_IND_X,
									sd.horz_grid_id[i - 1],
									POS_SPECTRUM_IND_W,
									LCD_DIR_HORIZONTAL,
									RGB((COL_SPECTRUM_GRAD),(COL_SPECTRUM_GRAD),(COL_SPECTRUM_GRAD)));

		//printf("vy: %d\n\r",sd.horz_grid_id[i - 1]);
	}

	// Vertical grid lines
	for(i = 1; i < 8; i++)
	{
		// Save x position for repaint
		sd.vert_grid_id[i - 1] = (POS_SPECTRUM_IND_X + 32*i + 1);

		// Draw
		UiLcdHy28_DrawStraightLine(	sd.vert_grid_id[i - 1],
									(POS_SPECTRUM_IND_Y -  4),
									(POS_SPECTRUM_IND_H - 15),
									LCD_DIR_VERTICAL,
									RGB((COL_SPECTRUM_GRAD),(COL_SPECTRUM_GRAD),(COL_SPECTRUM_GRAD)));

		//printf("vx: %d\n\r",sd.vert_grid_id[i - 1]);
	}

	// Frequency bar separator
	UiLcdHy28_DrawHorizLineWithGrad(POS_SPECTRUM_IND_X,(POS_SPECTRUM_IND_Y + POS_SPECTRUM_IND_H - 20),POS_SPECTRUM_IND_W,COL_SPECTRUM_GRAD);

	// Frequency bar text
	UiLcdHy28_PrintText((POS_SPECTRUM_IND_X +   1),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_IND_H - 17),"24",Grey,Black,4);
	UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 239),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_IND_H - 17),"24",Grey,Black,4);
	UiLcdHy28_PrintText((POS_SPECTRUM_IND_X +  58),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_IND_H - 17),"12",Grey,Black,4);
	UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 186),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_IND_H - 17),"12",Grey,Black,4);
	UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 126),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_IND_H - 17), "0",Grey,Black,4);
}*/

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCreateDigiPanel
//* Object              : draw the digital modes info panel
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
/*static void UiDriverCreateDigiPanel(void)
{
	ulong i;

	// Draw top band
	for(i = 0; i < 16; i++)
		UiLcdHy28_DrawHorizLineWithGrad(POS_SPECTRUM_IND_X,(POS_SPECTRUM_IND_Y - 20 + i),POS_SPECTRUM_IND_W,COL_SPECTRUM_GRAD);

	// Top band text - middle caption
	UiLcdHy28_PrintText(			(POS_SPECTRUM_IND_X + 85),
									(POS_SPECTRUM_IND_Y - 18),
									"DIGI PANEL",
									Grey,
									RGB((COL_SPECTRUM_GRAD*2),(COL_SPECTRUM_GRAD*2),(COL_SPECTRUM_GRAD*2)),0);

	// Draw control left and right border
	for(i = 0; i < 2; i++)
	{
		UiLcdHy28_DrawStraightLine(	(POS_SPECTRUM_IND_X - 2 + i),
									(POS_SPECTRUM_IND_Y - 20),
									(POS_SPECTRUM_IND_H + 12),
									LCD_DIR_VERTICAL,
									RGB(COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD));

		UiLcdHy28_DrawStraightLine(	(POS_SPECTRUM_IND_X + POS_SPECTRUM_IND_W - 2 + i),
									(POS_SPECTRUM_IND_Y - 20),
									(POS_SPECTRUM_IND_H + 12),
									LCD_DIR_VERTICAL,
									RGB(COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD));
	}

	// Clear old spectrum part + frequency bar
	UiLcdHy28_DrawFullRect(POS_SPECTRUM_IND_X,POS_SPECTRUM_IND_Y - 4,POS_SPECTRUM_IND_H - 2,POS_SPECTRUM_IND_W - 2,Black);
}*/

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverInitFrequency
//* Object              : set default values, some could be overwritten later
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverInitFrequency(void)
{
	ulong i;

	// Clear band values array
	for(i = 0; i < MAX_BANDS; i++)
	{
		band_dial_value[i] = 0xFFFFFFFF;	// clear dial values
		band_decod_mode[i] = DEMOD_USB; 	// clear decode mode
	}

	// Lower bands default to LSB mode
	for(i = 0; i < 4; i++)
		band_decod_mode[i] = DEMOD_LSB;

	// Init frequency publics(set diff values so update on LCD will be done)
	df.value_old	= 0;
	df.value_new	= 0;
	df.tune_old 	= tune_bands[ts.band_mode];
	df.tune_new 	= tune_bands[ts.band_mode];
	df.selected_idx = 3; 		// 1 Khz startup step
	df.tunning_step	= tune_steps[df.selected_idx];
	df.update_skip	= 0;		// skip value to compensate for fast dial rotation - test!!!
	df.temp_factor	= 0;
	df.temp_enabled = 0;		// startup state of TCXO

	//if(ts.band_mode == BAND_MODE_4)
	//	df.transv_freq = TRANSVT_FREQ_A;
	//else
	df.transv_freq	= 0;	// LO freq, zero on HF, 42 Mhz on 4m

	//df.tx_shift		= 0;		// offcet fo tx
	df.de_detent	= 0;

	// Set virtual segments initial value (diff than zero!)
	df.dial_010_mhz	= 1;
	df.dial_001_mhz	= 4;
	df.dial_100_khz	= 0;
	df.dial_010_khz	= 0;
	df.dial_001_khz	= 0;
	df.dial_100_hz	= 0;
	df.dial_010_hz	= 0;
	df.dial_001_hz	= 0;
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateFrequency
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverUpdateFrequency(char skip_encoder_check)
{
	ulong		loc_tune_new,dial_freq,tune_freq;
	//uchar		old_rf_gain = ts.rf_gain;
	ushort		col = White;

	// On band change we don't have to check the encoder
	if(skip_encoder_check)
		goto skip_check;

	// Check encoder
	if(!UiDriverCheckFrequencyEncoder())
		return;

skip_check:

	// Mute Audio - codec or rf gain
	//Codec_Volume(0);					- old code
	Codec_Mute(1);

	// Get value, while blocking update
	loc_tune_new = df.tune_new;

	// Calculate display frequency
	dial_freq = ((loc_tune_new/4) + df.transv_freq);

	// Clear not used segments on display frequency
	dial_freq /= df.tunning_step;
	dial_freq *= df.tunning_step;

	// Calculate actual tune frequency
	tune_freq = (dial_freq - df.transv_freq)*4;

	// Frequency range check, moved from si570 routine here
	if((tune_freq > SI570_MAX_FREQ) || (tune_freq < SI570_MIN_FREQ))
	{
		Codec_Mute(0);

		//printf("out of freq err: %d\n\r",tune_freq);
		df.tune_new = df.tune_old;						// reload old value
		return;
	}

	// Extra tunning actions
	if(ts.txrx_mode == TRX_MODE_RX)
	{
		// Add RIT on receive
		tune_freq += (ts.rit_value*80);
	}
	else
	{
		// Substract CW offcet on TX
		if(ts.dmod_mode == DEMOD_CW)
			tune_freq -= CW_SIDETONE_OFFCET;
	}

	//printf("--------------------\n\r");
	//printf("dial: %dHz, tune: %dHz\n\r",dial_freq,tune_freq);

	// Set frequency
	if(ui_si570_set_frequency(tune_freq,CALIB_FREQ,df.temp_factor))
	{
		// Color in red
		col = Red;
	}

	// Update LCD
	UiDriverUpdateLcdFreq(dial_freq,col);

	// Display second freq if RIT on or in TX mode with shift
	//if((ts.rit_value != 0) || (df.tx_shift != 0))
	//	UiDriverUpdateSecondLcdFreq(tune_freq/4 + df.tx_shift);
	if(ts.rit_value != 0)
		UiDriverUpdateSecondLcdFreq(tune_freq/4);

	// Allow clear of spectrum display in its state machine
	sd.dial_moved = 1;

	// Save current freq
	df.tune_old = loc_tune_new;

	// Save the tuning step used during the last dial update
	// - really important so we know what segments to clear
	// during tune step change
	df.last_tune_step = df.tunning_step;

	// Save to Eeprom
	//TRX4M_VE_WriteFreq(loc_tune_new);

	// Unmute Audio
	// Update codec volume
	//  0 - 10: via codec command
	// 10 - 20: soft gain after decoder
	//if(ts.audio_gain < 10)
	//Codec_Volume((ts.audio_gain*8)); - old code
	Codec_Mute(0);

	// Prevent eeprom save on fast knob rotation
	// by resetting its flag
	es.skip = 0;
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateFrequencyFast
//* Object              : like upper, but no UI update
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverUpdateFrequencyFast(void)
{
	ulong		loc_tune_new,dial_freq,tune_freq;

	// Get value, while blocking update
	loc_tune_new = df.tune_new;

	// Calculate display frequency
	dial_freq = ((loc_tune_new/4) + df.transv_freq);

	// Clear not used segments on display frequency
	dial_freq /= df.tunning_step;
	dial_freq *= df.tunning_step;

	// Calculate actual tune frequency
	tune_freq = (dial_freq - df.transv_freq)*4;

	// Extra tunning actions
	if(ts.txrx_mode == TRX_MODE_RX)
	{
		// Add RIT on receive
		tune_freq += (ts.rit_value*80);
	}
	else
	{
		// Substract CW offset on TX
		if(ts.dmod_mode == DEMOD_CW)
			tune_freq -= CW_SIDETONE_OFFCET;
	}

	//printf("--------------------\n\r");
	//printf("dial: %dHz, tune: %dHz\n\r",dial_freq,tune_freq);

	// Set frequency
	ui_si570_set_frequency(tune_freq,CALIB_FREQ,df.temp_factor);

	// Allow clear of spectrum display in its state machine
	sd.dial_moved = 1;

	// Save current freq
	df.tune_old = loc_tune_new;

	// Save the tunning step used during the last dial update
	// - really important so we know what segments to clear
	// during tune step change
	df.last_tune_step = df.tunning_step;

	// Prevent eeprom save on fast knob rotation
	// by resetting its flag
	es.skip = 0;
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateLcdFreq
//* Object              : this function will split LCD freq display control
//* Object              : and update as it is 7 segments indicator
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverUpdateLcdFreq(ulong dial_freq,ushort color)
{
	uchar		d_10mhz,d_1mhz;
	uchar		d_100khz,d_10khz,d_1khz;
	uchar		d_100hz,d_10hz,d_1hz;

	char		digit[2];

	// Terminate
	digit[1] = 0;

	//printf("--------------------\n\r");
	//printf("dial: %dHz\n\r",dial_freq);
	//printf("dial_001_mhz: %d\n\r",df.dial_001_mhz);
	//printf("dial_100_khz: %d\n\r",df.dial_100_khz);
	//printf("dial_010_khz: %d\n\r",df.dial_010_khz);
	//printf("dial_001_khz: %d\n\r",df.dial_001_khz);
	//printf("dial_100_hz:  %d\n\r",df.dial_100_hz);
	//printf("dial_010_hz:  %d\n\r",df.dial_010_hz);
	//printf("dial_001_hz:  %d\n\r",df.dial_001_hz);

	// -----------------------
	// See if 10 Mhz needs update
	d_10mhz = (dial_freq/10000000);
	if(d_10mhz != df.dial_010_mhz)
	{
		//printf("10 mhz diff: %d\n\r",d_10mhz);

		// To string
		digit[0] = 0x30 + (d_10mhz & 0x0F);

		// Update segment
		if(d_10mhz)
			UiLcdHy28_PrintText((POS_TUNE_FREQ_X + 0),POS_TUNE_FREQ_Y,digit,color,Black,1);
		else
			UiLcdHy28_PrintText((POS_TUNE_FREQ_X + 0),POS_TUNE_FREQ_Y,digit,Black,Black,1);	// mask the zero

		// Save value
		df.dial_010_mhz = d_10mhz;
	}

	// -----------------------
	// See if 1 Mhz needs update
	d_1mhz = (dial_freq%10000000)/1000000;
	if(d_1mhz != df.dial_001_mhz)
	{
		//printf("1 mhz diff: %d\n\r",d_1mhz);

		// To string
		digit[0] = 0x30 + (d_1mhz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_TUNE_FREQ_X + LARGE_FONT_WIDTH),POS_TUNE_FREQ_Y,digit,color,Black,1);

		// Save value
		df.dial_001_mhz = d_1mhz;
	}

	// -----------------------
	// See if 100 khz needs update
	d_100khz = (dial_freq%1000000)/100000;
	if(d_100khz != df.dial_100_khz)
	{
		//printf("100 khz diff: %d\n\r",d_100khz);

		// To string
		digit[0] = 0x30 + (d_100khz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_TUNE_FREQ_X + LARGE_FONT_WIDTH*3),POS_TUNE_FREQ_Y,digit,color,Black,1);

		// Save value
		df.dial_100_khz = d_100khz;
	}

	// -----------------------
	// See if 10 khz needs update
	d_10khz = (dial_freq%100000)/10000;
	if(d_10khz != df.dial_010_khz)
	{
		//printf("10 khz diff: %d\n\r",d_10khz);

		// To string
		digit[0] = 0x30 + (d_10khz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_TUNE_FREQ_X + LARGE_FONT_WIDTH*4),POS_TUNE_FREQ_Y,digit,color,Black,1);

		// Save value
		df.dial_010_khz = d_10khz;
	}

	// -----------------------
	// See if 1 khz needs update
	d_1khz = (dial_freq%10000)/1000;
	if(d_1khz != df.dial_001_khz)
	{
		//printf("1 khz diff: %d\n\r",d_1khz);

		// To string
		digit[0] = 0x30 + (d_1khz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_TUNE_FREQ_X + LARGE_FONT_WIDTH*5),POS_TUNE_FREQ_Y,digit,color,Black,1);

		// Save value
		df.dial_001_khz = d_1khz;
	}

	// -----------------------
	// See if 100 hz needs update
	d_100hz = (dial_freq%1000)/100;
	if(d_100hz != df.dial_100_hz)
	{
		//printf("100 hz diff: %d\n\r",d_100hz);

		// To string
		digit[0] = 0x30 + (d_100hz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_TUNE_FREQ_X + LARGE_FONT_WIDTH*7),POS_TUNE_FREQ_Y,digit,color,Black,1);

		// Save value
		df.dial_100_hz = d_100hz;
	}

	// -----------------------
	// See if 10 hz needs update
	d_10hz = (dial_freq%100)/10;
	if(d_10hz != df.dial_010_hz)
	{
		//printf("10 hz diff: %d\n\r",d_10hz);

		// To string
		digit[0] = 0x30 + (d_10hz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_TUNE_FREQ_X + LARGE_FONT_WIDTH*8),POS_TUNE_FREQ_Y,digit,color,Black,1);

		// Save value
		df.dial_010_hz = d_10hz;
	}

	// -----------------------
	// See if 1 hz needs update
	d_1hz = (dial_freq%10)/1;
	if(d_1hz != df.dial_001_hz)
	{
		//printf("1 hz diff: %d\n\r",d_1hz);

		// To string
		digit[0] = 0x30 + (d_1hz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_TUNE_FREQ_X + LARGE_FONT_WIDTH*9),POS_TUNE_FREQ_Y,digit,color,Black,1);

		// Save value
		df.dial_001_hz = d_1hz;
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateSecondLcdFreq
//* Object              : second freq indicator
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverUpdateSecondLcdFreq(ulong dial_freq)
{
	uchar		d_10mhz,d_1mhz;
	uchar		d_100khz,d_10khz,d_1khz;
	uchar		d_100hz,d_10hz,d_1hz;

	char		digit[2];

	// Terminate
	digit[1] = 0;

	//printf("--------------------\n\r");
	//printf("dial: %dHz\n\r",dial_freq);
	//printf("dial_001_mhz: %d\n\r",df.dial_001_mhz);
	//printf("dial_100_khz: %d\n\r",df.dial_100_khz);
	//printf("dial_010_khz: %d\n\r",df.dial_010_khz);
	//printf("dial_001_khz: %d\n\r",df.dial_001_khz);
	//printf("dial_100_hz:  %d\n\r",df.dial_100_hz);
	//printf("dial_010_hz:  %d\n\r",df.dial_010_hz);
	//printf("dial_001_hz:  %d\n\r",df.dial_001_hz);

	// Second Frequency
	//UiLcdHy28_PrintText((POS_TUNE_FREQ_X + 175),(POS_TUNE_FREQ_Y + 8),"14.000.000",Grey,Black,0);

	// -----------------------
	// See if 10 Mhz needs update
	d_10mhz = (dial_freq/10000000);
	if(d_10mhz != df.sdial_010_mhz)
	{
		//printf("10 mhz diff: %d\n\r",d_10mhz);

		// To string
		digit[0] = 0x30 + (d_10mhz & 0x0F);

		// Update segment
		if(d_10mhz)
			UiLcdHy28_PrintText((POS_TUNE_SFREQ_X + 0),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);
		else
			UiLcdHy28_PrintText((POS_TUNE_SFREQ_X + 0),POS_TUNE_SFREQ_Y,digit,Black,Black,0);	// mask the zero

		// Save value
		df.sdial_010_mhz = d_10mhz;
	}

	// -----------------------
	// See if 1 Mhz needs update
	d_1mhz = (dial_freq%10000000)/1000000;
	if(d_1mhz != df.sdial_001_mhz)
	{
		//printf("1 mhz diff: %d\n\r",d_1mhz);

		// To string
		digit[0] = 0x30 + (d_1mhz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_TUNE_SFREQ_X + SMALL_FONT_WIDTH),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);

		// Save value
		df.sdial_001_mhz = d_1mhz;
	}

	// -----------------------
	// See if 100 khz needs update
	d_100khz = (dial_freq%1000000)/100000;
	if(d_100khz != df.sdial_100_khz)
	{
		//printf("100 khz diff: %d\n\r",d_100khz);

		// To string
		digit[0] = 0x30 + (d_100khz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_TUNE_SFREQ_X + SMALL_FONT_WIDTH*3),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);

		// Save value
		df.sdial_100_khz = d_100khz;
	}

	// -----------------------
	// See if 10 khz needs update
	d_10khz = (dial_freq%100000)/10000;
	if(d_10khz != df.sdial_010_khz)
	{
		//printf("10 khz diff: %d\n\r",d_10khz);

		// To string
		digit[0] = 0x30 + (d_10khz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_TUNE_SFREQ_X + SMALL_FONT_WIDTH*4),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);

		// Save value
		df.sdial_010_khz = d_10khz;
	}

	// -----------------------
	// See if 1 khz needs update
	d_1khz = (dial_freq%10000)/1000;
	if(d_1khz != df.sdial_001_khz)
	{
		//printf("1 khz diff: %d\n\r",d_1khz);

		// To string
		digit[0] = 0x30 + (d_1khz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_TUNE_SFREQ_X + SMALL_FONT_WIDTH*5),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);

		// Save value
		df.sdial_001_khz = d_1khz;
	}

	// -----------------------
	// See if 100 hz needs update
	d_100hz = (dial_freq%1000)/100;
	if(d_100hz != df.sdial_100_hz)
	{
		//printf("100 hz diff: %d\n\r",d_100hz);

		// To string
		digit[0] = 0x30 + (d_100hz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_TUNE_SFREQ_X + SMALL_FONT_WIDTH*7),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);

		// Save value
		df.sdial_100_hz = d_100hz;
	}

	// -----------------------
	// See if 10 hz needs update
	d_10hz = (dial_freq%100)/10;
	if(d_10hz != df.sdial_010_hz)
	{
		//printf("10 hz diff: %d\n\r",d_10hz);

		// To string
		digit[0] = 0x30 + (d_10hz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_TUNE_SFREQ_X + SMALL_FONT_WIDTH*8),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);

		// Save value
		df.sdial_010_hz = d_10hz;
	}

	// -----------------------
	// See if 1 hz needs update
	d_1hz = (dial_freq%10)/1;
	if(d_1hz != df.sdial_001_hz)
	{
		//printf("1 hz diff: %d\n\r",d_1hz);

		// To string
		digit[0] = 0x30 + (d_1hz & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_TUNE_SFREQ_X + SMALL_FONT_WIDTH*9),POS_TUNE_SFREQ_Y,digit,Grey,Black,0);

		// Save value
		df.sdial_001_hz = d_1hz;
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeTunningStep
//* Object              : Change tunning step
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangeTunningStep(uchar is_up)
{
	ulong 	idx = df.selected_idx;
	ulong	dial_freq;

	if(is_up)
	{
		// Increase step index or reset
		if(idx < (MAX_STEPS - 1))
			idx++;
		else
			idx = 0;
	}
	else
	{
		// Decrease step index or reset
		if(idx)
			idx--;
		else
			idx = (MAX_STEPS - 1);
	}

	//printf("--------------------------\n\r");
	//printf("step_o: %d\n\r",  df.tunning_step);
	//printf("step_l: %d\n\r",  df.last_tune_step);

	//printf("tune_o: %dHz\n\r",(df.tune_old/4));
	//printf("tune_n: %dHz\n\r",(df.tune_new/4));

	// Clear segments when we go from large to small
	// tunning step
	if(idx < df.selected_idx)
	{
		//printf("clear segments\n\r");

		// Get current
		dial_freq = df.tune_new;

		// To display frequency
		dial_freq = ((dial_freq/4) + df.transv_freq);

		// Remove leftovers(use saved tunning step from encoder update)
		dial_freq /= df.last_tune_step;
		dial_freq *= df.last_tune_step;

		// To tune frequency
		dial_freq -= df.transv_freq;
		dial_freq *= 4;

		// Update
		df.tune_old = dial_freq;
		df.tune_new = dial_freq;

		//printf("tune_n: %dHz\n\r",(df.tune_new/4));
	}

	// Update publics
	df.tunning_step	= tune_steps[idx];
	df.selected_idx = idx;

	//printf("step_n: %d\n\r",  df.tunning_step);

	// Update step on screen
	UiDriverShowStep(idx);

	// Save to Eeprom
	//TRX4M_VE_WriteStep(idx);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverKeypadCheck
//* Object              : implemented as state machine, to avoid interrupts
//* Object              : and stall of app loop
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverKeypadCheck(void)
{
	ulong i;

	// State machine - processing old click
	if(ks.button_processed)
		return;

	// State machine - click or release(debounce filter)
	if(!ks.button_pressed)
	{
		// Scan inputs - 16 buttons in total, but on different ports
		for(i = 0; i < 16; i++)
		{
			// Read each pin of the port, based on the declared pin map
			if(!GPIO_ReadInputDataBit(bm[i].port,bm[i].button))
			{
				// Change state to clicked
				ks.button_id		= i;
				ks.button_pressed	= 1;
				ks.button_released	= 0;

				// Debounce filter
				non_os_delay();
				non_os_delay();
				non_os_delay();
				non_os_delay();

				//printf("button_pressed %02x\n\r",ks.button_id);

				// Exit, we process just one click at a time
				break;
			}
		}
	}
	else
	{
		// Debounce counter
		ks.debounce_time++;

		// Still pressed ?
		if(GPIO_ReadInputDataBit(bm[ks.button_id].port,bm[ks.button_id].button))
		{
			// Change state from click to released,
			// and processing flag on
			ks.button_pressed 	= 0;
			ks.button_released 	= 1;
			ks.button_processed	= 1;

			//printf("button_released %02x\n\r",ks.button_id);
		}
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeDemodMode
//* Object              : change demodulator mode
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangeDemodMode(void)
{
	ulong loc_mode = ts.dmod_mode;	// copy to local, so IRQ is not affected

	// Increase mode
	loc_mode++;

	// Check for overflow
	if(loc_mode == DEMOD_MAX_MODE)
		loc_mode = DEMOD_USB;

	// Finally update public flag
	ts.dmod_mode = loc_mode;

	// Set SoftDDS in CW mode
	if(ts.dmod_mode == DEMOD_CW)
		softdds_setfreq(500.0,ts.samp_rate,0);
	else
		softdds_setfreq(0.0,ts.samp_rate,0);

	// Set default filter on Mode change
	//UiDriverSelectBandFilter();

	// Update Decode Mode (USB/LSB/AM/FM/CW)
	UiDriverShowMode();

	// Change function buttons caption
	//UiDriverCreateFunctionButtons(false);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeBand
//* Object              : change band
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangeBand(uchar is_up)
{
	ulong 	curr_band_index;	// index in band table of currently selected band
	ulong	new_band_index;		// index of the new selected band

	ulong 	new_band_freq;		// new dial frequency

	//printf("-----------> change band\n\r");

	// Do not allow band change during TX
	if(ts.txrx_mode == TRX_MODE_TX)
		return;

	curr_band_index = ts.band_mode;

	//printf("current index: %d and freq: %d\n\r",curr_band_index,tune_bands[ts.band_mode]);

	// Save old band values
	if(curr_band_index < (MAX_BANDS - 1))
	{
		// Save dial
		band_dial_value[curr_band_index] = df.tune_old;

		// Save decode mode
		band_decod_mode[curr_band_index] = ts.dmod_mode;

		//printf("saved freq: %d and mode: %d\n\r",band_dial_value[curr_band_index],band_decod_mode[curr_band_index]);
	}

	// Handle direction
	if(is_up)
	{
		if(curr_band_index < (MAX_BANDS - 1))
		{
			//printf("going up band\n\r");

			// Increase
			new_band_freq  = tune_bands[curr_band_index + 1];
			new_band_index = curr_band_index + 1;
		}
		else
			return;
	}
	else
	{
		if(curr_band_index)
		{
			//printf("going down band\n\r");

			// Decrease
			new_band_freq  = tune_bands[curr_band_index - 1];
			new_band_index = curr_band_index - 1;
		}
		else
			return;
	}
	//printf("new band index: %d and freq: %d\n\r",new_band_index,new_band_freq);

	// Mute Audio
	//Codec_Volume(0);
	//Codec_Mute(1);

	// Load frequency value - either from memory or default for
	// the band if this is first band selection
	if(band_dial_value[new_band_index] != 0xFFFFFFFF)
	{
		//printf("load value from memory\n\r");

		// Load old frequency from memory
		df.tune_new = band_dial_value[new_band_index];
	}
	else
	{
		//printf("load default band freq\n\r");

		// Load default band startup frequency
		df.tune_new = new_band_freq;
	}

	df.transv_freq = 0;

	// Display frequency update
	UiDriverUpdateFrequency(1);

	// Also reset second freq display
	UiDriverUpdateSecondLcdFreq(df.tune_new/4);

	// Change decode mode if need to
	if(ts.dmod_mode != band_decod_mode[new_band_index])
	{
		// Update mode
		ts.dmod_mode = band_decod_mode[new_band_index];

		// Update Decode Mode (USB/LSB/AM/FM/CW)
		UiDriverShowMode();
	}

	// Create Band value
	UiDriverShowBand(new_band_index);

	// Set TX power factor
	UiDriverSetBandPowerFactor(new_band_index);

	// Set filters
	UiDriverChangeBandFilter(new_band_index,0);

	// Finally update public flag
	ts.band_mode = new_band_index;

	// Unmute Audio
	// Update codec volume
	//  0 - 10: via codec command
	// 10 - 20: soft gain after decoder
	//if(ts.audio_gain < 10)
	//Codec_Volume((ts.audio_gain*8));
	//Codec_Mute(0);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCheckFrequencyEncoder
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static bool UiDriverCheckFrequencyEncoder(void)
{
	int 		pot_diff;

	// Skip too regular read of the timer value, to avoid flickering
//	df.update_skip++;
//	if(df.update_skip < FREQ_UPDATE_SKIP)
//		return false;

//	df.update_skip = 0;

	// Load pot value
	df.value_new = TIM_GetCounter(TIM8);

	// Ignore lower value flickr
	if(df.value_new < ENCODER_FLICKR_BAND)
		return false;

	// Ignore higher value flickr
	if(df.value_new > (FREQ_ENCODER_RANGE/FREQ_ENCODER_LOG_D) + ENCODER_FLICKR_BAND)
		return false;

	// No change, return
	if(df.value_old == df.value_new)
		return false;

#ifdef USE_DETENTED_ENCODERS
	// SW de-detent routine
	df.de_detent++;
	if(df.de_detent < USE_DETENTED_VALUE)
	{
		df.value_old = df.value_new; // update and skip
		return false;
	}
	df.de_detent = 0;
#endif

	//printf("freq pot: %d \n\r",df.value_new);

	// Encoder value to difference
	if(df.value_new > df.value_old)
		pot_diff = +1;
	else
		pot_diff = -1;

	//printf("pot diff: %d\n\r",pot_diff);

	// Finaly convert to frequency incr/decr
	if(pot_diff < 0)
		df.tune_new -= (df.tunning_step * 4);
	else
		df.tune_new += (df.tunning_step * 4);

	// Updated
	df.value_old = df.value_new;

	return true;
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCheckEncoderOne
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverCheckEncoderOne(void)
{
	char 	temp[10];
	int 	pot_diff;

	// Only in RX mode when not calibrating
	if((ts.txrx_mode != TRX_MODE_RX) && (!ts.calib_mode))
		return;

	eos.value_new = TIM_GetCounter(TIM3);

	// Ignore lower value flickr
	if(eos.value_new < ENCODER_FLICKR_BAND)
		return;

	// Ignore lower value flickr
	if(eos.value_new > (ENCODER_ONE_RANGE/ENCODER_ONE_LOG_D) + ENCODER_FLICKR_BAND)
		return;

	// No change, return
	if(eos.value_old == eos.value_new)
		return;

#ifdef USE_DETENTED_ENCODERS
	// SW de-detent routine
	eos.de_detent++;
	if(eos.de_detent < USE_DETENTED_VALUE)
	{
		eos.value_old = eos.value_new; // update and skip
		return;
	}
	eos.de_detent = 0;
#endif

	//printf("gain pot: %d\n\r",gs.value_new);

	// Encoder value to difference
	if(eos.value_new > eos.value_old)
		pot_diff = +1;
	else
		pot_diff = -1;

	//printf("pot diff: %d\n\r",pot_diff);

	// PA bias set
	if(ts.calib_mode)
	{
		ulong bias_val;

		if(pot_diff < 0)
		{
			if(ts.pa_bias)
				ts.pa_bias -= 1;
		}
		else
		{
			if(ts.pa_bias < 50)
				ts.pa_bias += 1;
		}

		bias_val = 200 + ts.pa_bias;
		if(bias_val > 255)
			bias_val = 255;

		// Set DAC Channel1 DHR12L register
		DAC_SetChannel2Data(DAC_Align_8b_R,bias_val);

		// UI update
		UiDriverChangePaBias(1,1);

		goto skip_update;
	}

	// Take appropriate action
	switch(ts.enc_one_mode)
	{
		// Update audio volume
		case ENC_ONE_MODE_AUDIO_GAIN:
		{
			// Convert to Audio Gain incr/decr
			if(pot_diff < 0)
			{
				if(ts.audio_gain)
					ts.audio_gain -= 1;
			}
			else
			{
				if(ts.audio_gain < 14)
					ts.audio_gain += 1;
			}

			// Value to string
			sprintf(temp,"%02d",ts.audio_gain);

			// Update screen indicator
			UiLcdHy28_PrintText((POS_AG_IND_X + 38),(POS_AG_IND_Y + 1), temp,White,Black,0);

			// Update codec volume
			//  0 - 10: via codec command
			// 10 - 20: soft gain after decoder
			if(ts.audio_gain < 10)
				Codec_Volume((ts.audio_gain*8));

			break;
		}

		// Sidetone gain
		case ENC_ONE_MODE_ST_GAIN:
		{
			// Convert to Audio Gain incr/decr
			if(pot_diff < 0)
			{
				if(ts.st_gain)
					ts.st_gain -= 1;
			}
			else
			{
				if(ts.st_gain < 7)
					ts.st_gain += 1;
			}

			// Value to string
			sprintf(temp,"%02d",ts.st_gain);

			// Update screen indicator
			UiLcdHy28_PrintText((POS_SG_IND_X + 30),(POS_SG_IND_Y + 1), temp,White,Black,0);

			break;
		}

		default:
			break;
	}

skip_update:

	// Updated
	eos.value_old = eos.value_new;
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCheckEncoderTwo
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverCheckEncoderTwo(void)
{
	//char 	temp[10];
	int 	pot_diff;

	// Only in RX mode
	if(ts.txrx_mode != TRX_MODE_RX)
		return;

	ews.value_new = TIM_GetCounter(TIM4);

	// Ignore lower value flickr
	if(ews.value_new < ENCODER_FLICKR_BAND)
		return;

	// Ignore lower value flickr
	if(ews.value_new > (ENCODER_TWO_RANGE/ENCODER_TWO_LOG_D) + ENCODER_FLICKR_BAND)
		return;

	// No change, return
	if(ews.value_old == ews.value_new)
		return;

#ifdef USE_DETENTED_ENCODERS
	// SW de-detent routine
	ews.de_detent++;
	if(ews.de_detent < USE_DETENTED_VALUE)
	{
		ews.value_old = ews.value_new; // update and skip
		return;
	}
	ews.de_detent = 0;
#endif

	//printf("gain pot: %d\n\r",gs.value_new);

	// Encoder value to difference
	if(ews.value_new > ews.value_old)
		pot_diff = +1;
	else
		pot_diff = -1;

	//printf("pot diff: %d\n\r",pot_diff);

	// Update IQ gain
	if(ts.calib_mode)
	{
		// Convert to incr/decr
		if(pot_diff < 0)
		{
			if(ts.iq_gain_balance > -32)
				ts.iq_gain_balance -= 1;
		}
		else
		{
			if(ts.iq_gain_balance < 32)
				ts.iq_gain_balance += 1;
		}

		// Gain balance
		UiDriverChangeIQGainBalance(1,1);

		goto skip_update;
	}

	// Take appropriate action
	switch(ts.enc_two_mode)
	{
		// Update rf volume
		case ENC_TWO_MODE_RF_GAIN:
		{
			// Convert to Audio Gain incr/decr
			if(pot_diff < 0)
			{
				if(ts.rf_gain)
					ts.rf_gain -= 1;
			}
			else
			{
				if(ts.rf_gain < 8)
					ts.rf_gain += 1;
			}

			// RF gain
			UiDriverChangeRfGain(1);

			// Save to Eeprom
			//TRX4M_VE_WriteRFG(ts.rf_gain);

			break;
		}

		// Update rf gain
		case ENC_TWO_MODE_RF_ATTEN:
		{
			ulong atten_val;

			// Convert to Audio Gain incr/decr
			if(pot_diff < 0)
			{
				if(ts.rf_atten)
					ts.rf_atten -= 1;
			}
			else
			{
				if(ts.rf_atten < 8)
					ts.rf_atten += 1;
			}

			// Use half range 0- 1.8V
			atten_val = (ts.rf_atten * 16);
			if(atten_val > 127)
				atten_val = 127;

			// Set DAC Channel1 DHR12L register - JFET attenuator
			DAC_SetChannel1Data(DAC_Align_8b_R, atten_val);

			// RF Attenuator
			UiDriverChangeRfAttenuator(1);

			// Save to Eeprom
			//TRX4M_VE_WriteRFG(ts.rf_gain);

			break;
		}

		default:
			break;
	}

skip_update:

	// Updated
	ews.value_old = ews.value_new;
}


//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCheckEncoderThree
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverCheckEncoderThree(void)
{
	int 	pot_diff;

	// Only in RX mode, if not calibrating
	if((ts.txrx_mode != TRX_MODE_RX) && (!ts.calib_mode))
		return;

	ets.value_new = TIM_GetCounter(TIM5);

	// Ignore lower value flicker
	if(ets.value_new < ENCODER_FLICKR_BAND)
		return;

	// Ignore higher value flicker
	if(ets.value_new > (ENCODER_THR_RANGE/ENCODER_THR_LOG_D) + ENCODER_FLICKR_BAND)
		return;

	// No change, return
	if(ets.value_old == ets.value_new)
		return;

#ifdef USE_DETENTED_ENCODERS
	// SW de-detent routine
	ets.de_detent++;
	if(ets.de_detent < USE_DETENTED_VALUE)
	{
		ets.value_old = ets.value_new; // update and skip
		return;
	}
	ets.de_detent = 0;
#endif

	//printf("fir pot: %d\n\r",fs.value_new);

	// Encoder value to difference
	if(ets.value_new > ets.value_old)
		pot_diff = +1;
	else
		pot_diff = -1;

	//printf("pot diff: %d\n\r",pot_diff);

	// Update IQ Phase
	if(ts.calib_mode)
	{
		// Convert to incr/decr
		if(pot_diff < 0)
		{
			if(ts.iq_phase_balance > -32)
				ts.iq_phase_balance -= 1;
		}
		else
		{
			if(ts.iq_phase_balance < 32)
				ts.iq_phase_balance += 1;
		}

		// Phase balance
		UiDriverChangeIQPhaseBalance(1,1);

		goto skip_update;
	}

	// Take appropriate action
	switch(ts.enc_thr_mode)
	{
		// Update RIT value
		case ENC_THREE_MODE_RIT:
		{
			// Convert to RIT incr/decr
			if(pot_diff < 0)
			{
				if(ts.rit_value > -50)
					ts.rit_value -= 1;
			}
			else
			{
				if(ts.rit_value < 50)
					ts.rit_value += 1;
			}

			// Update RIT
			UiDriverChangeRit(1);

			// Change frequency
			UiDriverUpdateFrequency(1);

			break;
		}

		// Keyer speed
		case ENC_THREE_MODE_CW_SPEED:
		{
			// Convert to Audio Gain incr/decr
			if(pot_diff < 0)
			{
				if(ts.keyer_speed > 5)
					ts.keyer_speed -= 1;
			}
			else
			{
				if(ts.keyer_speed < 48)
					ts.keyer_speed += 1;
			}

			UiDriverChangeKeyerSpeed(1);

			break;
		}

		default:
			break;
	}

skip_update:

	// Updated
	ets.value_old = ets.value_new;
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeEncoderOneMode
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangeEncoderOneMode(uchar skip)
{
	uchar l_mode;

	if(!skip)
	{
		ts.enc_one_mode++;
		if(ts.enc_one_mode >= ENC_ONE_MAX_MODE)
			ts.enc_one_mode = ENC_ONE_MODE_AUDIO_GAIN;

		l_mode = ts.enc_one_mode;
	}
	else
	{
		ts.enc_one_mode = ENC_ONE_MAX_MODE;
		l_mode 			= 100;
	}

	switch(l_mode)
	{
		case ENC_ONE_MODE_AUDIO_GAIN:
		{
			// Audio gain enabled
			UiDriverChangeAfGain(1);

			// Sidetone disabled
			UiDriverChangeStGain(0);

			break;
		}

		case ENC_ONE_MODE_ST_GAIN:
		{
			// Audio gain disabled
			UiDriverChangeAfGain(0);

			// Sidetone enabled
			UiDriverChangeStGain(1);

			break;
		}

		// Disable all
		default:
		{
			// Audio gain disabled
			UiDriverChangeAfGain(0);

			// Sidetone enabled
			UiDriverChangeStGain(0);

			break;
		}
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeEncoderTwoMode
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangeEncoderTwoMode(uchar skip)
{
	uchar 	l_mode;

	if(!skip)
	{
		ts.enc_two_mode++;
		if(ts.enc_two_mode >= ENC_TWO_MAX_MODE)
			ts.enc_two_mode = ENC_TWO_MODE_RF_GAIN;

		l_mode = ts.enc_two_mode;
	}
	else
	{
		ts.enc_two_mode = ENC_TWO_MAX_MODE;
		l_mode 			= 100;
	}

	switch(l_mode)
	{
		case ENC_TWO_MODE_RF_GAIN:
		{
			// RF gain
			UiDriverChangeRfGain(1);

			// RF Attenuator
			UiDriverChangeRfAttenuator(0);

			break;
		}

		case ENC_TWO_MODE_RF_ATTEN:
		{
			// RF gain
			UiDriverChangeRfGain(0);

			// RF Attenuator
			UiDriverChangeRfAttenuator(1);

			break;
		}

		// Disable all
		default:
		{
			// RF gain
			UiDriverChangeRfGain(0);

			// RF Attenuator
			UiDriverChangeRfAttenuator(0);

			break;
		}
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeEncoderThreeMode
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangeEncoderThreeMode(uchar skip)
{
	uchar 	l_mode;

	if(!skip)
	{
		ts.enc_thr_mode++;
		if(ts.enc_thr_mode >= ENC_THREE_MAX_MODE)
			ts.enc_thr_mode = ENC_THREE_MODE_RIT;

		l_mode = ts.enc_thr_mode;
	}
	else
	{
		ts.enc_thr_mode = ENC_THREE_MAX_MODE;
		l_mode 			= 100;
	}

	switch(l_mode)
	{
		case ENC_THREE_MODE_RIT:
		{
			// RIT
			UiDriverChangeRit(1);

			// CW speed
			UiDriverChangeKeyerSpeed(0);

			break;
		}

		case ENC_THREE_MODE_CW_SPEED:
		{
			// RIT
			UiDriverChangeRit(0);

			// CW speed
			UiDriverChangeKeyerSpeed(1);

			break;
		}

		// Disable all
		default:
		{
			// RIT
			UiDriverChangeRit(0);

			// CW speed
			UiDriverChangeKeyerSpeed(0);

			break;
		}
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverSelectBandFilter
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
/*static void UiDriverSelectBandFilter(void)
{
	switch(ts.dmod_mode)
	{
		case DEMOD_USB:
		case DEMOD_LSB:
		case DEMOD_DIGI:
		{
			ts.filter_id = AUDIO_FIR_3P6KHZ;
			UiDriverChangeFilter(0);
			break;
		}

		case DEMOD_AM:
		{
			ts.filter_id = AUDIO_FIR_10KHZ;
			UiDriverChangeFilter(0);
			break;
		}

		case DEMOD_CW:
		{
			ts.filter_id = AUDIO_FIR_1P8KHZ;
			UiDriverChangeFilter(0);
			break;
		}

		default:
			break;
	}
}*/

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeAfGain
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangeAfGain(uchar enabled)
{
	ushort 	color = Grey;
	char	temp[100];

	if(enabled)
		color = White;

	UiLcdHy28_DrawEmptyRect( POS_AG_IND_X,POS_AG_IND_Y,13,57,Grey);

	if(enabled)
		UiLcdHy28_PrintText((POS_AG_IND_X + 1), (POS_AG_IND_Y + 1),"AFG",Black,Grey,0);
	else
		UiLcdHy28_PrintText((POS_AG_IND_X + 1), (POS_AG_IND_Y + 1),"AFG",Grey1,Grey,0);

	sprintf(temp,"%02d",ts.audio_gain);
	UiLcdHy28_PrintText    ((POS_AG_IND_X + 38),(POS_AG_IND_Y + 1), temp,color,Black,0);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeStGain
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangeStGain(uchar enabled)
{
	ushort 	color = Grey;
	char	temp[100];

	if(enabled)
		color = White;

	UiLcdHy28_DrawEmptyRect( POS_SG_IND_X,POS_SG_IND_Y,13,49,Grey);

	if(enabled)
		UiLcdHy28_PrintText    ((POS_SG_IND_X + 1), (POS_SG_IND_Y + 1),"STG",Black,Grey,0);
	else
		UiLcdHy28_PrintText    ((POS_SG_IND_X + 1), (POS_SG_IND_Y + 1),"STG",Grey1,Grey,0);

	sprintf(temp,"%02d",ts.st_gain);
	UiLcdHy28_PrintText    ((POS_SG_IND_X + 30),(POS_SG_IND_Y + 1), temp,color,Black,0);
}


//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeKeyerMode
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangeKeyerMode(void)
{
	ushort color = White;

	// Draw top line
	UiLcdHy28_DrawStraightLine(POS_KM_IND_X,(POS_KM_IND_Y - 1),56,LCD_DIR_HORIZONTAL,Grey);

	switch(ts.keyer_mode)
	{
		case CW_MODE_IAM_B:
			UiLcdHy28_PrintText((POS_KM_IND_X),(POS_KM_IND_Y)," IAM B ",color,Blue,0);
			break;
		case CW_MODE_IAM_A:
			UiLcdHy28_PrintText((POS_KM_IND_X),(POS_KM_IND_Y)," IAM A ",color,Blue,0);
			break;
		case CW_MODE_STRAIGHT:
			UiLcdHy28_PrintText((POS_KM_IND_X),(POS_KM_IND_Y)," STR K ",color,Blue,0);
			break;
		default:
			break;
	}

	// Update CW gen module
	cw_gen_init();
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangePowerLevel
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangePowerLevel(void)
{
	ushort color = White;

	// Draw top line
	UiLcdHy28_DrawStraightLine(POS_PW_IND_X,(POS_PW_IND_Y - 1),56,LCD_DIR_HORIZONTAL,Grey);

	switch(ts.power_level)
	{
		case PA_LEVEL_5W:
			UiLcdHy28_PrintText((POS_PW_IND_X),(POS_PW_IND_Y),"   5W  ",color,Blue,0);
			break;
		case PA_LEVEL_2W:
			UiLcdHy28_PrintText((POS_PW_IND_X),(POS_PW_IND_Y),"   2W  ",color,Blue,0);
			break;
		case PA_LEVEL_1W:
			UiLcdHy28_PrintText((POS_PW_IND_X),(POS_PW_IND_Y),"   1W  ",color,Blue,0);
			break;
		case PA_LEVEL_0_5W:
			UiLcdHy28_PrintText((POS_PW_IND_X),(POS_PW_IND_Y),"  0.5W ",color,Blue,0);
			break;
		default:
			UiLcdHy28_PrintText((POS_PW_IND_X),(POS_PW_IND_Y),"  FULL ",color,Blue,0);
			break;
	}

	// Set TX power factor - to reflect changed power
	UiDriverSetBandPowerFactor(ts.band_mode);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeKeyerSpeed
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangeKeyerSpeed(uchar enabled)
{
	ushort 	color = Grey;
	char	temp[100];

	if(enabled)
		color = White;

	UiLcdHy28_DrawEmptyRect( POS_KS_IND_X,POS_KS_IND_Y,13,49,Grey);

	if(enabled)
		UiLcdHy28_PrintText((POS_KS_IND_X + 1), (POS_KS_IND_Y + 1),"WPM",Black,Grey,0);
	else
		UiLcdHy28_PrintText((POS_KS_IND_X + 1), (POS_KS_IND_Y + 1),"WPM",Grey1,Grey,0);

	memset(temp,0,100);
	sprintf(temp,"%2d",ts.keyer_speed);

	UiLcdHy28_PrintText    ((POS_KS_IND_X + 30),(POS_KS_IND_Y + 1), temp,color,Black,0);

	// Update CW gen module
	if(enabled)
		cw_gen_init();
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeRfGain
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangeRfGain(uchar enabled)
{
	ushort 	color = Grey;
	char	temp[100];

	if(enabled)
		color = White;

	UiLcdHy28_DrawEmptyRect( POS_RF_IND_X,POS_RF_IND_Y,13,57,Grey);

	if(enabled)
		UiLcdHy28_PrintText((POS_RF_IND_X + 1), (POS_RF_IND_Y + 1),"RFG",Black,Grey,0);
	else
		UiLcdHy28_PrintText((POS_RF_IND_X + 1), (POS_RF_IND_Y + 1),"RFG",Grey1,Grey,0);

	sprintf(temp,"%02d",ts.rf_gain);
	UiLcdHy28_PrintText    ((POS_RF_IND_X + 38),(POS_RF_IND_Y + 1), temp,color,Black,0);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeRfAttenuator
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangeRfAttenuator(uchar enabled)
{
	ushort 	color = Grey;
	char	temp[100];

	if(enabled)
		color = White;

	UiLcdHy28_DrawEmptyRect( POS_RA_IND_X,POS_RA_IND_Y,13,49,Grey);

	if(enabled)
		UiLcdHy28_PrintText    ((POS_RA_IND_X + 1), (POS_RA_IND_Y + 1),"ATT",Black,Grey,0);
	else
		UiLcdHy28_PrintText    ((POS_RA_IND_X + 1), (POS_RA_IND_Y + 1),"ATT",Grey1,Grey,0);

	sprintf(temp,"%02d",ts.rf_atten);
	UiLcdHy28_PrintText    ((POS_RA_IND_X + 30),(POS_RA_IND_Y + 1), temp,color,Black,0);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeIQGainBalance
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangeIQGainBalance(uchar enabled,uchar visible)
{
	ushort 	color = Grey;
	char	temp[100];

	// Delete control
	if(!visible)
	{
		UiLcdHy28_DrawFullRect( POS_BG_IND_X,POS_BG_IND_Y,14,110,Black);
		return;
	}

	if(enabled)
		color = White;

	UiLcdHy28_DrawEmptyRect( POS_BG_IND_X,POS_BG_IND_Y,13,109,Grey);

	if(enabled)
		UiLcdHy28_PrintText    ((POS_BG_IND_X + 1), (POS_BG_IND_Y + 1),"IQ GAIN",Black,Grey,0);
	else
		UiLcdHy28_PrintText    ((POS_BG_IND_X + 1), (POS_BG_IND_Y + 1),"IQ GAIN",Grey1,Grey,0);

	if(ts.iq_gain_balance >= 0)
		sprintf(temp,"+%i",ts.iq_gain_balance);
	else
		sprintf(temp,"%i", ts.iq_gain_balance);

	UiLcdHy28_PrintText((POS_BG_IND_X + 82),(POS_BG_IND_Y + 1),"000",Black,Black,0); // clear screen
	UiLcdHy28_PrintText((POS_BG_IND_X + 82),(POS_BG_IND_Y + 1), temp,color,Black,0);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeIQPhaseBalance
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangeIQPhaseBalance(uchar enabled,uchar visible)
{
	ushort 	color = Grey;
	char	temp[100];

	// Delete control
	if(!visible)
	{
		UiLcdHy28_DrawFullRect( POS_BP_IND_X,POS_BP_IND_Y,14,110,Black);
		return;
	}

	if(enabled)
		color = White;

	UiLcdHy28_DrawEmptyRect(POS_BP_IND_X,POS_BP_IND_Y,13,109,Grey);

	if(enabled)
		UiLcdHy28_PrintText    ((POS_BP_IND_X + 1), (POS_BP_IND_Y + 1),"IQ PHAS",Black,Grey,0);
	else
		UiLcdHy28_PrintText    ((POS_BP_IND_X + 1), (POS_BP_IND_Y + 1),"IQ PHAS",Grey1,Grey,0);

	if(ts.iq_phase_balance >= 0)
		sprintf(temp,"+%i",ts.iq_phase_balance);
	else
		sprintf(temp,"%i", ts.iq_phase_balance);

	UiLcdHy28_PrintText((POS_BP_IND_X + 82),(POS_BP_IND_Y + 1),"000",Black,Black,0); // clear screen
	UiLcdHy28_PrintText((POS_BP_IND_X + 82),(POS_BP_IND_Y + 1), temp,color,Black,0);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangePaBias
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangePaBias(uchar enabled,uchar visible)
{
	ushort 	color = Grey;
	char	temp[100];

	// Delete control
	if(!visible)
	{
		UiLcdHy28_DrawFullRect( POS_PB_IND_X,POS_PB_IND_Y,14,110,Black);
		return;
	}

	if(enabled)
		color = White;

	UiLcdHy28_DrawEmptyRect( POS_PB_IND_X,POS_PB_IND_Y,13,109,Grey);

	if(enabled)
		UiLcdHy28_PrintText    ((POS_PB_IND_X + 1), (POS_PB_IND_Y + 1),"PA BIAS",Black,Grey,0);
	else
		UiLcdHy28_PrintText    ((POS_PB_IND_X + 1), (POS_PB_IND_Y + 1),"PA BIAS",Grey1,Grey,0);

	sprintf(temp,"+%d",ts.pa_bias);

	UiLcdHy28_PrintText((POS_PB_IND_X + 82),(POS_PB_IND_Y + 1),"000",Black,Black,0); // clear screen
	UiLcdHy28_PrintText((POS_PB_IND_X + 82),(POS_PB_IND_Y + 1), temp,color,Black,0);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeRit
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangeRit(uchar enabled)
{
	char	temp[100];
	ushort 	color = Grey;

	if(enabled)
		color = White;

	UiLcdHy28_DrawEmptyRect( POS_RIT_IND_X,POS_RIT_IND_Y,13,57,Grey);

	if(enabled)
		UiLcdHy28_PrintText    ((POS_RIT_IND_X + 1), (POS_RIT_IND_Y + 1),"RIT",Black,Grey,0);
	else
		UiLcdHy28_PrintText    ((POS_RIT_IND_X + 1), (POS_RIT_IND_Y + 1),"RIT",Grey1,Grey,0);

	if(ts.rit_value >= 0)
		sprintf(temp,"+%i",ts.rit_value);
	else
		sprintf(temp,"%i", ts.rit_value);

	UiLcdHy28_PrintText((POS_RIT_IND_X + 30),(POS_RIT_IND_Y + 1),"000",Black,Black,0); // clear screen
	UiLcdHy28_PrintText((POS_RIT_IND_X + 30),(POS_RIT_IND_Y + 1), temp,color,Black,0);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverChangeFilter
//* Object              : change audio filter, based on public flag
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverChangeFilter(uchar ui_only_update)
{
	ushort fcolor = Grey;

	fcolor = White;
	UiLcdHy28_PrintText(POS_FIR_IND_X,  POS_FIR_IND_Y,       "  FIL  ",	White, 	Orange, 0);

	// Do a filter re-load
	if(!ui_only_update)
		audio_driver_set_rx_audio_filter();

	// Draw top line
	UiLcdHy28_DrawStraightLine(POS_FIR_IND_X,(POS_FIR_IND_Y - 1),56,LCD_DIR_HORIZONTAL,Grey);

	// Clear screen
	UiLcdHy28_PrintText(POS_FIR_IND_X,(POS_FIR_IND_Y + 15),"00000", Black, Black,  0);

	// Update screen indicator
	switch(ts.filter_id)
	{
		case AUDIO_FIR_1P8KHZ:
			UiLcdHy28_PrintText(POS_FIR_IND_X,(POS_FIR_IND_Y + 15),"  1.8k", fcolor,Black,0);
			break;

		case AUDIO_FIR_2P3KHZ:
			UiLcdHy28_PrintText(POS_FIR_IND_X,(POS_FIR_IND_Y + 15),"  2.6k", fcolor,Black,0);
			break;

		case AUDIO_FIR_3P6KHZ:
			UiLcdHy28_PrintText(POS_FIR_IND_X,(POS_FIR_IND_Y + 15),"  3.6k", fcolor,Black,0);
			break;

		case AUDIO_FIR_10KHZ:
			UiLcdHy28_PrintText(POS_FIR_IND_X,(POS_FIR_IND_Y + 15),"   10k", fcolor,Black,0);
			break;

		default:
			break;
	}

	// Save to Eeprom
	//TRX4M_VE_WriteFir(ts.filter_id);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverInitSpectrumDisplay
//* Object              : FFT init
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverInitSpectrumDisplay(void)
{
	arm_status	a;

	// Init publics
	sd.state 		= 0;
	sd.samp_ptr 	= 0;
	sd.skip_process = 0;
	sd.enabled		= 0;
	sd.dial_moved	= 0;

	// Init FFT structures
	a = arm_rfft_init_q15((arm_rfft_instance_q15 *)&sd.S,(arm_cfft_radix4_instance_q15 *)&sd.S_CFFT,FFT_IQ_BUFF_LEN,FFT_QUADRATURE_PROC,1);
	if(a != ARM_MATH_SUCCESS)
	{
		printf("fft init err: %d\n\r",a);
		return;
	}

	// Ready
	sd.enabled		= 1;
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverReDrawSpectrumDisplay
//* Object              : state machine implementation
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverReDrawSpectrumDisplay(void)
{
	ulong i;

	// Only in RX mode
	if(ts.txrx_mode != TRX_MODE_RX)
		return;

	// No spectrum display in DIGI modes
	//if(ts.dmod_mode == DEMOD_DIGI)
	//	return;

	// Nothing to do here otherwise
	if(!sd.enabled)
		return;

	// The state machine will rest
	// in between states
//	sd.skip_process++;
//	if(sd.skip_process < 1000)
//		return;

//	sd.skip_process = 0;

	// Process implemented as state machine
	switch(sd.state)
	{
		// Stage 1 - apply gain to collected IQ samples
		case 1:
		{
			for(i = 0; i < FFT_IQ_BUFF_LEN; i++)
				sd.FFT_Samples[i] *= 60;	// was 20

			//arm_mult_q15(sd.FFT_Samples, hamm_wnd_vector, sd.FFT_Samples, FFT_IQ_BUFF_LEN);

			sd.state++;
			break;
		}

		// Stage 2 - do FFT calc on the samples
		case 2:
		{
			arm_rfft_q15((arm_rfft_instance_q15 *)&sd.S,(q15_t *)(sd.FFT_Samples),(q15_t *)(sd.FFT_Samples));

			sd.state++;
			break;
		}

		// Stage 3 - do magnitude processing
		case 3:
		{
			// Save old data - we will use later to mask pixel on the control
			for(i = 0; i < FFT_IQ_BUFF_LEN/2; i++)
				sd.FFT_BkpData[i]  = sd.FFT_MagData[i];

			// Calculate magnitude
			arm_cmplx_mag_q15((q15_t *)(sd.FFT_Samples),(q15_t *)(sd.FFT_MagData),(FFT_IQ_BUFF_LEN/2));

			sd.state++;
			break;
		}

		// Stage 4 - equalise magnitude data
		case 4:
		{
			uint32_t	index;
			q15_t		max,divisor;
			uint32_t 	i;
			//float 		cur;

			// Get the max value of the samples array
			arm_max_q15((q15_t *)(sd.FFT_MagData),(FFT_IQ_BUFF_LEN/2),&max, &index);

			// Calc div to fit into the screen
			divisor	= max/(SPECTRUM_HEIGHT - 5);

			// Equalize, by averaging or window function
			for(i = 0; i < (FFT_IQ_BUFF_LEN/2); i++)
			{
				sd.FFT_MagData[i] = (sd.FFT_MagData[i]/divisor);

				// Window func do not work
				//cur = 2.0 * log2(sd.FFT_MagData[i] * sd.FFT_MagData[i]);
				//sd.FFT_MagData[i] = (q15_t)cur;
			}

			sd.state++;
			break;
		}

		// Stage 5 - update LCD control
		case 5:
		{
			// Left part of screen(mask and update)
			UiLcdHy28_DrawSpectrum((q15_t *)(sd.FFT_BkpData + FFT_IQ_BUFF_LEN/4),Black,0);
			UiLcdHy28_DrawSpectrum((q15_t *)(sd.FFT_MagData + FFT_IQ_BUFF_LEN/4),White,0);

			// Right part of screen(mask and update)
			UiLcdHy28_DrawSpectrum((q15_t *)(sd.FFT_BkpData),                    Black,1);
			UiLcdHy28_DrawSpectrum((q15_t *)(sd.FFT_MagData),				     White,1);

			sd.state++;
			break;
		}

		// Stage 6 - clear LCD control when dial was moved
		case 6:
		{
			if(sd.dial_moved)
			{
				// Clear spectrum control, if VFO dial was moved
				//UiLcdHy28_DrawFullRect((POS_SPECTRUM_IND_X + 1),(POS_SPECTRUM_IND_Y + 1),(POS_SPECTRUM_IND_H - 2),(POS_SPECTRUM_IND_W - 4),Black);

				// Reset flag
				sd.dial_moved = 0;
			}

			sd.state = 0;
			break;
		}

		// Stage 0 - collection of data by the Audio driver
		default:
			break;
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateUsbKeyboardStatus
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
/*static void UiDriverUpdateUsbKeyboardStatus(void)
{
	// No change, nothing to process
	if(kbs.new_state == kbs.old_state)
		return;

	switch(kbs.new_state)
	{
		// Nothing connected
		case 0:
			UiLcdHy28_PrintText(POS_KBD_IND_X,POS_KBD_IND_Y,"KBD",Grey,Black,0);
			break;

		// Some device attached
		case 1:
			UiLcdHy28_PrintText(POS_KBD_IND_X,POS_KBD_IND_Y,"DEV",Grey,Black,0);
			break;

		// Keyboard detected
		case 2:
			UiLcdHy28_PrintText(POS_KBD_IND_X,POS_KBD_IND_Y,"KBD",Blue,Black,0);
			break;

		default:
			break;
	}

	// Set as done
	kbs.old_state = kbs.new_state;
}*/

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverPowerOffCheck
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverPowerOffCheck(void)
{
	ulong wait = 0;

	if(!ts.power_off_req)
		return;

	// Wait power button release (software de-bounce)
	while(!GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13))
	{
		wait++;
		if(wait > POWER_BUTTON_HOLD_TIME)
		{
			// Clear flag
			ts.power_off_req = 0;

			break;
		}
	}

	// Debounce finished ?
	if(ts.power_off_req)
	{
		ts.power_off_req = 0;
		return;
	}

	// Clear flag
	ts.power_off_req = 0;

	// Do not poweroff in TX mode
	if(ts.txrx_mode != TRX_MODE_RX)
		return;

	//printf("going to sleep\n\r");

	// Enter power off mode
	mchf_board_power_off();
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverHandleSmeter
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverHandleSmeter(void)
{
	uchar 	val;
	float	db_val;
	int		i_val;

	// Only in RX mode
	if(ts.txrx_mode != TRX_MODE_RX)
		return;

	sm.skip++;
	if(sm.skip < S_MET_UPD_SKIP)
		return;

	sm.skip = 0;

	if(sm.s_count == S_MET_SAMP_CNT)
	{
		db_val 	= 10.0 * log10(sm.curr_max);
		i_val	= (int)db_val;
		i_val  -= 40;

		if(i_val < 0)
			i_val = 0;

		//printf("%d ",i_val);

		val = (uchar)i_val;

		UiDriverUpdateTopMeterA(val,sm.old);

		sm.s_count  = 0;
		sm.curr_max = 0;
		sm.old		= val;
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverHandleSWRMeter
//* Object              : Power and SWR indicator
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverHandleSWRMeter(void)
{
	ushort	val_p,val_s = 0;
	//float 	rho,swr;

	// Only in TX mode
	if(ts.txrx_mode != TRX_MODE_TX)
		return;

	swrm.skip++;
	if(swrm.skip < SWR_SAMPLES_SKP)
		return;

	swrm.skip = 0;

	// Collect samples
	if(swrm.p_curr < SWR_SAMPLES_CNT)
	{
		// Get next sample
		val_p = ADC_GetConversionValue(ADC2);	// forward
		val_s = ADC_GetConversionValue(ADC3);	// return

		// Add to accumulator
		swrm.pwr_aver = swrm.pwr_aver + val_p;
		swrm.swr_aver = swrm.swr_aver + val_s;

		swrm.p_curr++;

		//printf("sample no %d\n\r",swrm.p_curr);
		return;
	}

	// Get average
	val_p  = swrm.pwr_aver/SWR_SAMPLES_CNT;
	val_s  = swrm.swr_aver/SWR_SAMPLES_CNT;

	//printf("aver power %d, aver ret %d\n\r", val_p,val_s);

	// Transmitter protection
	if(val_s > 2000)
	{
		// Display
		UiLcdHy28_PrintText(((POS_SM_IND_X + 18) + 140),(POS_SM_IND_Y + 59),"PROT",Red,Black,4);

		// Disable tx - not used for now
		//ts.tx_power_factor	= 0.0;
	}

	//UiDriverUpdateTopMeterA((uchar)(val_p/190),0);

	// Show 1W
	if((val_p > POWER_1W_MIN) && (val_p < POWER_1W_MAX))
		UiDriverUpdateTopMeterA(3,0);

	// Show 2W
	if((val_p > POWER_2W_MIN) && (val_p < POWER_2W_MAX))
		UiDriverUpdateTopMeterA(6,0);

	// Show 3W
	if((val_p > POWER_3W_MIN) && (val_p < POWER_3W_MAX))
		UiDriverUpdateTopMeterA(9,0);

	// Show 4W
	if((val_p > POWER_4W_MIN) && (val_p < POWER_4W_MAX))
		UiDriverUpdateTopMeterA(12,0);

	// Show 5W
	if((val_p > POWER_5W_MIN) && (val_p < POWER_5W_MAX))
		UiDriverUpdateTopMeterA(15,0);

	// Show 6W
	if((val_p > POWER_6W_MIN) && (val_p < POWER_6W_MAX))
		UiDriverUpdateTopMeterA(18,0);

	// Show 7W
	if((val_p > POWER_7W_MIN) && (val_p < POWER_7W_MAX))
		UiDriverUpdateTopMeterA(21,0);

	// Show 8W
	if((val_p > POWER_8W_MIN) && (val_p < POWER_8W_MAX))
		UiDriverUpdateTopMeterA(24,0);

	// Show 9W
	if((val_p > POWER_9W_MIN) && (val_p < POWER_9W_MAX))
		UiDriverUpdateTopMeterA(27,0);

	// Show 10W
	if((val_p > POWER_10W_MIN) && (val_p < POWER_10W_MAX))
		UiDriverUpdateTopMeterA(30,0);

	// Show overload
	if(val_p > POWER_10W_MAX)
		UiDriverUpdateTopMeterA(34,0);

	// Just test
	//UiDriverUpdateBtmMeter((uchar)(val_s / 250));

	// From http://ac6v.com/swrmeter.html
	// not working, to fix !!!
	//
	//rho 	= (float)sqrt((val_s/val_p));
	//swr 	= (1 + rho)/(1 - rho);
	//swr 	= (swr * 30);
	//val_s	 = ((ushort)swr);
	//printf("swr %i\n\r", val_s);

	// Display SWR
	//UiDriverUpdateBtmMeter((uchar)(val_s / 10));

	// Reset accumulator
	swrm.pwr_aver = 0;
	swrm.swr_aver = 0;
	swrm.p_curr   = 0;
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverHandlePowerSupply
//* Object              : display external voltage
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverHandlePowerSupply(void)
{
	ulong	val_p;
	uchar	v10,v100,v1000,v10000;
	//char 	cap1[50];
	char	digit[2];

	pwmt.skip++;
	if(pwmt.skip < POWER_SAMPLES_SKP)
		return;

	pwmt.skip = 0;

	// Collect samples
	if(pwmt.p_curr < POWER_SAMPLES_CNT)
	{
		val_p = ADC_GetConversionValue(ADC1);

		// Add to accumulator
		pwmt.pwr_aver = pwmt.pwr_aver + val_p;
		pwmt.p_curr++;

		return;
	}

	// Get average
	val_p  = pwmt.pwr_aver/POWER_SAMPLES_CNT;

	// Correct for divider
	//val_p -= 550;
	val_p *= 4;

	// Terminate
	digit[1] = 0;

	// -----------------------
	// See if 10V needs update
	v10000 = (val_p%100000)/10000;
	if(v10000 != pwmt.v10000)
	{
		//printf("10V diff: %d\n\r",v10000);

		// To string
		digit[0] = 0x30 + (v10000 & 0x0F);

		// Update segment
		if(v10000)
			UiLcdHy28_PrintText((POS_PWR_IND_X + SMALL_FONT_WIDTH*0),POS_PWR_IND_Y,digit,COL_PWR_IND,Black,0);
		else
			UiLcdHy28_PrintText((POS_PWR_IND_X + SMALL_FONT_WIDTH*0),POS_PWR_IND_Y,digit,Black,Black,0);

		// Save value
		pwmt.v10000 = v10000;
	}

	// -----------------------
	// See if 1V needs update
	v1000 = (val_p%10000)/1000;
	if(v1000 != pwmt.v1000)
	{
		//printf("1V diff: %d\n\r",v1000);

		// To string
		digit[0] = 0x30 + (v1000 & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_PWR_IND_X + SMALL_FONT_WIDTH*1),POS_PWR_IND_Y,digit,COL_PWR_IND,Black,0);

		// Save value
		pwmt.v1000 = v1000;
	}

	// -----------------------
	// See if 0.1V needs update
	v100 = (val_p%1000)/100;
	if(v100 != pwmt.v100)
	{
		//printf("0.1V diff: %d\n\r",v100);

		// To string
		digit[0] = 0x30 + (v100 & 0x0F);

		// Update segment(skip the dot)
		UiLcdHy28_PrintText((POS_PWR_IND_X + SMALL_FONT_WIDTH*3),POS_PWR_IND_Y,digit,COL_PWR_IND,Black,0);

		// Save value
		pwmt.v100 = v100;
	}

	// -----------------------
	// See if 0.01V needs update
	v10 = (val_p%100)/10;
	if(v10 != pwmt.v10)
	{
		//printf("0.01V diff: %d\n\r",v10);

		// To string
		digit[0] = 0x30 + (v10 & 0x0F);

		// Update segment(skip the dot)
		UiLcdHy28_PrintText((POS_PWR_IND_X + SMALL_FONT_WIDTH*4),POS_PWR_IND_Y,digit,COL_PWR_IND,Black,0);

		// Save value
		pwmt.v10 = v10;
	}

	// Reset accumulator
	pwmt.p_curr 	= 0;
	pwmt.pwr_aver 	= 0;
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateLoMeter
//* Object              : LO temperature compensation indicator
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverUpdateLoMeter(uchar val,uchar active)
{
	uchar 	i,v_s;
	int		col = White;

	// Do not waste time redrawing if outside of the range
	if(val > 26)
		return;

	// Indicator height
	v_s = 3;

	// Draw first indicator
	for(i = 1; i < 26; i++)
	{
		if(val == i)
			col = Blue;
		else
			col = White;

		if(!active)
			col = Grey;

		// Lines
		UiLcdHy28_DrawStraightLine(((POS_TEMP_IND_X + 1) + i*4),((POS_TEMP_IND_Y + 21) - v_s),v_s,LCD_DIR_VERTICAL,col);
		UiLcdHy28_DrawStraightLine(((POS_TEMP_IND_X + 2) + i*4),((POS_TEMP_IND_Y + 21) - v_s),v_s,LCD_DIR_VERTICAL,col);
		UiLcdHy28_DrawStraightLine(((POS_TEMP_IND_X + 3) + i*4),((POS_TEMP_IND_Y + 21) - v_s),v_s,LCD_DIR_VERTICAL,col);
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCreateTemperatureDisplay
//* Object              : draw ui
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverCreateTemperatureDisplay(uchar enabled,uchar create)
{
	if(create)
	{
		// Top part - name and temperature display
		UiLcdHy28_DrawEmptyRect( POS_TEMP_IND_X,POS_TEMP_IND_Y,14,109,Grey);

		// LO tracking indicator
		UiLcdHy28_DrawEmptyRect( POS_TEMP_IND_X,POS_TEMP_IND_Y + 14,10,109,Grey);

		// Temperature - initial draw
		UiLcdHy28_PrintText((POS_TEMP_IND_X + 50),(POS_TEMP_IND_Y + 1), "  25.0C",Grey,Black,0);
	}

	if(enabled)
	{
		// Control name
		UiLcdHy28_PrintText((POS_TEMP_IND_X + 1), (POS_TEMP_IND_Y + 1),"TCXO ",Black,Grey,0);

		// Lock indicator
		UiLcdHy28_PrintText((POS_TEMP_IND_X + 45),(POS_TEMP_IND_Y + 1),"*",Red,Black,0);
	}
	else
	{
		// Control name
		UiLcdHy28_PrintText((POS_TEMP_IND_X + 1), (POS_TEMP_IND_Y + 1),"TCXO ",Grey1,Grey,0);

		// Lock indicator
		UiLcdHy28_PrintText((POS_TEMP_IND_X + 45),(POS_TEMP_IND_Y + 1),"*",Grey,Black,0);
	}

	// Meter
	UiDriverUpdateLoMeter(13,enabled);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverCreateTemperatureDisplay
//* Object              : refresh ui
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverRefreshTemperatureDisplay(uchar enabled,int temp)
{
	uchar	v1000,v10000,v100000,v1000000;
	char	digit[2];

	//printf("temp: %i\n\r",temp);

	// Terminate
	digit[1] = 0;

	// -----------------------
	// See if 100C needs update
	v1000000 = (temp%10000000)/1000000;
	if(v1000000 != lo.v1000000)
	{
		//printf("100C diff: %d\n\r",v1000000);

		// To string
		digit[0] = 0x30 + (v1000000 & 0x0F);

		// Update segment (optional)
		if(v1000000)
			UiLcdHy28_PrintText((POS_TEMP_IND_X + 50 + SMALL_FONT_WIDTH*1),(POS_TEMP_IND_Y + 1),digit,Grey,Black,0);
		else
			UiLcdHy28_PrintText((POS_TEMP_IND_X + 50 + SMALL_FONT_WIDTH*1),(POS_TEMP_IND_Y + 1),digit,Black,Black,0);

		// Save value
		lo.v1000000 = v1000000;
	}

	// -----------------------
	// See if 10C needs update
	v100000 = (temp%1000000)/100000;
	if(v100000 != lo.v100000)
	{
		//printf("10C diff: %d\n\r",v100000);

		// To string
		digit[0] = 0x30 + (v100000 & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_TEMP_IND_X + 50 + SMALL_FONT_WIDTH*2),(POS_TEMP_IND_Y + 1),digit,Grey,Black,0);

		// Save value
		lo.v100000 = v100000;
	}

	// -----------------------
	// See if 1C needs update
	v10000 = (temp%100000)/10000;
	if(v10000 != lo.v10000)
	{
		//printf("1C diff: %d\n\r",v10000);

		// To string
		digit[0] = 0x30 + (v10000 & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_TEMP_IND_X + 50 + SMALL_FONT_WIDTH*3),(POS_TEMP_IND_Y + 1),digit,Grey,Black,0);

		// Save value
		lo.v10000 = v10000;
	}

	// -----------------------
	// See if 0.1C needs update
	v1000 = (temp%10000)/1000;
	if(v1000 != lo.v1000)
	{
		//printf("0.1C diff: %d\n\r",v1000);

		// To string
		digit[0] = 0x30 + (v1000 & 0x0F);

		// Update segment
		UiLcdHy28_PrintText((POS_TEMP_IND_X + 50 + SMALL_FONT_WIDTH*5),(POS_TEMP_IND_Y + 1),digit,Grey,Black,0);

		// Save value
		lo.v1000 = v1000;
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverHandleLoTemperature
//* Object              : display LO temperature and compensate drift
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverHandleLoTemperature(void)
{
	int		temp = 0;
	int		comp;
	uchar	tblp;

	// No need to process if no chip avail
	if(lo.sensor_present)
		return;

	lo.skip++;
	if(lo.skip < LO_COMP_SKP)
		return;

	lo.skip = 0;

	// Get current temperature
	if(ui_si570_read_temp(&temp) != 0)
		return;

	// Refresh UI
	UiDriverRefreshTemperatureDisplay(1,temp);

	// Compensate only if enabled
	if(!df.temp_enabled)
		return;

	//printf("temp: %i\n\r",temp);

	// Temperature to unsigned table pointer
	tblp  = (uchar)((temp%1000000)/100000);
	tblp *= 10;
	tblp += (uchar)((temp%100000)/10000);
	//printf("ptr: %d\n\r",tblp);

	// Update drift indicator, no overload
	UiDriverUpdateLoMeter(tblp - 30,1);

	// Check for overflow
	if(tblp > 100)
	{
		//printf("out of range\n\r");
		return;
	}

	// Value from freq table
	comp = tcxo_table_20m[tblp];
	//printf("comp:  %d - %i\n\r",tblp,comp);

	// Change needed ?
	if(lo.comp == comp)
		return;

	// Update frequency, without reflecting it on the LCD
	if(comp != (-1))
	{
		df.temp_factor = comp;
		UiDriverUpdateFrequencyFast();

		// Save to public, to skip not needed update
		// when we are in 1C range
		lo.comp = comp;

		// Lock indicator
		UiLcdHy28_PrintText((POS_TEMP_IND_X + 45),(POS_TEMP_IND_Y + 1),"*",Blue,Black,0);
	}
	else
	{
		// No compensation - commented out - keep last compensation value
		//df.temp_factor = 0;
		//UiDriverUpdateFrequencyFast();

		// Lock indicator
		UiLcdHy28_PrintText((POS_TEMP_IND_X + 45),(POS_TEMP_IND_Y + 1),"*",Red,Black,0);
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverEditMode
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
/*static void UiDriverEditMode(void)
{
	char symb[2];

	// Is edit mode ?
	if(kbs.set_mode != 1)
		return;

	// Key pressed
	if(kbs.last_char == 0)
		return;

	//printf("key = %02x ",kbs.last_char);

	// Handle CR
	if(kbs.last_char == 0x0a)
	{
		kbs.edit_item_id++;
		if(kbs.edit_item_id == 3)
			kbs.edit_item_id = 0;

		// Switch items
		switch(kbs.edit_item_id)
		{
			case 0:
			{
				UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 5),(POS_SPECTRUM_IND_Y +  0),"Call:  ",White,Black,0);
				UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 5),(POS_SPECTRUM_IND_Y + 15),"Loc:   ",Grey, Black,0);
				UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 5),(POS_SPECTRUM_IND_Y + 30),"Power: ",Grey, Black,0);
				break;
			}

			case 1:
			{
				UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 5),(POS_SPECTRUM_IND_Y +  0),"Call:  ",Grey,Black,0);
				UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 5),(POS_SPECTRUM_IND_Y + 15),"Loc:   ",White, Black,0);
				UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 5),(POS_SPECTRUM_IND_Y + 30),"Power: ",Grey, Black,0);
				break;
			}

			case 2:
			{
				UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 5),(POS_SPECTRUM_IND_Y +  0),"Call:  ",Grey,Black,0);
				UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 5),(POS_SPECTRUM_IND_Y + 15),"Loc:   ",Grey, Black,0);
				UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 5),(POS_SPECTRUM_IND_Y + 30),"Power: ",White, Black,0);
				break;
			}

			default:
				break;
		}

		// Reset hor ptr
		kbs.edit_item_hor = 0;
	}
	else
	{
		symb[0] = kbs.last_char;
		symb[1] = 0;

		// Print items
		switch(kbs.edit_item_id)
		{
			case 0:
			{
				// Add to buffer
				kbs.item_0[kbs.edit_item_hor] = kbs.last_char;

				UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 80 + (kbs.edit_item_hor*12)),(POS_SPECTRUM_IND_Y +  0),symb,Grey,Black,0);
				break;
			}

			case 1:
			{
				// Add to buffer
				kbs.item_1[kbs.edit_item_hor] = kbs.last_char;

				UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 80 + (kbs.edit_item_hor*12)),(POS_SPECTRUM_IND_Y + 15),symb,Grey, Black,0);
				break;
			}

			case 2:
			{
				// Add to buffer
				kbs.item_2[kbs.edit_item_hor] = kbs.last_char;

				UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + 80 + (kbs.edit_item_hor*12)),(POS_SPECTRUM_IND_Y + 30),symb,Grey, Black,0);
				break;
			}

			default:
				break;
		}

		// Move cursor right
		kbs.edit_item_hor++;
		if(kbs.edit_item_hor == 10)
			kbs.edit_item_hor = 0;
	}

	// Clear public
	kbs.last_char = 0;
}*/

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverSwitchOffPtt
//* Object              : PTT button release handling
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
ulong ptt_break = 0;
static void UiDriverSwitchOffPtt(void)
{
	// Not when tuning
	if(ts.tune)
		return;

	// PTT on
	if(ts.ptt_req)
	{
		if(ts.txrx_mode == TRX_MODE_RX)
		{
			ts.txrx_mode = TRX_MODE_TX;
			ui_driver_toggle_tx();
		}

		ts.ptt_req = 0;
		return;
	}

	// When CAT driver is running
	// skip auto return to RX, but do the
	// delayed unmute
	if(kd.enabled)
		goto unmute_only;

	// PTT off
	if((ts.dmod_mode == DEMOD_USB)||(ts.dmod_mode == DEMOD_LSB))
	{
		// PTT flag on ?
		if(ts.txrx_mode == TRX_MODE_TX)
		{
			// PTT line released ?
			if(GPIO_ReadInputDataBit(PADDLE_DAH_PIO,PADDLE_DAH))
			{
				// Lock to prevent IRQ re-entrance
				//ts.txrx_lock = 1;

				ptt_break++;
				if(ptt_break < 15)
					return;

				ptt_break = 0;

				// Back to RX
				ts.txrx_mode = TRX_MODE_RX;
				ui_driver_toggle_tx();				// PTT

				// Unlock
				//ts.txrx_lock = 0;
			}
		}
	}

unmute_only:

	// Handle Codec return to RX
	UiDriverDelayedUnmute();
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverDelayedUnmute
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverDelayedUnmute(void)
{
	// Is delayed auto mute requested ?
	if(ts.audio_unmute)
	{
		// Delay counter
		unmute_delay++;
		//if(unmute_delay == 3000)
		if(unmute_delay == 3)
		{
			// Unmute
			if(ts.audio_gain < 10)
				Codec_Volume((ts.audio_gain*8));
			else
				Codec_Volume((80));

			// Reset all publics
			unmute_delay 	= 0;
			ts.audio_unmute = 0;
		}
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverSetBandPowerFactor
//* Object              : TX chain gain is not const for the 3-30 Mhz range
//* Input Parameters    : so adjust here
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverSetBandPowerFactor(uchar band)
{
	// Display clear
	UiLcdHy28_PrintText(((POS_SM_IND_X + 18) + 140),(POS_SM_IND_Y + 59),"PROT",Black,Black,4);

	// If full power selected, no need to check band
	if(ts.power_level == PA_LEVEL_FULL)
	{
		ts.tx_power_factor	= 1.00;
		return;
	}

	switch(band)
	{
		// -------------------------------------------------------------
		// 80m
		case BAND_MODE_80:
		{
			switch(ts.power_level)
			{
				case PA_LEVEL_0_5W:
					ts.tx_power_factor	= 0.03;
					break;

				case PA_LEVEL_1W:
					ts.tx_power_factor	= 0.04;
					break;

				case PA_LEVEL_2W:
					ts.tx_power_factor	= 0.052;
					break;

				default:	// 5W
					ts.tx_power_factor	= 0.08;
					break;
			}

			break;
		}

		// -------------------------------------------------------------
		// 60m
		case BAND_MODE_60:
		{
			switch(ts.power_level)
			{
				case PA_LEVEL_0_5W:
					ts.tx_power_factor	= 0.025;
					break;

				case PA_LEVEL_1W:
					ts.tx_power_factor	= 0.035;
					break;

				case PA_LEVEL_2W:
					ts.tx_power_factor	= 0.045;
					break;

				default:	// 5W
					ts.tx_power_factor	= 0.10;
					break;
			}

			break;
		}

		// -------------------------------------------------------------
		// 40m
		case BAND_MODE_40:
		{
			switch(ts.power_level)
			{
				case PA_LEVEL_0_5W:
					ts.tx_power_factor	= 0.035;
					break;

				case PA_LEVEL_1W:
					ts.tx_power_factor	= 0.05;
					break;

				case PA_LEVEL_2W:
					ts.tx_power_factor	= 0.065;
					break;

				default:	// 5W
					ts.tx_power_factor	= 0.10;
					break;
			}

			break;
		}

		// -------------------------------------------------------------
		// 30m
		case BAND_MODE_30:
		{
			switch(ts.power_level)
			{
				case PA_LEVEL_0_5W:
					ts.tx_power_factor	= 0.04;
					break;

				case PA_LEVEL_1W:
					ts.tx_power_factor	= 0.06;
					break;

				case PA_LEVEL_2W:
					ts.tx_power_factor	= 0.08;
					break;

				default:	// 5W
					ts.tx_power_factor	= 0.13;
					break;
			}

			break;
		}

		// -------------------------------------------------------------
		// 20m
		case BAND_MODE_20:
		{
			switch(ts.power_level)
			{
				case PA_LEVEL_0_5W:
					ts.tx_power_factor	= 0.08;
					break;

				case PA_LEVEL_1W:
					ts.tx_power_factor	= 0.115;
					break;

				case PA_LEVEL_2W:
					ts.tx_power_factor	= 0.145;
					break;

				default:	// 5W
					ts.tx_power_factor	= 0.30;
					break;
			}

			break;
		}

		// -------------------------------------------------------------
		// 17m
		case BAND_MODE_17:
		{
			switch(ts.power_level)
			{
				case PA_LEVEL_0_5W:
					ts.tx_power_factor	= 0.08;
					break;

				case PA_LEVEL_1W:
					ts.tx_power_factor	= 0.12;
					break;

				case PA_LEVEL_2W:
					ts.tx_power_factor	= 0.16;
					break;

				default:	// 5W
					ts.tx_power_factor	= 0.40;
					break;
			}

			break;
		}

		// -------------------------------------------------------------
		// 15m
		case BAND_MODE_15:
		{
			switch(ts.power_level)
			{
				case PA_LEVEL_0_5W:
					ts.tx_power_factor	= 0.11;
					break;

				case PA_LEVEL_1W:
					ts.tx_power_factor	= 0.16;
					break;

				case PA_LEVEL_2W:
					ts.tx_power_factor	= 0.20;
					break;

				default:	// 5W
					ts.tx_power_factor	= 0.90;
					break;
			}

			break;
		}

		// -------------------------------------------------------------
		// 12m
		case BAND_MODE_12:
		{
			switch(ts.power_level)
			{
				case PA_LEVEL_0_5W:
					ts.tx_power_factor	= 0.13;
					break;

				case PA_LEVEL_1W:
					ts.tx_power_factor	= 0.20;
					break;

				case PA_LEVEL_2W:
					ts.tx_power_factor	= 0.30;
					break;

				default:	// 5W
					ts.tx_power_factor	= 1.00;	// actually 3.5W without attenuation
					break;
			}

			break;
		}

		// -------------------------------------------------------------
		// 10m
		case BAND_MODE_10:
		{
			switch(ts.power_level)
			{
				case PA_LEVEL_0_5W:
					ts.tx_power_factor	= 0.20;
					break;

				case PA_LEVEL_1W:
					ts.tx_power_factor	= 0.40;
					break;

				case PA_LEVEL_2W:
					ts.tx_power_factor	= 1.00;
					break;

				default:	// 5W
					ts.tx_power_factor	= 1.00;	// actually 1.5W without attenuation
					break;
			}

			break;
		}

		default:
			ts.tx_power_factor	= 0.50;
			break;
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverLoadEepromValues
//* Object              : load saved values on driver start
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverLoadEepromValues(void)
{
	ushort value,value1;

	// ------------------------------------------------------------------------------------
	// Try to read Band and Mode saved values
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_BAND_MODE], &value) == 0)
	{
		ts.band_mode = value & 0xFF;
		ts.dmod_mode = (value >> 8) & 0xFF;		// demodulator mode might not be right for saved band!
		//printf("-->band and mode loaded\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read Freq saved values
	if(	(EE_ReadVariable(VirtAddVarTab[EEPROM_FREQ_HIGH], &value) == 0) &&
		(EE_ReadVariable(VirtAddVarTab[EEPROM_FREQ_LOW], &value1) == 0))
	{
		ulong saved = (value << 16) | (value1);

		// We have loaded from eeprom the last used band, but can't just
		// load saved frequency, as it could be out of band, so do a
		// boundary check first
		if((saved >= tune_bands[ts.band_mode]) && (saved <= (tune_bands[ts.band_mode] + size_bands[ts.band_mode])))
		{
			df.tune_new = saved;
			//printf("-->frequency loaded\n\r");
		}
		else
		{
			// Load default for this band
			df.tune_new = tune_bands[ts.band_mode];
			//printf("-->base frequency loaded\n\r");
		}
	}

	// ------------------------------------------------------------------------------------
	// Try to read Step saved values
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_FREQ_STEP], &value) == 0)
	{
		df.selected_idx = value;
		df.tunning_step	= tune_steps[df.selected_idx];
		//printf("-->freq step loaded\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read TX Audio Source saved values
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_TX_AUDIO_SRC], &value) == 0)
	{
		ts.tx_audio_source = value;
		//printf("-->TX audio source loaded\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read TCXO saved values
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_TCXO_STATE], &value) == 0)
	{
		df.temp_enabled = value;
		//printf("-->TCXO state loaded\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read PA BIAS saved values
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_PA_BIAS], &value) == 0)
	{
		ts.pa_bias = value;
		//printf("-->PA BIAS loaded: %d\n\r",ts.pa_bias);
	}

	// ------------------------------------------------------------------------------------
	// Try to read Audio Gain saved values
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_AUDIO_GAIN], &value) == 0)
	{
		ts.audio_gain = value;
		//printf("-->Audio Gain loaded\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read RF Gain saved values
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_RF_GAIN], &value) == 0)
	{
		ts.rf_gain = value;
		//printf("-->RF Gain loaded\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read RIT saved values
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_RIT], &value) == 0)
	{
		ts.rit_value = (short)value;
		//printf("-->RIT loaded\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read Attenuator saved values
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_ATTEN], &value) == 0)
	{
		ts.rf_atten = value;
		//printf("-->Attenuator loaded\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read Power level saved values
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_POWER], &value) == 0)
	{
		ts.power_level = value;
		//printf("-->Power level loaded\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read Keyer speed saved values
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_KEYER_SPEED], &value) == 0)
	{
		ts.keyer_speed = value;
		//printf("-->Keyer speed loaded\n\r");

		// Extra init needed
		UiDriverChangeKeyerSpeed(1);
	}

	// ------------------------------------------------------------------------------------
	// Try to read Keyer mode saved values
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_KEYER_MODE], &value) == 0)
	{
		ts.keyer_mode = value;
		//printf("-->Keyer mode loaded\n\r");

		// Extra init needed
		UiDriverChangeKeyerMode();
	}

	// ------------------------------------------------------------------------------------
	// Try to read Sidetone Gain saved values
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_SIDETONE_GAIN], &value) == 0)
	{
		ts.st_gain = value;
		//printf("-->Sidetone Gain loaded\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read MIC BOOST saved values
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_MIC_BOOST], &value) == 0)
	{
		ts.mic_boost = value;
	}
	else	// create
	{
		EE_WriteVariable(VirtAddVarTab[EEPROM_MIC_BOOST],ts.mic_boost);
		//printf("-->MIC BOOST state created\n\r");
	}

	// Next setting...
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverSaveEepromValues
//* Object              : save changed values on regular intervals
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiDriverSaveEepromValues(void)
{
	ushort value,value1;

	// Only in RX mode, if not calibrating
	if((ts.txrx_mode != TRX_MODE_RX) && (!ts.calib_mode))
		return;

	es.skip++;
	if(es.skip < EEPROM_SAVE_SKP)
		return;

	es.skip = 0;

	//printf("eeprom save activate\n\r");

	// ------------------------------------------------------------------------------------
	// Read Band and Mode saved values - update if changed
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_BAND_MODE], &value) == 0)
	{
		ushort new,change;

		change 	= 0;
		new 	= 0;

		// Check first half
		if(ts.band_mode != (value & 0xFF))
		{
			new 	= ts.band_mode;
			change 	= 1;
		}

		// Check second
		if(ts.dmod_mode != ((value >> 8) & 0xFF))
		{
			new	   |= (ts.dmod_mode << 8);
			change 	= 1;
		}

		// Update var if changed
		if(change)
		{
			EE_WriteVariable(VirtAddVarTab[EEPROM_BAND_MODE], new);
			//printf("-->band and mode saved\n\r");
		}
	}
	else	// create
	{
		EE_WriteVariable(VirtAddVarTab[EEPROM_BAND_MODE],(ts.band_mode|(ts.dmod_mode << 8)));
		//printf("-->band and mode var created\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read Freq saved values - update if changed
	if(	(EE_ReadVariable(VirtAddVarTab[EEPROM_FREQ_HIGH], &value) == 0) &&
		(EE_ReadVariable(VirtAddVarTab[EEPROM_FREQ_LOW], &value1) == 0))
	{
		// Check first half
		if((df.tune_new >> 16) != value)
		{
			EE_WriteVariable(VirtAddVarTab[EEPROM_FREQ_HIGH],(df.tune_new >> 16));
			//printf("-->freq high saved\n\r");
		}

		// Check second half
		if((df.tune_new & 0xFFFF) != value1)
		{
			EE_WriteVariable(VirtAddVarTab[EEPROM_FREQ_LOW],(df.tune_new & 0xFFFF));
			//printf("-->freq low saved\n\r");
		}
	}
	else	// create
	{
		EE_WriteVariable(VirtAddVarTab[EEPROM_FREQ_HIGH],(df.tune_new >> 16));
		EE_WriteVariable(VirtAddVarTab[EEPROM_FREQ_LOW],(df.tune_new & 0xFFFF));
		//printf("-->freq vars created\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read Step saved values - update if changed
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_FREQ_STEP], &value) == 0)
	{
		if(df.selected_idx != value)
		{
			EE_WriteVariable(VirtAddVarTab[EEPROM_FREQ_STEP],df.selected_idx);
			//printf("-->freq step saved\n\r");
		}
	}
	else	// create
	{
		EE_WriteVariable(VirtAddVarTab[EEPROM_FREQ_STEP],df.selected_idx);
		//printf("-->freq step created\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read TX Audio Source saved values - update if changed
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_TX_AUDIO_SRC], &value) == 0)
	{
		if(ts.tx_audio_source != value)
		{
			EE_WriteVariable(VirtAddVarTab[EEPROM_TX_AUDIO_SRC],ts.tx_audio_source);
			//printf("-->TX audio source saved\n\r");
		}
	}
	else	// create
	{
		EE_WriteVariable(VirtAddVarTab[EEPROM_TX_AUDIO_SRC],ts.tx_audio_source);
		//printf("-->TX audio source created\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read TCXO saved values - update if changed
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_TCXO_STATE], &value) == 0)
	{
		if(df.temp_enabled != value)
		{
			EE_WriteVariable(VirtAddVarTab[EEPROM_TCXO_STATE],df.temp_enabled);
			//printf("-->TCXO state saved\n\r");
		}
	}
	else	// create
	{
		EE_WriteVariable(VirtAddVarTab[EEPROM_TCXO_STATE],df.temp_enabled);
		//printf("-->TCXO state created\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read BIAS saved values - update if changed
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_PA_BIAS], &value) == 0)
	{
		// Create only!!!
		// Update implemented in calibration menu
		//
		//if(ts.pa_bias != value)
		//{
		//	EE_WriteVariable(VirtAddVarTab[EEPROM_PA_BIAS],ts.pa_bias);
		//	printf("-->PA BIAS saved\n\r");
		//}
	}
	else	// create
	{
		EE_WriteVariable(VirtAddVarTab[EEPROM_PA_BIAS],ts.pa_bias);
		//printf("-->PA BIAS created\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read Audio Gain saved values - update if changed
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_AUDIO_GAIN], &value) == 0)
	{
		if(ts.audio_gain != value)
		{
			EE_WriteVariable(VirtAddVarTab[EEPROM_AUDIO_GAIN],ts.audio_gain);
			//printf("-->Audio Gain saved\n\r");
		}
	}
	else	// create
	{
		EE_WriteVariable(VirtAddVarTab[EEPROM_AUDIO_GAIN],ts.audio_gain);
		//printf("-->Audio Gain created\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read RF Gain saved values - update if changed
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_RF_GAIN], &value) == 0)
	{
		if(ts.rf_gain != value)
		{
			EE_WriteVariable(VirtAddVarTab[EEPROM_RF_GAIN],ts.rf_gain);
			//printf("-->RF Gain saved\n\r");
		}
	}
	else	// create
	{
		EE_WriteVariable(VirtAddVarTab[EEPROM_RF_GAIN],ts.rf_gain);
		//printf("-->RF Gain created\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read RIT saved values - update if changed
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_RIT], &value) == 0)
	{
		if(ts.rit_value != value)
		{
			EE_WriteVariable(VirtAddVarTab[EEPROM_RIT],(ushort)ts.rit_value);
			//printf("-->RIT Gain saved\n\r");
		}
	}
	else	// create
	{
		EE_WriteVariable(VirtAddVarTab[EEPROM_RIT],(ushort)ts.rit_value);
		//printf("-->RIT Gain created\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read Attenuator saved values - update if changed
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_ATTEN], &value) == 0)
	{
		if(ts.rf_atten != value)
		{
			EE_WriteVariable(VirtAddVarTab[EEPROM_ATTEN],ts.rf_atten);
			//printf("-->Attenuator saved\n\r");
		}
	}
	else	// create
	{
		EE_WriteVariable(VirtAddVarTab[EEPROM_ATTEN],ts.rf_atten);
		//printf("-->Attenuator created\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read power level saved values - update if changed
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_POWER], &value) == 0)
	{
		if(ts.power_level != value)
		{
			EE_WriteVariable(VirtAddVarTab[EEPROM_POWER],ts.power_level);
			//printf("-->power level saved\n\r");
		}
	}
	else	// create
	{
		EE_WriteVariable(VirtAddVarTab[EEPROM_POWER],ts.power_level);
		//printf("-->power level created\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read Keyer speed saved values - update if changed
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_KEYER_SPEED], &value) == 0)
	{
		if(ts.keyer_speed != value)
		{
			EE_WriteVariable(VirtAddVarTab[EEPROM_KEYER_SPEED],ts.keyer_speed);
			//printf("-->Keyer speed saved\n\r");
		}
	}
	else	// create
	{
		EE_WriteVariable(VirtAddVarTab[EEPROM_KEYER_SPEED],ts.keyer_speed);
		//printf("-->Keyer speed created\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read Keyer mode saved values - update if changed
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_KEYER_MODE], &value) == 0)
	{
		if(ts.keyer_mode != value)
		{
			EE_WriteVariable(VirtAddVarTab[EEPROM_KEYER_MODE],ts.keyer_mode);
			//printf("-->Keyer mode saved\n\r");
		}
	}
	else	// create
	{
		EE_WriteVariable(VirtAddVarTab[EEPROM_KEYER_MODE],ts.keyer_mode);
		//printf("-->Keyer mode created\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read Sidetone Gain saved values - update if changed
	if(EE_ReadVariable(VirtAddVarTab[EEPROM_SIDETONE_GAIN], &value) == 0)
	{
		if(ts.st_gain != value)
		{
			EE_WriteVariable(VirtAddVarTab[EEPROM_SIDETONE_GAIN],ts.st_gain);
			//printf("-->Sidetone Gain saved\n\r");
		}
	}
	else	// create
	{
		EE_WriteVariable(VirtAddVarTab[EEPROM_SIDETONE_GAIN],ts.st_gain);
		//printf("-->Sidetone Gain created\n\r");
	}

	// Next setting...
}
