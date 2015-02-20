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

#ifndef __UI_EEPROM_H
#define __UI_EEPROM_H
//
// Exports
//
void UiDriverSaveEepromValuesPowerDown(void);
void UiDriverLoadEepromValues(void);
//
//
#define	MAX_MENU_ITEM		26		// Number of menu items
#define	MAX_RADIO_CONFIG_ITEMS	33//19	// Number of radio configuration menu items
//
#endif
