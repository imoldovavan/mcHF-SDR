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
#define BUFF_LEN 			128
//
// -----------------------------
// Half of total buffer
#define	IQ_BUFSZ 	(BUFF_LEN/2)

// Audio filter
#define FIR_RXAUDIO_BLOCK_SIZE		1
#define FIR_RXAUDIO_NUM_TAPS		48

// Audio driver publics
typedef struct AudioDriverState
{
	// Stereo buffers
	float					i_buffer[IQ_BUFSZ];
	float 					q_buffer[IQ_BUFSZ];
	float 					a_buffer[IQ_BUFSZ];

	// Lock audio filter flag
	uchar					af_dissabled;

} AudioDriverState;

// -----------------------------
// FFT buffer (128, 512 or 2048)
#define FFT_IQ_BUFF_LEN		512

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
	int		curr_max;

	uchar	old;
	//uchar	max_upd;

} SMeter;

// Exports
void audio_driver_init(void);
void audio_driver_stop(void);
void audio_driver_set_rx_audio_filter(void);
void audio_driver_thread(void);

#ifdef USE_24_BITS
void I2S_RX_CallBack(int32_t *src, int32_t *dst, int16_t size, uint16_t ht);
#else
void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t size, uint16_t ht);
#endif

#endif
