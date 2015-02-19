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

#include "codec.h"
#include "i2s.h"
#include "cw_gen.h"

#include <limits.h>

#include "audio_driver.h"
#include "ui_driver.h"

// SSB filters - now handled in ui_driver to allow I/Q phase adjustment

//#include "filters/q_rx_filter.h"
//#include "filters/i_rx_filter.h"
//#include "filters/q_tx_filter.h"
//#include "filters/i_tx_filter.h"

// Audio filters
#include "filters/fir_10k.h"
#include "filters/fir_3_6k.h"
#include "filters/fir_2_3k.h"
#include "filters/fir_1_8k.h"
//
// 48kHz IIR lattice ARMA filters with time-reversed elements
//
#include "filters/iir_300hz.h"
#include "filters/iir_500hz.h"
#include "filters/iir_1_8k.h"
#include "filters/iir_2_3k.h"
#include "filters/iir_3_6k.h"
#include "filters/iir_10k.h"

#include "filters/iir_2k7_tx_bpf.h"
//
#include "filters/fir_rx_decimate_4.h"	// with low-pass filtering
#include "filters/fir_rx_decimate_4_min_lpf.h"	// This has minimized LPF for the 10 kHz filter mode
#include "filters/fir_rx_interpolate_16.h"	// filter for interpolate-by-16 operation
#include "filters/fir_rx_interpolate_16_10kHz.h"	// This has relaxed LPF for the 10 kHz filter mode

static void Audio_Init(void);

// ---------------------------------
// DMA buffers for I2S
__IO int16_t 	tx_buffer[BUFF_LEN+1];
__IO int16_t	rx_buffer[BUFF_LEN+1];

int16_t	test_a[5000];	// grab a large chunk of RAM - for testing, and to prevent "memory leak" anomalies (kludgy work-around)
//
float32_t	lms1_nr_delay[LMS_NR_DELAYBUF_SIZE_MAX+16];
//
float32_t	lms2_nr_delay[LMS_NOTCH_DELAYBUF_SIZE_MAX + 16];
//
//
float32_t errsig1[65];
float32_t errsig2[65];
float32_t result[65];
//
float32_t					osc_q[IQ_BUFSZ];
float32_t					osc_i[IQ_BUFSZ];

//
// LMS Filters for RX
arm_lms_norm_instance_f32	lms1Norm_instance;
arm_lms_instance_f32	lms1_instance;
float32_t	lms1StateF32[192];
float32_t	lms1NormCoeff_f32[192];
//
arm_lms_norm_instance_f32	lms2Norm_instance;
arm_lms_instance_f32	lms2_instance;
float32_t	lms2StateF32[192];
float32_t	lms2NormCoeff_f32[192];
//
//
//
float32_t	agc_delay	[AGC_DELAY_BUFSIZE+16];
//
//
int16_t	test_b[5000];	// grab a large chunk of RAM - for testing, and to prevent "memory leak" anomalies (kludgy work-around)
//
// Audio RX - Decimator
static	arm_fir_decimate_instance_f32	DECIMATE_RX;
__IO float32_t			decimState[FIR_RXAUDIO_BLOCK_SIZE + FIR_RXAUDIO_NUM_TAPS - 1];
//
int16_t	test_c[1000];	// grab a large chunk of RAM - for testing, and to prevent "memory leak" anomalies (kludgy work-around)
//
// Audio RX - Interpolator
static	arm_fir_interpolate_instance_f32 INTERPOLATE_RX;
__IO float32_t			interpState[FIR_RXAUDIO_BLOCK_SIZE + FIR_RXAUDIO_NUM_TAPS - 1];
//
int16_t	test_d[1000];	// grab a large chunk of RAM - for testing, and to prevent "memory leak" anomalies (kludgy work-around)
//
// variables for RX IIR filters
static float32_t		iir_rx_state[FIR_RXAUDIO_BLOCK_SIZE+FIR_RXAUDIO_NUM_TAPS-1];
static arm_iir_lattice_instance_f32	IIR_PreFilter;
//
int16_t	test_e[1000];	// grab a large chunk of RAM - for testing, and to prevent "memory leak" anomalies (kludgy work-around)
//
//
// variables for TX IIR filter
//
float32_t		iir_tx_state[FIR_RXAUDIO_BLOCK_SIZE+FIR_RXAUDIO_NUM_TAPS-1];
arm_iir_lattice_instance_f32	IIR_TXFilter;

//
// RX Hilbert transform (90 degree) FIR filters
//

__IO	arm_fir_instance_f32 	FIR_I;
__IO	arm_fir_instance_f32 	FIR_Q;
//
__IO	arm_fir_instance_f32	FIR_I_TX;
__IO	arm_fir_instance_f32	FIR_Q_TX;
// ---------------------------------

// Transceiver state public structure
extern __IO TransceiverState 	ts;

// Public paddle state
extern __IO PaddleState			ps;

// Spectrum display public
__IO	SpectrumDisplay			sd;

// Audio driver publics
__IO	AudioDriverState		ads;

// S meter public
__IO	SMeter					sm;
//
// Keypad driver publics
extern __IO	KeypadState				ks;
//
extern __IO	FilterCoeffs		fc;
//
//*----------------------------------------------------------------------------
//* Function Name       : audio_driver_init
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void audio_driver_init(void)
{
	ulong word_size;


#ifdef DEBUG_BUILD
	printf("audio driver init...\n\r");
#endif

	word_size = WORD_SIZE_16;

	// CW module init
	cw_gen_init();

	// Audio filter disabled
	ads.af_dissabled = 1;

	// Reset S meter public
	sm.skip		= 0;
	sm.s_count	= 0;
	sm.curr_max	= 0;
	sm.gain_calc = 0;	// gain calculation used for S-meter

	ads.agc_val = 1;			// Post AF Filter gain (AGC)
	ads.alc_val = 1;			// TX audio auto-level-control (ALC)
	//
	ads.decimation_rate	=	RX_DECIMATION_RATE_12KHZ;		// Decimation rate, when enabled
	//
	// Set AGC rate
	//
	if(ts.agc_mode == AGC_SLOW)
		ads.agc_decay = AGC_SLOW_DECAY;
	else if(ts.agc_mode == AGC_FAST)
		ads.agc_decay = AGC_FAST_DECAY;
	else if(ts.agc_mode == AGC_CUSTOM)	{	// calculate custom AGC setting
		ads.agc_decay = (float)ts.agc_custom_decay;
		ads.agc_decay += 30;
		ads.agc_decay /= 10;
		ads.agc_decay = -ads.agc_decay;
		ads.agc_decay = powf(10, ads.agc_decay);
	}
	else
		ads.agc_decay = AGC_MED_DECAY;
	//
	// get RF gain value
	ads.agc_rf_gain = (float)ts.rf_gain;
	ads.agc_rf_gain -= 20;
	ads.agc_rf_gain /= 10;
	ads.agc_rf_gain = powf(10, ads.agc_rf_gain);
	//
	//
	// calculate ALC decay (release) time constant
	//
	ads.alc_decay = (float)ts.alc_decay;
	ads.alc_decay += 35;
	ads.alc_decay /= 10;
	ads.alc_decay *= -1;
	ads.alc_decay = powf(10, ads.alc_decay);
	//
	ads.pre_filter_gain = 8;		// Pre AF filter gain (AGC)
	//
	// calculate values that set the maximum AGC sensitivity/lowest S-meter reading
	//
	ads.agc_knee = AGC_KNEE;
	ads.agc_val_max = AGC_VAL_MAX_REF / MAX_RF_GAIN_DEFAULT+1;

	if(ts.max_rf_gain <= MAX_RF_GAIN_MAX)	{
		ads.agc_knee = AGC_KNEE_REF * (float)(ts.max_rf_gain + 1);
		ads.agc_val_max = AGC_VAL_MAX_REF / ((float)(ts.max_rf_gain + 1));
		ads.post_agc_gain = POST_AGC_GAIN_SCALING_REF / (float)(ts.max_rf_gain + 1);
	}
	else	{
		ads.agc_knee = AGC_KNEE_REF * MAX_RF_GAIN_DEFAULT+1;
		ads.agc_val_max = AGC_VAL_MAX_REF / MAX_RF_GAIN_DEFAULT+1;
		ads.post_agc_gain = POST_AGC_GAIN_SCALING_REF /  (float)(ts.max_rf_gain + 1);
	}
	//
	UiCalcNB_AGC();		// set up noise blanker AGC values
	//
	UiCWSidebandMode();	// set up CW sideband mode setting
	//
	ads.Osc_Cos = CONV_NCO_COS;
	ads.Osc_Sin = CONV_NCO_SIN;
	ads.Osc_Vect_Q = 1;
	ads.Osc_Vect_I = 0;
	ads.Osc_Gain = 0;
	ads.Osc_Q = 0;
	ads.Osc_I = 0;
	//
	ads.tx_filter_adjusting = 0;	// used to disable TX I/Q filter during adjustment
	// Audio init
	Audio_Init();

	// Codec init
	Codec_Init(ts.samp_rate,word_size);

	// Codec settle delay
	non_os_delay();

	// I2S hardware init
	I2S_Block_Init();

	// Start DMA transfers
	I2S_Block_Process((uint32_t)&tx_buffer, (uint32_t)&rx_buffer, BUFF_LEN);

	// Audio filter enabled
	ads.af_dissabled = 0;

#ifdef DEBUG_BUILD
	printf("audio driver init ok\n\r");
#endif
}

//*----------------------------------------------------------------------------
//* Function Name       : audio_driver_stop
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void audio_driver_stop(void)
{
	I2S_Block_Stop();
}

//*----------------------------------------------------------------------------
//* Function Name       : audio_driver_thread
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void audio_driver_thread(void)
{
	// nothing
}

//*----------------------------------------------------------------------------
//* Function Name       : audio_driver_set_rx_audio_filter
//* Object              :
//* Object              : select audio filter
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
// WARNING:  You CANNOT reliably use the built-in IIR and FIR "init" functions when using CONST-based coefficient tables!  If you do so, you risk filters
//	not initializing properly!  If you use the "init" functions, you MUST copy CONST-based coefficient tables to RAM first!
//  This information is from recommendations by online references for using ARM math/DSP functions
//

