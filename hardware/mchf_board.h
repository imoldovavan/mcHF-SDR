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
#ifndef __MCHF_BOARD_H
#define __MCHF_BOARD_H
                                              
// HW libs
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_i2c.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_dac.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_rtc.h"
#include "stm32f4xx_pwr.h"
#include "stm32f4xx_fsmc.h"
#include "stm32f4xx_wwdg.h"
#include "stm32f4xx_flash.h"
#include "misc.h"
#include "core_cm4.h"

#include "stm32f4xx.h"
#include "mchf_types.h"

// -----------------------------------------------------------------------------
#define 	DEVICE_STRING   		"mcHF QRP Transceiver"
#define 	AUTHOR_STRING   		"K Atanassov - M0NKA 2014"

#define 	TRX4M_VER_MAJOR			0
#define 	TRX4M_VER_MINOR			0
#define 	TRX4M_VER_RELEASE		0
#define 	TRX4M_VER_BUILD			181

// -----------------------------------------------------------------------------
//#define 	DEBUG_BUILD

#define		WD_REFRESH_WINDOW		80
#define		WD_REFRESH_COUNTER		127

// -----------------------------------------------------------------------------
//						PORT PINS ALLOCATION
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// ---- 						PORT A										----
// -----------------------------------------------------------------------------
// pin 0
#define ENC_THREE_CH1 			GPIO_Pin_0
#define ENC_THREE_CH1_SOURCE	GPIO_PinSource0
#define ENC_THREE_CH1_PIO       GPIOA
// pin 1
#define ENC_THREE_CH2 			GPIO_Pin_1
#define ENC_THREE_CH2_SOURCE	GPIO_PinSource1
#define ENC_THREE_CH2_PIO       GPIOA
// pin 2
#define ADC3_FWD				GPIO_Pin_2
#define ADC3_FWD_SOURCE			GPIO_PinSource2
#define ADC3_FWD_PIO       		GPIOA
// pin 3
#define ADC2_RET				GPIO_Pin_3
#define ADC2_RET_SOURCE			GPIO_PinSource3
#define ADC2_RET_PIO       		GPIOA
// pin 4
#define DAC0 					GPIO_Pin_4
#define DAC0_SOURCE				GPIO_PinSource4
#define DAC0_PIO       			GPIOA
// pin 5
#define DAC1 					GPIO_Pin_5
#define DAC1_SOURCE				GPIO_PinSource5
#define DAC1_PIO       			GPIOA
// pin 6
#define ADC1_PWR				GPIO_Pin_6
#define ADC1_PWR_SOURCE			GPIO_PinSource6
#define ADC1_PWR_PIO       		GPIOA
// pin 7
#define BAND0		 			GPIO_Pin_7
#define BAND0_SOURCE			GPIO_PinSource7
#define BAND0_PIO       		GPIOA
// pin 8
#define BAND1		 			GPIO_Pin_8
#define BAND1_SOURCE			GPIO_PinSource8
#define BAND1_PIO       		GPIOA
// pin 9
#define DEBUG_PRINT	 			GPIO_Pin_9
#define DEBUG_PRINT_SOURCE		GPIO_PinSource9
#define DEBUG_PRINT_PIO    		GPIOA
// pin 10
#define BAND2 					GPIO_Pin_10
#define BAND2_SOURCE			GPIO_PinSource10
#define BAND2_PIO       		GPIOA
// pin 11
// USB DFU
//
// pin 12
// USB DFU
//
//
// pin 13
// SWDIO
//
// pin 14
// SWCLK
//
//
// pin 15
#define CODEC_I2S_WS			GPIO_Pin_15
#define CODEC_I2S_WS_SOURCE		GPIO_PinSource15
#define CODEC_I2S_WS_PIO  		GPIOA
//
// -----------------------------------------------------------------------------
// ---- 						PORT B										----
// -----------------------------------------------------------------------------
// pin 0
#define BUTTON_BNDM				GPIO_Pin_0
#define BUTTON_BNDM_SOURCE		GPIO_PinSource0
#define BUTTON_BNDM_PIO       	GPIOB
// pin 1
#define PTT_CNTR 				GPIO_Pin_1
#define PTT_CNTR_SOURCE			GPIO_PinSource1
#define PTT_CNTR_PIO       		GPIOB
// pin 2
#define BUTTON_BNDP 			GPIO_Pin_2
#define BUTTON_BNDP_SOURCE		GPIO_PinSource2
#define BUTTON_BNDP_PIO       	GPIOB
// pin 3
#define BUTTON_M2 				GPIO_Pin_3
#define BUTTON_M2_SOURCE		GPIO_PinSource3
#define BUTTON_M2_PIO       	GPIOB
// pin 4
#define ENC_ONE_CH1 			GPIO_Pin_4
#define ENC_ONE_CH1_SOURCE		GPIO_PinSource4
#define ENC_ONE_CH1_PIO       	GPIOB
// pin 5
#define ENC_ONE_CH2 			GPIO_Pin_5
#define ENC_ONE_CH2_SOURCE		GPIO_PinSource5
#define ENC_ONE_CH2_PIO       	GPIOB
// pin 6
#define I2C1_SCL_PIN            GPIO_Pin_6
#define I2C1_SCL_PINSRC         GPIO_PinSource6
#define I2C1_SCL_GPIO           GPIOB
// pin 7
#define I2C1_SDA_PIN            GPIO_Pin_7
#define I2C1_SDA_PINSRC         GPIO_PinSource7
#define I2C1_SDA_GPIO           GPIOB
// pin 8
#define BUTTON_G3 				GPIO_Pin_8
#define BUTTON_G3_SOURCE		GPIO_PinSource8
#define BUTTON_G3_PIO       	GPIOB
// pin 9
#define GREEN_LED 				GPIO_Pin_9
#define GREEN_LED_SOURCE		GPIO_PinSource9
#define GREEN_LED_PIO       	GPIOB
// pin 10
#define I2C2_SCL_PIN            GPIO_Pin_10
#define I2C2_SCL_PINSRC         GPIO_PinSource10
#define I2C2_SCL_GPIO           GPIOB
// pin 11
#define I2C2_SDA_PIN            GPIO_Pin_11
#define I2C2_SDA_PINSRC         GPIO_PinSource11
#define I2C2_SDA_GPIO           GPIOB
// pin 12
#define RED_LED 				GPIO_Pin_12
#define RED_LED_SOURCE			GPIO_PinSource12
#define RED_LED_PIO       		GPIOB
// pin 13
#define LCD_SCK 				GPIO_Pin_13
#define LCD_SCK_SOURCE			GPIO_PinSource13
#define LCD_SCK_PIO         	GPIOB
// pin 14
// USB HOST
//
// pin 15
// USB HOST
//
//
// -----------------------------------------------------------------------------
// ---- 						PORT C										----
// -----------------------------------------------------------------------------
// pin 0
#define BUTTON_G4 				GPIO_Pin_0
#define BUTTON_G4_SOURCE		GPIO_PinSource0
#define BUTTON_G4_PIO       	GPIOC
// pin 1
#define BUTTON_M3 				GPIO_Pin_1
#define BUTTON_M3_SOURCE		GPIO_PinSource1
#define BUTTON_M3_PIO       	GPIOC
// pin 2
#define LCD_MISO 				GPIO_Pin_2
#define LCD_MISO_SOURCE			GPIO_PinSource2
#define LCD_MISO_PIO         	GPIOC
// pin 3
#define LCD_MOSI 				GPIO_Pin_3
#define LCD_MOSI_SOURCE			GPIO_PinSource3
#define LCD_MOSI_PIO         	GPIOC
// pin 4
#define BUTTON_STEPM			GPIO_Pin_4
#define BUTTON_STEPM_SOURCE		GPIO_PinSource4
#define BUTTON_STEPM_PIO       	GPIOC
// pin 5
#define BUTTON_STEPP			GPIO_Pin_5
#define BUTTON_STEPP_SOURCE		GPIO_PinSource5
#define BUTTON_STEPP_PIO       	GPIOC
// pin 6
#define FREQ_ENC_CH1 			GPIO_Pin_6
#define FREQ_ENC_CH1_SOURCE		GPIO_PinSource6
#define FREQ_ENC_CH1_PIO        GPIOC
// pin 7
#define FREQ_ENC_CH2 			GPIO_Pin_7
#define FREQ_ENC_CH2_SOURCE		GPIO_PinSource7
#define FREQ_ENC_CH2_PIO        GPIOC
// pin 8
#define POWER_DOWN 				GPIO_Pin_8
#define POWER_DOWN_SOURCE		GPIO_PinSource8
#define POWER_DOWN_PIO         	GPIOC
// pin 9
#define CODEC_CLOCK 			GPIO_Pin_9
#define CODEC_CLOCK_SOURCE		GPIO_PinSource9
#define CODEC_CLOCK_PIO         GPIOC
// pin 10
#define CODEC_I2S_SCK 			GPIO_Pin_10
#define CODEC_I2S_SCK_SOURCE	GPIO_PinSource10
#define CODEC_I2S_SCK_PIO       GPIOC
// pin 11
#define CODEC_I2S_SDI 			GPIO_Pin_11
#define CODEC_I2S_SDI_SOURCE	GPIO_PinSource11
#define CODEC_I2S_SDI_PIO       GPIOC
// pin 12
#define CODEC_I2S_SDO 			GPIO_Pin_12
#define CODEC_I2S_SDO_SOURCE	GPIO_PinSource12
#define CODEC_I2S_SDO_PIO       GPIOC
// pin 13
#define BUTTON_PWR				GPIO_Pin_13
#define BUTTON_PWR_SOURCE		GPIO_PinSource13
#define BUTTON_PWR_PIO       	GPIOC
// pin 14
#define BUTTON_M1				GPIO_Pin_14
#define BUTTON_M1_SOURCE		GPIO_PinSource14
#define BUTTON_M1_PIO       	GPIOC
// pin 15
#define BUTTON_F3				GPIO_Pin_15
#define BUTTON_F3_SOURCE		GPIO_PinSource15
#define BUTTON_F3_PIO       	GPIOC
//
// -----------------------------------------------------------------------------
// ---- 						PORT D										----
// -----------------------------------------------------------------------------
// pin 0
#define LCD_D2					GPIO_Pin_0
#define LCD_D2_SOURCE			GPIO_PinSource0
#define LCD_D2_PIO      		GPIOD
// pin 1
#define LCD_D3					GPIO_Pin_1
#define LCD_D3_SOURCE			GPIO_PinSource1
#define LCD_D3_PIO      		GPIOD
// pin 2
#define LCD_BACKLIGHT			GPIO_Pin_2
#define LCD_BACKLIGHT_SOURCE	GPIO_PinSource2
#define LCD_BACKLIGHT_PIO      	GPIOD
// pin 3
#define LCD_RESET				GPIO_Pin_3
#define LCD_RESET_SOURCE		GPIO_PinSource3
#define LCD_RESET_PIO      		GPIOD
// pin 4
#define LCD_RD					GPIO_Pin_4
#define LCD_RD_SOURCE			GPIO_PinSource4
#define LCD_RD_PIO      		GPIOD
// pin 5
#define LCD_WR					GPIO_Pin_5
#define LCD_WR_SOURCE			GPIO_PinSource5
#define LCD_WR_PIO      		GPIOD
// pin 6
#define BUTTON_F1				GPIO_Pin_6
#define BUTTON_F1_SOURCE		GPIO_PinSource6
#define BUTTON_F1_PIO       	GPIOD
// pin 7
#define LCD_CSA					GPIO_Pin_7
#define LCD_CSA_SOURCE			GPIO_PinSource7
#define LCD_CSA_PIO      		GPIOD
// pin 8
#define LCD_D15					GPIO_Pin_8
#define LCD_D15_SOURCE			GPIO_PinSource8
#define LCD_D15_PIO      		GPIOD
// pin 9
#define LCD_D16					GPIO_Pin_9
#define LCD_D16_SOURCE			GPIO_PinSource9
#define LCD_D16_PIO      		GPIOD
// pin 10
#define LCD_D17					GPIO_Pin_10
#define LCD_D17_SOURCE			GPIO_PinSource10
#define LCD_D17_PIO      		GPIOD
// pin 11
#define LCD_RS					GPIO_Pin_11
#define LCD_RS_SOURCE			GPIO_PinSource11
#define LCD_RS_PIO      		GPIOD
// pin 12
#define ENC_TWO_CH1 			GPIO_Pin_12
#define ENC_TWO_CH1_SOURCE		GPIO_PinSource12
#define ENC_TWO_CH1_PIO         GPIOD
// pin 13
#define ENC_TWO_CH2 			GPIO_Pin_13
#define ENC_TWO_CH2_SOURCE		GPIO_PinSource13
#define ENC_TWO_CH2_PIO         GPIOD
// pin 14
#define LCD_D0					GPIO_Pin_14
#define LCD_D0_SOURCE			GPIO_PinSource14
#define LCD_D0_PIO      		GPIOD
// pin 15
#define LCD_D1					GPIO_Pin_15
#define LCD_D1_SOURCE			GPIO_PinSource15
#define LCD_D1_PIO      		GPIOD
//
// -----------------------------------------------------------------------------
// ---- 						PORT E										----
// -----------------------------------------------------------------------------
// pin 0
#define PADDLE_DAH				GPIO_Pin_0
#define PADDLE_DAH_SOURCE		GPIO_PinSource0
#define PADDLE_DAH_PIO       	GPIOE
// pin 1
#define PADDLE_DIT				GPIO_Pin_1
#define PADDLE_DIT_SOURCE		GPIO_PinSource1
#define PADDLE_DIT_PIO       	GPIOE
// pin 2
#define BUTTON_F2				GPIO_Pin_2
#define BUTTON_F2_SOURCE		GPIO_PinSource2
#define BUTTON_F2_PIO       	GPIOE
// pin 3
#define BUTTON_F4				GPIO_Pin_3
#define BUTTON_F4_SOURCE		GPIO_PinSource3
#define BUTTON_F4_PIO       	GPIOE
// pin 4
#define BUTTON_G2				GPIO_Pin_4
#define BUTTON_G2_SOURCE		GPIO_PinSource4
#define BUTTON_G2_PIO       	GPIOE
// pin 5
#define BUTTON_F5				GPIO_Pin_5
#define BUTTON_F5_SOURCE		GPIO_PinSource5
#define BUTTON_F5_PIO       	GPIOE
// pin 6
#define BUTTON_G1				GPIO_Pin_6
#define BUTTON_G1_SOURCE		GPIO_PinSource6
#define BUTTON_G1_PIO       	GPIOE
// pin 7
#define LCD_D4					GPIO_Pin_7
#define LCD_D4_SOURCE			GPIO_PinSource7
#define LCD_D4_PIO      		GPIOE
// pin 8
#define LCD_D5					GPIO_Pin_8
#define LCD_D5_SOURCE			GPIO_PinSource8
#define LCD_D5_PIO      		GPIOE
// pin 9
#define LCD_D6					GPIO_Pin_9
#define LCD_D6_SOURCE			GPIO_PinSource9
#define LCD_D6_PIO      		GPIOE
// pin 10
#define LCD_D7					GPIO_Pin_10
#define LCD_D7_SOURCE			GPIO_PinSource10
#define LCD_D7_PIO      		GPIOE
// pin 11
#define LCD_D10					GPIO_Pin_11
#define LCD_D10_SOURCE			GPIO_PinSource11
#define LCD_D10_PIO      		GPIOE
// pin 12
#define LCD_CS 					GPIO_Pin_12
#define LCD_CS_SOURCE			GPIO_PinSource12
#define LCD_CS_PIO         		GPIOE
// pin 13
#define LCD_D12					GPIO_Pin_13
#define LCD_D12_SOURCE			GPIO_PinSource13
#define LCD_D12_PIO      		GPIOE
// pin 14
#define LCD_D13					GPIO_Pin_14
#define LCD_D13_SOURCE			GPIO_PinSource14
#define LCD_D13_PIO      		GPIOE
// pin 15
#define LCD_D14					GPIO_Pin_15
#define LCD_D14_SOURCE			GPIO_PinSource15
#define LCD_D14_PIO      		GPIOE
//
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Buttons map structure
typedef struct ButtonMap
{
	GPIO_TypeDef 	*port;
	ushort			button;

} ButtonMap;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#define POWER_BUTTON_HOLD_TIME		1000000

