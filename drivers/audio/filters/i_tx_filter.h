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

#ifndef __I_TX_FILTER_H
#define __I_TX_FILTER_H

#define I_TX_BLOCK_SIZE			1
#define I_TX_NUM_TAPS			31

const float i_tx_coeffs[I_TX_NUM_TAPS] =
{
  	 	   -0.000001471048f,
		   -0.000008733459f,
		    0.000020899328f,
		   -0.000119307579f,
		    0.000073745873f,
		   -0.000833593046f,
		   -0.000250466363f,
		   -0.003477179239f,
		   -0.003120310594f,
		   -0.009819740014f,
		   -0.014799302400f,
		   -0.019970367225f,
		   -0.050950886022f,
		   -0.030234028630f,
		   -0.215745393756f,
		    0.318897616171f,
		    0.215745393756f,
		   -0.030234028630f,
		    0.050950886022f,
		   -0.019970367225f,
		    0.014799302400f,
		   -0.009819740014f,
		    0.003120310594f,
		   -0.003477179239f,
		    0.000250466363f,
		   -0.000833593046f,
		   -0.000073745873f,
		   -0.000119307579f,
		   -0.000020899328f,
		   -0.000008733459f,
		    0.000001471048f
};

static float32_t 		FirState_I_TX[128];
arm_fir_instance_f32 	FIR_I_TX;

#endif
