/*
 * ________________________________________________________________________________________________________
 * Copyright (c) 2020 GebraBit Inc. All rights reserved.
 *
 * This software, related documentation and any modifications thereto (collectively “Software”) is subject
 * to GebraBit and its licensors' intellectual property rights under U.S. and international copyright
 * and other intellectual property rights laws. 
 *
 * GebraBit and its licensors retain all intellectual property and proprietary rights in and to the Software
 * and any use, reproduction, disclosure or distribution of the Software without an express license agreement
 * from GebraBit is strictly prohibited.
 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT 
 * NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT IN  
 * NO EVENT SHALL GebraBit BE LIABLE FOR ANY DIRECT, SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, 
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THE SOFTWARE.
 * @Author       	: Mehrdad Zeinali
 * ________________________________________________________________________________________________________
 */
#include "GebraBit_SGP41.h"
#include "sensirion_gas_index_algorithm.h"
GasIndexAlgorithmParams nox_params;
GasIndexAlgorithmParams voc_params;
/*=========================================================================================================================================
 * @param     cmd    Command that will be writen 
 * @return    None
 ========================================================================================================================================*/
void GB_SGP41_Send_Command(GebraBit_SGP41 * SGP41 , uint16_t cmd)
{
	SGP41->COMMAND = (SGP41_Command)cmd ;
	SGP41->CMD_BUFFER[0] = (uint8_t)((cmd & 0xFF00) >> 8);
	SGP41->CMD_BUFFER[1] = (uint8_t)((cmd & 0x00FF) >> 0);
	HAL_I2C_Master_Transmit(SGP41_I2C,SGP41_WRITE_ADDRESS,SGP41->CMD_BUFFER,2,1000000);
}
/*=========================================================================================================================================
 * @param     cmd    Command that will be writen 
 * @return    None
 ========================================================================================================================================*/
void GB_SGP41_Write_Data(GebraBit_SGP41 * SGP41 , uint16_t cmd , uint16_t data1 , uint16_t data2 )
{
	SGP41->COMMAND = (SGP41_Command)cmd ;
	SGP41->CMD_BUFFER[0] = (uint8_t)((cmd & 0xFF00) >> 8);
  SGP41->CMD_BUFFER[1] = (uint8_t)((cmd & 0x00FF) >> 0);
	SGP41->CMD_BUFFER[2] = (uint8_t)((data1 & 0xFF00) >> 8);
	SGP41->CMD_BUFFER[3] = (uint8_t)((data1 & 0x00FF) >> 0);
	GB_SGP41_CRC_Generate(SGP41,&SGP41->CMD_BUFFER[2], 2);
	SGP41->CMD_BUFFER[4] = SGP41->SGP41_CRC ;
	SGP41->CMD_BUFFER[5] = (uint8_t)((data2 & 0xFF00) >> 8) ;
	SGP41->CMD_BUFFER[6] = (uint8_t)((data2 & 0x00FF) >> 0) ;
	GB_SGP41_CRC_Generate(SGP41,&SGP41->CMD_BUFFER[5], 2);
	SGP41->CMD_BUFFER[7] = SGP41->SGP41_CRC ;
  HAL_I2C_Master_Transmit(SGP41_I2C,SGP41_WRITE_ADDRESS,SGP41->CMD_BUFFER, 8, 1000);
}
/*=========================================================================================================================================
 * @param     cmd    Command that will be writen 
 * @return    None
 ========================================================================================================================================*/
void GB_SGP41_Read_Data(GebraBit_SGP41 * SGP41 , uint8_t* buffer, uint16_t size)
{
	uint16_t i, j;
  HAL_I2C_Master_Receive(SGP41_I2C, (uint16_t)SGP41_WRITE_ADDRESS,buffer, size, 100000);
	for (i = 0, j = 0; i < size; i += WORD_SIZE + CRC8_LEN) 
	 {
     GB_SGP41_CRC_Check(SGP41,&buffer[i], WORD_SIZE, buffer[i + WORD_SIZE]);
     buffer[j++] = buffer[i];
     buffer[j++] = buffer[i + 1];
    }
}

