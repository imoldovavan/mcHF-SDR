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

// Audio Driver
#include "audio_driver.h"
#include "cw_gen.h"

// UI Driver
#include "ui_driver.h"
#include "ui_rotary.h"
#include "ui_lcd_hy28.h"

// Keyboard Driver
#include "keyb_driver.h"

// Misc
#include "softdds.h"

// Eeprom
#include "eeprom.h"

// Transceiver state public structure
__IO TransceiverState ts;

// ----------------------------------------------------
// Create a time reference incremented by 1 mS and 10mS
//__IO uint32_t LocalTime_1MS  = 0;
//__IO uint32_t LocalTime_10MS = 0;
//__IO uint32_t LocalTime_Over = 0;
// ----------------------------------------------------

// USB Host
//extern USB_OTG_CORE_HANDLE          USB_OTG_Core_dev;

// TIM5 publics
//extern __IO uint32_t PeriodValue;
//extern __IO uint32_t CaptureNumber;
//uint16_t tmpCC4[2] = {0, 0};

const uint16_t VirtAddVarTab[NB_OF_VAR] =
{
		VAR_ADDR_1,
		VAR_ADDR_2,
		VAR_ADDR_3,
		VAR_ADDR_4,
		VAR_ADDR_5,
		VAR_ADDR_6,
		VAR_ADDR_7,
		VAR_ADDR_8,
		VAR_ADDR_9,
		VAR_ADDR_10,
		VAR_ADDR_11,
		VAR_ADDR_12,
		VAR_ADDR_13,
		VAR_ADDR_14,
		VAR_ADDR_15,
		VAR_ADDR_16,
		VAR_ADDR_17,
		VAR_ADDR_18,
		VAR_ADDR_19,
		VAR_ADDR_20,
		VAR_ADDR_21
};

// System tick if needed
__IO uint32_t TimingDelay = 0;

uchar wd_init_enabled = 0;

//*----------------------------------------------------------------------------
//* Function Name       : CriticalError
//* Object              : should never be here, really
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void CriticalError(ulong error)
{
	NVIC_SystemReset();
}

//*----------------------------------------------------------------------------
//* Function Name       : NMI_Handler
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void NMI_Handler(void)
{
	printf("NMI_Handler called\n\r");
	CriticalError(1);
}

//*----------------------------------------------------------------------------
//* Function Name       : HardFault_Handler
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void HardFault_Handler(void)
{
	printf("HardFault_Handler called\n\r");
	CriticalError(2);
}

//*----------------------------------------------------------------------------
//* Function Name       : MemManage_Handler
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void MemManage_Handler(void)
{
	printf("MemManage_Handler called\n\r");
	CriticalError(3);
}

//*----------------------------------------------------------------------------
//* Function Name       : BusFault_Handler
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void BusFault_Handler(void)
{
	printf("BusFault_Handler called\n\r");
	CriticalError(4);
}

//*----------------------------------------------------------------------------
//* Function Name       : UsageFault_Handler
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UsageFault_Handler(void)
{
	printf("UsageFault_Handler called\n\r");
	CriticalError(5);
}

//*----------------------------------------------------------------------------
//* Function Name       : SVC_Handler
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void SVC_Handler(void)
{
	printf("SVC_Handler called\n\r");
	CriticalError(6);
}

//*----------------------------------------------------------------------------
//* Function Name       : DebugMon_Handler
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void DebugMon_Handler(void)
{
	printf("DebugMon_Handler called\n\r");
	CriticalError(7);
}

//*----------------------------------------------------------------------------
//* Function Name       : SysTick_Handler
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void SysTick_Handler(void)
{
//!	TimingDelay++;

	// Process UI refresh
	ui_driver_irq();
}

//*----------------------------------------------------------------------------
//* Function Name       : EXTI0_IRQHandler
//* Object              : paddles dah
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void EXTI0_IRQHandler(void)
{
	// Checks whether the User Button EXTI line is asserted
	if (EXTI_GetITStatus(EXTI_Line0) != RESET)
	{
		// Call handler
		if(ts.dmod_mode == DEMOD_CW)
			cw_gen_dah_IRQ();

		// PTT activate
		if((ts.dmod_mode == DEMOD_USB)||(ts.dmod_mode == DEMOD_LSB))
		{
			// Prevent re-entrance
			//if((ts.txrx_mode == TRX_MODE_RX) && (ts.txrx_lock == 0))
			//{
			//	// Direct switch here
			//	ts.txrx_mode = TRX_MODE_TX;
			//	ui_driver_toggle_tx();			// PTT
			//}

			ts.ptt_req = 1;
		}
	}

	// Clears the EXTI's line pending bit
	EXTI_ClearITPendingBit(EXTI_Line0);
}

//*----------------------------------------------------------------------------
//* Function Name       : EXTI1_IRQHandler
//* Object              : paddles dit
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void EXTI1_IRQHandler(void)
{
	// Checks whether the User Button EXTI line is asserted
	if (EXTI_GetITStatus(EXTI_Line1) != RESET)
	{
		// Call handler
		if(ts.dmod_mode == DEMOD_CW)
			cw_gen_dit_IRQ();
	}

	// Clears the EXTI's line pending bit
	EXTI_ClearITPendingBit(EXTI_Line1);
}

