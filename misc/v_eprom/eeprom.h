/**
  ******************************************************************************
  * @file    EEPROM_Emulation/inc/eeprom.h 
  * @author  MCD Application Team
   * @version V1.0.0
  * @date    10-October-2011
  * @brief   This file contains all the functions prototypes for the EEPROM 
  *          emulation firmware library.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __EEPROM_H
#define __EEPROM_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

/* Exported constants --------------------------------------------------------*/
/* Define the size of the sectors to be used */
#define PAGE_SIZE               (uint32_t)0x4000  /* Page size = 16KByte */

/* Device voltage range supposed to be [2.7V to 3.6V], the operation will 
   be done by word  */
#define VOLTAGE_RANGE           (uint8_t)VoltageRange_3

/* EEPROM start address in Flash */
#define EEPROM_START_ADDRESS  ((uint32_t)0x08008000) /* EEPROM emulation start address:
                                                  from sector2 : after 16KByte of used 
                                                  Flash memory */

/* Pages 0 and 1 base and end addresses */
#define PAGE0_BASE_ADDRESS    ((uint32_t)(EEPROM_START_ADDRESS + 0x0000))
#define PAGE0_END_ADDRESS     ((uint32_t)(EEPROM_START_ADDRESS + (PAGE_SIZE - 1)))
#define PAGE0_ID               FLASH_Sector_2

#define PAGE1_BASE_ADDRESS    ((uint32_t)(EEPROM_START_ADDRESS + 0x4000))
#define PAGE1_END_ADDRESS     ((uint32_t)(EEPROM_START_ADDRESS + (2 * PAGE_SIZE - 1)))
#define PAGE1_ID               FLASH_Sector_3

/* Used Flash pages for EEPROM emulation */
#define PAGE0                 ((uint16_t)0x0000)
#define PAGE1                 ((uint16_t)0x0001)

/* No valid page define */
#define NO_VALID_PAGE         ((uint16_t)0x00AB)

/* Page status definitions */
#define ERASED                ((uint16_t)0xFFFF)     /* Page is empty */
#define RECEIVE_DATA          ((uint16_t)0xEEEE)     /* Page is marked to receive data */
#define VALID_PAGE            ((uint16_t)0x0000)     /* Page containing valid data */

/* Valid pages in read and write defines */
#define READ_FROM_VALID_PAGE  ((uint8_t)0x00)
#define WRITE_IN_VALID_PAGE   ((uint8_t)0x01)

/* Page full define */
#define PAGE_FULL             ((uint8_t)0x80)

/* Variables' number */
#define NB_OF_VAR             ((uint8_t)0xff)