#define TRX_MODE_RX					0
#define TRX_MODE_TX					1

#define DEMOD_USB					0
#define DEMOD_LSB					1
#define DEMOD_CW					2
#define DEMOD_AM					3
#define DEMOD_DIGI					4
#define DEMOD_MAX_MODE				4

#define RTC_OSC_FREQ				32768

// Transverter oscillator adds shift
#define		TRANSVT_FREQ_A	 		42000000

// Calibration value for SI570
#define		CALIB_FREQ		 		0		//70

// Total bands supported
#define	MAX_BANDS					9

// Bands definition
// - ID
// - SI570 startup freq
// - size in Hz
//
#define	BAND_MODE_80				0
#define	BAND_FREQ_80				14000000
#define	BAND_SIZE_80				300000
//
#define	BAND_MODE_60				1
#define	BAND_FREQ_60				21032000
#define	BAND_SIZE_60				150000
//
#define	BAND_MODE_40				2
#define	BAND_FREQ_40				28000000
#define	BAND_SIZE_40				200000
//
#define	BAND_MODE_30				3
#define	BAND_FREQ_30				40200000
#define	BAND_SIZE_30				150000
//
#define	BAND_MODE_20				4
#define	BAND_FREQ_20				56000000
#define	BAND_SIZE_20				350000
//
#define	BAND_MODE_17				5
#define	BAND_FREQ_17				72200000
#define	BAND_SIZE_17				150000
//
#define	BAND_MODE_15				6
#define	BAND_FREQ_15				84000000
#define	BAND_SIZE_15				400000
//
#define	BAND_MODE_12				7
#define	BAND_FREQ_12				99200000
#define	BAND_SIZE_12				200000
//
#define	BAND_MODE_10				8
#define	BAND_FREQ_10				112000000
#define	BAND_SIZE_10				2700000