void audio_driver_set_rx_audio_filter(void)
{
	uint32_t	i;
	float	mu_calc;
	bool	dsp_inhibit_temp;

	// Lock - temporarily disable filter

	dsp_inhibit_temp = ts.dsp_inhibit;
	ts.dsp_inhibit = 1;	// disable DSP while doing adjustment
	ads.af_dissabled = 1;

	switch(ts.filter_id)	{
		case AUDIO_300HZ:
		    IIR_PreFilter.numStages = IIR_300hz_numStages;		// number of stages
		    if(ts.filter_300Hz_select == 1)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_300hz_500_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_300hz_500_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_300Hz_select == 2)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_300hz_550_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_300hz_550_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_300Hz_select == 3)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_300hz_600_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_300hz_600_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_300Hz_select == 4)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_300hz_650_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_300hz_650_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_300Hz_select == 5)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_300hz_700_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_300hz_700_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_300Hz_select == 7)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_300hz_800_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_300hz_800_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_300Hz_select == 8)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_300hz_850_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_300hz_850_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_300Hz_select == 9)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_300hz_900_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_300hz_900_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_300Hz_select == 10)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_300hz_950_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_300hz_950_pvCoeffs;	// point to ladder coefficients
		    }
		    else	{	// default value
				IIR_PreFilter.pkCoeffs = (float *)IIR_300hz_750_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_300hz_750_pvCoeffs;	// point to ladder coefficients
		    }
			break;
		case AUDIO_500HZ:
		    IIR_PreFilter.numStages = IIR_500hz_numStages;		// number of stages
		    if(ts.filter_500Hz_select == 1)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_500hz_550_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_500hz_550_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_500Hz_select == 2)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_500hz_650_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_500hz_650_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_500Hz_select == 4)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_500hz_850_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_500hz_850_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_500Hz_select == 5)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_500hz_950_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_500hz_950_pvCoeffs;	// point to ladder coefficients
		    }
		    else	{	// default value
				IIR_PreFilter.pkCoeffs = (float *)IIR_500hz_750_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_500hz_750_pvCoeffs;	// point to ladder coefficients
		    }
			break;
		case AUDIO_1P8KHZ:
		    IIR_PreFilter.numStages = IIR_1k8_numStages;		// number of stages
		    if(ts.filter_1k8_select == 1)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_1k8_1k125_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_1k8_1k125_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_1k8_select == 2)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_1k8_1k275_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_1k8_1k275_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_1k8_select == 4)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_1k8_1k575_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_1k8_1k575_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_1k8_select == 5)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_1k8_1k725_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_1k8_1k725_pvCoeffs;	// point to ladder coefficients
		    }
		    else	{	// default value
				IIR_PreFilter.pkCoeffs = (float *)IIR_1k8_1k425_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_1k8_1k425_pvCoeffs;	// point to ladder coefficients
		    }
			break;
		case AUDIO_2P3KHZ:
		    IIR_PreFilter.numStages = IIR_2k3_numStages;		// number of stages
		    if(ts.filter_2k3_select == 1)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_2k3_1k275_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_2k3_1k275_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_2k3_select == 3)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_2k3_1k562_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_2k3_1k562_pvCoeffs;	// point to ladder coefficients
		    }
		    else if(ts.filter_2k3_select == 4)	{
				IIR_PreFilter.pkCoeffs = (float *)IIR_2k3_1k712_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_2k3_1k712_pvCoeffs;	// point to ladder coefficients
		    }
		    else	{	// default value
				IIR_PreFilter.pkCoeffs = (float *)IIR_2k3_1k412_pkCoeffs;	// point to reflection coefficients
				IIR_PreFilter.pvCoeffs = (float *)IIR_2k3_1k412_pvCoeffs;	// point to ladder coefficients
		    }
			break;
		case AUDIO_3P6KHZ:
		    IIR_PreFilter.numStages = IIR_3k6_numStages;		// number of stages
			IIR_PreFilter.pkCoeffs = (float *)IIR_3k6_pkCoeffs;	// point to reflection coefficients
			IIR_PreFilter.pvCoeffs = (float *)IIR_3k6_pvCoeffs;	// point to ladder coefficients
			break;
		case AUDIO_10KHZ:
		    IIR_PreFilter.numStages = IIR_10k_numStages;		// number of stages
			IIR_PreFilter.pkCoeffs = (float *)IIR_10k_pkCoeffs;	// point to reflection coefficients
			IIR_PreFilter.pvCoeffs = (float *)IIR_10k_pvCoeffs;	// point to ladder coefficients
			break;
		default:
			break;
	}
	//
	// Initialize IIR filter state buffer
 	//
    for(i = 0; i < FIR_RXAUDIO_BLOCK_SIZE+FIR_RXAUDIO_NUM_TAPS-1; i++)	{	// initialize state buffer to zeroes
    	iir_rx_state[i] = 0;
    }
	IIR_PreFilter.pState = (float32_t *)&iir_rx_state;					// point to state array for IIR filter

	//
	// Initialize LMS (DSP Noise reduction) filter
	// It is (sort of) initalized "twice" since this it what it seems to take for the LMS function to
	// start reliably and consistently!
	//
	uint16_t	calc_taps;
	//
	if((ts.dsp_nr_numtaps < DSP_NR_NUMTAPS_MIN) || (ts.dsp_nr_numtaps > DSP_NR_NUMTAPS_MAX))
		calc_taps = DSP_NR_NUMTAPS_DEFAULT;
	else
		calc_taps = (uint16_t)ts.dsp_nr_numtaps;
	//
	// Load settings into instance structure
	//
	// LMS instance 1 is pre-AGC DSP NR
	// LMS instance 3 is post-AGC DSP NR
	//
	lms1Norm_instance.numTaps = calc_taps;
	lms1Norm_instance.pCoeffs = lms1NormCoeff_f32;
	lms1Norm_instance.pState = lms1StateF32;
	//
	// Calculate "mu" (convergence rate) from user "DSP Strength" setting.  This needs to be significantly de-linearized to
	// squeeze a wide range of adjustment (e.g. several magnitudes) into a fairly small numerical range.
	//
	mu_calc = (float)ts.dsp_nr_strength;		// get user setting
	/*
	mu_calc = DSP_NR_STRENGTH_MAX-mu_calc;		// invert (0 = minimum))
	mu_calc /= 2.6;								// scale calculation
	mu_calc *= mu_calc;							// square value
	mu_calc += 1;								// offset by one
	mu_calc /= 40;								// rescale
	mu_calc += 1;								// prevent negative log result
	mu_calc = log10f(mu_calc);					// de-linearize
	lms1Norm_instance.mu = mu_calc;				//
	*/
	//
	// New DSP NR "mu" calculation method as of 0.0.214
	//
	mu_calc /= 2;	// scale input value
	mu_calc += 2;	// offset zero value
	mu_calc /= 10;	// convert from "bels" to "deci-bels"
	mu_calc = powf(10,mu_calc);		// convert to ratio
	mu_calc = 1/mu_calc;			// invert to fraction
	lms1Norm_instance.mu = mu_calc;

	// Debug display of mu calculation
//	char txt[16];
//	sprintf(txt, " %d ", (ulong)(mu_calc * 10000));
//	UiLcdHy28_PrintText(POS_BOTTOM_BAR_F3_X,POS_BOTTOM_BAR_F3_Y,txt,0xFFFF,0,0);
//
//
	for(i = 0; i < LMS_NR_DELAYBUF_SIZE_MAX; i++)	{		// clear LMS delay buffers
		lms1_nr_delay[i] = 0;
	}
	//
	for(i = 0; i < 192; i++)	{		// clear LMS state buffer
		lms1StateF32[i] = 0;			// zero state buffer
		if(ts.reset_dsp_nr)	{			// are we to reset the coefficient buffer as well?
			lms1NormCoeff_f32[i] = 0;		// yes - zero coefficient buffers
		}
	}
	//
	// use "canned" init to initialize the filter coefficients
	//
	arm_lms_norm_init_f32(&lms1Norm_instance, calc_taps, &lms1NormCoeff_f32[0], &lms1StateF32[0], (float32_t)mu_calc, 64);
	//
	//
	if((ts.dsp_nr_delaybuf_len > DSP_NR_BUFLEN_MAX) || (ts.dsp_nr_delaybuf_len < DSP_NR_BUFLEN_MIN))
			ts.dsp_nr_delaybuf_len = DSP_NR_BUFLEN_DEFAULT;
	//
	// LMS instance 2 - Automatic Notch Filter
	//
	calc_taps = DSP_NOTCH_NUM_TAPS;
	lms2Norm_instance.numTaps = calc_taps;
	lms2Norm_instance.pCoeffs = lms2NormCoeff_f32;
	lms2Norm_instance.pState = lms2StateF32;
	//
	// Calculate "mu" (convergence rate) from user "Notch ConvRate" setting
	//
	mu_calc = (float)ts.dsp_notch_mu;		// get user setting
	mu_calc = mu_calc;		// (0 = slowest)
	mu_calc += 1;
	mu_calc /= 1500;
	mu_calc += 1;
	mu_calc = log10f(mu_calc);
	//
	// use "canned" init to initialize the filter coefficients
	//
	arm_lms_norm_init_f32(&lms2Norm_instance, calc_taps, &lms2NormCoeff_f32[0], &lms2StateF32[0], (float32_t)mu_calc, 64);

	//
	for(i = 0; i < LMS_NR_DELAYBUF_SIZE_MAX; i++)		// clear LMS delay buffer
		lms2_nr_delay[i] = 0;
	//
	for(i = 0; i < 192; i++)	{		// clear LMS state and coefficient buffers
		lms2StateF32[i] = 0;			// zero state buffer
		if(ts.reset_dsp_nr)				// are we to reset the coefficient buffer?
			lms2NormCoeff_f32[i] = 0;		// yes - zero coefficient buffer
	}
	//
	if((ts.dsp_notch_delaybuf_len > DSP_NOTCH_BUFLEN_MAX) || (ts.dsp_notch_delaybuf_len < DSP_NOTCH_BUFLEN_MIN))
				ts.dsp_nr_delaybuf_len = DSP_NOTCH_DELAYBUF_DEFAULT;
	//
	// Adjust decimation rate based on selected filter
	//
	if(ts.filter_id != AUDIO_10KHZ)	{		// Not 10 kHz receiver bandwidth
		ads.decimation_rate = RX_DECIMATION_RATE_12KHZ;
		DECIMATE_RX.pCoeffs = &FirRxDecimate[0];		// Filter coefficients for lower-rate (slightly strong LPF)
		INTERPOLATE_RX.pCoeffs = &FirRxInterpolate[0];	// Filter coefficients
	}
	else	{								// This *IS* the 10 kHz receiver bandwidth
		ads.decimation_rate = RX_DECIMATION_RATE_24KHZ;
		DECIMATE_RX.pCoeffs = &FirRxDecimateMinLPF[0];	// Filter coefficients for higher rate (weak LPF:  Hilbert is used for main LPF!)
		INTERPOLATE_RX.pCoeffs = &FirRxInterpolate10KHZ[0];	// Filter coefficients for higher rate (relaxed LPF)
	}
	//
	ads.agc_decimation_scaling = (float)ads.decimation_rate;
	ads.agc_delay_buflen = AGC_DELAY_BUFSIZE/(ulong)ads.decimation_rate;	// calculate post-AGC delay based on post-decimation sampling rate
	//
    // Set up RX decimation/filter
	DECIMATE_RX.M = ads.decimation_rate;			// Decimation factor  (48 kHz / 4 = 12 kHz)
	DECIMATE_RX.numTaps = RX_DECIMATE_NUM_TAPS;		// Number of taps in FIR filter

	DECIMATE_RX.pState = &decimState[0];			// Filter state variables
	//
	// Set up RX interpolation/filter
	// NOTE:  Phase Length MUST be an INTEGER and is the number of taps divided by the decimation rate, and it must be greater than 1.
	//
	INTERPOLATE_RX.L = ads.decimation_rate;			// Interpolation factor, L  (12 kHz * 4 = 48 kHz)
	INTERPOLATE_RX.phaseLength = RX_INTERPOLATE_NUM_TAPS/ads.decimation_rate;	// Phase Length ( numTaps / L )
	INTERPOLATE_RX.pState = &interpState[0];		// Filter state variables
	//
	for(i = 0; i < FIR_RXAUDIO_BLOCK_SIZE; i++)	{	// Initialize all filter state variables
		decimState[i] = 0;
		interpState[i] = 0;
	}
	//
	ads.dsp_zero_count = 0;		// initialize "zero" count to detect if DSP has crashed
	//
	// Unlock - re-enable filtering
	//
	ads.af_dissabled = 0;
	ts.dsp_inhibit = dsp_inhibit_temp;
	//
}