#define VAR_ADDR_1				0xAA01
#define VAR_ADDR_2				0xAA02
#define VAR_ADDR_3				0xAA03
#define VAR_ADDR_4				0xAA04
#define VAR_ADDR_5				0xAA05
#define VAR_ADDR_6				0xAA06
#define VAR_ADDR_7				0xAA07
#define VAR_ADDR_8				0xAA08
#define VAR_ADDR_9				0xAA09
#define VAR_ADDR_10				0xAA0A
#define VAR_ADDR_11				0xAA0B
#define VAR_ADDR_12				0xAA0C
#define VAR_ADDR_13				0xAA0D
#define VAR_ADDR_14				0xAA0E
#define VAR_ADDR_15				0xAA0F
#define VAR_ADDR_16				0xAA10
#define VAR_ADDR_17				0xAA11
#define VAR_ADDR_18				0xAA12
#define VAR_ADDR_19				0xAA13
#define VAR_ADDR_20				0xAA14
#define VAR_ADDR_21				0xAA15
#define VAR_ADDR_22				0xAA16
#define VAR_ADDR_23				0xAA17
#define VAR_ADDR_24				0xAA18
#define VAR_ADDR_25				0xAA19
#define VAR_ADDR_26				0xAA1A
#define VAR_ADDR_27				0xAA1B
#define VAR_ADDR_28				0xAA1C
#define VAR_ADDR_29				0xAA1D
#define VAR_ADDR_30				0xAA1E
#define VAR_ADDR_31				0xAA1F
#define VAR_ADDR_32				0xAA20
#define VAR_ADDR_33				0xAA21
#define VAR_ADDR_34				0xAA22
#define VAR_ADDR_35				0xAA23
#define VAR_ADDR_36				0xAA24
#define VAR_ADDR_37				0xAA25
#define VAR_ADDR_38				0xAA26
#define VAR_ADDR_39				0xAA27
#define VAR_ADDR_40				0xAA28
#define VAR_ADDR_41				0xAA29
#define VAR_ADDR_42				0xAA2A
#define VAR_ADDR_43				0xAA2B
#define VAR_ADDR_44				0xAA2C
#define VAR_ADDR_45				0xAA2D
#define VAR_ADDR_46				0xAA2E
#define VAR_ADDR_47				0xAA2F
#define VAR_ADDR_48				0xAA30
#define VAR_ADDR_49				0xAA31
#define VAR_ADDR_50				0xAA32
#define VAR_ADDR_51				0xAA33
#define VAR_ADDR_52				0xAA34
#define VAR_ADDR_53				0xAA35
#define VAR_ADDR_54				0xAA36
#define VAR_ADDR_55				0xAA37
#define VAR_ADDR_56				0xAA38
#define VAR_ADDR_57				0xAA39
#define VAR_ADDR_58				0xAA3A
#define VAR_ADDR_59				0xAA3B
#define VAR_ADDR_60				0xAA3C
#define VAR_ADDR_61				0xAA3D
#define VAR_ADDR_62				0xAA3E
#define VAR_ADDR_63				0xAA3F
#define VAR_ADDR_64				0xAA40
#define VAR_ADDR_65				0xAA41
#define VAR_ADDR_66				0xAA42
#define VAR_ADDR_67				0xAA43
#define VAR_ADDR_68				0xAA44
#define VAR_ADDR_69				0xAA45
#define VAR_ADDR_70				0xAA46
#define VAR_ADDR_71				0xAA47
#define VAR_ADDR_72				0xAA48
#define VAR_ADDR_73				0xAA49
#define VAR_ADDR_74				0xAA4A
#define VAR_ADDR_75				0xAA4B
#define VAR_ADDR_76				0xAA4C
#define VAR_ADDR_77				0xAA4D
#define VAR_ADDR_78				0xAA4E
#define VAR_ADDR_79				0xAA4F
#define VAR_ADDR_80				0xAA50
#define VAR_ADDR_81				0xAA51
#define VAR_ADDR_82				0xAA52
#define VAR_ADDR_83				0xAA53
#define VAR_ADDR_84				0xAA54
#define VAR_ADDR_85				0xAA55
#define VAR_ADDR_86				0xAA56
#define VAR_ADDR_87				0xAA57
#define VAR_ADDR_88				0xAA58
#define VAR_ADDR_89				0xAA59
#define VAR_ADDR_90				0xAA5A
#define VAR_ADDR_91				0xAA5B
#define VAR_ADDR_92				0xAA5C
#define VAR_ADDR_93				0xAA5D
#define VAR_ADDR_94				0xAA5E
#define VAR_ADDR_95				0xAA5F
#define VAR_ADDR_96				0xAA60
#define VAR_ADDR_97				0xAA61
#define VAR_ADDR_98				0xAA62
#define VAR_ADDR_99				0xAA63
#define VAR_ADDR_100			0xAA64
#define VAR_ADDR_101			0xAA65
#define VAR_ADDR_102			0xAA66
#define VAR_ADDR_103			0xAA67
#define VAR_ADDR_104			0xAA68
#define VAR_ADDR_105			0xAA6A
#define VAR_ADDR_106			0xAA6B
#define VAR_ADDR_107			0xAA6C
#define VAR_ADDR_108			0xAA6D
#define VAR_ADDR_109			0xAA6E
#define VAR_ADDR_110			0xAA6F
#define VAR_ADDR_111			0xAA70
#define VAR_ADDR_112			0xAA71
#define VAR_ADDR_113			0xAA72
#define VAR_ADDR_114			0xAA73
#define VAR_ADDR_115			0xAA74
#define VAR_ADDR_116			0xAA75
#define VAR_ADDR_117			0xAA76
#define VAR_ADDR_118			0xAA77
#define VAR_ADDR_119			0xAA78
#define VAR_ADDR_120			0xAA79
#define VAR_ADDR_121			0xAA7A
#define VAR_ADDR_122			0xAA7B
#define VAR_ADDR_123			0xAA7C
#define VAR_ADDR_124			0xAA7D
#define VAR_ADDR_125			0xAA7E
#define VAR_ADDR_126			0xAA7F
#define VAR_ADDR_127			0xAA80
#define VAR_ADDR_128			0xAA81
#define VAR_ADDR_129			0xAA82
#define VAR_ADDR_130			0xAA83
#define VAR_ADDR_131			0xAA84
#define VAR_ADDR_132			0xAA85
#define VAR_ADDR_133			0xAA86
#define VAR_ADDR_134			0xAA87
#define VAR_ADDR_135			0xAA88
#define VAR_ADDR_136			0xAA89
#define VAR_ADDR_137			0xAA8A
#define VAR_ADDR_138			0xAA8B
#define VAR_ADDR_139			0xAA8C
#define VAR_ADDR_140			0xAA8D
#define VAR_ADDR_141			0xAA8E
#define VAR_ADDR_142			0xAA8F
#define VAR_ADDR_143			0xAA90
#define VAR_ADDR_144			0xAA91
#define VAR_ADDR_145			0xAA92
#define VAR_ADDR_146			0xAA93
#define VAR_ADDR_147			0xAA94
#define VAR_ADDR_148			0xAA95
#define VAR_ADDR_149			0xAA96
#define VAR_ADDR_150			0xAA97
#define VAR_ADDR_151			0xAA98
#define VAR_ADDR_152			0xAA99
#define VAR_ADDR_153			0xAA9A
#define VAR_ADDR_154			0xAA9B
#define VAR_ADDR_155			0xAA9C
#define VAR_ADDR_156			0xAA9D
#define VAR_ADDR_157			0xAA9E
#define VAR_ADDR_158			0xAA9F
#define VAR_ADDR_159			0xAAA0
#define VAR_ADDR_160			0xAAA1
#define VAR_ADDR_161			0xAAA2
#define VAR_ADDR_162			0xAAA2
#define VAR_ADDR_163			0xAAA3
#define VAR_ADDR_164			0xAAA4
#define VAR_ADDR_165			0xAAA5
#define VAR_ADDR_166			0xAAA6
#define VAR_ADDR_167			0xAAA7
#define VAR_ADDR_168			0xAAA8
#define VAR_ADDR_169			0xAAA9
#define VAR_ADDR_170			0xAAAA
#define VAR_ADDR_171			0xAAAB
#define VAR_ADDR_172			0xAAAC
#define VAR_ADDR_173			0xAAAD
#define VAR_ADDR_174			0xAAAE
#define VAR_ADDR_175			0xAAAF
#define VAR_ADDR_176			0xAAB0
#define VAR_ADDR_177			0xAAB1
#define VAR_ADDR_178			0xAAB2
#define VAR_ADDR_179			0xAAB3
#define VAR_ADDR_180			0xAAB4
#define VAR_ADDR_181			0xAAB5
#define VAR_ADDR_182			0xAAB6
#define VAR_ADDR_183			0xAAB7
#define VAR_ADDR_184			0xAAB8
#define VAR_ADDR_185			0xAAB9
#define VAR_ADDR_186			0xAABA
#define VAR_ADDR_187			0xAABB
#define VAR_ADDR_188			0xAABC
#define VAR_ADDR_189			0xAABD
#define VAR_ADDR_190			0xAABE
#define VAR_ADDR_191			0xAABF
//
//
/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
uint16_t EE_Init(void);
uint16_t EE_ReadVariable(uint16_t VirtAddress, uint16_t* Data);
uint16_t EE_WriteVariable(uint16_t VirtAddress, uint16_t Data);

#endif /* __EEPROM_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
