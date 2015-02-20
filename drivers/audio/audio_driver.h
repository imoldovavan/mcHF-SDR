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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AUDIO_DRIVER_H
#define __AUDIO_DRIVER_H

#include "arm_math.h"

// 16 or 24 bits from Codec
//
//#define USE_24_BITS

// -----------------------------
// With 128 words buffer we have
// 120uS processing time in the
// IRQ and 530uS idle time
// (48kHz sampling - USB)
//
// -----------------------------
// FFT buffer (128, 512 or 2048)
#define FFT_IQ_BUFF_LEN		512
//
#define BUFF_LEN 			128
//
//
// -----------------------------
// Half of total buffer
#define	IQ_BUFSZ 	(BUFF_LEN/2)

// Audio filter
#define FIR_RXAUDIO_BLOCK_SIZE		1
#define FIR_RXAUDIO_NUM_TAPS		48
//
#define CODEC_DEFAULT_GAIN		0x1F	// Gain of line input to start with
#define	ADC_CLIP_WARN_THRESHOLD	4096	// This is at least 12dB below the clipping threshold of the A/D converter itself
//
// Audio driver publics
typedef struct AudioDriverState
{
	// Stereo buffers
	float32_t					i_buffer[IQ_BUFSZ+1];
	float32_t 					q_buffer[IQ_BUFSZ+1];
	float32_t 					a_buffer[IQ_BUFSZ+1];
	float32_t 					b_buffer[IQ_BUFSZ+1];
	float32_t					c_buffer[IQ_BUFSZ+1];
	float32_t					d_buffer[IQ_BUFSZ+1];
	float32_t					e_buffer[IQ_BUFSZ+1];
	float32_t					f_buffer[IQ_BUFSZ+1];
	//
	// Lock audio filter flag
	uchar					af_dissabled;

	uchar					tx_filter_adjusting;	// used to disable TX I/Q filter during phase adjustment

	// AGC and audio related variables

	float 					agc_val;			// "live" receiver AGC value
	float					agc_valbuf[BUFF_LEN];	// holder for "running" AGC value
	float					agc_holder;			// used to hold AGC value during transmit and tuning
	float					agc_decay;			// decay rate (speed) of AGC
	float					agc_rf_gain;
	float					agc_knee;			// "knee" for AGC operation
	float					agc_val_max;		// maximum AGC gain (at minimum signal)
	//
	float					pre_filter_gain;
	uchar					codec_gain;
	float					codec_gain_calc;
	bool					adc_clip;
	bool					adc_half_clip;
	bool					adc_quarter_clip;
	float					peak_audio;			// used for audio metering to detect the peak audio level

	float					alc_val;			// "live" transmitter ALC value
	float					alc_decay;			// decay rate (speed) of ALC
	float					post_agc_gain;		// post AGC gain scaling

	uchar					decimation_rate;		// current decimation/interpolation rate
	ulong					agc_delay_buflen;		// AGC delay buffer length
	float					agc_decimation_scaling;	// used to adjust AGC timing based on sample rate
	//
	float					nb_agc_filt;			// used for the filtering/determination of the noise blanker AGC level
	float					nb_sig_filt;
	ulong					dsp_zero_count;			// used for detecting zero output from DSP which can occur if it crashes
	float					dsp_nr_sample;			// used for detecting a problem with the DSP (e.g. crashing)
	//
	float					Osc_Cos;
	float					Osc_Sin;
	float					Osc_Vect_Q;
	float					Osc_Vect_I;
	float					Osc_Q;
	float					Osc_I;
	float					Osc_Gain;

} AudioDriverState;

//
// FFT will work with quadrature signals
#define FFT_QUADRATURE_PROC	1