uchar audio_check_nr_dsp_state(void)
{
	uchar i;

	for(i = 0; i < 192; i++)	{
		if((lms1NormCoeff_f32[i] <= -1) || (lms1NormCoeff_f32[i] >= 1))
			return(1);
	}

	return(0);

}
//
//*----------------------------------------------------------------------------
//* Function Name       : Audio_Init
//* Object              :
//* Object              : init filters
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void Audio_Init(void)
{
	uint32_t	i;

	// -------------------
	// Init TX audio filter - Do so "manually" since built-in init functions don't work with CONST coefficients
	//
	IIR_TXFilter.numStages = IIR_TX_2k7_numStages;		// number of stages
	IIR_TXFilter.pkCoeffs = (float *)IIR_TX_2k7_pkCoeffs;	// point to reflection coefficients
	IIR_TXFilter.pvCoeffs = (float *)IIR_TX_2k7_pvCoeffs;	// point to ladder coefficients

    for(i = 0; i < FIR_RXAUDIO_BLOCK_SIZE+FIR_RXAUDIO_NUM_TAPS-1; i++)	{	// initialize state buffer to zeroes
    	iir_tx_state[i] = 0;
    }
    IIR_TXFilter.pState = (float32_t *)&iir_tx_state;


    // Decimation/Interpolation is set up "manually" because the built-in functions do NOT work reliably with coefficients
    // stored in CONST memory!
    //
	if((ts.dsp_nr_delaybuf_len < DSP_NR_BUFLEN_MIN) || (ts.dsp_nr_delaybuf_len > DSP_NR_BUFLEN_MAX))
		ts.dsp_nr_delaybuf_len = DSP_NR_BUFLEN_DEFAULT;
	//
	// -------------------
	// Init RX audio filters
	audio_driver_set_rx_audio_filter();
}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : audio_rx_noise_blanker
//* Object              : noise blanker
//* Object              :
//* Input Parameters    : I/Q 16 bit audio data, size of buffer
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void audio_rx_noise_blanker(int16_t *src, int16_t size)
{
	static int16_t	delay_buf[34];
	static uchar	delbuf_inptr = 0, delbuf_outptr = 2;
	ulong	i;
	float	sig;
	float  nb_short_setting;
	static float avg_sig;
	static	uchar	nb_delay = 0;
	static float	nb_agc = 0;
	//
	if((!ts.nb_setting) || (ts.nb_disable) || (ts.dmod_mode == DEMOD_AM) || (ts.filter_id == AUDIO_10KHZ))	{// bail out if noise blanker disabled, in AM mode, or set to 10 kHz
		return;
	}

	nb_short_setting = (float)ts.nb_setting;		// convert and rescale NB1 setting for higher resolution in adjustment
	nb_short_setting /= 2;

	for(i = 0; i < size/2; i+=4)	{		// Noise blanker function - "Unrolled" 4x for maximum execution speed
		//
		sig = (float)abs(*src);		// get signal amplitude.  We need only look at one of the two audio channels since they will be the same.
		sig /= ads.codec_gain_calc;	// Scale for codec A/D gain adjustment
		//
//		avg_sig = (avg_sig * NB_AVG_WEIGHT) + ((float)(*src) * NB_SIG_WEIGHT);	// IIR-filtered short-term average signal level (e.g. low-pass audio)
		//
		delay_buf[delbuf_inptr++] = *src;	// copy first byte into delay buffer
		delay_buf[delbuf_inptr++] = *(src+1);	// copy second byte into delay buffer
		//
		nb_agc = (ads.nb_agc_filt * nb_agc) + (ads.nb_sig_filt * sig);		// IIR-filtered "AGC" of current overall signal level
		//
		if(((sig) > (nb_agc * (((MAX_NB_SETTING/2) + 1.75) - nb_short_setting))) && (nb_delay == 0))	{	// did a pulse exceed the threshold?
			nb_delay = 16;		// yes - set the blanking duration counter
		}
		//
		if(!nb_delay)	{		// blank counter not active
			*src = delay_buf[delbuf_outptr++];		// pass through delayed audio, unchanged
			*(src+1) = delay_buf[delbuf_outptr++];
		}
		else	{	// It is within the blanking pulse period
			*src = 0; // (int16_t)avg_sig;		// set the audio buffer to "mute" during the blanking period
			*(src+1) = 0; //(int16_t)avg_sig;
			nb_delay--;						// count down the number of samples that we are to blank
		}
		//
//		delbuf_inptr += 2;					// update count of input of circular delay buffer
//		delbuf_outptr = delbuf_inptr + 2;	// output is always just "after" output of delay buffer
		delbuf_outptr &= 0x1e;				// set limit to count, forcing lsb of pointer to zero.
		delbuf_inptr &= 0x1e;
		//
		src++;								// update pointer to source material
		src++;								// (the I/Q pair of audio data)
		//
		// Next "unrolled" instance
		//
		sig = (float)abs(*src);		// get signal amplitude.  We need only look at one of the two audio channels since they will be the same.
		sig /= ads.codec_gain_calc;	// Scale for codec A/D gain adjustment
		//
//		avg_sig = (avg_sig * NB_AVG_WEIGHT) + ((float)(*src) * NB_SIG_WEIGHT);	// IIR-filtered short-term average signal level (e.g. low-pass audio)
		//
		delay_buf[delbuf_inptr++] = *src;	// copy first byte into delay buffer
		delay_buf[delbuf_inptr++] = *(src+1);	// copy second byte into delay buffer
		//
		nb_agc = (NB_AGC_FILT * nb_agc) + (NB_SIG_FILT * sig);		// IIR-filtered "AGC" of current overall signal level
		//
		if(((sig) > (nb_agc * (((MAX_NB_SETTING/2) + 1.75) - nb_short_setting))) && (nb_delay == 0))	{	// did a pulse exceed the threshold?
			nb_delay = 16;		// yes - set the blanking duration counter
		}
		//
		if(!nb_delay)	{		// blank counter not active
			*src = delay_buf[delbuf_outptr++];		// pass through delayed audio, unchanged
			*(src+1) = delay_buf[delbuf_outptr++];
		}
		else	{	// It is within the blanking pulse period
			*src = 0; // (int16_t)avg_sig;		// set the audio buffer to "mute" during the blanking period
			*(src+1) = 0; //(int16_t)avg_sig;
			nb_delay--;						// count down the number of samples that we are to blank
		}
		//
//		delbuf_inptr += 2;					// update count of input of circular delay buffer
//		delbuf_outptr = delbuf_inptr + 2;	// output is always just "after" output of delay buffer
		delbuf_outptr &= 0x1e;				// set limit to count, forcing lsb of pointer to zero.
		delbuf_inptr &= 0x1e;
		//
		src++;								// update pointer to source material
		src++;								// (the I/Q pair of audio data)
		//
		// Next "unrolled" instance
		//
		sig = (float)abs(*src);		// get signal amplitude.  We need only look at one of the two audio channels since they will be the same.
		sig /= ads.codec_gain_calc;	// Scale for codec A/D gain adjustment
		//
//		avg_sig = (avg_sig * NB_AVG_WEIGHT) + ((float)(*src) * NB_SIG_WEIGHT);	// IIR-filtered short-term average signal level (e.g. low-pass audio)
		//
		delay_buf[delbuf_inptr++] = *src;	// copy first byte into delay buffer
		delay_buf[delbuf_inptr++] = *(src+1);	// copy second byte into delay buffer
		//
		nb_agc = (NB_AGC_FILT * nb_agc) + (NB_SIG_FILT * sig);		// IIR-filtered "AGC" of current overall signal level
		//
		if(((sig) > (nb_agc * (((MAX_NB_SETTING/2) + 1.75) - nb_short_setting))) && (nb_delay == 0))	{	// did a pulse exceed the threshold?
			nb_delay = 16;		// yes - set the blanking duration counter
		}
		//
		if(!nb_delay)	{		// blank counter not active
			*src = delay_buf[delbuf_outptr++];		// pass through delayed audio, unchanged
			*(src+1) = delay_buf[delbuf_outptr++];
		}
		else	{	// It is within the blanking pulse period
			*src = 0; // (int16_t)avg_sig;		// set the audio buffer to "mute" during the blanking period
			*(src+1) = 0; //(int16_t)avg_sig;
			nb_delay--;						// count down the number of samples that we are to blank
		}
		//
//		delbuf_inptr += 2;					// update count of input of circular delay buffer
//		delbuf_outptr = delbuf_inptr + 2;	// output is always just "after" output of delay buffer
		delbuf_outptr &= 0x1e;				// set limit to count, forcing lsb of pointer to zero.
		delbuf_inptr &= 0x1e;
		//
		src++;								// update pointer to source material
		src++;								// (the I/Q pair of audio data)
		//
		// Last "unrolled" instance
		//
		sig = (float)abs(*src);		// get signal amplitude.  We need only look at one of the two audio channels since they will be the same.
		sig /= ads.codec_gain_calc;	// Scale for codec A/D gain adjustment
		//
//		avg_sig = (avg_sig * NB_AVG_WEIGHT) + ((float)(*src) * NB_SIG_WEIGHT);	// IIR-filtered short-term average signal level (e.g. low-pass audio)
		//
		delay_buf[delbuf_inptr++] = *src;	// copy first byte into delay buffer
		delay_buf[delbuf_inptr++] = *(src+1);	// copy second byte into delay buffer
		//
		nb_agc = (NB_AGC_FILT * nb_agc) + (NB_SIG_FILT * sig);		// IIR-filtered "AGC" of current overall signal level
		//
		if(((sig) > (nb_agc * (((MAX_NB_SETTING/2) + 1.75) - nb_short_setting))) && (nb_delay == 0))	{	// did a pulse exceed the threshold?
			nb_delay = 16;		// yes - set the blanking duration counter
		}
		//
		if(!nb_delay)	{		// blank counter not active
			*src = delay_buf[delbuf_outptr++];		// pass through delayed audio, unchanged
			*(src+1) = delay_buf[delbuf_outptr++];
		}
		else	{	// It is within the blanking pulse period
			*src = 0; // (int16_t)avg_sig;		// set the audio buffer to "mute" during the blanking period
			*(src+1) = 0; //(int16_t)avg_sig;
			nb_delay--;						// count down the number of samples that we are to blank
		}
		//
//		delbuf_inptr += 2;					// update count of input of circular delay buffer
//		delbuf_outptr = delbuf_inptr + 2;	// output is always just "after" output of delay buffer
		delbuf_outptr &= 0x1e;				// set limit to count, forcing lsb of pointer to zero.
		delbuf_inptr &= 0x1e;
		//
		src++;								// update pointer to source material
		src++;								// (the I/Q pair of audio data)
	}
}