// encoder one
#define ENC_ONE_MODE_AUDIO_GAIN		0
#define ENC_ONE_MODE_ST_GAIN		1
#define ENC_ONE_MAX_MODE			2

// encoder two
#define ENC_TWO_MODE_RF_GAIN		0
#define ENC_TWO_MODE_RF_ATTEN		1
#define ENC_TWO_MAX_MODE			2

// encoder three
#define ENC_THREE_MODE_RIT			0
#define ENC_THREE_MODE_CW_SPEED		1
#define ENC_THREE_MAX_MODE			2

// Audio filters
#define AUDIO_FIR_1P8KHZ			0
#define AUDIO_FIR_2P3KHZ			1
#define AUDIO_FIR_3P6KHZ			2
#define AUDIO_FIR_10KHZ				3
#define AUDIO_MAX_FIR				4

#define CW_MODE_IAM_B				0
#define CW_MODE_IAM_A				1
#define CW_MODE_STRAIGHT			2
#define CW_MAX_MODE					3

#define PA_LEVEL_FULL				0
#define PA_LEVEL_5W					1
#define PA_LEVEL_2W					2
#define PA_LEVEL_1W					3
#define PA_LEVEL_0_5W				4
#define PA_LEVEL_MAX_ENTRY			5

#define	US_DELAY					15  // 15 gives 1 uS delay in loop without optimization(O0)