// Spectrum display
typedef struct SpectrumDisplay
{
	// FFT state
	arm_rfft_instance_q15  			S;
	arm_cfft_radix4_instance_q15  	S_CFFT;

	// Samples buffer
	q15_t	FFT_Samples[FFT_IQ_BUFF_LEN];
	q15_t	FFT_MagData[FFT_IQ_BUFF_LEN/2];
	q15_t	FFT_BkpData[FFT_IQ_BUFF_LEN/2];
	q15_t	FFT_DspData[FFT_IQ_BUFF_LEN/2];		// Rescaled and de-linearized display data
	float	FFT_AVGData[FFT_IQ_BUFF_LEN/2];		// IIR low-pass filtered FFT buffer data

	// Current data ptr
	ulong	samp_ptr;

	// Skip flag for FFT processing
	ulong	skip_process;

	// Addresses of vertical grid lines on x axis
	ushort  vert_grid_id[7];

	// Addresses of horizontal grid lines on x axis
	ushort  horz_grid_id[3];

	// State machine current state
	uchar 	state;

	// Init done flag
	uchar 	enabled;

	// Flag to indicate frequency change,
	// we need it to clear spectrum control
	uchar	dial_moved;

	// Variables used in spectrum display AGC
	//
	float spectrum_display_gain;	// gain control for input to FFT
	//
	float mag_calc;		// spectrum display rescale control

	uchar	use_spi;	// TRUE if display uses SPI mode

	uchar	magnify;	// TRUE if in magnify mode

	float	rescale_rate;	// this holds the rate at which the rescaling happens when the signal appears/disappears

	float	agc_rate;		// this hold AGC rate for the Spectrum Display

} SpectrumDisplay;


// -----------------------------
// S meter sample time
#define S_MET_SAMP_CNT		512

// S meter drag delay
#define S_MET_UPD_SKIP		10		//10000

// S meter public
typedef struct SMeter
{
	ulong	skip;

	ulong	s_count;
	float	gain_calc;
	int		curr_max;

	uchar	old;
	//uchar	max_upd;

} SMeter;
//
// AGC Time constants
// C. Turner, KA7OEI
//
#define	AGC_KNEE	1000//4000	// ADC "knee" threshold for AGC action
//
#define	AGC_KNEE_REF		1000
#define	AGC_VAL_MAX_REF		4096
#define	POST_AGC_GAIN_SCALING_REF	1.333

#define	AGC_ATTACK		0.033	// Attack time multiplier for AGC
//
#define AGC_FAST_DECAY	0.0002	// Decay rate multiplier for "Fast" AGC
#define AGC_MED_DECAY	0.00006	// Decay rate multiplier for "Medium" AGC
#define AGC_SLOW_DECAY	0.00001 // Decay rate for multiplier  "Slow" AGC

#define	AGC_VAL_MIN		0.02	// Minimum AGC gain multiplier (e.g. gain reduction of 34dB)
//#define AGC_VAL_MAX		4096//1024	// Maximum AGC gain multiplier (e.g. gain multiplication of 60dB)

#define	AGC_PREFILTER_MAXGAIN	5 	// Scaling factor for RF gain adjustment (e.g. factor by which RFG will be multiplied to yield actual RFG multiplier
#define AGC_PREFILTER_MINGAIN	0.5	// Minimum "RFG" gain multiplier (e.g. gain reduction of 6 dB)
//
#define AGC_PREFILTER_HISIG_THRESHOLD	0.1	// Threshold at which adjustment of RFGAIN (pre-filter) gain adjustment will occur
#define AGC_PREFILTER_LOWSIG_THRESHOLD	1.0	// Threshold at which adjustment of RFGAIN (pre-filter) gain adjustment will occur
#define AGC_PREFILTER_ATTACK_RATE		0.0002	// Attack rate for RFG reduction
#define AGC_PREFILTER_DECAY_RATE		0.000002	// Decay rate for RFG gain recovery
//
#define AGC_PREFILTER_MAX_SIGNAL	1		// maximum level of pre-filtered signal
//
#define POST_AGC_GAIN_SCALING	1.333//0.333	// Used to rescale the post-filter audio level to a value suitable for the codec.  This sets the line level output
									// to approx. 1000mV peak-peak.