//
//*----------------------------------------------------------------------------
//* Function Name       : audio_rx_freq_conv
//* Object              : Does I/Q frequency conversion
//* Object              :
//* Input Parameters    : size of array on which to work; dir: determines direction of shift - see below;  Also uses variables in ads structure
//* Output Parameters   : uses variables in ads structure
//* Functions called    :
//*----------------------------------------------------------------------------
static void audio_rx_freq_conv(int16_t size, int16_t dir)
{
	ulong 		i;
	float32_t	rad_calc;
//	static float32_t	q_temp, i_temp;
	static bool flag = 0;
	//
	// Below is the "on-the-fly" version of the frequency translater, generating a "live" version of the oscillator, which can be any
	// frequency, based on the values of "ads.Osc_Cos" and "ads.Osc_Sin".  While this does function, the generation of the SINE takes a LOT
	// of processor time!
	// This version also lacks the "dir" (direction) control which selects either high or low side translation.
/*
	for(i = 0; i < size/2; i++)	{
		// generate local oscillator on-the-fly:  This takes a lot of processor time!
		ads.Osc_Q = (ads.Osc_Vect_Q * ads.Osc_Cos) - (ads.Osc_Vect_I * ads.Osc_Sin);	// Q channel of oscillator
		ads.Osc_I = (ads.Osc_Vect_I * ads.Osc_Cos) + (ads.Osc_Vect_Q * ads.Osc_Sin);	// I channel of oscillator
		ads.Osc_Gain = 1.95 - ((ads.Osc_Vect_Q * ads.Osc_Vect_Q) + (ads.Osc_Vect_I * ads.Osc_Vect_I));	// Amplitude control of oscillator
		// rotate vectors while maintaining constant oscillator amplitude
		ads.Osc_Vect_Q = ads.Osc_Gain * ads.Osc_Q;
		ads.Osc_Vect_I = ads.Osc_Gain * ads.Osc_I;
		// do frequency conversion
		i_temp = ads.i_buffer[i];
		q_temp = ads.q_buffer[i];
		ads.i_buffer[i] = (i_temp * ads.Osc_Q) + (q_temp * ads.Osc_I);
		ads.q_buffer[i] = (q_temp * ads.Osc_Q) - (i_temp * ads.Osc_I);
		//
	}
*/
	//
	// Below is the frequency translation code that uses a "pre-calculated" sine wave - which means that the translation must be done at a sub-
	// multiple of the sample frequency.  This pre-calculation eliminates the processor overhead required to generate a sine wave on the fly.
	// This also makes extensive use of the optimized ARM vector instructions for the calculation of the final I/Q vectors
	//
	// Pre-calculate 6 kHz quadrature sine wave ONCE for the conversion
	//
	if(!flag)	{		// have we already calculated the sine wave?
		for(i = 0; i < size/2; i++)	{		// No, let's do it!
			rad_calc = (float32_t)i;		// convert to float the current position within the buffer
			rad_calc /= (size/2);			// make this a fraction
			rad_calc *= (PI * 2);			// convert to radians
			rad_calc *= 4;					// multiply by number of cycles that we want within this block (4 = 6 kHz)
			//
			osc_q[i] = arm_cos_f32(rad_calc);	// get sine and cosine values and store in pre-calculated array
			osc_i[i] = arm_sin_f32(rad_calc);
		}
		flag = 1;	// signal that once we have generated the quadrature sine waves, we should not do it again
	}
	//
	// Do frequency conversion
	//
	if(!dir)	{	// Conversion is "above" on RX (LO needs to be set lower)
		arm_mult_f32(ads.i_buffer, osc_q, ads.a_buffer, size/2);	// multiply products for converted I channel
		arm_mult_f32(ads.q_buffer, osc_i, ads.b_buffer, size/2);
		//
		arm_mult_f32(ads.q_buffer, osc_q, ads.c_buffer, size/2);	// multiply products for converted Q channel
		arm_mult_f32(ads.i_buffer, osc_i, ads.d_buffer, size/2);
		//
		arm_add_f32(ads.a_buffer, ads.b_buffer, ads.i_buffer, size/2);	// summation for I channel
		arm_sub_f32(ads.c_buffer, ads.d_buffer, ads.q_buffer, size/2);	// difference for Q channel
	}
	else	{	// Conversion is "below" on RX (LO needs to be set higher)
		arm_mult_f32(ads.q_buffer, osc_q, ads.a_buffer, size/2);	// multiply products for converted I channel
		arm_mult_f32(ads.i_buffer, osc_i, ads.b_buffer, size/2);
		//
		arm_mult_f32(ads.i_buffer, osc_q, ads.c_buffer, size/2);	// multiply products for converted Q channel
		arm_mult_f32(ads.q_buffer, osc_i, ads.d_buffer, size/2);
		//
		arm_add_f32(ads.a_buffer, ads.b_buffer, ads.q_buffer, size/2);	// summation for I channel
		arm_sub_f32(ads.c_buffer, ads.d_buffer, ads.i_buffer, size/2);	// difference for Q channel
	}
}

