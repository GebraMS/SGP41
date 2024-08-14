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
#ifndef	__SGP41_H__
#define	__SGP41_H__
#include "main.h"
#include "stdint.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "math.h"
#include "i2c.h"


/************************************************
 *              USER REGISTER MAP               *
 ***********************************************/ 
#define SGP41_I2C		              		    						   &hi2c1
#define SGP41_ADDRESS 									  						    0x59
#define SGP41_WRITE_ADDRESS 			  		 							   ((SGP41_ADDRESS<<1)|0)
#define SGP41_READ_ADDRESS 						   							   ((SGP41_ADDRESS<<1)|1)
#define SGP41_GET_SERIAL_NUMBER   										   0x3682
#define SGP41_PERFORM_SELF_TEST   										   0x280E
#define SGP41_EXECUTE_CONDITIONING											 0x2612
#define SGP41_MEASURE_RAW_SIGNAL												 0x2619
#define SGP41_TURN_HEATER_OFF    			  	 							 0x3615										
/*----------------------------------------------*
 *           USER REGISTER MAP End              *
 *----------------------------------------------*/ 
 /************************************************
 *         MSB Bit Start Location Begin         *
 ***********************************************/ 
#define START_MSB_BIT_AT_0                    0
#define START_MSB_BIT_AT_1                    1
#define START_MSB_BIT_AT_2                    2
#define START_MSB_BIT_AT_3                    3
#define START_MSB_BIT_AT_4                    4
#define START_MSB_BIT_AT_5                    5
#define START_MSB_BIT_AT_6                    6
#define START_MSB_BIT_AT_7                    7
/*----------------------------------------------*
 *        MSB Bit Start Location End            *
 *----------------------------------------------*/ 
/************************************************
 *          Bit Field Length Begin              *
 ***********************************************/ 
#define BIT_LENGTH_1                          1
#define BIT_LENGTH_2                          2
#define BIT_LENGTH_3                          3
#define BIT_LENGTH_4                          4
#define BIT_LENGTH_5                          5
#define BIT_LENGTH_6                          6
#define BIT_LENGTH_7                          7
#define BIT_LENGTH_8                          8
/*----------------------------------------------*
 *          Bit Field Length End                *
 *----------------------------------------------*/
#define DATA_BUFFER_SIZE             					 9
#define CMD_BUFFER_SIZE										 		 8
#define CRC8_POLYNOMIAL 											0x31
#define CRC8_INIT 														0xFF
#define CRC8_LEN 															1
#define WORD_SIZE															2
/**************************************************
 *     Values For Disable And Enable Functions    *
 **************************************************/ 
typedef enum Ability
{  
	Disable = 0     ,                      
	Enable     
}SGP41_Ability;    
/**************************************************
 *  								Values For Alert							*
 **************************************************/ 
typedef enum Activity 
{
  NOT_DONE = 0,
  DONE     = 1
} SGP41_Activity;
/**************************************************
 *  Values For Periodic Data Acquisition Mode			*
 **************************************************/ 
typedef enum Command
{
 GET_SERIAL_NUMBER   	=	0x3682 ,
 PERFORM_SELF_TEST   	=	0x280E ,
 EXECUTE_CONDITIONING	=	0x2612 ,
 MEASURE_RAW_SIGNAL		=	0x2619 ,
 TURN_HEATER_OFF    	=	0x3615	
}SGP41_Command;

/*************************************************
 *           Values For CRC Status    	         *
 **************************************************/ 
typedef enum CRC_Status 
{  
	CRC_ERROR = 0     ,                      
	CRC_OK     
}SGP41_CRC_Status;

 /*************************************************
 *  Defining SGP41 Register & Data As Struct   *
 **************************************************/
typedef	struct SGP41
{
	  uint8_t                       	   REGISTER_CACHE;
		uint8_t														 BUFFER[DATA_BUFFER_SIZE];
	  SGP41_Command 								     COMMAND;
	  uint8_t														 CMD_BUFFER[CMD_BUFFER_SIZE];
	  uint16_t                       	   SERIAL_NUMBER1;
	  uint16_t                       	   SERIAL_NUMBER2;
	  uint16_t                       	   SERIAL_NUMBER3;
	  SGP41_Activity                  	 SELF_TEST;
  	SGP41_Ability											 HUMIDITY_COMPENSATION;
	  SGP41_Activity										 HEATER_OFF;
	  float															 COMPENSATION_HUMIDITY;
		float															 COMPENSATION_TEMPERATURE;
	  uint16_t												   DEFAULT_HUMIDITY;
	  uint16_t												   DEFAULT_TEMPERATURE;
	  SGP41_Activity                     FACTORY_RESET; 
	  uint8_t 													 SGP41_CRC;
	  SGP41_CRC_Status								   CRC_CHECK;
		uint16_t                           SRAW_VOC;
		uint16_t													 SRAW_NOX;
	  int32_t														 VOC_INDEX_VALUE;
    int32_t														 NOX_INDEX_VALUE;  
}GebraBit_SGP41;
/*
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
/********************************************************
 *       Declare SGP41 Configuration Functions         *
 ********************************************************/
extern void GB_SGP41_Send_Command(GebraBit_SGP41 * SGP41 , uint16_t cmd)  ;
extern void GB_SGP41_Write_Data(GebraBit_SGP41 * SGP41 , uint16_t cmd , uint16_t data1 , uint16_t data2 ) ;
extern void GB_SGP41_Read_Data(GebraBit_SGP41 * SGP41 , uint8_t* buffer, uint16_t size) ;
extern void GB_SGP41_Delay_uSecond(uint32_t useconds)      ;
extern void GB_SGP41_CRC_Generate(GebraBit_SGP41 * SGP41 ,const uint8_t* data, uint16_t count)  ;
extern void GB_SGP41_CRC_Check(GebraBit_SGP41 * SGP41 ,const uint8_t* data,uint16_t count,uint8_t checksum)    ;
extern void GB_SGP41_Get_Serial_Number(GebraBit_SGP41 * SGP41 )    ;
extern void GB_SGP41_Perform_Self_Test(GebraBit_SGP41 * SGP41 )      ;
extern void GB_SGP41_Turn_Heater_Off(GebraBit_SGP41 * SGP41 )  ;
extern void GB_SGP41_Deactivate_Humidity_Compensation (GebraBit_SGP41 * SGP41)          ;
extern void GB_SGP41_Set_Compensation_Humidity_Temperature_Values(GebraBit_SGP41 * SGP41 , float humidity  , float temp , SGP41_Ability Compensation )   ;
extern void GB_SGP41_Execute_Conditioning(GebraBit_SGP41 * SGP41) ;  
extern void GB_SGP41_Measure_Raw_Signal(GebraBit_SGP41 * SGP41) ;
extern void GB_SGP41_initialize( GebraBit_SGP41 * SGP41 )     ;
extern void GB_SGP41_Configuration(GebraBit_SGP41 * SGP41, SGP41_Ability Compensation)    ;
extern void GB_SGP41_Get_Data(GebraBit_SGP41 * SGP41)         ;




#endif
