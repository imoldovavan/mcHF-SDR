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

// SSB filters
#include "filters/q_rx_filter.h"
#include "filters/i_rx_filter.h"
#include "filters/q_tx_filter.h"
#include "filters/i_tx_filter.h"

// Audio filters
#include "filters/fir_10k.h"
#include "filters/fir_3_6k.h"
#include "filters/fir_2_3k.h"
#include "filters/fir_1_8k.h"

static void Audio_Init(void);

// ---------------------------------
// DMA buffers for I2S
__IO int16_t 	tx_buffer[BUFF_LEN];
__IO int16_t	rx_buffer[BUFF_LEN];
// ---------------------------------

// ---------------------------------
// Audio RX filter
arm_fir_instance_f32 	FIR_A;
__IO float32_t 			firState_A[FIR_RXAUDIO_BLOCK_SIZE + FIR_RXAUDIO_NUM_TAPS - 1];
// ---------------------------------
// Audio TX filter
arm_fir_instance_f32 	FIR_B;
__IO float32_t 			firState_B[FIR_RXAUDIO_BLOCK_SIZE + FIR_RXAUDIO_NUM_TAPS - 1];
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

	// Audio filter enabled
	ads.af_dissabled = 0;

	// Reset S meter public
	sm.skip		= 0;
	sm.s_count	= 0;
	sm.curr_max	= 0;

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
void audio_driver_set_rx_audio_filter(void)
{
	const float *f_coeff;

	switch(ts.filter_id)
	{
		case AUDIO_FIR_1P8KHZ:
			f_coeff = &Fir1p8k[0];
			break;
		case AUDIO_FIR_2P3KHZ:
			f_coeff = &Fir2p3k[0];
			break;
		case AUDIO_FIR_3P6KHZ:
			f_coeff = &Fir3p6k[0];
			break;
		case AUDIO_FIR_10KHZ:
			f_coeff = &Fir10k[0];
			break;
		default:
			break;
	}

	// Lock
	ads.af_dissabled = 1;

	arm_fir_init_f32(&FIR_A, FIR_RXAUDIO_NUM_TAPS, (float *)f_coeff, (float *)firState_A, FIR_RXAUDIO_BLOCK_SIZE);

	// Unlock
	ads.af_dissabled = 0;
}

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
	// -------------------
	// Init I/Q RX filters
	arm_fir_init_f32(&FIR_I,I_NUM_TAPS,(float32_t *)&i_rx_coeffs[0], &FirState_I[0],I_BLOCK_SIZE);
	arm_fir_init_f32(&FIR_Q,Q_NUM_TAPS,(float32_t *)&q_rx_coeffs[0], &FirState_Q[0],Q_BLOCK_SIZE);

	// -------------------
	// Init I/Q TX filters
	arm_fir_init_f32(&FIR_I_TX,I_TX_NUM_TAPS,(float32_t *)&i_tx_coeffs[0], &FirState_I_TX[0],I_TX_BLOCK_SIZE);
	arm_fir_init_f32(&FIR_Q_TX,Q_TX_NUM_TAPS,(float32_t *)&q_tx_coeffs[0], &FirState_Q_TX[0],Q_TX_BLOCK_SIZE);

	// -------------------
	// Init TX audio filter (2.6 kHz)
	arm_fir_init_f32(&FIR_B,FIR_RXAUDIO_NUM_TAPS,(float *)&Fir2p3k[0],(float *)firState_B,FIR_RXAUDIO_BLOCK_SIZE);

	// -------------------
	// Init RX audio filters
	audio_driver_set_rx_audio_filter();
}

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
	ulong 		i;
	float 		f_sum;
	int			aver;

	// ------------------------
	// Split stereo channels
	for(i = 0; i < size/2; i++)
	{
		// Collect I/Q samples
		if(sd.state == 0)
		{
			sd.FFT_Samples[sd.samp_ptr] = *(src + 1);
			sd.samp_ptr++;
			sd.FFT_Samples[sd.samp_ptr] = *(src + 0);
			sd.samp_ptr++;

			// On overload, update state machine,
			// reset pointer and wait
			if(sd.samp_ptr == FFT_IQ_BUFF_LEN)
			{
				sd.samp_ptr = 0;
				sd.state    = 1;
			}
		}

		// 16 bit format
		ads.i_buffer[i] = (float)*src++;
		ads.q_buffer[i] = (float)*src++;

		// Apply gain before filter
		if(ts.rf_gain)
		{
			ads.i_buffer[i] *= ts.rf_gain;
			ads.q_buffer[i] *= ts.rf_gain;
		}
	}

	// ------------------------
	// IQ SSB processing
	if((ts.dmod_mode == DEMOD_USB) || (ts.dmod_mode == DEMOD_LSB) || (ts.dmod_mode == DEMOD_DIGI))
	{
		// shift 0 degrees FIR
		arm_fir_f32(&FIR_I,(float *)(ads.i_buffer),(float *)(ads.i_buffer),size/2);

		// shift +90 degrees FIR
		arm_fir_f32(&FIR_Q,(float *)(ads.q_buffer),(float *)(ads.q_buffer),size/2);
	}

	// ------------------------
	// Demodulator
	for(i = 0; i < size/2; i++)
	{
		switch(ts.dmod_mode)
		{
			case DEMOD_USB:
			case DEMOD_DIGI:
				ads.a_buffer[i] = ads.i_buffer[i] + ads.q_buffer[i];
				break;

			case DEMOD_LSB:
				ads.a_buffer[i] = ads.i_buffer[i] - ads.q_buffer[i];
				break;

			case DEMOD_AM:
			{
				f_sum  = ads.i_buffer[i] * ads.i_buffer[i];
				f_sum += ads.q_buffer[i] * ads.q_buffer[i];
				f_sum  = sqrtf(f_sum);

				ads.a_buffer[i] = f_sum;

				/*f_sum  = ads.i_buffer[i] * ads.i_buffer[i];
				f_sum += ads.q_buffer[i] * ads.q_buffer[i];
				f_sum  = sqrtf(f_sum);

				signal = f_sum - carrier;

				carrier += signal/(float)(1 << 10);

				ads.a_buffer[i] = signal;*/

				break;
			}

			case DEMOD_CW:
				ads.a_buffer[i] = ads.i_buffer[i] - ads.q_buffer[i];
				break;

			default:
				ads.a_buffer[i] = 0;
				break;
		}
	}

	// ------------------------
	// Apply audio filter
	if(!ads.af_dissabled)
		arm_fir_f32(&FIR_A,(float *)(ads.a_buffer),(float *)(ads.a_buffer),size/2);

	// ------------------------
	// Combine stereo
	for(i = 0; i < size/2; i++)
	{
		// ------------------------
		// S meter data
		if(sm.s_count < S_MET_SAMP_CNT)
		{
			f_sum = ads.a_buffer[i] * ads.a_buffer[i];
			aver = (int)f_sum;

			if(aver > sm.curr_max)
				sm.curr_max = aver;

			sm.s_count++;
		}

		// AF gain
		//  0 - 10: via codec command
		// 10 - 20: soft gain after decoder
		if(ts.audio_gain > 10)
			ads.a_buffer[i] *= (ts.audio_gain - 10);

		// Save in 16 bit format
		*dst++ = (int16_t)ads.a_buffer[i];				// save Speaker channel, variable level

		// Save in 16 bit format
		*dst++ = (int16_t)(ads.a_buffer[i] * 10);		// save LINE OUT channel - const level
	}
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
	ulong 		i;
	int32_t		temp;

	// -----------------------------
	// Tuning handler
	//
	if(ts.tune)
	{
		softdds_runf(ads.i_buffer,ads.q_buffer,size/2);

		// Output I and Q as stereo
		for(i = 0; i < size/2; i++)
		{
			// Equalise based on band
			ads.i_buffer[i] *= ts.tx_power_factor;
			ads.q_buffer[i] *= ts.tx_power_factor;

			*dst++ = (int16_t)ads.i_buffer[i];	// save left channel
			*dst++ = (int16_t)ads.q_buffer[i];	// save right channel
		}
		return;
	}

	// -----------------------------
	// AM hadler - not implemented yet!!!
	//
	if(ts.dmod_mode == DEMOD_AM)
	{
		for(i = 0; i < size; i++)
			*dst++ = *src++;

		return;
	}

	// -----------------------------
	// CW handler
	//
	if(ts.dmod_mode == DEMOD_CW)
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
				// Apply gain and balance, to cancel out the sideband
				//ads.i_buffer[i] *= 1.0;
				//ads.q_buffer[i] *= 0.95;

				// Equalise based on band
				ads.i_buffer[i] *= ts.tx_power_factor;
				ads.q_buffer[i] *= ts.tx_power_factor;

				*dst++ = (int16_t)ads.i_buffer[i];
				*dst++ = (int16_t)ads.q_buffer[i];
			}
		}

		return;
	}

	// ------------------------
	// SSB processor
	//
	if((ts.dmod_mode == DEMOD_LSB) || (ts.dmod_mode == DEMOD_USB))
	{
		// Fill I and Q buffers with left channel(same as right)
		for(i = 0; i < size/2; i++)
		{
			// Copy to single buffer
			ads.a_buffer[i] = (float)*src;

			// Next sample
			src += 2;
		}

		// Apply audio filter - 2.6 kHz
		arm_fir_f32(&FIR_B,(float *)(ads.a_buffer),(float *)(ads.a_buffer),size/2);

		// Apply gain
		for(i = 0; i < size/2; i++)
		{
			if(ts.tx_audio_source == TX_AUDIO_LINEIN)
				ads.a_buffer[i] *= 10;
			else
				ads.a_buffer[i] *= 5;
		}

		// + 45 deg to I data
		arm_fir_f32(&FIR_I_TX,(float *)(ads.a_buffer),(float *)(ads.i_buffer),size/2);

		// - 45 deg to Q data
		arm_fir_f32(&FIR_Q_TX,(float *)(ads.a_buffer),(float *)(ads.q_buffer), size/2);

		// ------------------------
		// Output I and Q as stereo
		for(i = 0; i < size/2; i++)
		{
			// Apply gain on LINEIN
			//if(ts.tx_audio_source == TX_AUDIO_LINEIN)
			//{
			//	ads.i_buffer[i] *= 10;
			//	ads.q_buffer[i] *= 10;
			//}

			// Equalise based on band
			ads.i_buffer[i] *= ts.tx_power_factor;
			ads.q_buffer[i] *= ts.tx_power_factor;

			// Prepare data for DAC
			if(ts.dmod_mode == DEMOD_USB)
			{
				*dst++ = (int16_t)ads.i_buffer[i];	// save left channel
				*dst++ = (int16_t)ads.q_buffer[i];	// save right channel
			}
			else
			{
				*dst++ = (int16_t)ads.q_buffer[i];	// save left channel
				*dst++ = (int16_t)ads.i_buffer[i];	// save right channel
			}
		}
	}
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
	if(ts.txrx_mode == TRX_MODE_RX)
		audio_rx_processor(src,dst,size);
	else
		audio_tx_processor(src,dst,size);
}