#define CW_SIDETONE_OFFCET			500

// Audio sources for TX modulation
#define TX_AUDIO_MIC				0
#define TX_AUDIO_LINEIN				1
#define TX_AUDIO_MAX_ITEMS			2

// Eeprom items IDs - if updating, make sure eeprom.h list
// is updated as well!!!
#define EEPROM_BAND_MODE			0
#define EEPROM_FREQ_HIGH			1
#define EEPROM_FREQ_LOW				2
#define EEPROM_FREQ_STEP			3
#define EEPROM_TX_AUDIO_SRC			4
#define EEPROM_TCXO_STATE			5
#define EEPROM_PA_BIAS				6
#define EEPROM_AUDIO_GAIN			7
#define EEPROM_RF_GAIN				8
#define EEPROM_RIT					9
#define EEPROM_ATTEN				10
#define EEPROM_POWER				11
#define EEPROM_KEYER_SPEED			12
#define EEPROM_KEYER_MODE			13
#define EEPROM_SIDETONE_GAIN		14
#define EEPROM_MIC_BOOST			15

// Transceiver state public structure
typedef struct TransceiverState
{
	// Sampling rate public flag
	ulong 	samp_rate;

	// Virtual pots public values
	short  	rit_value;
	uchar 	audio_gain;
	uchar 	rf_gain;
	uchar 	rf_atten;
	uchar	st_gain;
	uchar	pa_bias;

	// flag to show delayed request for unmute afte TX->RX change (remove clicks)
	uchar	audio_unmute;

	int  	iq_gain_balance;
	int		iq_phase_balance;

	// Equalisation factor
	float	tx_power_factor;

	// Transceiver calibration mode flag
	uchar	calib_mode;

	// Ham band public flag
	// index of bands table in Flash
	uchar 	band_mode;

	// Receive/Transmit public flag
	uchar 	txrx_mode;

	// TX/RX IRQ lock, to prevent reentrance
	//uchar	txrx_lock;
	uchar	ptt_req;

	// Unattended TX public flag
	//uchar 	auto_mode;

	// Demodulator mode public flag
	uchar 	dmod_mode;

	// Digital mode public flag
	//uchar 	digi_mode;

	// FIR encoder current mode
	//uchar 	fir_enc_mode;

	// Gain encoder current mode
	//uchar 	gain_enc_mode;			// old var, to be removed
	uchar 	enc_one_mode;
	uchar 	enc_two_mode;
	uchar 	enc_thr_mode;

	// Audio filter ID
	uchar	filter_id;

	// Power off flag
	uchar	power_off_req;

	// Eth to UI driver requests flag
	uchar	LcdRefreshReq;

	// Eth to UI public flag
	uchar	new_band;
	uchar	new_mode;
	uchar	new_digi_mode;

	// Current CW mode
	uchar	keyer_mode;
	uchar	keyer_speed;

	uchar	power_level;

	uchar 	tx_audio_source;

	// Microphone gain boost of +20dB via Codec command (TX)
	uchar	mic_boost;

	// Global tuning flag - in every demod mode
	uchar 	tune;

} TransceiverState;

//#define CODEC_USE_SPI

#define DEBUG_COM                        USART1
  
#define non_os_delay()						\
do {							\
  register unsigned int i;				\
  for (i = 0; i < 1000000; ++i)				\
    __asm__ __volatile__ ("nop\n\t":::"memory");	\
} while (0)

#define non_os_delay_a()						\
do {							\
  register unsigned int i;				\
  for (i = 0; i < 10000000; ++i)				\
    __asm__ __volatile__ ("nop\n\t":::"memory");	\
} while (0)

// ------------------------------------------------------------------
// Exports

void mchf_board_green_led(int state);
void mchf_board_red_led(int state);

void mchf_board_switch_tx(char mode);
void mchf_board_power_off(void);

void mchf_board_init(void);
void mchf_board_post_init(void);

// in main.c
void CriticalError(ulong error);

#endif
