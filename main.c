/**
  ******************************************************************************
  * @file    FLASH_Program/main.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    11-November-2013
  * @brief   This example provides a description of how to program the FLASH 
  *          memory integrated within STM32F429I-DISCO Devices.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"

/** @addtogroup STM32F429I_DISCOVERY_Examples
  * @{
  */

/** @addtogroup FLASH_Program
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t uwStartSector = 0;
uint32_t uwEndSector = 0;
uint32_t uwAddress = 0;
uint32_t uwSectorCounter = 0;

__IO uint32_t uwData32 = 0;
__IO uint32_t uwMemoryProgramStatus = 0;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static uint32_t GetSector(uint32_t Address);
/**
  * @brief   Main program
  * @param  None
  * @retval None
  */

void SetSysClockTo84(void)
{
  /* SYSCLK, HCLK, PCLK2 and PCLK1 configuration -----------------------------*/   
  /* RCC system reset(for debug purpose) */
  RCC_DeInit();

  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);

  /* Wait till HSE is ready */
  ErrorStatus HSEStartUpStatus = RCC_WaitForHSEStartUp();

  if (HSEStartUpStatus == SUCCESS)
  {
    /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(ENABLE);

    /* Flash 2 wait state */
    FLASH_SetLatency(FLASH_Latency_2);
 
    /* PLL configuration */
    RCC_PLLConfig(RCC_PLLSource_HSE, 6, 252, 4, 7);

    /* Enable PLL */ 
    RCC_PLLCmd(ENABLE);

    /* Wait till PLL is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {
    }

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    /* Wait till PLL is used as system clock source */
    while (RCC_GetSYSCLKSource() != 0x08)
    {
    }
  }
  else
  { /* If HSE fails to start-up, the application will have wrong clock configuration.
       User can add here some code to deal with this error */    

    /* Go to infinite loop */
    while (1)
    {
    }
  }
}

void erase_data(uint32_t uwStartSector,uint32_t uwEndSector)
{
  uint32_t uwSectorCounter = uwStartSector;
  /* Strat the erase operation */
  
  while (uwSectorCounter <= uwEndSector) 
  {
    /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
       be done by word */ 
    if (FLASH_EraseSector(uwSectorCounter, VoltageRange_3) != FLASH_COMPLETE)
    { 
      USART1_puts("Erase error\r\n");
      /* Error occurred while sector erase. 
         User can add here some code to deal with this error  */
    }
    /* jump to the next sector */
    if (uwSectorCounter == FLASH_Sector_11)
    {
      uwSectorCounter += 40;
    } 
    else 
    {
      uwSectorCounter += 8;
    }
  }
}

void program_data(uint32_t uwStartAddress, uint32_t uwEndAddress){
  uint32_t uwAddress = uwStartAddress;
  uint32_t program_error_count = 0;
  while (uwAddress < uwEndAddress)
  {
    if ( !FLASH_ProgramWord(uwAddress, DATA_32) == FLASH_COMPLETE)
      program_error_count ++ ;
    uwAddress = uwAddress + 4;
  }
  if(program_error_count)
    USART1_puts("Program error\r\n");
}

void print_data(uint32_t uwStartAddress, uint32_t uwEndAddress)
{
  uint32_t uwAddress = uwStartAddress;
  int newline_count = 1;

  while (uwAddress < uwEndAddress)
  {
    uwData32 = *(__IO uint32_t*)uwAddress;
    USART1_putsHex(uwData32);
    if(newline_count %= 4)
      USART1_puts(" ");
    else
      USART1_puts("\r\n");

    uwAddress = uwAddress + 4;
    newline_count ++;
  }
  USART1_puts("\r\n\r\n");
}

