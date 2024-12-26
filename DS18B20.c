#include "main.h"

//复位DS18B20
void DS18B20_Rst(void)	   
{                 
	DS18B20_IO_OUT(); 	//SET PG11 OUTPUT
	HAL_GPIO_WritePin( DQ_TPC_GPIO_Port, DQ_TPC_Pin, GPIO_PIN_RESET); 	//拉低DQ	
	HAL_Delay_us(750);    	//拉低750us
	HAL_GPIO_WritePin( DQ_TPC_GPIO_Port, DQ_TPC_Pin, GPIO_PIN_SET); 	//DQ=1
	HAL_Delay_us(15);     	//15US
}

uint8_t DS18B20_Check(void) 	   
{   
	uint8_t retry=0;
	DS18B20_IO_IN();	//SET PG11 INPUT	 
	while (HAL_GPIO_ReadPin( DQ_TPC_GPIO_Port, DQ_TPC_Pin)&&retry<200)
	{
		retry++;
		HAL_Delay_us(1);
	};	 
	if(retry>=200)return 1;
	else retry=0;
	while (!HAL_GPIO_ReadPin( DQ_TPC_GPIO_Port, DQ_TPC_Pin)&&retry<240)
	{
		retry++;
		HAL_Delay_us(1);
	};
	if(retry>=240)return 1;	    
	return 0;
}

uint8_t DS18B20_Read_Bit(void) 			 
{
	uint8_t data;
	DS18B20_IO_OUT();	//SET PG11 OUTPUT
	HAL_GPIO_WritePin( DQ_TPC_GPIO_Port, DQ_TPC_Pin, GPIO_PIN_RESET); 	//拉低DQ
	HAL_Delay_us(2); 
	HAL_GPIO_WritePin( DQ_TPC_GPIO_Port, DQ_TPC_Pin, GPIO_PIN_SET); 	//DQ=1
	DS18B20_IO_IN();	//SET PG11 INPUT
	HAL_Delay_us(12);
	if(HAL_GPIO_ReadPin( DQ_TPC_GPIO_Port, DQ_TPC_Pin))data=1;
	else data=0;	 
	HAL_Delay_us(50);           
	return data;
}

uint8_t DS18B20_Read_Byte(void)    
{        
	uint8_t i,j,dat;
	dat=0;
	for (i=1;i<=8;i++) 
	{
		j=DS18B20_Read_Bit();
		dat=(j<<7)|(dat>>1);
	}						    
	return dat;
}

void DS18B20_Write_Byte(uint8_t dat)    
 {             
	uint8_t j;
	uint8_t testb;
	DS18B20_IO_OUT();	//SET PG11 OUTPUT;
	for (j=1;j<=8;j++) 
	{
		testb=dat&0x01;
		dat=dat>>1;
		if (testb) 
		{
			HAL_GPIO_WritePin( DQ_TPC_GPIO_Port, DQ_TPC_Pin, GPIO_PIN_RESET);	// Write 1
			HAL_Delay_us(2);                            
			HAL_GPIO_WritePin( DQ_TPC_GPIO_Port, DQ_TPC_Pin, GPIO_PIN_SET);
			HAL_Delay_us(60);             
		}
		else 
		{
			HAL_GPIO_WritePin( DQ_TPC_GPIO_Port, DQ_TPC_Pin, GPIO_PIN_RESET);	// Write 0
			HAL_Delay_us(60);             
			HAL_GPIO_WritePin( DQ_TPC_GPIO_Port, DQ_TPC_Pin, GPIO_PIN_SET);
			HAL_Delay_us(2);                          
		}
	}
}

//开始温度转换
void DS18B20_Start(void) 
{   						               
    DS18B20_Rst();	   
	DS18B20_Check();	 
    DS18B20_Write_Byte(0xcc);	// skip rom
    DS18B20_Write_Byte(0x44);	// convert
}

short DS18B20_Get_Temp(void)   
{	
	uint8_t temp;
	uint8_t TL,TH;
	short tem;
	DS18B20_Start ();  			// ds1820 start convert
	DS18B20_Rst();
	DS18B20_Check();	 
	DS18B20_Write_Byte(0xcc);	// skip rom
	DS18B20_Write_Byte(0xbe);	// convert	    
	TL=DS18B20_Read_Byte(); 	// LSB   
	TH=DS18B20_Read_Byte(); 	// MSB  
		  
	if(TH>7)
	{
		TH=~TH;
		TL=~TL; 
		temp=0;					//温度为负  
	}
	else temp=1;				//温度为正	  
	tem=TH; 					//获得高八位
	tem<<=8;    
	tem+=TL;					//获得底八位
	tem=tem*0.625;		//转换     
	if(temp)return tem; 		//返回温度值
	else return -tem;    
}