/*=========================================================================================================================================
 * @brief     Reset SGP41
 * @param     SGP41   SGP41 Struct RESET variable
 * @return    Nothing
 ========================================================================================================================================*/
void GB_SGP41_Delay_uSecond(uint32_t useconds) 
{
    uint32_t msec = useconds / 1000;
    if (useconds % 1000 > 0) {
        msec++;
    }
    /*
     * Increment by 1 if STM32F1 driver version less than 1.1.1
     * Old firmwares of STM32F1 sleep 1ms shorter than specified in HAL_Delay.
     * This was fixed with firmware 1.6 (driver version 1.1.1), so we have to
     * fix it ourselves for older firmwares
     */
    if (HAL_GetHalVersion() < 0x01010100) {
        msec++;
    }
    HAL_Delay(msec);
}

/*=========================================================================================================================================
 * @brief     Check CRC
 * @param     SGP41   SGP41 Struct  CRC_CHECK variable
 * @param     value        Value that must be compare with crc
 * @param     crc          CRC Value
 * @return    Nothing
 ========================================================================================================================================*/

void GB_SGP41_CRC_Generate(GebraBit_SGP41 * SGP41 ,const uint8_t* data, uint16_t count) 
{
    uint16_t current_byte;
    uint8_t crc = CRC8_INIT;
    uint8_t crc_bit;
    /* calculates 8-Bit checksum with given polynomial */
    for (current_byte = 0; current_byte < count; current_byte++)
	  {
        crc ^= (data[current_byte]);
        for (crc_bit = 8; crc_bit > 0; --crc_bit)
			   {
            if (crc & 0x80)
                crc = (crc << 1) ^ CRC8_POLYNOMIAL;
            else
                crc = (crc << 1);
         }
    }
	SGP41->SGP41_CRC = crc ;
}
/*=========================================================================================================================================
 * @brief     Check CRC
 * @param     SGP41   SGP41 Struct  CRC_CHECK variable
 * @param     value        Value that must be compare with crc
 * @param     crc          CRC Value
 * @return    Nothing
 ========================================================================================================================================*/
void GB_SGP41_CRC_Check(GebraBit_SGP41 * SGP41 ,const uint8_t* data,uint16_t count,uint8_t checksum)
{
	GB_SGP41_CRC_Generate(SGP41 , data, count);
	if (SGP41->SGP41_CRC == checksum)
		SGP41->CRC_CHECK = CRC_OK;
	else
		SGP41->CRC_CHECK = CRC_ERROR;
} 

/*=========================================================================================================================================
 * @brief     Enable Or Disable On Chip Heater
 * @param     SGP41   SGP41 Struct ON_CHIP_HEATER variable
 * @param     heater        Value is from SGP41_Heater Enume
 * @return    Nothing
 ========================================================================================================================================*/
void GB_SGP41_Get_Serial_Number(GebraBit_SGP41 * SGP41 )  
{
	memset(SGP41->BUFFER, 0, 9*sizeof(uint8_t));  
  GB_SGP41_Send_Command( SGP41 , SGP41_GET_SERIAL_NUMBER );
  GB_SGP41_Delay_uSecond(1000);
	GB_SGP41_Read_Data(  SGP41 , SGP41->BUFFER ,9);
	SGP41->SERIAL_NUMBER1 = (uint16_t)SGP41->BUFFER[0] << 8 | (uint16_t)SGP41->BUFFER[1];
	SGP41->SERIAL_NUMBER2 = (uint16_t)SGP41->BUFFER[2] << 8 | (uint16_t)SGP41->BUFFER[3];
	SGP41->SERIAL_NUMBER3 = (uint16_t)SGP41->BUFFER[4] << 8 | (uint16_t)SGP41->BUFFER[5];
}

/*=========================================================================================================================================
 * @brief     Enable Or Disable On Chip Heater
 * @param     SGP41   SGP41 Struct ON_CHIP_HEATER variable
 * @param     heater        Value is from SGP41_Heater Enume
 * @return    Nothing
 ========================================================================================================================================*/