//
//*----------------------------------------------------------------------------
//* Function Name       : audio_rx_processor
//* Object              :
//* Object              : audio sample processor
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void audio_rx_processor(int16_t *src, int16_t *dst, int16_t size)
{

	static ulong 		i;
	static float 		f_sum;
	static float		max_signal, min_signal;
	uint32_t			result_index;
	static float		agc_calc;
	static float		audio_temp;
	float				am_agc;
	//
	int16_t				psize;		// processing size, with decimation
	//
	static ulong		lms1_inbuf = 0, lms1_outbuf = 0;
	static ulong		lms2_inbuf = 0, lms2_outbuf = 0;
	static ulong		agc_delay_inbuf = 0, agc_delay_outbuf = 0;
	//
	float				post_agc_gain_scaling;


	psize = size/(int16_t)ads.decimation_rate;	// rescale sample size inside decimated portion based on decimation factor
	//
	//
	audio_rx_noise_blanker(src, size);		// do noise blanker function
	//
	max_signal = 0;		// Init peak detector - used as part of secondary AGC loop, below
	//
	// ------------------------
	// Split stereo channels
	for(i = 0; i < size/2; i++)
	{
		//
		// Collect I/Q samples
		if(sd.state == 0)
		{
			sd.FFT_Samples[sd.samp_ptr] = *(src + 1);
			sd.samp_ptr++;
			sd.FFT_Samples[sd.samp_ptr] = *(src);
			sd.samp_ptr++;

			// On overload, update state machine,
			// reset pointer and wait
			if(sd.samp_ptr >= FFT_IQ_BUFF_LEN)
			{
				sd.samp_ptr = 0;
				sd.state    = 1;
			}
		}
		//
		if(*src > ADC_CLIP_WARN_THRESHOLD/4)	{		// This is the release threshold for the auto RF gain
			ads.adc_quarter_clip = 1;
			if(*src > ADC_CLIP_WARN_THRESHOLD/2)	{		// This is the trigger threshold for the auto RF gain
					ads.adc_half_clip = 1;
					if(*src > ADC_CLIP_WARN_THRESHOLD)			// This is the threshold for the red clip indicator on S-meter
						ads.adc_clip = 1;
			}
		}
		//
		// 16 bit format - convert to float and increment
		ads.i_buffer[i] = (float32_t)*src++;
		ads.q_buffer[i] = (float32_t)*src++;
		//
	}
	//
	// Apply gain corrections for pre-filter AGC and I/Q gain balancing
	//
	audio_temp = ads.pre_filter_gain * (float)ts.rx_adj_gain_var_i;		// calculate pre-filter AGC and gain factors for I channel
	arm_scale_f32(ads.i_buffer, (float32_t)audio_temp, ads.i_buffer, size/2);	// do both gain calculations in one operation
	//
	audio_temp = ads.pre_filter_gain * (float)ts.rx_adj_gain_var_q;		// calculate pre-filter AGC and gain factors for Q channel
	arm_scale_f32(ads.q_buffer, (float32_t)audio_temp, ads.q_buffer, size/2);	// do both gain calculations in one operation
	//
	//
	if(ts.iq_freq_mode)	{		// is receive frequency conversion to be done?
		if(ts.iq_freq_mode == 1)			// RX LO LOW mode
			audio_rx_freq_conv(size, 1);
		else								// RX LO HIGH mode
			audio_rx_freq_conv(size, 0);
	}
	//
	// ------------------------
	// IQ SSB processing - Do 0-90 degree Phase-added Hilbert Transform
	// *** *EXCEPT* in AM mode - see the function "UiCalcRxPhaseAdj()"
	//    In AM, the FIR below does ONLY low-pass filtering appropriate for the filter bandwidth selected when in AM mode, in
	//	  which case there is NO audio phase shift applied to the I/Q channels.
	//
	arm_fir_f32(&FIR_I,(float32_t *)(ads.i_buffer),(float32_t *)(ads.i_buffer),size/2);	// shift 0 degree FIR
	arm_fir_f32(&FIR_Q,(float32_t *)(ads.q_buffer),(float32_t *)(ads.q_buffer),size/2);	// shift +90 degrees FIR (plus RX IQ phase adjustment)
	//
	//	Demodulation, optimized using fast ARM math functions as much as possible
	//
	switch(ts.dmod_mode)	{
		case DEMOD_USB:
		case DEMOD_DIGI:
			arm_add_f32(ads.i_buffer, ads.q_buffer, ads.a_buffer, size/2);	// sum of I and Q
			break;
		case DEMOD_LSB:
			arm_sub_f32(ads.i_buffer, ads.q_buffer, ads.a_buffer, size/2);	// difference of I and Q
			break;
		case DEMOD_CW:
			if(!ts.cw_lsb)	// is this USB RX mode?  (LSB of mode byte was zero)
				arm_add_f32(ads.i_buffer, ads.q_buffer, ads.a_buffer, size/2);	// sum of I and Q
			else	// No, it is LSB RX mode
				arm_sub_f32(ads.i_buffer, ads.q_buffer, ads.a_buffer, size/2);	// difference of I and Q
			break;
		case DEMOD_AM:
			arm_mult_f32(ads.i_buffer, ads.i_buffer, ads.a_buffer, size/2);		// square I - store in buffer "a"
			arm_mult_f32(ads.q_buffer, ads.q_buffer, ads.b_buffer, size/2);		// square Q - store in buffer "b"
			arm_add_f32(ads.a_buffer, ads.b_buffer, ads.a_buffer, size/2);		// sum squares - store in buffer "a"
			for(i = 0; i < size/2; i++)	{										// square root of contents
				arm_sqrt_f32(ads.a_buffer[i], &ads.a_buffer[i]);				// Unroll this function for maximum MCU efficiency
				i++;
				arm_sqrt_f32(ads.a_buffer[i], &ads.a_buffer[i]);
				i++;
				arm_sqrt_f32(ads.a_buffer[i], &ads.a_buffer[i]);
				i++;
				arm_sqrt_f32(ads.a_buffer[i], &ads.a_buffer[i]);
			}
			arm_mean_f32(ads.a_buffer, size/2, &am_agc);	// get "average" value of "a" buffer - the recovered DC value - for the AGC
			am_agc *= AM_SCALING;	// rescale AM AGC to match SSB scaling
			break;
		default:	// do USB demod as default if we end up here!
			arm_add_f32(ads.i_buffer, ads.q_buffer, ads.a_buffer, size/2);	// sum of I and Q
			break;
	}

	//
	// Do decimation down to lower rate for heavy-duty processing to reduce processor load
	//
	arm_fir_decimate_f32(&DECIMATE_RX, ads.a_buffer, ads.a_buffer, size/2);		// LPF built into decimation (Yes, you can decimate-in-place!)
	//
	// DSP Automatic Notch Filter using LMS (Least Mean Squared) algorithm
	//
	if((!ads.af_dissabled) && (ts.dsp_active & 4) && (ts.dmod_mode != DEMOD_CW) && (!ts.dsp_inhibit))	{	// No notch in CW mode
		arm_copy_f32(ads.a_buffer, &lms2_nr_delay[lms2_inbuf], psize/2);	// put new data into the delay buffer
		//
		arm_lms_norm_f32(&lms2Norm_instance, ads.a_buffer, &lms2_nr_delay[lms2_outbuf], errsig2, ads.a_buffer, psize/2);
		//
		lms2_inbuf += psize/2;
		lms2_outbuf = lms2_inbuf + psize/2;
		if(lms2_inbuf >= ts.dsp_notch_delaybuf_len-1)
			lms2_inbuf -= ts.dsp_notch_delaybuf_len;
		if(lms2_outbuf >= ts.dsp_notch_delaybuf_len-1)
			lms2_outbuf -= ts.dsp_notch_delaybuf_len;
	}
	//
	// DSP noise reduction using LMS (Least Mean Squared) algorithm
	// This is the pre-filter/AGC instance
	//
	if((ts.dsp_active & 1) && (!(ts.dsp_active & 2)) && (!ads.af_dissabled) && (!ts.dsp_inhibit))	{	// Do this if enabled and "Pre-AGC" DSP NR enabled
		arm_copy_f32(ads.a_buffer, &lms1_nr_delay[lms1_inbuf], psize/2);	// put new data into the delay buffer
		//
		arm_lms_norm_f32(&lms1Norm_instance, ads.a_buffer, &lms1_nr_delay[lms1_outbuf], ads.a_buffer, errsig1 ,psize/2);
		//
		// Detect if the DSP output has gone to (near) zero output - a sign of it crashing!
		//
		if((((ulong)fabs(ads.a_buffer[0])) * DSP_ZERO_DET_MULT_FACTOR) < DSP_OUTPUT_MINVAL)	{	// is DSP level too low?
			// For some stupid reason we can't just compare above to a small fractional value  (e.g. "x < 0.001") so we must multiply it first!
			if(ads.dsp_zero_count < MAX_DSP_ZERO_COUNT)	{
				ads.dsp_zero_count++;
			}
		}
		else
			ads.dsp_zero_count = 0;
		//
		ads.dsp_nr_sample = ads.a_buffer[0];		// provide a sample of the DSP output for additional crash detection
		//
		lms1_inbuf += psize/2;	// bump input to the next location
		lms1_outbuf = lms1_inbuf + psize/2;	// advance output to same distance ahead of input
		if(lms1_inbuf >= ts.dsp_nr_delaybuf_len-1)		// handle wrap-around of circular buffers
			lms1_inbuf -= ts.dsp_nr_delaybuf_len;
		if(lms1_outbuf >= ts.dsp_nr_delaybuf_len-1)
			lms1_outbuf -= ts.dsp_nr_delaybuf_len;
	}
	//
	// ------------------------
	// Apply audio filter
	if((!ads.af_dissabled)	&& (ts.filter_id != AUDIO_10KHZ))	{	// we don't need to filter if running in 10 kHz mode (Hilbert filter does the job!)
		// IIR ARMA-type lattice filter
		arm_iir_lattice_f32(&IIR_PreFilter, (float *)ads.a_buffer, (float *)ads.a_buffer, psize/2);
	}
	//
	// find maximum signal in buffer
	arm_max_f32(ads.a_buffer, psize/2, &max_signal, &result_index);	// find maximum value (positive)
	arm_min_f32(ads.a_buffer, psize/2, &min_signal, &result_index);	// find "maximum" value (negative)
	min_signal = fabs(min_signal);		// convert negative peak to positive
	if(min_signal > max_signal)			// pick whichever is the "strongest"
		max_signal = min_signal;
	//
	// AGC function (KA7OEI)
	//
	for(i = 0; i < psize/2; i++)	{
		if(ts.agc_mode != AGC_OFF)	{
			if(ts.dmod_mode == DEMOD_AM)		// if in AM, get the recovered DC voltage
				agc_calc = am_agc * ads.agc_val;
			else								// not AM - get the amplitude of the recovered audio
				agc_calc = max_signal * ads.agc_val;	// calculate current level by scaling it with AGC value
			//
			if(agc_calc < ads.agc_knee)	{	// is audio below AGC "knee" value?
				ads.agc_val += ads.agc_val*ads.agc_decay * ads.agc_decimation_scaling;	// Yes - Increase gain slowly for AGC DECAY - scale time constant with decimation
			}
			else	{
				ads.agc_val -= ads.agc_val * AGC_ATTACK;	// Fast attack to increase attenuation (do NOT scale w/decimation or else oscillation results)
				if(ads.agc_val <= AGC_VAL_MIN)	// Prevent zero or "negative" gain values
					ads.agc_val = AGC_VAL_MIN;
			}
			if(ads.agc_val >= ads.agc_rf_gain)	// limit AGC to reasonable values when low/no signals present
				ads.agc_val = ads.agc_rf_gain;
				if(ads.agc_val >= ads.agc_val_max)	// limit maximum gain under no-signal conditions
					ads.agc_val = ads.agc_val_max;
		}
		else	// AGC Off - manual AGC gain
			ads.agc_val = ads.agc_rf_gain;
		//
		ads.agc_valbuf[i] = ads.agc_val;			// store in "running" AGC history buffer for later application to audio data
	}

	//
	// Delay the post-AGC audio slightly so that the AGC's "attack" will very slightly lead the audio being acted upon by the AGC.
	// This eliminates a "click" that can occur when a very strong signal appears due to the AGC lag.  The delay is adjusted based on
	// decimation rate so that it is constant for all settings.
	//
	arm_copy_f32(ads.a_buffer, &agc_delay[agc_delay_inbuf], psize/2);	// put new data into the delay buffer
	arm_copy_f32(&agc_delay[agc_delay_outbuf], ads.a_buffer, psize/2);	// take old data out of the delay buffer
	// Update the in/out pointers to the AGC delay buffer
	agc_delay_inbuf += psize/2;
	agc_delay_outbuf = agc_delay_inbuf + psize/2;
	if(agc_delay_inbuf >= ads.agc_delay_buflen-1)
		agc_delay_inbuf -= ads.agc_delay_buflen;
	if(agc_delay_outbuf >= ads.agc_delay_buflen-1)
		agc_delay_outbuf -= ads.agc_delay_buflen;

	//
	// Now apply pre-calculated AGC values to delayed audio
	//
	arm_mult_f32(ads.a_buffer, ads.agc_valbuf, ads.a_buffer, psize/2);		// do vector multiplication to apply delayed "running" AGC data
	//
	//
	// DSP noise reduction using LMS (Least Mean Squared) algorithm
	// This is the post-filter, post-AGC instance
	//
	if((ts.dsp_active & 1) && (ts.dsp_active & 2) && (!ads.af_dissabled) && (!ts.dsp_inhibit))	{	// Do DSP NR if enabled and if post-DSP NR enabled
		arm_copy_f32(ads.a_buffer, &lms1_nr_delay[lms1_inbuf], psize/2);	// put new data into the delay buffer
		//
		arm_lms_norm_f32(&lms1Norm_instance, ads.a_buffer, &lms1_nr_delay[lms1_outbuf], ads.a_buffer, errsig1, psize/2);
		//
		// "DC" bias detection to determine if the DSP has crashed
		//
		// Detect if the DSP output has gone to (near) zero output - a sign of it crashing!
		//
		if((((ulong)fabs(ads.a_buffer[0])) * DSP_ZERO_DET_MULT_FACTOR) < DSP_OUTPUT_MINVAL)	{	// is DSP level too low?
			// For some stupid reason we can't just compare it to a small fractional value  (e.g. "x < 0.001") so we must multiply it first!
			if(ads.dsp_zero_count < MAX_DSP_ZERO_COUNT)	{
				ads.dsp_zero_count++;
			}
		}
		else
			ads.dsp_zero_count = 0;
		//
		ads.dsp_nr_sample = ads.a_buffer[0];		// provide a sample of the DSP output for additional crash detection
		//
		lms1_inbuf += psize/2;
		lms1_outbuf = lms1_inbuf + psize/2;
		if(lms1_inbuf >= ts.dsp_nr_delaybuf_len-1)
			lms1_inbuf -= ts.dsp_nr_delaybuf_len;
		if(lms1_outbuf >= ts.dsp_nr_delaybuf_len-1)
			lms1_outbuf -= ts.dsp_nr_delaybuf_len;
	}
	if(ts.filter_id != AUDIO_10KHZ)
		post_agc_gain_scaling = POST_AGC_GAIN_SCALING_DECIMATE_4;
	else
		post_agc_gain_scaling = POST_AGC_GAIN_SCALING_DECIMATE_2;
	//
	// Scale audio to according to AGC setting, demodulation mode and required fixed levels
	//
	if(ts.dmod_mode == DEMOD_AM)
		arm_scale_f32(ads.a_buffer,(float32_t)(ads.post_agc_gain * post_agc_gain_scaling * (AM_SCALING * AM_AUDIO_SCALING)), ads.a_buffer, psize/2);	// apply fixed amount of audio gain scaling to make the audio levels correct along with AGC
	else
		arm_scale_f32(ads.a_buffer,(float32_t)(ads.post_agc_gain * post_agc_gain_scaling), ads.a_buffer, psize/2);	// apply fixed amount of audio gain scaling to make the audio levels correct along with AGC
	//
	// resample back to original sample rate while doing low-pass filtering to minimize aliasing effects
	arm_fir_interpolate_f32(&INTERPOLATE_RX, ads.a_buffer, ads.b_buffer, psize/2);
	//
	//
	arm_scale_f32(ads.b_buffer, LINE_OUT_SCALING_FACTOR, ads.a_buffer, size/2);		// Do fixed scaling of audio for LINE OUT and copy to "a" buffer
	//
	// AF gain in "ts.audio_gain-active"
	//  0 - 16: via codec command
	// 17 - 20: soft gain after decoder
	//
	if(ts.audio_gain > 16)	// is volume control above highest hardware setting?
		arm_scale_f32(ads.b_buffer, (float32_t)ts.audio_gain_active, ads.b_buffer, size/2);	// yes, do software volume control adjust on "b" buffer
	//
	// Transfer processed audio to DMA buffer
	//
	i = 0;			// init sample transfer counter
	while(i < size/2)	{						// transfer to DMA buffer and do conversion to INT - Unrolled to speed it up
		*dst++ = (int16_t)ads.b_buffer[i];		// Speaker channel (variable level)
		*dst++ = (int16_t)ads.a_buffer[i++];		// LINE OUT (constant level)

		*dst++ = (int16_t)ads.b_buffer[i];		// Speaker channel (variable level)
		*dst++ = (int16_t)ads.a_buffer[i++];		// LINE OUT (constant level)

		*dst++ = (int16_t)ads.b_buffer[i];		// Speaker channel (variable level)
		*dst++ = (int16_t)ads.a_buffer[i++];		// LINE OUT (constant level)

		*dst++ = (int16_t)ads.b_buffer[i];		// Speaker channel (variable level)
		*dst++ = (int16_t)ads.a_buffer[i++];		// LINE OUT (constant level)
	}
	//
	// Process for pre-filter AGC gain.  This is done outside the main loop since it is only updated once per DMA cycle, anyway...
	//
	ads.pre_filter_gain = 1;

	/*
	if(max_signal > AGC_PREFILTER_MAX_SIGNAL)	{	// Is the signal EXTREMELY strong?
		ads.pre_filter_gain -= AGC_PREFILTER_ATTACK_RATE;		// decrease pre-filter gain
	}
	else if(max_signal < AGC_PREFILTER_MAX_SIGNAL/4)	{	// Is the signal weaker?
		ads.pre_filter_gain += AGC_PREFILTER_DECAY_RATE;		// yes, increase gain a bit, now
	}
	// catch RF gain if it goes above the setting of the RF gain control
	if(ads.pre_filter_gain > AGC_PREFILTER_MAXGAIN)	{	// it is above the RFG setting, so...
		ads.pre_filter_gain = AGC_PREFILTER_MAXGAIN;	// limit maximum rf gain to RFG setting
	}
	if(ads.pre_filter_gain < AGC_PREFILTER_MINGAIN)		// Prevent gain from being reduced too much
		ads.pre_filter_gain = AGC_PREFILTER_MINGAIN;
	*/
	//
}