int main(void)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       files (startup_stm32f429_439xx.s) before to branch to application main. 
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f4xx.c file
     */
  
  SetSysClockTo84();

  // Initailize usart
  RCC_Configuration();
  GPIO_Configuration();
  USART1_Configuration();

  /* Initialize LEDs onSTM32F429I-DISCO */
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED4);

  USART1_puts("flash demo start\r\n");
  

  USART1_puts("Flash unlock\r\n");
  FLASH_Unlock();
  
  /* Clear pending flags (if any) */  
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 
  /* disable write protection */
  FLASH_OB_Unlock();
  FLASH_OB_WRP1Config(OB_WRP_Sector_All, DISABLE);
  if (FLASH_OB_Launch() != FLASH_COMPLETE)
    USART1_puts("Disable write protection error\r\n");
  FLASH_OB_Lock();


  USART1_puts("Erase sector 12 13\r\n");
  erase_data(GetSector(ADDR_FLASH_SECTOR_12), GetSector(ADDR_FLASH_SECTOR_13) );
  USART1_puts("Sector 12 (first 80 bytes)\r\n");
  print_data(ADDR_FLASH_SECTOR_12, ADDR_FLASH_SECTOR_12 + 80);

  USART1_puts("Sector 13 (first 80 bytes)\r\n");
  print_data(ADDR_FLASH_SECTOR_13, ADDR_FLASH_SECTOR_13 + 80);

  USART1_puts("Program 0x32F429DC to sector 12\r\n");
  program_data(ADDR_FLASH_SECTOR_12, ADDR_FLASH_SECTOR_13);
  USART1_puts("Sector 12 (first 80 bytes)\r\n");
  print_data(ADDR_FLASH_SECTOR_12, ADDR_FLASH_SECTOR_12 + 80);

  USART1_puts("Enable write protection to sector 13\r\n");
  FLASH_OB_Unlock();
  FLASH_OB_WRP1Config(OB_WRP_Sector_13, ENABLE);
  if (FLASH_OB_Launch() != FLASH_COMPLETE)
    USART1_puts("Write protection error...\r\n");
  FLASH_OB_Lock();

  USART1_puts("Attempt to program 0x32F429DC to sector 13\r\n");
  program_data(ADDR_FLASH_SECTOR_13, ADDR_FLASH_SECTOR_14);
  USART1_puts("sector 13 (first 80 bytes)\r\n");
  print_data(ADDR_FLASH_SECTOR_13, ADDR_FLASH_SECTOR_13 + 80);

  USART1_puts("Flash lock\r\n");
  FLASH_Lock(); 

  while (1)
  {
  }
}

static uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;
  
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_Sector_0;  
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_Sector_1;  
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_Sector_2;  
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_Sector_3;  
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_Sector_4;  
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_Sector_5;  
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_Sector_6;  
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_Sector_7;  
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_Sector_8;  
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_Sector_9;  
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_Sector_10;  
  }
  else if((Address < ADDR_FLASH_SECTOR_12) && (Address >= ADDR_FLASH_SECTOR_11))
  {
    sector = FLASH_Sector_11;  
  }

  else if((Address < ADDR_FLASH_SECTOR_13) && (Address >= ADDR_FLASH_SECTOR_12))
  {
    sector = FLASH_Sector_12;  
  }
  else if((Address < ADDR_FLASH_SECTOR_14) && (Address >= ADDR_FLASH_SECTOR_13))
  {
    sector = FLASH_Sector_13;  
  }
  else if((Address < ADDR_FLASH_SECTOR_15) && (Address >= ADDR_FLASH_SECTOR_14))
  {
    sector = FLASH_Sector_14;  
  }
  else if((Address < ADDR_FLASH_SECTOR_16) && (Address >= ADDR_FLASH_SECTOR_15))
  {
    sector = FLASH_Sector_15;  
  }
  else if((Address < ADDR_FLASH_SECTOR_17) && (Address >= ADDR_FLASH_SECTOR_16))
  {
    sector = FLASH_Sector_16;  
  }
  else if((Address < ADDR_FLASH_SECTOR_18) && (Address >= ADDR_FLASH_SECTOR_17))
  {
    sector = FLASH_Sector_17;  
  }
  else if((Address < ADDR_FLASH_SECTOR_19) && (Address >= ADDR_FLASH_SECTOR_18))
  {
    sector = FLASH_Sector_18;  
  }
  else if((Address < ADDR_FLASH_SECTOR_20) && (Address >= ADDR_FLASH_SECTOR_19))
  {
    sector = FLASH_Sector_19;  
  }
  else if((Address < ADDR_FLASH_SECTOR_21) && (Address >= ADDR_FLASH_SECTOR_20))
  {
    sector = FLASH_Sector_20;  
  } 
  else if((Address < ADDR_FLASH_SECTOR_22) && (Address >= ADDR_FLASH_SECTOR_21))
  {
    sector = FLASH_Sector_21;  
  }
  else if((Address < ADDR_FLASH_SECTOR_23) && (Address >= ADDR_FLASH_SECTOR_22))
  {
    sector = FLASH_Sector_22;  
  }
  else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_23))*/
  {
    sector = FLASH_Sector_23;  
  }
  return sector;
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