/*
M403Z 
*/
void GB_SGP41_Perform_Self_Test(GebraBit_SGP41 * SGP41 )  
{
	uint16_t self_test ;
	memset(SGP41->BUFFER, 0, 9*sizeof(uint8_t)); 
  GB_SGP41_Send_Command( SGP41 , SGP41_PERFORM_SELF_TEST );
  GB_SGP41_Delay_uSecond(320000);
	GB_SGP41_Read_Data(  SGP41 , SGP41->BUFFER ,3);
	self_test =(uint16_t)SGP41->BUFFER[0] << 8 | (uint16_t)SGP41->BUFFER[1];
  if (self_test&0x000F)	
		SGP41->SELF_TEST = NOT_DONE; 
	else
   	SGP41->SELF_TEST = DONE; 
}	
/*=========================================================================================================================================
 * @brief     Enable Or Disable On Chip Heater
 * @param     SGP41   SGP41 Struct ON_CHIP_HEATER variable
 * @param     heater        Value is from SGP41_Heater Enume
 * @return    Nothing
 ========================================================================================================================================*/
void GB_SGP41_Turn_Heater_Off(GebraBit_SGP41 * SGP41 )  
{
	SGP41->HEATER_OFF = DONE; 
  GB_SGP41_Send_Command( SGP41 , SGP41_TURN_HEATER_OFF );
  GB_SGP41_Delay_uSecond(1000);
}	
/*=========================================================================================================================================
 * @brief     Enable Or Disable On Chip Heater
 * @param     SGP41   SGP41 Struct ON_CHIP_HEATER variable
 * @param     heater        Value is from SGP41_Heater Enume
 * @return    Nothing
 ========================================================================================================================================*/
void GB_SGP41_Deactivate_Humidity_Compensation(GebraBit_SGP41 * SGP41)  
{
  SGP41->DEFAULT_HUMIDITY 	 = 0x8000 ;
  SGP41->DEFAULT_TEMPERATURE = 0x6666 ;
}
/*=========================================================================================================================================
 * @brief     Enable Or Disable On Chip Heater
 * @param     SGP41   SGP41 Struct ON_CHIP_HEATER variable
 * @param     heater        Value is from SGP41_Heater Enume
 * @return    Nothing
 ========================================================================================================================================*/
void GB_SGP41_Set_Compensation_Humidity_Temperature_Values(GebraBit_SGP41 * SGP41 , float humidity  , float temp , SGP41_Ability Compensation )  
{
	if (Compensation==Enable) 
	 {	
		SGP41->HUMIDITY_COMPENSATION = Enable;
	  SGP41->COMPENSATION_HUMIDITY 	 = humidity ;
    SGP41->COMPENSATION_TEMPERATURE = temp ;
    SGP41->DEFAULT_HUMIDITY 	 = (uint16_t)humidity * 65535 / 100; ;
    SGP41->DEFAULT_TEMPERATURE = (uint16_t)(temp + 45) * 65535 / 175; ;
	 }
	else
	 {
		SGP41->HUMIDITY_COMPENSATION = Disable;
		SGP41->DEFAULT_HUMIDITY 	   = 0x8000 ;
    SGP41->DEFAULT_TEMPERATURE   = 0x6666 ;
	 }
}
/*=========================================================================================================================================
 * @brief     Enable Or Disable On Chip Heater
 * @param     SGP41   SGP41 Struct ON_CHIP_HEATER variable
 * @param     heater        Value is from SGP41_Heater Enume
 * @return    Nothing
 ========================================================================================================================================*/
void GB_SGP41_Execute_Conditioning(GebraBit_SGP41 * SGP41)  
{
  GB_SGP41_Write_Data(SGP41 , SGP41_EXECUTE_CONDITIONING  , SGP41->DEFAULT_HUMIDITY  , SGP41->DEFAULT_TEMPERATURE );
	GB_SGP41_Delay_uSecond(50000);
	GB_SGP41_Read_Data(  SGP41 , SGP41->BUFFER ,3);
	SGP41->SRAW_VOC =(uint16_t)SGP41->BUFFER[0] << 8 | (uint16_t)SGP41->BUFFER[1];
}
/*=========================================================================================================================================
 * @brief     Enable Or Disable On Chip Heater
 * @param     SGP41   SGP41 Struct ON_CHIP_HEATER variable
 * @param     heater        Value is from SGP41_Heater Enume
 * @return    Nothing
 ========================================================================================================================================*/
