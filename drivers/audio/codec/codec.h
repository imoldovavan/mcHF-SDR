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

#ifndef __CODEC_H
#define __CODEC_H

#define WORD_SIZE_16 					0
#define WORD_SIZE_32 					1

#define CODEC_I2S                      SPI3
#define CODEC_I2S_EXT                  I2S3ext
#define CODEC_I2S_CLK                  RCC_APB1Periph_SPI3
#define CODEC_I2S_ADDRESS              (SPI3_BASE    + 0x0C)
#define CODEC_I2S_EXT_ADDRESS          (I2S3ext_BASE + 0x0C)
#define CODEC_I2S_GPIO_AF              GPIO_AF_SPI3
#define CODEC_I2S_IRQ                  SPI3_IRQn
#define CODEC_I2S_EXT_IRQ              SPI3_IRQn

#define AUDIO_I2S_IRQHandler           SPI3_IRQHandler
#define AUDIO_I2S_EXT_IRQHandler       SPI3_IRQHandler

#define AUDIO_I2S_DMA_CLOCK            RCC_AHB1Periph_DMA1
#define AUDIO_I2S_DMA_STREAM           DMA1_Stream5
#define AUDIO_I2S_DMA_DREG             CODEC_I2S_ADDRESS
#define AUDIO_I2S_DMA_CHANNEL          DMA_Channel_0
#define AUDIO_I2S_DMA_IRQ              DMA1_Stream5_IRQn
#define AUDIO_I2S_DMA_FLAG_TC          DMA_FLAG_TCIF5
#define AUDIO_I2S_DMA_FLAG_HT          DMA_FLAG_HTIF5
#define AUDIO_I2S_DMA_FLAG_FE          DMA_FLAG_FEIF5
#define AUDIO_I2S_DMA_FLAG_TE          DMA_FLAG_TEIF5
#define AUDIO_I2S_DMA_FLAG_DME         DMA_FLAG_DMEIF5
#define AUDIO_I2S_EXT_DMA_STREAM       DMA1_Stream2
#define AUDIO_I2S_EXT_DMA_DREG         CODEC_I2S_EXT_ADDRESS
#define AUDIO_I2S_EXT_DMA_CHANNEL      DMA_Channel_2
#define AUDIO_I2S_EXT_DMA_IRQ          DMA1_Stream2_IRQn
#define AUDIO_I2S_EXT_DMA_FLAG_TC      DMA_FLAG_TCIF2
#define AUDIO_I2S_EXT_DMA_FLAG_HT      DMA_FLAG_HTIF2
#define AUDIO_I2S_EXT_DMA_FLAG_FE      DMA_FLAG_FEIF2
#define AUDIO_I2S_EXT_DMA_FLAG_TE      DMA_FLAG_TEIF2
#define AUDIO_I2S_EXT_DMA_FLAG_DME     DMA_FLAG_DMEIF2

uint32_t Codec_Init(uint32_t AudioFreq,ulong word_size);
void 	 Codec_RX_TX(void);
void 	 Codec_Volume(uchar vol);
void 	 Codec_Mute(uchar state);

void     Codec_AudioInterface_Init(uint32_t AudioFreq);
void     Codec_Reset(uint32_t AudioFreq,ulong word_size);
uint32_t Codec_WriteRegister(uint8_t RegisterAddr, uint16_t RegisterValue);
void     Codec_GPIO_Init(void);

#endif
