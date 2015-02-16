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

//#include "mchf_sw_i2c.h"
#include "mchf_hw_i2c2.h"
#include "codec.h"

// Mask for the bit EN of the I2S CFGR register
#define I2S_ENABLE_MASK                 0x0400

#define CODEC_STANDARD                	0x04
#define I2S_STANDARD                   	I2S_Standard_Phillips

#define W8731_ADDR_0 					0x1A		// CS = 0, MODE to GND
#define W8731_ADDR_1 					0x1B		// CS = 1, MODE to GND

// The 7 bits Codec address (sent through I2C interface)
#define CODEC_ADDRESS           		(W8731_ADDR_0<<1)
// --------------------------------------------------
// Registers
#define W8731_LEFT_LINE_IN				0x00		// 0000000
#define W8731_RIGHT_LINE_IN				0x01		// 0000001

#define W8731_LEFT_HEADPH_OUT			0x02		// 0000010
#define W8731_RIGHT_HEADPH_OUT			0x03		// 0000011

#define W8731_ANLG_AU_PATH_CNTR			0x04		// 0000100
#define W8731_DIGI_AU_PATH_CNTR			0x05		// 0000101

#define W8731_POWER_DOWN_CNTR			0x06		// 0000110
#define W8731_DIGI_AU_INTF_FORMAT		0x07		// 0000111

#define W8731_SAMPLING_CNTR				0x08		// 0001000
#define W8731_ACTIVE_CNTR				0x09		// 0001001
#define W8731_RESET						0x0F		// 0001111

// -------------------------------------------------

#define W8731_DEEMPH_CNTR 				0x06

// Demodulator mode public flag
extern __IO ulong demod_mode;

// Transceiver state public structure
extern __IO TransceiverState ts;

//*----------------------------------------------------------------------------
//* Function Name       : Codec_Init
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
uint32_t Codec_Init(uint32_t AudioFreq,ulong word_size)
{
	// Configure the Codec related IOs
	Codec_GPIO_Init();   

	// Configure the I2S peripheral
	Codec_AudioInterface_Init(AudioFreq);

	// Reset the Codec Registers
	Codec_Reset(AudioFreq,word_size);

	return 0;
}

//*----------------------------------------------------------------------------
//* Function Name       : Codec_Reset
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void Codec_Reset(uint32_t AudioFreq,ulong word_size)
{
	//printf("codec init, freq = %d\n\r",AudioFreq);

	// Reset register
	if(Codec_WriteRegister(W8731_RESET, 0) != 0)
		return;

	// Reg 00: Left Line In (0dB, mute off)
	Codec_WriteRegister(W8731_LEFT_LINE_IN,0x001F);

	// Reg 01: Right Line In (0dB, mute off)
	Codec_WriteRegister(W8731_RIGHT_LINE_IN,0x001F);

	// Reg 02: Left Headphone out (0dB)
	//Codec_WriteRegister(0x02,0x0079);
	// Reg 03: Right Headphone out (0dB)
	//Codec_WriteRegister(0x03,0x0079);

	Codec_Volume(0);

	// Reg 04: Analog Audio Path Control (DAC sel, ADC line, Mute Mic)
	Codec_WriteRegister(W8731_ANLG_AU_PATH_CNTR,0x0012);

	// Reg 05: Digital Audio Path Control(all filters disabled)
	// De-emphasis control, bx11x - 48kHz
	//                      bx00x - off
	// DAC soft mute		b1xxx - mute on
	//						b0xxx - mute off
	//
	Codec_WriteRegister(W8731_DIGI_AU_PATH_CNTR,W8731_DEEMPH_CNTR);

	// Reg 06: Power Down Control (Clk off, Osc off, Mic Off)
	Codec_WriteRegister(W8731_POWER_DOWN_CNTR,0x0062);

	// Reg 07: Digital Audio Interface Format (i2s, 16/32 bit, slave)
	if(word_size == WORD_SIZE_16)
		Codec_WriteRegister(W8731_DIGI_AU_INTF_FORMAT,0x0002);
	else
		Codec_WriteRegister(W8731_DIGI_AU_INTF_FORMAT,0x000E);

	// Reg 08: Sampling Control (Normal, 256x, 48k ADC/DAC)
	// master clock: 12.5 Mhz
	if(AudioFreq == I2S_AudioFreq_48k) Codec_WriteRegister(W8731_SAMPLING_CNTR,0x0000);
	if(AudioFreq == I2S_AudioFreq_32k) Codec_WriteRegister(W8731_SAMPLING_CNTR,0x0018);
	if(AudioFreq == I2S_AudioFreq_8k ) Codec_WriteRegister(W8731_SAMPLING_CNTR,0x000C);

	// Reg 09: Active Control
	Codec_WriteRegister(W8731_ACTIVE_CNTR,0x0001);
}

