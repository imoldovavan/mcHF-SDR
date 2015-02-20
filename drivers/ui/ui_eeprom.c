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
#include "codec.h"
#include "ui_menu.h"
//
//
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

// SSB filters

#include "filters/q_rx_filter.h"
#include "filters/i_rx_filter.h"
#include "filters/q_tx_filter.h"
#include "filters/i_tx_filter.h"


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
//* Function Name       : UiDriverLoadEepromValues
//* Object              : load saved values on driver start
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverLoadEepromValues(void)
{
	ushort value,value1;
	uint16_t uint_val;
	int16_t	*int_val;

	// Do a sample reads to "prime the pump" before we start...
	// This is to make the function work reliabily after boot-up
	//
	Read_VirtEEPROM(EEPROM_ZERO_LOC_UNRELIABLE, &value);	// We pick this variable - but any would do...
	//
	// ------------------------------------------------------------------------------------
	// Try to read Band and Mode saved values
	if(Read_VirtEEPROM(EEPROM_BAND_MODE, &value) == 0)
	{
		ts.band = value & 0x00FF;
		if(ts.band > MAX_BANDS-1)			// did we get an insane value from EEPROM?
			ts.band = BAND_MODE_80;		//	yes - set to 80 meters
		//
		ts.dmod_mode = (value >> 8) & 0x0F;		// demodulator mode might not be right for saved band!
		if(ts.dmod_mode > DEMOD_MAX_MODE)		// valid mode value from EEPROM?
			ts.dmod_mode = DEMOD_LSB;			// no - set to LSB
		//
		ts.filter_id = (value >> 12) & 0x0F;	// get filter setting
		if((ts.filter_id >= AUDIO_MAX_FILTER) || (ts.filter_id < AUDIO_MIN_FILTER))		// audio filter invalid?
			ts.filter_id = AUDIO_DEFAULT_FILTER;	// set default audio filter
		//
		//printf("-->band and mode loaded\n\r");
	}
	// ------------------------------------------------------------------------------------
	// Try to read Freq saved values
	if(	(Read_VirtEEPROM(EEPROM_FREQ_HIGH, &value) == 0) &&
		(Read_VirtEEPROM(EEPROM_FREQ_LOW, &value1) == 0))
	{
		ulong saved = (value << 16) | (value1);

		// We have loaded from eeprom the last used band, but can't just
		// load saved frequency, as it could be out of band, so do a
		// boundary check first
		if((saved >= tune_bands[ts.band]) && (saved <= (tune_bands[ts.band] + size_bands[ts.band])))
		{
			df.tune_new = saved;
			//printf("-->frequency loaded\n\r");
		}
		else
		{
			// Load default for this band
			df.tune_new = tune_bands[ts.band];
			//printf("-->base frequency loaded\n\r");
		}
	}
	//
	// Try to read saved per-band values for frequency, mode and filter
	//
	ulong i, saved;
	//
	for(i = 0; i < MAX_BANDS; i++)	{		// read from stored bands
		// ------------------------------------------------------------------------------------
		// Try to read Band and Mode saved values
		//
		if(Read_VirtEEPROM(EEPROM_BAND0_MODE + i, &value) == 0)			{
			// Note that ts.band will, by definition, be equal to index "i"
			//
			band_decod_mode[i] = (value >> 8) & 0x0F;		// demodulator mode might not be right for saved band!
			if(band_decod_mode[i] > DEMOD_MAX_MODE)		// valid mode value from EEPROM?
				band_decod_mode[i] = DEMOD_LSB;			// no - set to LSB
			//
			band_filter_mode[i] = (value >> 12) & 0x0F;	// get filter setting
			if((band_filter_mode[i] >= AUDIO_MAX_FILTER) || (ts.filter_id < AUDIO_MIN_FILTER))		// audio filter invalid?
				band_filter_mode[i] = AUDIO_DEFAULT_FILTER;	// set default audio filter
			//
			//printf("-->band, mode and filter setting loaded\n\r");
		}

		// ------------------------------------------------------------------------------------
		// Try to read Freq saved values
		if(	(Read_VirtEEPROM(EEPROM_BAND0_FREQ_HIGH + i, &value) == 0) && (Read_VirtEEPROM(EEPROM_BAND0_FREQ_LOW + i, &value1) == 0))	{
			saved = (value << 16) | (value1);
			//
			// We have loaded from eeprom the last used band, but can't just
			// load saved frequency, as it could be out of band, so do a
			// boundary check first
			//
			if((saved >= tune_bands[i]) && (saved <= (tune_bands[i] + size_bands[i])))	{
				band_dial_value[i] = saved;
				//printf("-->frequency loaded\n\r");
			}
			else	{
				// Load default for this band
				band_dial_value[i] = tune_bands[i] + 100;
				//printf("-->base frequency loaded\n\r");
			}
		}
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Step saved values
	if(Read_VirtEEPROM(EEPROM_FREQ_STEP, &value) == 0)
	{
		if(value >= MAX_STEPS -1)		// did we get step size value outside the range?
			value = 3;					// yes - set to default size of 1 kHz steps
		//
		df.selected_idx = value;
		df.tuning_step	= tune_steps[df.selected_idx];
		//printf("-->freq step loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read TX Audio Source saved values
	if(Read_VirtEEPROM(EEPROM_TX_AUDIO_SRC, &value) == 0)
	{
		ts.tx_audio_source = value;
		//printf("-->TX audio source loaded\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read TCXO saved values
	if(Read_VirtEEPROM(EEPROM_TCXO_STATE, &value) == 0)
	{
		df.temp_enabled = value;
		//printf("-->TCXO state loaded\n\r");
	}
	// ------------------------------------------------------------------------------------
	// Try to read PA BIAS saved values
	if(Read_VirtEEPROM(EEPROM_PA_BIAS, &value) == 0)
	{
		if(value > MAX_PA_BIAS)	// prevent garbage value for bias
			value = DEFAULT_PA_BIAS;
		//
		ts.pa_bias = value;
		//
		ulong bias_val;

		bias_val = 100 + (ts.pa_bias * 2);
		if(bias_val > 255)
			bias_val = 255;

		// Set DAC Channel1 DHR12L register with bias value
		DAC_SetChannel2Data(DAC_Align_8b_R,bias_val);
		//printf("-->PA BIAS loaded: %d\n\r",ts.pa_bias);
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Audio Gain saved values
	if(Read_VirtEEPROM(EEPROM_AUDIO_GAIN, &value) == 0)
	{
		if(value > MAX_AUDIO_GAIN)	// set default gain if garbage value from EEPROM
			value = DEFAULT_AUDIO_GAIN;
		ts.audio_gain = value;
		//printf("-->Audio Gain loaded\n\r");
	}
	// ------------------------------------------------------------------------------------
	// Try to read RF Codec Gain saved values
	if(Read_VirtEEPROM(EEPROM_RX_CODEC_GAIN, &value) == 0)
	{
		if(value > MAX_RF_CODEC_GAIN_VAL)			// set default if invalid value
			value = DEFAULT_RF_CODEC_GAIN_VAL;
		//
		ts.rf_codec_gain = value;
		//printf("-->RF Codec Gain loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RF Codec Gain saved values
	if(Read_VirtEEPROM(EEPROM_RX_GAIN, &value) == 0)
	{
		if(value > MAX_RF_GAIN)			// set default if invalid value
			value = DEFAULT_RF_GAIN;
		//
		ts.rf_gain = value;
		//printf("-->RF Gain loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Noise Blanker saved values
	if(Read_VirtEEPROM(EEPROM_NB_SETTING, &value) == 0)
	{
		if(value > MAX_RF_ATTEN)	// invalid value?
			value = 0;				// yes - set to zero
		//
		ts.nb_setting = value;
		//printf("-->Noise Blanker value loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Power level saved values
	if(Read_VirtEEPROM(EEPROM_TX_POWER_LEVEL, &value) == 0)
	{
		if(value >= PA_LEVEL_MAX_ENTRY)
			value = PA_LEVEL_DEFAULT;
		//
		ts.power_level = value;
		//printf("-->Power level loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Keyer speed saved values
	if(Read_VirtEEPROM(EEPROM_KEYER_SPEED, &value) == 0)
	{
		if((value < MIN_KEYER_SPEED) || value > MAX_KEYER_SPEED)	// value out of range?
			value = DEFAULT_KEYER_SPEED;
		//
		ts.keyer_speed = value;
		//printf("-->Keyer speed loaded\n\r");

		// Extra init needed
		UiDriverChangeKeyerSpeed(1);
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Keyer mode saved values
	if(Read_VirtEEPROM(EEPROM_KEYER_MODE, &value) == 0)
	{
		if(ts.keyer_mode >= CW_MAX_MODE)	// invalid CW mode value?
			value = CW_MODE_IAM_B;	// set default mode
		//
		ts.keyer_mode = value;
		//printf("-->Keyer mode loaded\n\r");

		// Extra init needed
		cw_gen_init();

	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Sidetone Gain saved values
	if(Read_VirtEEPROM(EEPROM_SIDETONE_GAIN, &value) == 0)
	{
		if(value > SIDETONE_MAX_GAIN)			// out of range of gain settings?
			value = DEFAULT_SIDETONE_GAIN;		// yes, use default
		//
		ts.st_gain = value;
		//printf("-->Sidetone Gain loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read MIC BOOST saved values
	if(Read_VirtEEPROM(EEPROM_MIC_BOOST, &value) == 0)
	{
		if(value < 2)
			ts.mic_boost = value;
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read TX LSB Phase saved values
	if(Read_VirtEEPROM(EEPROM_TX_IQ_LSB_PHASE_BALANCE, &uint_val) == 0)
	{
		int_val = &uint_val;
		if((*int_val < MIN_TX_IQ_PHASE_BALANCE) || (*int_val > MAX_TX_IQ_PHASE_BALANCE))	// out of range
			*int_val = 0;		// yes, use zero
		//
		ts.tx_iq_lsb_phase_balance = *int_val;
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read TX USB Phase saved values
	if(Read_VirtEEPROM(EEPROM_TX_IQ_USB_PHASE_BALANCE, &uint_val) == 0)
	{
		int_val = &uint_val;
		if((*int_val < MIN_TX_IQ_PHASE_BALANCE) || (*int_val > MAX_TX_IQ_PHASE_BALANCE))	// out of range
			*int_val = 0;		// yes, use zero
		//
		ts.tx_iq_usb_phase_balance = *int_val;
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read TX LSB Gain saved values
	if(Read_VirtEEPROM(EEPROM_TX_IQ_LSB_GAIN_BALANCE, &uint_val) == 0)
	{
		int_val = &uint_val;
		if((*int_val < MIN_TX_IQ_GAIN_BALANCE) || (*int_val > MAX_TX_IQ_GAIN_BALANCE))	// out of range?
			*int_val = 0;		// yes, use zero
		//
		ts.tx_iq_lsb_gain_balance = *int_val;
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read TX USB Gain saved values
	if(Read_VirtEEPROM(EEPROM_TX_IQ_USB_GAIN_BALANCE, &uint_val) == 0)
	{
		int_val = &uint_val;
		if((*int_val < MIN_TX_IQ_GAIN_BALANCE) || (*int_val > MAX_TX_IQ_GAIN_BALANCE))	// out of range?
			*int_val = 0;		// yes, use zero
		//
		ts.tx_iq_usb_gain_balance = *int_val;
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RX LSB Phase saved values
	if(Read_VirtEEPROM(EEPROM_RX_IQ_LSB_PHASE_BALANCE, &uint_val) == 0)
	{
		int_val = &uint_val;
		if((*int_val < MIN_RX_IQ_PHASE_BALANCE) || (*int_val > MAX_RX_IQ_PHASE_BALANCE))	// out of range
			*int_val = 0;		// yes - set default
		//
		ts.rx_iq_lsb_phase_balance = *int_val;
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RX USB Phase saved values
	if(Read_VirtEEPROM(EEPROM_RX_IQ_USB_PHASE_BALANCE, &uint_val) == 0)
	{
		int_val = &uint_val;
		if((*int_val < MIN_RX_IQ_PHASE_BALANCE) || (*int_val > MAX_RX_IQ_PHASE_BALANCE))	// out of range
			*int_val = 0;		// yes - set default
		//
		ts.rx_iq_usb_phase_balance = *int_val;
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RX LSB Gain saved values
	if(Read_VirtEEPROM(EEPROM_RX_IQ_LSB_GAIN_BALANCE, &uint_val) == 0)
	{
		int_val = &uint_val;
		if((*int_val < MIN_RX_IQ_GAIN_BALANCE) || (*int_val > MAX_RX_IQ_GAIN_BALANCE))	// out of range?
			*int_val = 0;	// yes - set default
		//
		ts.rx_iq_lsb_gain_balance = *int_val;
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RX USB Gain saved values
	if(Read_VirtEEPROM(EEPROM_RX_IQ_USB_GAIN_BALANCE, &uint_val) == 0)
	{
		int_val = &uint_val;
		if((*int_val < MIN_RX_IQ_GAIN_BALANCE) || (*int_val > MAX_RX_IQ_GAIN_BALANCE))	// out of range?
			*int_val = 0;	// yes - set default
		//
		ts.rx_iq_usb_gain_balance = *int_val;
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RX Frequency Calibration
	if(Read_VirtEEPROM(EEPROM_FREQ_CAL, &uint_val) == 0)
	{
		int_val = &uint_val;
		if((*int_val < MIN_FREQ_CAL) || (*int_val > MAX_FREQ_CAL))	// out of range
			*int_val = 0;		// yes - set default
		//
		ts.freq_cal = *int_val;
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read AGC mode saved values
	if(Read_VirtEEPROM(EEPROM_AGC_MODE, &value) == 0)
	{
		if(value > AGC_MAX_MODE)				// out of range of AGC settings?
			value = AGC_DEFAULT;				// yes, use default
		//
		ts.agc_mode = value;
		//printf("-->AGC mode loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read MIC gain saved values
	if(Read_VirtEEPROM(EEPROM_MIC_GAIN, &value) == 0)
	{
		if((value > MIC_GAIN_MAX) || (value < MIC_GAIN_MIN))				// out of range of MIC gain settings?
			value = MIC_GAIN_DEFAULT;				// yes, use default
		//
		ts.tx_mic_gain = value;
		//printf("-->MIC gain loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read LIEN gain saved values
	if(Read_VirtEEPROM(EEPROM_LINE_GAIN, &value) == 0)
	{
		if((value > LINE_GAIN_MAX) || (value < LINE_GAIN_MIN))				// out of range of LINE gain settings?
			value = LINE_GAIN_DEFAULT;				// yes, use default
		//
		ts.tx_line_gain = value;
		//printf("-->LINE gain loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Sidetone Frequency saved values
	if(Read_VirtEEPROM(EEPROM_SIDETONE_FREQ, &value) == 0)
	{
		if((value > CW_SIDETONE_FREQ_MAX) || (value < CW_SIDETONE_FREQ_MIN))				// out of range of sidetone freq settings?
			value = CW_SIDETONE_FREQ_DEFAULT;				// yes, use default
		//
		ts.sidetone_freq = value;
		//printf("-->Sidetone freq. loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Spectrum Scope Speed saved values
	if(Read_VirtEEPROM(EEPROM_SPEC_SCOPE_SPEED, &value) == 0)
	{
		if((value > SPECTRUM_SCOPE_SPEED_MAX) || (value < SPECTRUM_SCOPE_SPEED_MIN))	// out of range of spectrum scope speed settings?
			value = SPECTRUM_SCOPE_SPEED_DEFAULT;				// yes, use default
		//
		ts.scope_speed = value;
		//printf("-->Spectrum scope speed loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Spectrum Scope Filter Strength saved values
	if(Read_VirtEEPROM(EEPROM_SPEC_SCOPE_FILTER, &value) == 0)
	{
		if((value > SPECTRUM_SCOPE_FILTER_MAX) || (value < SPECTRUM_SCOPE_FILTER_MIN))	// out of range of spectrum scope filter strength settings?
			value = SPECTRUM_SCOPE_FILTER_DEFAULT;				// yes, use default
		//
		ts.scope_filter = value;
		//printf("-->Spectrum scope filter strength loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Custom AGC Decay saved values
	if(Read_VirtEEPROM(EEPROM_AGC_CUSTOM_DECAY, &value) == 0)
	{
		if(value > AGC_CUSTOM_MAX)	// out of range Custom AGC Decay settings?
			value = AGC_CUSTOM_DEFAULT;				// yes, use default
		//
		ts.agc_custom_decay = value;
		//printf("-->Custom AGC Decay setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read color for spectrum scope trace saved values
	if(Read_VirtEEPROM(EEPROM_SPECTRUM_TRACE_COLOUR, &value) == 0)
	{
		if(value > SPEC_MAX_COLOUR)	// out of range Spectrum Scope color settings?
			value = SPEC_COLOUR_TRACE_DEFAULT;				// yes, use default
		//
		ts.scope_trace_colour = value;
		//printf("-->Spectrum Scope trace color loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read color for spectrum scope grid saved values
	if(Read_VirtEEPROM(EEPROM_SPECTRUM_GRID_COLOUR, &value) == 0)
	{
		if(value > SPEC_MAX_COLOUR)	// out of range Spectrum Scope color settings?
			value = SPEC_COLOUR_GRID_DEFAULT;				// yes, use default
		//
		ts.scope_grid_colour = value;
		//printf("-->Spectrum Scope grid color loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read color for spectrum scope scale saved values
	if(Read_VirtEEPROM(EEPROM_SPECTRUM_SCALE_COLOUR, &value) == 0)
	{
		if(value > SPEC_MAX_COLOUR)	// out of range Spectrum Scope color settings?
			value = SPEC_COLOUR_SCALE_DEFAULT;				// yes, use default
		//
		ts.scope_scale_colour = value;
		//printf("-->Spectrum Scope scale color loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read paddle reverse saved values
	if(Read_VirtEEPROM(EEPROM_PADDLE_REVERSE, &value) == 0)
	{
		if(value > 1)	// out of range paddle reverse boolean settings?
			value = 0;				// yes, use default (off)
		//
		ts.paddle_reverse = value;
		//printf("-->Paddle Reverse setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read CW TX>RX Delay saved values
	if(Read_VirtEEPROM(EEPROM_CW_RX_DELAY, &value) == 0)
	{
		if(value > CW_RX_DELAY_MAX)	// out of range CW TX>RX Delay settings?
			value = CW_RX_DELAY_DEFAULT;	// yes, use default
		//
		ts.cw_rx_delay = value;
		//printf("-->CW TX>RX Delay setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read maximum volume  saved values
	if(Read_VirtEEPROM(EEPROM_MAX_VOLUME, &value) == 0)
	{
		if((value < MAX_VOLUME_MIN) || (value > MAX_VOLUME_MAX))	// out range of maximum volume settings?
			value = MAX_VOLUME_DEFAULT;	// yes, use default
		//
		ts.audio_max_volume = value;
		//printf("-->maximum volume setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 300 Hz filter saved values
	if(Read_VirtEEPROM(EEPROM_FILTER_300HZ_SEL, &value) == 0)
	{
		if(value > MAX_300HZ_FILTER)	// out range of filter settings?
			value = FILTER_300HZ_DEFAULT;	// yes, use default
		//
		ts.filter_300Hz_select = value;
		//printf("-->300 Hz filter setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 500 Hz filter saved values
	if(Read_VirtEEPROM(EEPROM_FILTER_500HZ_SEL, &value) == 0)
	{
		if(value > MAX_500HZ_FILTER)	// out range of filter settings?
			value = FILTER_500HZ_DEFAULT;	// yes, use default
		//
		ts.filter_500Hz_select = value;
		//printf("-->500 Hz filter setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 1.8 kHz filter saved values
	if(Read_VirtEEPROM(EEPROM_FILTER_1K8_SEL, &value) == 0)
	{
		if(value > MAX_1K8_FILTER)	// out range of filter settings?
			value = FILTER_1K8_DEFAULT;	// yes, use default
		//
		ts.filter_1k8_select = value;
		//printf("-->1.8 kHz filter setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 2.3 kHz filter saved values
	if(Read_VirtEEPROM(EEPROM_FILTER_2K3_SEL, &value) == 0)
	{
		if(value > MAX_2K3_FILTER)	// out range of filter settings?
			value = FILTER_2K3_DEFAULT;	// yes, use default
		//
		ts.filter_2k3_select = value;
		//printf("-->2.3 kHz filter setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 3.6 kHz filter saved values
	if(Read_VirtEEPROM(EEPROM_FILTER_3K6_SEL, &value) == 0)
	{
		if(value > MAX_3K6_FILTER)	// out range of filter settings?
			value = FILTER_3K6_DEFAULT;	// yes, use default
		//
		ts.filter_3k6_select = value;
		//printf("-->3.6 kHz filter setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 10 kHz filter saved values
	if(Read_VirtEEPROM(EEPROM_FILTER_10K_SEL, &value) == 0)
	{
		if(value > MAX_10K_FILTER)	// out range of filter settings?
			value = FILTER_10K_DEFAULT;	// yes, use default
		//
		ts.filter_10k_select = value;
		//printf("-->10 kHz filter setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read calibration factor for forward power meter
	if(Read_VirtEEPROM(EEPROM_FWD_PWR_CAL, &value) == 0)
	{
		if((value > SWR_CAL_MAX) || (value < SWR_CAL_MIN))	// out range of calibration factor for forward power meter settings?
			value = SWR_CAL_DEFAULT;	// yes, use default
		//
		swrm.fwd_cal = value;
		//printf("-->calibration factor for forward power meter setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Transverter Offset Freq saved values
	if(	(Read_VirtEEPROM(EEPROM_XVERTER_OFFSET_HIGH, &value) == 0) &&
		(Read_VirtEEPROM(EEPROM_XVERTER_OFFSET_LOW, &value1) == 0))
	{
		ulong saved = (value << 16) | (value1);

		// We have loaded from eeprom the last used band, but can't just
		// load saved frequency, as it could be out of band, so do a
		// boundary check first
		if(saved <= XVERTER_OFFSET_MAX)		// is offset within allowed limits?
		{
			ts.xverter_offset = saved;			// yes, use this value
			//printf("-->frequency loaded\n\r");
		}
		else		// it's outside allowed limites - force to zero
		{
			// Load default for this band
			ts.xverter_offset = 0;
			//printf("-->base frequency loaded\n\r");
		}
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read transverter mode enable/disable
	if(Read_VirtEEPROM(EEPROM_XVERTER_DISP, &value) == 0)
	{
		if(value > XVERTER_MULT_MAX)	// if above maximum multipler value, it was bogus
			value = 0;	// reset to "off"
		//
		ts.xverter_mode = value;
		//printf("-->transverter mode enable/disable setting loaded\n\r");
	}
	//
	//
	// ------------------------------------------------------------------------------------
	// Try to read 80 meter 5 watt power calibration setting
	if(Read_VirtEEPROM(EEPROM_BAND0_5W, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN))	// if out of range of power setting, it was bogus
			value = TX_POWER_FACTOR_80_DEFAULT;	// reset to default for this band
		//
		ts.pwr_80m_5w_adj = value;
		//printf("-->80 meter 5 watt power calibration setting loaded\n\r");
	}
	//
	//
	// ------------------------------------------------------------------------------------
	// Try to read 60 meter 5 watt power calibration setting
	if(Read_VirtEEPROM(EEPROM_BAND1_5W, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN))	// if out of range of power setting, it was bogus
			value = TX_POWER_FACTOR_60_DEFAULT;	// reset to default for this band
		//
		ts.pwr_60m_5w_adj = value;
		//printf("-->60 meter 5 watt power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 40 meter 5 watt power calibration setting
	if(Read_VirtEEPROM(EEPROM_BAND2_5W, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN))	// if out of range of power setting, it was bogus
			value = TX_POWER_FACTOR_40_DEFAULT;	// reset to default for this band
		//
		ts.pwr_40m_5w_adj = value;
		//printf("-->40 meter 5 watt power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 30 meter 5 watt power calibration setting
	if(Read_VirtEEPROM(EEPROM_BAND3_5W, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN))	// if out of range of power setting, it was bogus
			value = TX_POWER_FACTOR_30_DEFAULT;	// reset to default for this band
		//
		ts.pwr_30m_5w_adj = value;
		//printf("-->30 meter 5 watt power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 20 meter 5 watt power calibration setting
	if(Read_VirtEEPROM(EEPROM_BAND4_5W, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN))	// if out of range of power setting, it was bogus
			value = TX_POWER_FACTOR_20_DEFAULT;	// reset to default for this band
		//
		ts.pwr_20m_5w_adj = value;
		//printf("-->20 meter 5 watt power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 17 meter 5 watt power calibration setting
	if(Read_VirtEEPROM(EEPROM_BAND5_5W, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN))	// if out of range of power setting, it was bogus
			value = TX_POWER_FACTOR_17_DEFAULT;	// reset to default for this band
		//
		ts.pwr_17m_5w_adj = value;
		//printf("-->17 meter 5 watt power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 15 meter 5 watt power calibration setting
	if(Read_VirtEEPROM(EEPROM_BAND6_5W, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN))	// if out of range of power setting, it was bogus
			value = TX_POWER_FACTOR_15_DEFAULT;	// reset to default for this band
		//
		ts.pwr_15m_5w_adj = value;
		//printf("-->15 meter 5 watt power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 12 meter 5 watt power calibration setting
	if(Read_VirtEEPROM(EEPROM_BAND7_5W, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN))	// if out of range of power setting, it was bogus
			value = TX_POWER_FACTOR_12_DEFAULT;	// reset to default for this band
		//
		ts.pwr_12m_5w_adj = value;
		//printf("-->12 meter 5 watt power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 10 meter 5 watt power calibration setting
	if(Read_VirtEEPROM(EEPROM_BAND8_5W, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN))	// if out of range of power setting, it was bogus
			value = TX_POWER_FACTOR_10_DEFAULT;	// reset to default for this band
		//
		ts.pwr_10m_5w_adj = value;
		//printf("-->10 meter 5 watt power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 80 meter FULL power calibration setting
	if(Read_VirtEEPROM(EEPROM_BAND0_FULL, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN))	// if out of range of power setting, it was bogus
			value = TX_POWER_FACTOR_80_DEFAULT;	// reset to default for this band
		//
		ts.pwr_80m_full_adj = value;
		//printf("-->80 meter FULL power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 60 meter FULL power calibration setting
	if(Read_VirtEEPROM(EEPROM_BAND1_FULL, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN))	// if out of range of power setting, it was bogus
			value = TX_POWER_FACTOR_60_DEFAULT;	// reset to default for this band
		//
		ts.pwr_60m_full_adj = value;
		//printf("-->60 meter FULL power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 40 meter FULL power calibration setting
	if(Read_VirtEEPROM(EEPROM_BAND2_FULL, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN))	// if out of range of power setting, it was bogus
			value = TX_POWER_FACTOR_40_DEFAULT;	// reset to default for this band
		//
		ts.pwr_40m_full_adj = value;
		//printf("-->40 meter FULL power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 30 meter FULL power calibration setting
	if(Read_VirtEEPROM(EEPROM_BAND3_FULL, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN))	// if out of range of power setting, it was bogus
			value = TX_POWER_FACTOR_30_DEFAULT;	// reset to default for this band
		//
		ts.pwr_30m_full_adj = value;
		//printf("-->30 meter FULL power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 20 meter FULL power calibration setting
	if(Read_VirtEEPROM(EEPROM_BAND4_FULL, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN))	// if out of range of power setting, it was bogus
			value = TX_POWER_FACTOR_20_DEFAULT;	// reset to default for this band
		//
		ts.pwr_20m_full_adj = value;
		//printf("-->20 meter FULL power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 17 meter FULL power calibration setting
	if(Read_VirtEEPROM(EEPROM_BAND5_FULL, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN))	// if out of range of power setting, it was bogus
			value = TX_POWER_FACTOR_17_DEFAULT;	// reset to default for this band
		//
		ts.pwr_17m_full_adj = value;
		//printf("-->17 meter FULL power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 15 meter FULL power calibration setting
	if(Read_VirtEEPROM(EEPROM_BAND6_FULL, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN))	// if out of range of power setting, it was bogus
			value = TX_POWER_FACTOR_15_DEFAULT;	// reset to default for this band
		//
		ts.pwr_15m_full_adj = value;
		//printf("-->15 meter FULL power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 12 meter FULL power calibration setting
	if(Read_VirtEEPROM(EEPROM_BAND7_FULL, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN))	// if out of range of power setting, it was bogus
			value = TX_POWER_FACTOR_12_DEFAULT;	// reset to default for this band
		//
		ts.pwr_12m_full_adj = value;
		//printf("-->12 meter FULL power calibration setting loaded\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 10 meter FULL power calibration setting
	if(Read_VirtEEPROM(EEPROM_BAND8_FULL, &value) == 0)
	{
		if((value > TX_POWER_FACTOR_MAX) || (value < TX_POWER_FACTOR_MIN))	// if out of range of power setting, it was bogus
			value = TX_POWER_FACTOR_10_DEFAULT;	// reset to default for this band
		//
		ts.pwr_10m_full_adj = value;
		//printf("-->10 meter FULL power calibration setting loaded\n\r");
	}
	//
	// Next setting...
}
//
//
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverSaveEepromValues
//* Object              : save all values to EEPROM - called on power-down.  Does not check to see if they have changed
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriverSaveEepromValuesPowerDown(void)
{
	ushort value,value1, i;

	// Only in RX mode, if not calibrating
	if((ts.txrx_mode != TRX_MODE_RX) && (!ts.calib_mode))
		return;

	//printf("eeprom save activate\n\r");

	// ------------------------------------------------------------------------------------
	// Read Band and Mode saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_BAND_MODE, &value) == 0)
	{
		ushort new;

		new 	= 0;
		new 	= ts.band;
		new	   |= (ts.dmod_mode << 8);
		new	   |= (ts.filter_id << 12);
		Write_VirtEEPROM(EEPROM_BAND_MODE, new);
		//printf("-->band and mode saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_BAND_MODE,(ts.band |(ts.dmod_mode & 0x0f << 8) | (ts.filter_id << 12) ));
		//printf("-->band and mode var created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Freq saved values - update if changed
	if(	(Read_VirtEEPROM(EEPROM_FREQ_HIGH, &value) == 0) && (Read_VirtEEPROM(EEPROM_FREQ_LOW, &value1) == 0)) {
		Write_VirtEEPROM(EEPROM_FREQ_HIGH,(df.tune_new >> 16));
		//printf("-->freq high saved\n\r");
		Write_VirtEEPROM(EEPROM_FREQ_LOW,(df.tune_new & 0xFFFF));
		//printf("-->freq low saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_FREQ_HIGH,(df.tune_new >> 16));
		Write_VirtEEPROM(EEPROM_FREQ_LOW,(df.tune_new & 0xFFFF));
		//printf("-->freq vars created\n\r");
	}
	//
	//
	// Save stored band/mode/frequency memory from RAM
	//
	for(i = 0; i < MAX_BANDS; i++)	{	// scan through each band's frequency/mode data
		// ------------------------------------------------------------------------------------
		// Read Band and Mode saved values - update if changed
		if(Read_VirtEEPROM(EEPROM_BAND0_MODE + i, &value) == 0)
		{
			ushort new;
			new 	= 0;

			// We do NOT check the band stored in the bottom byte as we have, by definition, saved that band at this index.
			//
			new	   |= (band_decod_mode[i] << 8);
			new	   |= (band_filter_mode[i] << 12);
			Write_VirtEEPROM(EEPROM_BAND0_MODE + i, new);
		}
		else	// create
		{
			Write_VirtEEPROM(EEPROM_BAND0_MODE + i,(((band_decod_mode[i] & 0x0f) << 8) | (band_filter_mode[i] << 12) ));
			//printf("-->band and mode var created\n\r");
		}
		//
		// Try to read Freq saved values - update if changed
		//
		if((Read_VirtEEPROM(EEPROM_BAND0_FREQ_HIGH + i, &value) == 0) && (Read_VirtEEPROM(EEPROM_BAND0_FREQ_LOW + i, &value1) == 0))	{
			Write_VirtEEPROM(EEPROM_BAND0_FREQ_HIGH + i,(band_dial_value[i] >> 16));
			//printf("-->freq high saved\n\r");
			Write_VirtEEPROM(EEPROM_BAND0_FREQ_LOW + i,(band_dial_value[i] & 0xFFFF));
			//printf("-->freq low saved\n\r");
		}
		else	// create
		{
			Write_VirtEEPROM(EEPROM_BAND0_FREQ_HIGH + i,(band_dial_value[i] >> 16));
			Write_VirtEEPROM(EEPROM_BAND0_FREQ_LOW + i,(band_dial_value[i] & 0xFFFF));
			//printf("-->freq vars created\n\r");
		}
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Step saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_FREQ_STEP, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_FREQ_STEP,df.selected_idx);
		//printf("-->freq step saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_FREQ_STEP,0);
		//printf("-->freq step created\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read TX Audio Source saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_TX_AUDIO_SRC, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_TX_AUDIO_SRC,ts.tx_audio_source);
		//printf("-->TX audio source saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_TX_AUDIO_SRC,0);
		//printf("-->TX audio source created\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read TCXO saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_TCXO_STATE, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_TCXO_STATE,df.temp_enabled);
		//printf("-->TCXO state saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_TCXO_STATE,0);
		//printf("-->TCXO state created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read BIAS saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_PA_BIAS, &value) == 0)
	{

	}
	else	// create only if not exist
	{
		Write_VirtEEPROM(EEPROM_PA_BIAS,0);
		//printf("-->PA BIAS created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Audio Gain saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_AUDIO_GAIN, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_AUDIO_GAIN,ts.audio_gain);
		//printf("-->Audio Gain saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_AUDIO_GAIN,0);
		//printf("-->Audio Gain created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RF Codec Gain saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_RX_CODEC_GAIN, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_RX_CODEC_GAIN,ts.rf_codec_gain);
		//printf("-->RF Codec Gain saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_RX_CODEC_GAIN,0);
		//printf("-->RF Codec Gain created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RF Gain saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_RX_GAIN, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_RX_GAIN,ts.rf_gain);
		//printf("-->RF Gain saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_RX_GAIN,0);
		//printf("-->RF Gain created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Noise Blanker saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_NB_SETTING, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_NB_SETTING,ts.nb_setting);
		//printf("-->Attenuator saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_NB_SETTING,0);
		//printf("-->Attenuator created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read power level saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_TX_POWER_LEVEL, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_TX_POWER_LEVEL,ts.power_level);
		//printf("-->power level saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_TX_POWER_LEVEL,0);
		//printf("-->power level created\n\r");
	}

	// ------------------------------------------------------------------------------------
	// Try to read Keyer speed saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_KEYER_SPEED, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_KEYER_SPEED,ts.keyer_speed);
		//printf("-->Keyer speed saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_KEYER_SPEED,0);
		//printf("-->Keyer speed created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Keyer mode saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_KEYER_MODE, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_KEYER_MODE,ts.keyer_mode);
		//printf("-->Keyer mode saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_KEYER_MODE,0);
		//printf("-->Keyer mode created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Sidetone Gain saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_SIDETONE_GAIN, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_SIDETONE_GAIN,ts.st_gain);
		//printf("-->Sidetone Gain saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_SIDETONE_GAIN,ts.st_gain);
		//printf("-->Sidetone Gain created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Frequency Calibration saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_FREQ_CAL, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_FREQ_CAL,ts.freq_cal);
		//printf("-->Frequency Calibration saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_FREQ_CAL,ts.freq_cal);
		//printf("-->Frequency Calibration\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read AGC Mode saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_AGC_MODE, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_AGC_MODE,ts.agc_mode);
		//printf("-->AGC mode saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_AGC_MODE,ts.agc_mode);
		//printf("-->AGC mode created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Microphone Gain saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_MIC_GAIN, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_MIC_GAIN,ts.tx_mic_gain);
		//printf("-->Microphone Gain saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_MIC_GAIN,ts.tx_mic_gain);
		//printf("-->Microphone Gain created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Line Gain saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_LINE_GAIN, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_LINE_GAIN,ts.tx_line_gain);
		//printf("-->Line Gain saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_LINE_GAIN,0);
		//printf("-->Line Gain created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Sidetone Frequency saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_SIDETONE_FREQ, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_SIDETONE_FREQ,ts.sidetone_freq);
		//printf("-->Sidetone Freq saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_SIDETONE_FREQ,0);
		//printf("-->Sidetone Freq created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Spectrum Scope Speed saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_SPEC_SCOPE_SPEED, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_SPEC_SCOPE_SPEED,ts.scope_speed);
		//printf("-->Spectrum Scope Speed saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_SPEC_SCOPE_SPEED,0);
		//printf("-->Spectrum Scope Speed created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Spectrum Scope Filter Strength saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_SPEC_SCOPE_FILTER, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_SPEC_SCOPE_FILTER,ts.scope_filter);
		//printf("-->Spectrum Scope Filter saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_SPEC_SCOPE_FILTER,0);
		//printf("-->Spectrum Scope Speed created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read AGC Custom Decay saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_AGC_CUSTOM_DECAY, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_AGC_CUSTOM_DECAY,ts.agc_custom_decay);
		//printf("-->AGC Custom Decay value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_AGC_CUSTOM_DECAY,0);
		//printf("-->AGC Custom Decay value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Scope trace color saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_SPECTRUM_TRACE_COLOUR, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_SPECTRUM_TRACE_COLOUR,ts.scope_trace_colour);
		//printf("-->Scope Trace Color value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_SPECTRUM_TRACE_COLOUR,0);
		//printf("-->Scope Trace Color value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Scope grid color saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_SPECTRUM_GRID_COLOUR, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_SPECTRUM_GRID_COLOUR,ts.scope_grid_colour);
		//printf("-->Scope Grid Color value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_SPECTRUM_GRID_COLOUR,0);
		//printf("-->Scope Grid Color value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Scope scale color saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_SPECTRUM_SCALE_COLOUR, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_SPECTRUM_SCALE_COLOUR,ts.scope_scale_colour);
		//printf("-->Scope Scale Color value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_SPECTRUM_SCALE_COLOUR,0);
		//printf("-->Scope Scale Color value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Paddle Reversal saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_PADDLE_REVERSE, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_PADDLE_REVERSE,ts.paddle_reverse);
		//printf("-->Scope Paddle Reverse value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_PADDLE_REVERSE,0);
		//printf("-->Paddle Reverse value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read CW TX>RX Delay saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_CW_RX_DELAY, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_CW_RX_DELAY,ts.cw_rx_delay);
		//printf("-->CW TX>RX Delay value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_CW_RX_DELAY,0);
		//printf("-->CW TX>RX Delay value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Max. volume saved values - update if changed
	if(Read_VirtEEPROM(EEPROM_MAX_VOLUME, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_MAX_VOLUME,ts.audio_max_volume);
		//printf("-->Max. volume value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_MAX_VOLUME,0);
		//printf("-->Max. volume value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 300 Hz filter values - update if changed
	if(Read_VirtEEPROM(EEPROM_FILTER_300HZ_SEL, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_FILTER_300HZ_SEL,ts.filter_300Hz_select);
		//printf("-->300 Hz filter value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_FILTER_300HZ_SEL,0);
		//printf("-->300 Hz filter value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 500 Hz filter values - update if changed
	if(Read_VirtEEPROM(EEPROM_FILTER_500HZ_SEL, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_FILTER_500HZ_SEL,ts.filter_500Hz_select);
		//printf("-->500 Hz filter value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_FILTER_500HZ_SEL,0);
		//printf("-->500 Hz filter value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 1.8 kHz filter values - update if changed
	if(Read_VirtEEPROM(EEPROM_FILTER_1K8_SEL, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_FILTER_1K8_SEL,ts.filter_1k8_select);
		//printf("-->1.8 kHz filter value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_FILTER_1K8_SEL,0);
		//printf("-->1.8 kHz filter value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 2.3 kHz filter values - update if changed
	if(Read_VirtEEPROM(EEPROM_FILTER_2K3_SEL, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_FILTER_2K3_SEL,ts.filter_2k3_select);
		//printf("-->2.3 kHz filter value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_FILTER_2K3_SEL,0);
		//printf("-->2.3 kHz filter value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 3.6 kHz filter values - update if changed
	if(Read_VirtEEPROM(EEPROM_FILTER_3K6_SEL, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_FILTER_3K6_SEL,ts.filter_3k6_select);
		//printf("-->3.6 kHz filter value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_FILTER_3K6_SEL,0);
		//printf("-->3.6 kHz filter value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 10 kHz filter values - update if changed
	if(Read_VirtEEPROM(EEPROM_FILTER_10K_SEL, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_FILTER_10K_SEL,ts.filter_10k_select);
		//printf("-->10 kHz filter value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_FILTER_10K_SEL,0);
		//printf("-->10 kHz filter value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read PA Bias values - update if changed
	if(Read_VirtEEPROM(EEPROM_PA_BIAS, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_PA_BIAS,ts.pa_bias);
		//printf("-->PA Bias value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_PA_BIAS,0);
		//printf("-->PA Bias value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read TX IQ LSB Gain Balance values - update if changed
	if(Read_VirtEEPROM(EEPROM_TX_IQ_LSB_GAIN_BALANCE, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_TX_IQ_LSB_GAIN_BALANCE, ts.tx_iq_lsb_gain_balance);
		//printf("-->TX IQ LSB Gain balance saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_TX_IQ_LSB_GAIN_BALANCE,0);
		//printf("-->TX IQ LSB Gain balance value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read TX IQ USB Gain Balance values - update if changed
	if(Read_VirtEEPROM(EEPROM_TX_IQ_USB_GAIN_BALANCE, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_TX_IQ_USB_GAIN_BALANCE, ts.tx_iq_usb_gain_balance);
		//printf("-->TX IQ USB Gain balance saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_TX_IQ_USB_GAIN_BALANCE,0);
		//printf("-->TX IQ USB Gain balance value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read TX IQ LSB Phase Balance values - update if changed
	if(Read_VirtEEPROM(EEPROM_TX_IQ_LSB_PHASE_BALANCE, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_TX_IQ_LSB_PHASE_BALANCE, ts.tx_iq_lsb_phase_balance);
		//printf("-->TX IQ LSB Phase balance saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_TX_IQ_LSB_PHASE_BALANCE,0);
		//printf("-->TX IQ LSB Phase balance value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read TX IQ USB Phase Balance values - update if changed
	if(Read_VirtEEPROM(EEPROM_TX_IQ_USB_PHASE_BALANCE, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_TX_IQ_USB_PHASE_BALANCE, ts.tx_iq_usb_phase_balance);
		//printf("-->TX IQ USB Phase balance saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_TX_IQ_USB_PHASE_BALANCE,0);
		//printf("-->TX IQ USB Phase balance value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RX IQ LSB Gain Balance values - update if changed
	if(Read_VirtEEPROM(EEPROM_RX_IQ_LSB_GAIN_BALANCE, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_RX_IQ_LSB_GAIN_BALANCE, ts.rx_iq_lsb_gain_balance);
		//printf("-->RX IQ LSB Gain balance saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_RX_IQ_LSB_GAIN_BALANCE,0);
		//printf("-->RX IQ LSB Gain balance value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RX IQ USB Gain Balance values - update if changed
	if(Read_VirtEEPROM(EEPROM_RX_IQ_USB_GAIN_BALANCE, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_RX_IQ_USB_GAIN_BALANCE, ts.rx_iq_usb_gain_balance);
		//printf("-->RX IQ USB Gain balance saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_RX_IQ_USB_GAIN_BALANCE,0);
		//printf("-->RX IQ USB Gain balance value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RX IQ LSB Phase Balance values - update if changed
	if(Read_VirtEEPROM(EEPROM_RX_IQ_LSB_PHASE_BALANCE, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_RX_IQ_LSB_PHASE_BALANCE, ts.rx_iq_lsb_phase_balance);
		//printf("-->RX IQ LSB Phase balance saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_RX_IQ_LSB_PHASE_BALANCE,0);
		//printf("-->RX IQ LSB Phase balance value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read RX IQ USB Phase Balance values - update if changed
	if(Read_VirtEEPROM(EEPROM_RX_IQ_USB_PHASE_BALANCE, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_RX_IQ_USB_PHASE_BALANCE, ts.rx_iq_usb_phase_balance);
		//printf("-->RX IQ USB Phase balance saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_RX_IQ_USB_PHASE_BALANCE,0);
		//printf("-->RX IQ USB Phase balance value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read SWR Forward power meter calibration value - update if changed
	if(Read_VirtEEPROM(EEPROM_FWD_PWR_CAL, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_FWD_PWR_CAL, swrm.fwd_cal);
		//printf("-->SWR Forward power meter calibration value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_FWD_PWR_CAL,0);
		//printf("-->SWR Forward power meter calibration value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Transverter frequency offset saved values - update if changed
	if(	(Read_VirtEEPROM(EEPROM_XVERTER_OFFSET_HIGH, &value) == 0) && (Read_VirtEEPROM(EEPROM_XVERTER_OFFSET_LOW, &value1) == 0)) {
		Write_VirtEEPROM(EEPROM_XVERTER_OFFSET_HIGH,(ts.xverter_offset >> 16));
		//printf("-->freq high saved\n\r");
		Write_VirtEEPROM(EEPROM_XVERTER_OFFSET_LOW,(ts.xverter_offset & 0xFFFF));
		//printf("-->freq low saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_XVERTER_OFFSET_HIGH,(ts.xverter_offset >> 16));
		Write_VirtEEPROM(EEPROM_XVERTER_OFFSET_LOW,(ts.xverter_offset & 0xFFFF));
		//printf("-->Transverter offset frequency created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read Transverter display offset mode enable - update if changed
	if(Read_VirtEEPROM(EEPROM_XVERTER_DISP, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_XVERTER_DISP, ts.xverter_mode);
		//printf("-->Transverter display offset mode enable value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_XVERTER_DISP,0);
		//printf("-->Transverter display offset mode enable value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 80m 5 watt power setting - update if changed
	if(Read_VirtEEPROM(EEPROM_BAND0_5W, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_BAND0_5W, ts.pwr_80m_5w_adj);
		//printf("-->80m 5 watt power setting value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_BAND0_5W,0);
		//printf("-->80m 5 watt power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 60m 5 watt power setting - update if changed
	if(Read_VirtEEPROM(EEPROM_BAND1_5W, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_BAND1_5W, ts.pwr_60m_5w_adj);
		//printf("-->60m 5 watt power setting value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_BAND1_5W,0);
		//printf("-->60m 5 watt power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 40m 5 watt power setting - update if changed
	if(Read_VirtEEPROM(EEPROM_BAND2_5W, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_BAND2_5W, ts.pwr_40m_5w_adj);
		//printf("-->40m 5 watt power setting value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_BAND2_5W,0);
		//printf("-->40m 5 watt power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 30m 5 watt power setting - update if changed
	if(Read_VirtEEPROM(EEPROM_BAND3_5W, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_BAND3_5W, ts.pwr_30m_5w_adj);
		//printf("-->80m 5 watt power setting value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_BAND3_5W,0);
		//printf("-->30m 5 watt power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 20m 5 watt power setting - update if changed
	if(Read_VirtEEPROM(EEPROM_BAND4_5W, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_BAND4_5W, ts.pwr_20m_5w_adj);
		//printf("-->20m 5 watt power setting value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_BAND4_5W,0);
		//printf("-->20m 5 watt power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 17m 5 watt power setting - update if changed
	if(Read_VirtEEPROM(EEPROM_BAND5_5W, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_BAND5_5W, ts.pwr_17m_5w_adj);
		//printf("-->17m 5 watt power setting value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_BAND5_5W,0);
		//printf("-->17m 5 watt power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 15m 5 watt power setting - update if changed
	if(Read_VirtEEPROM(EEPROM_BAND6_5W, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_BAND6_5W, ts.pwr_15m_5w_adj);
		//printf("-->15m 5 watt power setting value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_BAND6_5W,0);
		//printf("-->15m 5 watt power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 12m 5 watt power setting - update if changed
	if(Read_VirtEEPROM(EEPROM_BAND7_5W, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_BAND7_5W, ts.pwr_12m_5w_adj);
		//printf("-->12m 5 watt power setting value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_BAND7_5W,0);
		//printf("-->12m 5 watt power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 10m 5 watt power setting - update if changed
	if(Read_VirtEEPROM(EEPROM_BAND8_5W, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_BAND8_5W, ts.pwr_10m_5w_adj);
		//printf("-->10m 5 watt power setting value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_BAND8_5W,0);
		//printf("-->10m 5 watt power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 80m FULL power setting - update if changed
	if(Read_VirtEEPROM(EEPROM_BAND0_FULL, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_BAND0_FULL, ts.pwr_80m_full_adj);
		//printf("-->80m FULL power setting value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_BAND0_FULL,0);
		//printf("-->80m FULL power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 60m FULL power setting - update if changed
	if(Read_VirtEEPROM(EEPROM_BAND1_FULL, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_BAND1_FULL, ts.pwr_60m_full_adj);
		//printf("-->60m FULL power setting value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_BAND1_FULL,0);
		//printf("-->60m FULL power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 40m FULL power setting - update if changed
	if(Read_VirtEEPROM(EEPROM_BAND2_FULL, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_BAND2_FULL, ts.pwr_40m_full_adj);
		//printf("-->40m FULL power setting value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_BAND2_FULL,0);
		//printf("-->40m FULL power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 30m FULL power setting - update if changed
	if(Read_VirtEEPROM(EEPROM_BAND3_FULL, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_BAND3_FULL, ts.pwr_30m_full_adj);
		//printf("-->80m FULL power setting value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_BAND3_FULL,0);
		//printf("-->30m FULL power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 20m FULL power setting - update if changed
	if(Read_VirtEEPROM(EEPROM_BAND4_FULL, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_BAND4_FULL, ts.pwr_20m_full_adj);
		//printf("-->20m FULL power setting value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_BAND4_FULL,0);
		//printf("-->20m FULL power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 17m FULL power setting - update if changed
	if(Read_VirtEEPROM(EEPROM_BAND5_FULL, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_BAND5_FULL, ts.pwr_17m_full_adj);
		//printf("-->17m FULL power setting value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_BAND5_FULL,0);
		//printf("-->17m FULL power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 15m FULL power setting - update if changed
	if(Read_VirtEEPROM(EEPROM_BAND6_FULL, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_BAND6_FULL, ts.pwr_15m_full_adj);
		//printf("-->15m FULL power setting value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_BAND6_FULL,0);
		//printf("-->15m FULL power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 12m FULL power setting - update if changed
	if(Read_VirtEEPROM(EEPROM_BAND7_FULL, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_BAND7_FULL, ts.pwr_12m_full_adj);
		//printf("-->12m FULL power setting value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_BAND7_FULL,0);
		//printf("-->12m FULL power setting value created\n\r");
	}
	//
	// ------------------------------------------------------------------------------------
	// Try to read 10m FULL power setting - update if changed
	if(Read_VirtEEPROM(EEPROM_BAND8_FULL, &value) == 0)
	{
		Write_VirtEEPROM(EEPROM_BAND8_FULL, ts.pwr_10m_full_adj);
		//printf("-->10m FULL power setting value saved\n\r");
	}
	else	// create
	{
		Write_VirtEEPROM(EEPROM_BAND8_FULL,0);
		//printf("-->10m FULL power setting value created\n\r");
	}
	//
	//
	//
	// Next setting...
}