//
#define	POST_AGC_GAIN_SCALING_DECIMATE_4	3.46	// Used to scale audio from the decimation/interpolation-by-4 process (based square root of decimation factor)
//
#define	POST_AGC_GAIN_SCALING_DECIMATE_2	(POST_AGC_GAIN_SCALING_DECIMATE_4 * 0.6)	// Scales audio from decimation/interpolation-by-2 process
//
#define	AM_SCALING		2	//8.1			// Amount of gain multiplication to apply to audio and AGC to make recovery equal to that of SSB
#define	AM_AUDIO_SCALING	1.4			// Correction factor applied to audio demodulation to make amplitude equal to that of SSB demodulation
//
#define	AGC_GAIN_CAL	15500//22440		// multiplier value (linear, not DB) to calibrate the S-Meter reading to the AGC value
//
#define	AUTO_RFG_DECREASE_LOCKOUT	1
#define	AUTO_RFG_INCREASE_TIMER		10
//
#define	AGC_SLOW			0		// Mode setting for slow AGC
#define	AGC_MED				1		// Mode setting for medium AGC
#define	AGC_FAST			2		// Mode setting for fast AGC
#define	AGC_CUSTOM			3		// Mode setting for custom AGC
#define	AGC_OFF				4		// Mode setting for AGC off
#define	AGC_MAX_MODE		4		// Maximum for mode setting for AGC
#define	AGC_DEFAULT			AGC_MED	// Default!
//
#define	AGC_CUSTOM_MAX		30		// Maximum (slowest) setting for custom AGC
#define	AGC_CUSTOM_DEFAULT	12		// Default custom AGC setting (approx. equal to "medium")
#define AGC_CUSTOM_FAST_WARNING	2	// Value at or below which setting the custom AGC is likely to degrade audio
//
#define	MAX_RF_GAIN_MAX		6		// Maximum setting for "Max RF gain"
#define	MAX_RF_GAIN_DEFAULT	3
//
// Noise blanker constants
//
#define	NBLANK_AGC_ATTACK	0.33	// Attack time multiplier for AGC
//
#define NBLANK_AGC_DECAY	0.002	// Decay rate multiplier for "Fast" AGC
//
#define	MAX_NB_SETTING		20
#define	NB_WARNING1_SETTING	8		// setting at or above which NB warning1 (yellow) is given
#define	NB_WARNING2_SETTING	12		// setting at or above which NB warning2 (orange) is given
#define	NB_WARNING3_SETTING	16		// setting at or above which NB warning3 (red) is given
#define	NB_DURATION			4
//
#define	NB_AGC_FILT			0.999	// Component of IIR filter for recyling previous AGC value
#define	NB_SIG_FILT			0.001	// Component of IIR filter for present signal value's contribution to AGC
//
#define	NB_AVG_WEIGHT		0.80	// Weighting applied to average based on past signal for NB2
#define	NB_SIG_WEIGHT		0.20	// Weighting applied to present signal for NB2
//
//
#define	NB_MAX_AGC_SETTING	35		// maximum setting for noise blanker setting
#define	NB_AGC_DEFAULT		20		// Default setting for noise blanker AGC time constant adjust
//
// Values used for "custom" AGC settings
//
#define	MIN_CUST_AGC_VAL	10	// Minimum and maximum RF gain settings
#define	MAX_CUST_AGC_VAL	30
#define	CUST_AGC_OFFSET_VAL	30	// RF Gain offset value used in calculations
#define	CUST_AGC_VAL_DEFAULT	17.8	// Value for "medium" AGC value
//
#define	LINE_OUT_SCALING_FACTOR	10		// multiplication of audio for fixed LINE out level (nominally 1vpp)
//
#define	LINE_IN_GAIN_RESCALE	20		// multiplier for line input gain
#define	MIC_GAIN_RESCALE	2	// divisor for microphone gain setting
//
// ALC (Auto Level Control) for transmitter, constants
//
#define	ALC_VAL_MAX			1		// Maximum ALC Value is 1 (e.g. it can NEVER amplify)
#define	ALC_VAL_MIN			0.001	// Minimum ALC Value - it can provide up to 60dB of attenuation
#define	ALC_ATTACK			0.1//0.033	// Attack time for the ALC's gain control
#define	ALC_KNEE			30000	// The audio value threshold for the ALC operation
//
// Decay (release time) for ALC/Audio compressor
//
#define	ALC_DECAY_MAX		20		// Maximum (slowest) setting for ALC decay
#define	ALC_DECAY_DEFAULT	10		// Default custom ALC setting (approx. equal to AGC "medium")
//
// Audio post-filter (pre-alc) gain adjust
//
#define	ALC_POSTFILT_GAIN_MIN	1
#define	ALC_POSTFILT_GAIN_MAX	25
#define	ALC_POSTFILT_GAIN_DEFAULT	1
//
#define	TX_AUDIO_COMPRESSION_MAX		13	// 0 = least compression, 12 = most, 13 = EEPROM values
#define	TX_AUDIO_COMPRESSION_DEFAULT	2
//
#define	RX_DECIMATION_RATE_12KHZ		4		// Decimation/Interpolation rate in receive function for 12 kHz rates
#define	RX_DECIMATION_RATE_24KHZ		2		// Decimation/Interpolation rate in receive function for 24 kHz rates
//
// ************
// DSP system parameters
//
// Noise reduction
//
#define	LMS_NR_DELAYBUF_SIZE_MAX		512	// maximum size of LMS delay buffer for the noise reduction
//
//
#define	DSP_NR_STRENGTH_MAX		55//35	// Maximum menu setting for DSP "Strength"
#define	DSP_NR_STRENGTH_DEFAULT	15	// Default setting
//
#define	DSP_STRENGTH_YELLOW		25	// Threshold at and above which DSP number is yellow
#define	DSP_STRENGTH_ORANGE		35	// Threshold at and above which DSP number is orange
#define DSP_STRENGTH_RED		45	// Threshold at and above which DSP number is red
//
//
#define	DSP_NR_BUFLEN_MIN		48
#define	DSP_NR_BUFLEN_MAX		LMS_NR_DELAYBUF_SIZE_MAX
#define	DSP_NR_BUFLEN_DEFAULT	192
//
#define DSP_NR_NUMTAPS_MIN		32
#define	DSP_NR_NUMTAPS_MAX		128
#define	DSP_NR_NUMTAPS_DEFAULT	96
//
#define	MAX_DSP_ZERO_COUNT		2048
#define	DSP_ZERO_COUNT_ERROR	512
#define	DSP_ZERO_DET_MULT_FACTOR	10000000
#define	DSP_OUTPUT_MINVAL		1
#define	DSP_HIGH_LEVEL			10000
#define	DSP_CRASH_COUNT_THRESHOLD	35
//
// Automatic Notch Filter
//
#define	LMS_NOTCH_DELAYBUF_SIZE_MAX	1024
//
#define	DSP_NOTCH_NUM_TAPS		64		// number of taps for the DSP NOTCH FIR
//
#define	DSP_NOTCH_BUFLEN_MIN	96
#define	DSP_NOTCH_BUFLEN_MAX	LMS_NOTCH_DELAYBUF_SIZE_MAX
#define	DSP_NOTCH_DELAYBUF_DEFAULT	192
//
#define	DSP_NOTCH_MU_MAX		35
#define	DSP_NOTCH_MU_DEFAULT	10
//
#define	AGC_DELAY_BUFSIZE		(BUFF_LEN/2)*5	// Size of AGC delaying audio buffer - Must be a multiple of BUFF_LEN/2.
											// This is divided by the decimation rate so that the time delay is constant.