//*----------------------------------------------------------------------------
//* Function Name       : audio_tx_processor
//* Object              :
//* Object              : audio sample processor
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void audio_tx_processor(int16_t *src, int16_t *dst, int16_t size)
{
	static ulong 		i;
	static float		alc_calc;
	float				gain_calc;
	static ulong		alc_delay_inbuf = 0, alc_delay_outbuf = 0;
	int16_t				*ptr;

	// -----------------------------
	// TUNE mode handler for CW mode
	//
	if((ts.tune) && ((ts.dmod_mode != DEMOD_LSB) && (ts.dmod_mode != DEMOD_AM) && (ts.dmod_mode != DEMOD_USB)))	// Tune mode - but NOT in USB/LSB/AM mode
	{
		softdds_runf(ads.i_buffer,ads.q_buffer,size/2);

		// Output I and Q as stereo
		for(i = 0; i < size/2; i++)
		{
			// Equalise based on band
			ads.i_buffer[i] *= (float)ts.tx_power_factor;
			ads.q_buffer[i] *= (float)ts.tx_power_factor;

			// Adjust TX I and Q gain
			ads.i_buffer[i] *= (float)ts.tx_adj_gain_var_i;		// offset gain by equal and opposite directions
			ads.q_buffer[i] *= (float)ts.tx_adj_gain_var_q;		//

			if(!ts.cw_lsb)	{	// CW in USB mode
				*dst++ = (int16_t)ads.i_buffer[i];	// save left channel
				*dst++ = (int16_t)ads.q_buffer[i];	// save right channel
			}
			else	{			// CW in LSB mode
				*dst++ = (int16_t)ads.q_buffer[i];	// save left channel
				*dst++ = (int16_t)ads.i_buffer[i];	// save right channel
			}
		}
		return;
	}

	// -----------------------------
	// CW handler
	//
	else if(ts.dmod_mode == DEMOD_CW)
	{
		// Generate CW
		if(cw_gen_process(ads.i_buffer,ads.q_buffer,size) == 0)
		{
			// Pause or inactivity
			for(i = 0; i < size/2; i++)
			{
				*dst++ = 0;
				*dst++ = 0;
			}
		}
		else
		{
			// Copy soft DDS data
			for(i = 0; i < size/2; i++)
			{

				// Equalise based on band
				ads.i_buffer[i] *= (float)ts.tx_power_factor;
				ads.q_buffer[i] *= (float)ts.tx_power_factor;

				// Adjust TX I and Q gain
				ads.i_buffer[i] *= (float)ts.tx_adj_gain_var_i;		// offset gain by equal and opposite directions
				ads.q_buffer[i] *= (float)ts.tx_adj_gain_var_q;		//

				if(!ts.cw_lsb)	{	// CW in USB mode
					*dst++ = (int16_t)ads.i_buffer[i];	// save left channel
					*dst++ = (int16_t)ads.q_buffer[i];	// save right channel
				}
				else	{			// CW in LSB mode
					*dst++ = (int16_t)ads.q_buffer[i];	// save left channel
					*dst++ = (int16_t)ads.i_buffer[i];	// save right channel
				}
			}
		}

		return;
	}

	// ------------------------
	// SSB processor
	//
	else if((ts.dmod_mode == DEMOD_LSB) || (ts.dmod_mode == DEMOD_USB))		// Do this ONLY if in USB or LSB mode, of course!
	{
		if(ts.tune)	{	// TUNE mode?  If so, generate tone so we can adjust TX IQ phase and gain
			softdds_runf(ads.a_buffer,ads.a_buffer,size/2);		// load audio buffer with the tone - DDS produces quadrature channels, but we need only one
		}
		else	{		// Not tune mode - use audio from CODEC
			// Fill I and Q buffers with left channel(same as right)
			for(i = 0; i < size/2; i++)	{				// Copy to single buffer
				ads.a_buffer[i] = (float)*src;
				src += 2;								// Next sample
			}
		}
		//
		if(ts.tx_audio_source == TX_AUDIO_LINEIN)		// Are we in LINE IN mode?
			gain_calc = LINE_IN_GAIN_RESCALE;			// Yes - fixed gain scaling for line input - the rest is done in hardware
		else	{
			gain_calc = (float)ts.tx_mic_gain_mult;		// We are in MIC In mode:  Calculate Microphone gain
			gain_calc /= MIC_GAIN_RESCALE;				// rescale microphone gain to a reasonable range
		}

		// Apply gain if not in TUNE mode
		if(!ts.tune)	{
			for(i = 0; i < size/2; i++)	{
				ads.a_buffer[i] *= gain_calc;			// do software gain operation
				//
				if(fabs(ads.a_buffer[i]) > ads.peak_audio)				// is this an audio peak?
					ads.peak_audio = fabs(ads.a_buffer[i]);				// yes - save its amplitude for the "AUDIO" meter
			}
		}
		//
		//	TX audio filtering
		//
		if(!ts.tune)	// NOT in TUNE mode, apply the TX equalization filtering.  This "flattens" the audio
						// prior to being applied to the Hilbert transformer as well as added low-pass filtering.
						// It does this by applying a "peak" to the bottom end to compensate for the roll-off caused by the Hilbert
						// and then a gradual roll-off toward the high end.  The net result is a very flat (to better than 1dB) response
						// over the 275-2500 Hz range.
						//
			arm_iir_lattice_f32(&IIR_TXFilter, (float *)ads.a_buffer, (float *)ads.a_buffer, size/2);
		//
		// This is a phase-added 0-90 degree Hilbert transformer that also does low-pass and high-pass filtering
		// to the transmitted audio.  As noted above, it "clobbers" the low end, which is why we made up for it with the above filter.
		//
		if(ads.tx_filter_adjusting == 0)	{	//	is the filter NOT being adjusted?  (e.g. disable filter while we alter coefficients)
			// yes - apply transformation AND audio filtering to buffer data
			// + 0 deg to I data
			arm_fir_f32(&FIR_I_TX,(float32_t *)(ads.a_buffer),(float32_t *)(ads.i_buffer),size/2);

			// - 90 deg to Q data
			arm_fir_f32(&FIR_Q_TX,(float32_t *)(ads.a_buffer),(float32_t *)(ads.q_buffer), size/2);
		}

		if(!ts.tune)	{	// do post-filter gain calculations if we are NOT in TUNE mode
			// perform post-filter gain operation
			//
			gain_calc = (float)ts.alc_tx_postfilt_gain;		// get post-filter gain setting
			gain_calc /= 2;									// halve it
			gain_calc += 0.5;								// offset it so that 2 = unity
			arm_scale_f32(ads.i_buffer, (float32_t)gain_calc, ads.i_buffer, size/2);		// use optimized function to apply scaling to I/Q buffers
			arm_scale_f32(ads.q_buffer, (float32_t)gain_calc, ads.q_buffer, size/2);
		}

		// ------------------------
		// Do ALC processing on audio buffer
		for(i = 0; i < size/2; i++)
		{
			if(!ts.tune)	{	// if NOT in TUNE mode, do ALC processing
				// perform ALC on post-filtered audio (You will notice the striking similarity to the AGC code!)
				//
				alc_calc = fabs(ads.i_buffer[i] * ads.alc_val);	// calculate current level by scaling it with ALC value (both channels will be the same amplitude-wise)
				if(alc_calc < ALC_KNEE)	{	// is audio below ALC "knee" value?
					ads.alc_val += ads.alc_val*ads.alc_decay;	// (ALC DECAY) Yes - Increase gain slowly
				}
				else	{
					ads.alc_val -= ads.alc_val*ALC_ATTACK;	// Fast attack to increase gain
					if(ads.alc_val <= ALC_VAL_MIN)	// Prevent zero or "negative" gain values
						ads.alc_val = ALC_VAL_MIN;
				}
				if(ads.alc_val >= ALC_VAL_MAX)	// limit to fixed values within the code
					ads.alc_val = ALC_VAL_MAX;
			}
			else	{	// are we in TUNE mode?
				ads.alc_val = ALC_VAL_MAX;		// yes, disable ALC and set to MAXIMUM ALC gain (e.g. unity - no gain reduction)
			}
			ads.agc_valbuf[i] = ads.alc_val;			// store in "running" ALC history buffer for later application to audio data
		}
		//
		// Delay the post-AGC audio slightly so that the AGC's "attack" will very slightly lead the audio being acted upon by the AGC.
		// This eliminates a "click" that can occur when a very strong signal appears due to the AGC lag.  The delay is adjusted based on
		// decimation rate so that it is constant for all settings.
		//
		arm_copy_f32(ads.a_buffer, &agc_delay[alc_delay_inbuf], size/2);	// put new data into the delay buffer
		arm_copy_f32(&agc_delay[alc_delay_outbuf], ads.a_buffer, size/2);	// take old data out of the delay buffer
		//
		// Update the in/out pointers to the ALC delay buffer
		//
		alc_delay_inbuf += size/2;
		alc_delay_outbuf = alc_delay_inbuf + size/2;
		if(alc_delay_inbuf >= ALC_DELAY_BUFSIZE-1)
			alc_delay_inbuf -= ALC_DELAY_BUFSIZE;
		if(alc_delay_outbuf >= ALC_DELAY_BUFSIZE-1)
			alc_delay_outbuf -= ALC_DELAY_BUFSIZE;
		//
		arm_mult_f32(ads.i_buffer, ads.agc_valbuf, ads.i_buffer, size/2);		// Apply ALC gain corrections
		arm_mult_f32(ads.q_buffer, ads.agc_valbuf, ads.q_buffer, size/2);
		//
		if(ts.iq_freq_mode)	{		// is transmit frequency conversion to be done?
			if(ts.dmod_mode == DEMOD_LSB)	{		// Is it LSB?
				if(ts.iq_freq_mode == 1)			// yes - is it "RX LO HIGH" mode?
					audio_rx_freq_conv(size, 0);	// set conversion to "LO IS HIGH" mode
				else								// it is in "RX LO LOW" mode
					audio_rx_freq_conv(size, 1);	// set conversion to "RX LO LOW" mode
			}
			else	{								// It is USB!
				if(ts.iq_freq_mode == 1)			// yes - is it "RX LO HIGH" mode?
					audio_rx_freq_conv(size, 1);	// set conversion to "RX LO LOW" mode
				else								// it is in "RX LO LOW" mode
					audio_rx_freq_conv(size, 0);	// set conversion to "LO IS HIGH" mode
			}
		}
		//
		// Equalize based on band and simultaneously apply I/Q gain adjustments
		//
		arm_scale_f32(ads.i_buffer, (float32_t)(ts.tx_power_factor * ts.tx_adj_gain_var_i), ads.i_buffer, size/2);
		arm_scale_f32(ads.q_buffer, (float32_t)(ts.tx_power_factor * ts.tx_adj_gain_var_q), ads.q_buffer, size/2);
		//
		// ------------------------
		// Output I and Q as stereo
		for(i = 0; i < size/2; i++)
		{
			// Prepare data for DAC
			if(ts.dmod_mode == DEMOD_USB)	{
				*dst++ = (int16_t)ads.i_buffer[i];	// save left channel
				*dst++ = (int16_t)ads.q_buffer[i];	// save right channel
			}
			else	{		// Save in the opposite order for LSB
				*dst++ = (int16_t)ads.q_buffer[i];	// save left channel
				*dst++ = (int16_t)ads.i_buffer[i];	// save right channel
			}
		}
	}
	// -----------------------------
	// AM handler
	//
	else if(ts.dmod_mode == DEMOD_AM)	{	//	Is it in AM mode *AND* frequency translation active?
		if(ts.iq_freq_mode)	{
			// Fill I and Q buffers with left channel(same as right)
			for(i = 0; i < size/2; i++)	{				// Copy to single buffer
				ads.a_buffer[i] = (float)*src;
				src += 2;								// Next sample
			}
			//
			//
			if(ts.tx_audio_source == TX_AUDIO_LINEIN)		// Are we in LINE IN mode?
				gain_calc = LINE_IN_GAIN_RESCALE;			// Yes - fixed gain scaling for line input - the rest is done in hardware
			else	{
				gain_calc = (float)ts.tx_mic_gain_mult;		// We are in MIC In mode:  Calculate Microphone gain
				gain_calc /= MIC_GAIN_RESCALE;				// rescale microphone gain to a reasonable range
			}

			// Apply gain if not in TUNE mode
			if(!ts.tune)	{
				for(i = 0; i < size/2; i++)	{
					ads.a_buffer[i] *= gain_calc;			// do software gain operation
					//
					if(fabs(ads.a_buffer[i]) > ads.peak_audio)				// is this an audio peak?
						ads.peak_audio = fabs(ads.a_buffer[i]);				// yes - save its amplitude for the "AUDIO" meter
				}
			}
			//
			// Apply the TX equalization filtering:  This "flattens" the audio
			// prior to being applied to the Hilbert transformer as well as added low-pass filtering.
			// It does this by applying a "peak" to the bottom end to compensate for the roll-off caused by the Hilbert
			// and then a gradual roll-off toward the high end.  The net result is a very flat (to better than 1dB) response
			// over the 275-2500 Hz range.
			//
			if(!(ts.misc_flags1 & 8))	// Do the filtering *IF* it is to be enabled
				arm_iir_lattice_f32(&IIR_TXFilter, (float *)ads.a_buffer, (float *)ads.a_buffer, size/2);
			//
			// This is a phase-added 0-90 degree Hilbert transformer that also does low-pass and high-pass filtering
			// to the transmitted audio.  As noted above, it "clobbers" the low end, which is why we made up for it with the above filter.
			// Apply transformation AND audio filtering to buffer data
			//
			// + 0 deg to I data
			arm_fir_f32(&FIR_I_TX,(float32_t *)(ads.a_buffer),(float32_t *)(ads.i_buffer),size/2);
			// - 90 deg to Q data
			arm_fir_f32(&FIR_Q_TX,(float32_t *)(ads.a_buffer),(float32_t *)(ads.q_buffer), size/2);
			//
			// perform post-filter gain operation
			//
			gain_calc = (float)ts.alc_tx_postfilt_gain;		// get post-filter gain setting
			gain_calc /= 2;									// halve it
			gain_calc += 0.5;								// offset it so that 2 = unity
			//
			arm_scale_f32(ads.i_buffer, (float32_t)gain_calc, ads.i_buffer, size/2);		// use optimized function to apply scaling to I/Q buffers
			arm_scale_f32(ads.q_buffer, (float32_t)gain_calc, ads.q_buffer, size/2);
			//
			// ------------------------
			// Do ALC processing on audio buffer
			for(i = 0; i < size/2; i++)	{
				// perform ALC on post-filtered audio (You will notice the striking similarity to the AGC code!)
				//
				alc_calc = fabs(ads.i_buffer[i] * ads.alc_val);	// calculate current level by scaling it with ALC value (both channels will be the same amplitude-wise)
				if(alc_calc < ALC_KNEE)	{	// is audio below ALC "knee" value?
					ads.alc_val += ads.alc_val*ads.alc_decay;	// (ALC DECAY) Yes - Increase gain slowly
				}
				else	{
					ads.alc_val -= ads.alc_val*ALC_ATTACK;	// Fast attack to increase gain
					if(ads.alc_val <= ALC_VAL_MIN)	// Prevent zero or "negative" gain values
						ads.alc_val = ALC_VAL_MIN;
				}
				if(ads.alc_val >= ALC_VAL_MAX)	// limit to fixed values within the code
					ads.alc_val = ALC_VAL_MAX;
				//
				ads.agc_valbuf[i] = ads.alc_val * AM_ALC_GAIN_CORRECTION;	// store in "running" ALC history buffer for later application to audio data
			}
			//
			// Delay the post-AGC audio slightly so that the AGC's "attack" will very slightly lead the audio being acted upon by the AGC.
			// This eliminates a "click" that can occur when a very strong signal appears due to the AGC lag.  The delay is adjusted based
			// on the interaction of the sample rate and the ALC "attack" rate
			//
			arm_copy_f32(ads.a_buffer, &agc_delay[alc_delay_inbuf], size/2);	// put new data into the delay buffer
			arm_copy_f32(&agc_delay[alc_delay_outbuf], ads.a_buffer, size/2);	// take old data out of the delay buffer
			//
			// Update the in/out pointers to the ALC circular delay buffer
			//
			alc_delay_inbuf += size/2;
			alc_delay_outbuf = alc_delay_inbuf + size/2;
			if(alc_delay_inbuf >= ALC_DELAY_BUFSIZE-1)
				alc_delay_inbuf -= ALC_DELAY_BUFSIZE;
			if(alc_delay_outbuf >= ALC_DELAY_BUFSIZE-1)
				alc_delay_outbuf -= ALC_DELAY_BUFSIZE;
			//
			arm_mult_f32(ads.i_buffer, ads.agc_valbuf, ads.i_buffer, size/2);		// Apply ALC gain corrections
			arm_mult_f32(ads.q_buffer, ads.agc_valbuf, ads.q_buffer, size/2);
			//
			// First, generate the LOWER sideband of the AM signal
			//
			for(i = 0; i < size/2; i++)	{				// copy contents to temporary holding buffers
				ads.e_buffer[i] = ads.i_buffer[i];
				ads.f_buffer[i] = ads.q_buffer[i];
			}
			//
			for(i = 0; i < size/2; i++)	{		// generate AM carrier by applying a "DC bias" to the audio
				ads.i_buffer[i] += AM_CARRIER_LEVEL;
				ads.q_buffer[i] -= AM_CARRIER_LEVEL;
			}
			//
			if(ts.iq_freq_mode)	{		// is transmit frequency conversion to be done?
				if(ts.iq_freq_mode == 1)			// yes - LO is HIGH
					audio_rx_freq_conv(size, 0);
				else								// LO is HIGH
					audio_rx_freq_conv(size, 1);
			}
			//
			// Equalize based on band and simultaneously apply I/Q gain adjustments
			//
			arm_scale_f32(ads.i_buffer, (float32_t)(ts.tx_power_factor * ts.tx_adj_gain_var_i), ads.i_buffer, size/2);
			arm_scale_f32(ads.q_buffer, (float32_t)(ts.tx_power_factor * ts.tx_adj_gain_var_q), ads.q_buffer, size/2);
			//
			ptr = dst;	// save a copy of the destination pointer before we increment it for production of USB data
			//
			// Output I and Q as stereo
			for(i = 0; i < size/2; i++)	{
				// Prepare data for DAC
				*dst++ = (int16_t)ads.q_buffer[i];	// save left channel
				*dst++ = (int16_t)ads.i_buffer[i];	// save right channel
			}
			//
			// Now, generate upper sideband of the AM signal
			//
			for(i = 0; i < size/2; i++)	{				// restore data from temporary holding buffer
				ads.q_buffer[i] = (ads.e_buffer[i] * -1);	// make sure we invert the phase of the audio for this sideband!
				ads.i_buffer[i] = (ads.f_buffer[i] * -1);
			}
			//
			for(i = 0; i < size/2; i++)	{		// generate AM carrier by applying a "DC bias" to the audio
				ads.i_buffer[i] += AM_CARRIER_LEVEL;
				ads.q_buffer[i] -= AM_CARRIER_LEVEL;
			}
			//
			if(ts.iq_freq_mode)	{		// is transmit frequency conversion to be done?
				if(ts.iq_freq_mode == 1)			// yes - is it "RX LO HIGH" mode?
					audio_rx_freq_conv(size, 0);	// set "LO IS HIGH" mode
				else								// "RX LO LOW" mode
					audio_rx_freq_conv(size, 1);	// set conversion to "LO IS
			}
			//
			// Equalize based on band and simultaneously apply I/Q gain adjustments
			//
			arm_scale_f32(ads.i_buffer, (float32_t)(ts.tx_power_factor * ts.tx_adj_gain_var_i), ads.i_buffer, size/2);
			arm_scale_f32(ads.q_buffer, (float32_t)(ts.tx_power_factor * ts.tx_adj_gain_var_q), ads.q_buffer, size/2);
			//
			dst = ptr;	// restore the copy of the pointer
			//
			// Output I and Q as stereo
			for(i = 0; i < size/2; i++)	{
				//
				// Prepare data for DAC, adding the USB AM data to the already-existing LSB AM data
				//
				(*dst) += (int16_t)ads.q_buffer[i];	// save left channel
				dst++;
				(*dst) += (int16_t)ads.i_buffer[i];	// save right channel
				dst++;
			}
		}
		else	{	// Translate mode is NOT active - we CANNOT do full-carrier AM! (if we tried, we'd end up with DSB SSB!)
			for(i = 0; i < size/2; i++)	{				// send nothing out to the DAC
				*dst++ = 0;	// save left channel
				*dst++ = 0;	// save right channel
			}
		}
	}
	return;
}