void GB_SGP41_Measure_Raw_Signal(GebraBit_SGP41 * SGP41)  
{
  GB_SGP41_Write_Data(SGP41 , SGP41_MEASURE_RAW_SIGNAL  , SGP41->DEFAULT_HUMIDITY  , SGP41->DEFAULT_TEMPERATURE );
	GB_SGP41_Delay_uSecond(50000);
	GB_SGP41_Read_Data(  SGP41 , SGP41->BUFFER ,6);
	SGP41->SRAW_VOC =(uint16_t)SGP41->BUFFER[0] << 8 | (uint16_t)SGP41->BUFFER[1];
	SGP41->SRAW_NOX =(uint16_t)SGP41->BUFFER[2] << 8 | (uint16_t)SGP41->BUFFER[3];
}
/*=========================================================================================================================================
 * @brief     initialize SGP41
 * @param     SGP41     SGP41 Struct 
 * @return    Nothing
 ========================================================================================================================================*/ 
void GB_SGP41_initialize( GebraBit_SGP41 * SGP41 )
{
  GasIndexAlgorithm_init(&voc_params, GasIndexAlgorithm_ALGORITHM_TYPE_VOC);
  GasIndexAlgorithm_init(&nox_params, GasIndexAlgorithm_ALGORITHM_TYPE_NOX);
	GB_SGP41_Get_Serial_Number( SGP41 ) ;
	GB_SGP41_Perform_Self_Test( SGP41 ) ;
//	GB_SGP41_Deactivate_Humidity_Compensation( SGP41 ) ;

}
/*=========================================================================================================================================
 * @brief     Configure SGP41
 * @param     SGP41  Configure SGP41 
 * @return    Nothing
 ========================================================================================================================================*/ 
void GB_SGP41_Configuration(GebraBit_SGP41 * SGP41, SGP41_Ability Compensation)
{
	  GB_SGP41_Set_Compensation_Humidity_Temperature_Values(SGP41 ,35 , 21 , Compensation );
    for (int i = 0; i < 10; i++) 
	   {
      GB_SGP41_Delay_uSecond(1000000);
      GB_SGP41_Execute_Conditioning(SGP41);
    }
} 

/*=========================================================================================================================================
 * @brief     Get Data  
 * @param     SGP41     GebraBit_SGP41 Staruct
 * @return    Nothing
 ========================================================================================================================================*/ 
void GB_SGP41_Get_Data(GebraBit_SGP41 * SGP41)
{
 if(SGP41->HUMIDITY_COMPENSATION == Enable )
 {
	for (int i = 0; i < 100; i += 1) 
	 {
		// 1. Sleep: Measure every second (1Hz), as defined by the Gas Index
		// Algorithm prerequisite
		GB_SGP41_Delay_uSecond(1000000);
		// 2. Measure SHT4x  RH and T signals and convert to SGP41 ticks
		//read_compensation_values(&compensation_rh, &compensation_t);
		// 3. Measure SGP4x signals
    GB_SGP41_Measure_Raw_Signal( SGP41 ) ;
		// 4. Process raw signals by Gas Index Algorithm to get the VOC and NOx
		// index values
		GasIndexAlgorithm_process(&voc_params, SGP41->SRAW_VOC, &SGP41->VOC_INDEX_VALUE);
		GasIndexAlgorithm_process(&nox_params, SGP41->SRAW_NOX, &SGP41->NOX_INDEX_VALUE);	
   }
 }
 else
 {
  GB_SGP41_Measure_Raw_Signal( SGP41 ) ;
	GB_SGP41_Delay_uSecond(1000000);	
 }
}
/*----------------------------------------------------------------------------------------------------------------------------------------*
 *                                                                      End                                                               *
 *----------------------------------------------------------------------------------------------------------------------------------------*/
