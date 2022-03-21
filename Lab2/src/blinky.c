//*****************************************************************************
//
// project0.c - Example to demonstrate minimal TivaWare setup
//
// Copyright (c) 2012-2020 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.2.0.295 of the EK-TM4C1294XL Firmware Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include <stdio.h>
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"

//*****************************************************************************
//
// Define pin to LED mapping.
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>Project Zero (project0)</h1>
//!
//! This example demonstrates the use of TivaWare to setup the clocks and
//! toggle GPIO pins to make the LED blink. This is a good place to start
//! understanding your launchpad and the tools that can be used to program it.
//
//*****************************************************************************



//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
  while(1);
}
#endif

//*****************************************************************************
//
// Main 'C' Language entry point.  Toggle an LED using TivaWare.
//
//*****************************************************************************
volatile uint32_t cont_inter = 0;
//contar interrupcao para segundos
void SysTickIntHandler(void)
{
    cont_inter++;
} 
	
int
main(void)
{
    uint32_t leitura_pin = 0, cont_clock = 0;

	
    // Configuracao de perifericos (botao)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ); //habilita o periferico usado para o botao 
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ))// Determina se o periferico esta pronto
    { 
    }

    // Configuracao de perifericos (LED)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);//habilita o periferico usado para o led
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION))//Determina se o periferico esta pronto
    {
    }

    // GPIO Pin
    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);//configura o pino para uso como input do GPIO
	
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);//configura o pino para uso como output do GPIO
    GPIOPadConfigSet(GPIO_PORTJ_BASE ,GPIO_PIN_0,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);// set a configuracao do pad para um pino

	
    //habilitar systick e interrupcao
    IntMasterEnable();//habilita a interrupção do processador
    SysTickPeriodSet(10000000);//define o período do contador
    SysTickIntRegister(SysTickIntHandler);//registra um manipulador de interrupção para a interrupção do sysstick
    SysTickEnable();//habilita o contador
    SysTickIntEnable();//habilita a interrupcao	
	
    cont_inter = 0;
	
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0x00);//escreve um valor no pino especificado

    // espera 1s para acender o led equivalente a 12 interrupcoes
    while(cont_inter < 12){
    }
   
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);//escreve um valor no pino especificado    

    leitura_pin = GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0);//le o valor dos pinos especificados 
    cont_clock = SysTickValueGet();//pega o valor do contador   
	
    //le se o pino foi pressionado em ate 3s
    while (leitura_pin == 1 && cont_inter < 36)
    {
      	leitura_pin = GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0);//le o valor dos pinos especificados 
    }
    	if(cont_inter >= 36)
	{
		printf("Não foi pressionado o botão em menos de 3 segundos!\n");
	}
    else{
    //Quantidade Clocks
    cont_clock = (SysTickValueGet() - cont_clock) + (10000000 * cont_inter);
    float temp = 0.000000008333*cont_clock;
    
    printf("Quantidade Clocks: %d\n", cont_clock);
    printf("Segundos: %f\n", temp);
    }
}