//*----------------------------------------------------------------------------
//* Function Name       : I2S_RX_CallBack
//* Object              :
//* Object              : audio sample processor
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t size, uint16_t ht)
{
	static bool to_rx = 0;	// used as a flag to clear the RX
	static uchar lcd_dim = 0, lcd_dim_prescale = 0;
	static ulong tcount = 0;

	if((ts.txrx_mode == TRX_MODE_RX))	{
		if((to_rx) || (ts.buffer_clear))	{	// the first time back to RX, clear the buffers to reduce the "crash"
			to_rx = 0;							// caused by the content of the buffers from TX - used on return from SSB TX
			arm_fill_q15(0, dst, size);
			arm_fill_q15(0, src, size);
		}
		audio_rx_processor(src,dst,size);
	}
	else	{
		audio_tx_processor(src,dst,size);
		to_rx = 1;
	}
	//
	if(ts.unmute_delay_count)		// this updates at 375 Hz - used to time TX->RX delay
		ts.unmute_delay_count--;
	//
	ks.debounce_time++;				// keyboard debounce timer
	if(ks.debounce_time > DEBOUNCE_TIME_MAX)
		ks.debounce_time = DEBOUNCE_TIME_MAX;
	//
	// Perform LCD backlight PWM brightness function
	//

	if(!lcd_dim_prescale)	{	// Only update dimming PWM counter every fourth time through to reduce frequency below that of audible range
		if(lcd_dim < ts.lcd_backlight_brightness)
			LCD_BACKLIGHT_PIO->BSRRH = LCD_BACKLIGHT;	// LCD backlight off
		else
			LCD_BACKLIGHT_PIO->BSRRL = LCD_BACKLIGHT;	// LCD backlight on
		//
		lcd_dim++;
		lcd_dim &= 3;	// limit brightness PWM count to 0-3
	}
	lcd_dim_prescale++;
	lcd_dim_prescale &= 3;	// limit prescale count to 0-3
	//
	//
	tcount+=CLOCKS_PER_DMA_CYCLE;		// add the number of clock cycles that would have passed between DMA cycles
	if(tcount > CLOCKS_PER_CENTISECOND)	{	// has enough clock cycles for 0.01 second passed?
		tcount -= CLOCKS_PER_CENTISECOND;	// yes - subtract that many clock cycles
		ts.sysclock++;	// this clock updates at PRECISELY 100 Hz over the long term
	}
}