//*----------------------------------------------------------------------------
//* Function Name       : EXTI15_10_IRQHandler
//* Object              : power button irq here
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void EXTI15_10_IRQHandler(void)
{
	// power button interrupt
	if(EXTI_GetITStatus(EXTI_Line13) != RESET)
	{
		// Signal power off
		ts.power_off_req = 1;

		// Clear interrupt pending bit
		EXTI_ClearITPendingBit(EXTI_Line13);
	}
}

/*void TIM5_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM5, TIM_IT_CC4) != RESET)
  {
    // Get the Input Capture value
    tmpCC4[CaptureNumber++] = TIM_GetCapture4(TIM5);

    // Clear CC4 Interrupt pending bit
    TIM_ClearITPendingBit(TIM5, TIM_IT_CC4);

    if (CaptureNumber >= 2)
    {
      // Compute the period length
      PeriodValue = (uint16_t)(0xFFFF - tmpCC4[0] + tmpCC4[1] + 1);
    }
  }
}*/

//*----------------------------------------------------------------------------
//* Function Name       : TransceiverStateInit
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void TransceiverStateInit(void)
{
	// Defaults always
	ts.power_off_req	= 0;						// No power off
	ts.txrx_mode 		= TRX_MODE_RX;				// start in RX
	ts.samp_rate		= I2S_AudioFreq_48k;		// set sampling rate

	ts.enc_one_mode 	= ENC_ONE_MODE_AUDIO_GAIN;
	ts.enc_two_mode 	= ENC_TWO_MODE_RF_GAIN;
	ts.enc_thr_mode		= ENC_THREE_MODE_RIT;

	ts.band_mode  		= BAND_MODE_20;				// band from eeprom
	ts.dmod_mode 		= DEMOD_USB;				// demodulator mode
	ts.audio_gain		= 6;						// Set initial volume
	ts.rf_gain			= 4;						// Set default RF gain
	ts.rit_value		= 0;						// RIT value
	ts.filter_id		= AUDIO_FIR_3P6KHZ;			// Default filter
	ts.st_gain			= 5;						// Sidetone gain
	ts.keyer_mode		= CW_MODE_IAM_B;			// CW keyer mode
	ts.keyer_speed		= 20;						// CW keyer speed
	ts.rf_atten			= 0;						// RF attenuator
	ts.iq_gain_balance 	= 0;
	ts.iq_phase_balance = 0;
	ts.calib_mode		= 0;

	//ts.txrx_lock		= 0;						// unlocked on start
	ts.audio_unmute		= 0;						// delayed un-mute not needed

	ts.tx_audio_source	= TX_AUDIO_MIC;				// default source is microphone

	ts.tune				= 0;						// reset tuning flag

	ts.tx_power_factor	= 0.50;						// TX power factor

	ts.pa_bias			= 0;						// Use lowest possible voltage as default

	ts.power_level		= PA_LEVEL_2W;

	ts.mic_boost		= 0;
}

//*----------------------------------------------------------------------------
//* Function Name       : MiscInit
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void MiscInit(void)
{
	//printf("misc init...\n\r");

	// Init Soft DDS
	softdds_setfreq(0.0,ts.samp_rate,0);
	//softdds_setfreq(500.0,ts.samp_rate,0);
	//softdds_setfreq(1000.0,ts.samp_rate,0);
	//softdds_setfreq(2000.0,ts.samp_rate,0);
	//softdds_setfreq(3000.0,ts.samp_rate,0);
	//softdds_setfreq(4000.0,ts.samp_rate,0);

	//printf("misc init ok\n\r");
}

static void wd_reset(void)
{
	// Init WD
	if(!wd_init_enabled)
	{
		// Start watchdog
		WWDG_Enable(WD_REFRESH_COUNTER);

		// Reset
		wd_init_enabled = 1;
		TimingDelay 	= 0;

		return;
	}

	// 40mS flag for WD reset
	if(TimingDelay > 40)
	{
		TimingDelay = 0;
		//GREEN_LED_PIO->ODR ^= RED_LED;

		// Update WWDG counter
		WWDG_SetCounter(WD_REFRESH_COUNTER);
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : main
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
int main(void)
{
	// Set unbuffered mode for stdout (newlib)
	//setvbuf( stdout, 0, _IONBF, 0 );

	// HW init
	mchf_board_init();

	// Power on
	mchf_board_green_led(1);

	// Set default transceiver state
	TransceiverStateInit();

	// Show logo
	UiLcdHy28_ShowStartUpScreen(100);

	// Extra init
	MiscInit();

	// Virtual Eeprom init
	EE_Init();

	// Audio HW init
	audio_driver_init();

	// Usb Host driver init
	//keyb_driver_init();

	// UI HW init
	ui_driver_init();

#ifdef DEBUG_BUILD
	printf("== main loop starting ==\n\r");
#endif

	// Transceiver main loop
	for(;;)
	{
		// UI events processing
		ui_driver_thread();

		// Audio driver processing
		//audio_driver_thread();

		// USB Host driver processing
		//usbh_driver_thread();

		// Reset WD - not working
		//wd_reset();
	}
}
