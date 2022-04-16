#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include <stdio.h>
#include "driverlib/uart.h"
#include "driverlib/uart.c"
#include "driverlib/pin_map.h"
#include <inttypes.h>


#define GPIO_PA0_U0RX   0x00000001
#define GPIO_PA1_U0TX   0x00000401

uint8_t entrada_buffer[4];
uint8_t posicao;

//Interrupção UART
void UART_Interruption(void){
	//Pega o status da interrupção
	uint32_t status;
	status = UARTIntStatus(UART0_BASE,true);
	//limpa o buffer  
	UARTIntClear(UART0_BASE,status);
	//armazena a entrada
	entrada_buffer[posicao%4] = (uint8_t)UARTCharGetNonBlocking(UART0_BASE);
	posicao++;
}

//Inicializa UART0 a 115200 bps
//8 bits de dados, sem bit de paridade, FIFOs habilitadas
void UART0_inicializa_115200bps(void)
{

	//1 - Habilita periférico
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	//Aguarda até que o periférico esteja pronto 
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));
	//2 - Habilita UART0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	//Aguarda até que o módulo UART0 esteja pronto 
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));
	
	//3 - Inicializa o UART. Define a taxa de transmissão, o número de bits de dados, desliga a paridade. O UART é habilitado pela chamada de função.
	GPIOPinTypeUART(GPIO_PORTA_BASE,(GPIO_PIN_0|GPIO_PIN_1));
	UARTConfigSetExpClk(UART0_BASE,(uint32_t)120000000,(uint32_t)115200,(UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE));

	//4 - Habilita interrupt
	IntMasterEnable();
	
	//5 - Habilitar funções alternativas para os pinos PA1-0
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	
	//6 - Configuração portas seriais
	//desabilita a transmissão e recepção de FIFOs no UART.
	UARTFIFODisable(UART0_BASE);
	//habilita as interrupções UART
	UARTIntEnable(UART0_BASE,UART_INT_RX|UART_INT_RT); 
	//habilita as interrupções UART
	UARTIntRegister(UART0_BASE,UART_Interruption); 
}

int main()
{
	UART0_inicializa_115200bps();
	//entrada_buffer = 0;
	posicao = 0;
	uint8_t posicao_ant = 0;
	uint8_t *aux;
	
	while(1)
	{
        if(posicao_ant!=posicao){
		//salva o dado no auxiliar
		aux = &entrada_buffer[posicao_ant%4];
		//Se for maisculo transforma em minusculo
			if(((*aux)>=(uint8_t)65)&&((*aux)<=(uint8_t)90)){
			*aux += 32; 
			}

			if(UARTCharPutNonBlocking(UART0_BASE,entrada_buffer[posicao_ant%4])){
			posicao_ant++;
			}
        }
    }
}