//
#define	ALC_DELAY_BUFSIZE		(BUFF_LEN/2)*5		// Size of AGC delaying audio buffer - Must be a multiple of BUFF_LEN/2.
//
//
#define CLOCKS_PER_DMA_CYCLE	10656			// Number of 16 MHz clock cycles per DMA cycle
#define	CLOCKS_PER_CENTISECOND	160000			// Number of 16 MHz clock cycles per 0.01 second timing cycle
//
//
// The following are calibration constants for AM (transmitter) modulation
//
#define	AM_ALC_GAIN_CORRECTION	0.22			// This sets the output of the ALC for 100% modulation
#define	AM_CARRIER_LEVEL		5000			// This set the AM carrier level in A/D units
//
//
// The following refer to the software frequency conversion/translation done in receive and transmit to shift the signals away from the
// "DC" IF
//
// The following are terms used to set the NCO frequency of the conversion in the receiver - *IF* we were to use the on-the-fly sine generation
// (which we DON'T, since it is too processor-intensive!)
//
// rate = 2 * Pi * (NCO Freq) / (Sample Rate)
// CONV_NCO_SIN = sin(rate)
// CONV_NCO_COS = cos(rate)
//
#define	CONV_NCO_SIN	0.70710678118654752440084436210485
#define	CONV_NCO_COS	0.70710678118654752440084436210485
//
//
#define	FREQ_SHIFT_MAG	6000		// Amount of frequency shift, in Hz, when software frequency shift is enabled
//
#define	FREQ_IQ_CONV_MODE_OFF		0	// No frequency conversion
#define FREQ_IQ_CONV_LO_HIGH		1	// LO is above receive frequency in RX mode
#define	FREQ_IQ_CONV_LO_LOW			2	// LO is below receive frequency in RX mode
//
#define	FREQ_IQ_CONV_MODE_DEFAULT	FREQ_IQ_CONV_MODE_OFF
#define	FREQ_IQ_CONV_MODE_MAX		2
//
//
// Exports
void audio_driver_init(void);
void audio_driver_stop(void);
void audio_driver_set_rx_audio_filter(void);
void audio_driver_thread(void);
//uchar audio_check_nr_dsp_state(void);

#ifdef USE_24_BITS
void I2S_RX_CallBack(int32_t *src, int32_t *dst, int16_t size, uint16_t ht);
#else
void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t size, uint16_t ht);
#endif

#endif