//*----------------------------------------------------------------------------
//* Function Name       : Codec_RX_TX
//* Object              : switch codec mode
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void Codec_RX_TX(void)
{
	if(ts.txrx_mode == TRX_MODE_RX)
	{
		// First step - mute sound
		Codec_Volume(0);

		// Reg 04: Analog Audio Path Control (DAC sel, ADC line, Mute Mic)
		Codec_WriteRegister(W8731_ANLG_AU_PATH_CNTR,0x0012);

		// Reg 06: Power Down Control (Clk off, Osc off, Mic Off)
		Codec_WriteRegister(W8731_POWER_DOWN_CNTR,0x0062);

		// --------------------------------------------------------------
		// Test - route mic to headphones
		// Reg 04: Analog Audio Path Control (DAC sel, ADC Mic, Mic on)
		//Codec_WriteRegister(0x04,0x0014);
		// Reg 06: Power Down Control (Clk off, Osc off, Mic On)
		//Codec_WriteRegister(0x06,0x0061);
		// --------------------------------------------------------------

		// --------------------------------------------------------------
		// Wait settle, remove clicking
		//
		// Serious problem here - delay needed to remove clicking on back
		// to RX, but RXTX switching becomes very slow, need solution !!!
		// ToDo: ....
		//
		if(ts.dmod_mode == DEMOD_CW)
		{
			//non_os_delay();
			//non_os_delay();
			non_os_delay();
			non_os_delay();
			non_os_delay();

			// Update codec volume
			//  0 - 10: via codec command
			// 10 - 20: soft gain after decoder
			if(ts.audio_gain < 10)
				Codec_Volume((ts.audio_gain*8));
			else
				Codec_Volume((80));
		}
		else
		{
			// Request delayed unmute by audio driver,
			// work for SSB PTT, but not on CW due to lack
			// of IRQ locking (need further testing...)
			ts.audio_unmute = 1;
		}
	}
	else
	{
		// First step - mute sound
		Codec_Volume(0);

		// Select source or leave it as it is
		// PHONE out is muted, normal exit routed to TX modulator
		// input audio is routed via 4066 switch
		if(ts.tx_audio_source == TX_AUDIO_MIC)
		{
			// Reg 04: Analog Audio Path Control (DAC sel, ADC Mic, Mic on)
			if(!ts.mic_boost)
				Codec_WriteRegister(W8731_ANLG_AU_PATH_CNTR,0x0014);	// mic boost off
			else
				Codec_WriteRegister(W8731_ANLG_AU_PATH_CNTR,0x0015);	// mic boost on

			// Reg 06: Power Down Control (Clk off, Osc off, Mic On)
			Codec_WriteRegister(W8731_POWER_DOWN_CNTR,0x0061);
		}

		// Leave sidetone on CW
		if(ts.dmod_mode == DEMOD_CW)
		{
			// ToDo: use tx_power_factor coefficient and fix variable
			// sidetone level based on power
			//int	adj = (int)(ts.tx_power_factor * 100);
			//adj = 100/adj;

			Codec_Volume((ts.st_gain*11));
		}
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : Codec_Volume
//* Object              : audio vol control in RX mode
//* Object              : input: 0 - 80
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void Codec_Volume(uchar vol)
{
	ulong lv = vol;

	lv += 0x2F;

	if(lv < 0x2F) lv = 0x2F;	// limit min value
	if(lv > 0x7F) lv = 0x7F; 	// limit max value

	//printf("codec reg: 0x%02x\n\r",lv);

	// Reg 03: LINE OUT - const level
	Codec_WriteRegister(W8731_RIGHT_HEADPH_OUT,0x65);

	// Reg 02: Speaker - variable volume
	Codec_WriteRegister(W8731_LEFT_HEADPH_OUT,lv);
}

//*----------------------------------------------------------------------------
//* Function Name       : Codec_Mute
//* Object              : mew method of mute via soft mute of the DAC
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void Codec_Mute(uchar state)
{
	// Reg 05: Digital Audio Path Control(all filters disabled)
	// De-emphasis control, bx11x - 48kHz
	//                      bx00x - off
	// DAC soft mute		b1xxx - mute on
	//						b0xxx - mute off
	//
	if(state)
		Codec_WriteRegister(W8731_DIGI_AU_PATH_CNTR,(W8731_DEEMPH_CNTR|0x08));	// mute
	else
		Codec_WriteRegister(W8731_DIGI_AU_PATH_CNTR,(W8731_DEEMPH_CNTR));		// mute off
}

//*----------------------------------------------------------------------------
//* Function Name       : Codec_WriteRegister
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
uint32_t Codec_WriteRegister(uint8_t RegisterAddr, uint16_t RegisterValue)
{
	uchar 	res;
	//ushort 	msg;

	// Assemble 2-byte data in WM8731 format
	uint8_t Byte1 = ((RegisterAddr<<1)&0xFE) | ((RegisterValue>>8)&0x01);
	uint8_t Byte2 = RegisterValue&0xFF;
	
	// Combine spi msg
	//msg = (Byte1 << 8) | Byte2;

	//printf("codec write, reg = %02x,val = %02x\n\r",Byte1,Byte2);

	res = mchf_hw_i2c2_WriteRegister(CODEC_ADDRESS,Byte1,Byte2);
	if(res)
	{
#ifdef DEBUG_BUILD
		printf("err codec i2c: %d\n\r",res);
#endif
	}

	return res;
}

//*----------------------------------------------------------------------------
//* Function Name       : Codec_AudioInterface_Init
//* Object              : init I2S
//* Object              : I2S PLL already enabled in startup file!
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void Codec_AudioInterface_Init(uint32_t AudioFreq)
{
	I2S_InitTypeDef I2S_InitStructure;

	// Enable the CODEC_I2S peripheral clock
	RCC_APB1PeriphClockCmd(CODEC_I2S_CLK, ENABLE);

	// CODEC_I2S peripheral configuration for master TX
	SPI_I2S_DeInit(CODEC_I2S);
	I2S_InitStructure.I2S_AudioFreq = AudioFreq;
	I2S_InitStructure.I2S_Standard = I2S_STANDARD;
	I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;
	I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;
	I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;
	I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable;	// using MCO2

	// Initialise the I2S main channel for TX
	I2S_Init(CODEC_I2S, &I2S_InitStructure);
	
	// Initialise the I2S extended channel for RX
	I2S_FullDuplexConfig(CODEC_I2S_EXT, &I2S_InitStructure);
}

//*----------------------------------------------------------------------------
//* Function Name       : Codec_GPIO_Init
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void Codec_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	// CODEC_I2S output pins configuration: WS, SCK SD0 and SDI pins
	GPIO_InitStructure.GPIO_Pin 	= CODEC_I2S_SCK | CODEC_I2S_SDO | CODEC_I2S_SDI;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_NOPULL;
	GPIO_Init(CODEC_I2S_SDO_PIO, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = CODEC_I2S_WS;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(CODEC_I2S_WS_PIO, &GPIO_InitStructure);

	// Configure MCO2 (PC9)
	GPIO_InitStructure.GPIO_Pin = CODEC_CLOCK;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(CODEC_CLOCK_PIO, &GPIO_InitStructure);

	// Output I2S PLL via MCO2 pin - 12.288 Mhz
	RCC_MCO2Config(RCC_MCO2Source_PLLI2SCLK, RCC_MCO2Div_3);

	// Connect pins to I2S peripheral
	GPIO_PinAFConfig(CODEC_I2S_WS_PIO,	CODEC_I2S_WS_SOURCE,  CODEC_I2S_GPIO_AF);
	GPIO_PinAFConfig(CODEC_I2S_SDO_PIO, CODEC_I2S_SCK_SOURCE, CODEC_I2S_GPIO_AF);
	GPIO_PinAFConfig(CODEC_I2S_SDO_PIO,	CODEC_I2S_SDO_SOURCE, CODEC_I2S_GPIO_AF);
	GPIO_PinAFConfig(CODEC_I2S_SDO_PIO, CODEC_I2S_SDI_SOURCE, CODEC_I2S_GPIO_AF);
}
