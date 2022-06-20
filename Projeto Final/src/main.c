#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "utils/uartstdio.h"
#include "tx_api.h"
#include "tx_port.h"
#include <string.h>
#include "driverlib/interrupt.h"
#include "driverlib/systick.h"
#include "system_TM4C1294.h"


#define GPIO_PA1_U0TX           0x00000401
#define GPIO_PA0_U0RX           0x00000001

#define DEMO_BYTE_POOL_SIZE     9120
#define DEMO_STACK_SIZE         1024
#define DEMO_QUEUE_SIZE         100

TX_THREAD thread_controll;
TX_THREAD thread1;
TX_THREAD thread2;
TX_THREAD thread3;
TX_BYTE_POOL byte_pool;
TX_MUTEX mutex;
TX_QUEUE esquerda_uart;
TX_QUEUE centro_uart;
TX_QUEUE direita_uart;
TX_QUEUE esquerda_interna;
TX_QUEUE centro_interna;
TX_QUEUE direita_interna;

UCHAR byte_pool_memory[DEMO_BYTE_POOL_SIZE];

UINT status_mutex;


void UARTInit(void)
{
  // Enable UART0
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));
  
  // Initialize the UART for console I/O.  
  UARTConfigSetExpClk(UART0_BASE,SystemCoreClock,(uint32_t)115200,(UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE));
  UARTFIFOEnable(UART0_BASE);
  
  // Enable the GPIO Peripheral used by the UART.  
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));
  
 
  // Configure GPIO Pins for UART mode.  
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE,(GPIO_PIN_0|GPIO_PIN_1));
}

int main(void)
{
  IntMasterEnable(); 
  SysTickPeriodSet(2500000);  
  SysTickIntEnable();
  SysTickEnable();
  UARTInit();

  tx_kernel_enter(); //inicializa o ThreadX e inicia o escalonador de tarefas
}


void inicializacao(char elevador)
{
	
        status_mutex = tx_mutex_get(&mutex, TX_WAIT_FOREVER); //TX_WAIT_FOREVER faz com que o thread de chamada seja suspenso indefinidamente até que um bloco de memória esteja disponível.
	if(status_mutex != TX_SUCCESS) //TX_SUCCESS bloco de memória bem-sucedida
		return;
	
	UARTCharPut(UART0_BASE, elevador);
  	UARTCharPut(UART0_BASE, 'r');
  	UARTCharPut(UART0_BASE, '\n');
  	UARTCharPut(UART0_BASE, '\r');
	
  	tx_thread_sleep(5);
	
  	UARTCharPut(UART0_BASE, elevador);
  	UARTCharPut(UART0_BASE, 'f');
  	UARTCharPut(UART0_BASE, '\n');
  	UARTCharPut(UART0_BASE, '\r');
  	  
  	status_mutex = tx_mutex_put(&mutex);
  	
  	if (status_mutex != TX_SUCCESS)
		return;
}

void Led_On_Off(char elevador, char andar, int On)
{
	status_mutex = tx_mutex_get(&mutex, TX_WAIT_FOREVER);
	if (status_mutex != TX_SUCCESS)
		return;
	
	UARTCharPut(UART0_BASE, elevador);
	if(On == 1)
		UARTCharPut(UART0_BASE, 'L');
	else
		UARTCharPut(UART0_BASE, 'D');
		UARTCharPut(UART0_BASE, andar);
		UARTCharPut(UART0_BASE, '\r');
	
	status_mutex = tx_mutex_put(&mutex);
	
	if (status_mutex != TX_SUCCESS)
		return;
}
 
void Portas(char elevador)
{
	status_mutex = tx_mutex_get(&mutex, TX_WAIT_FOREVER);
	if (status_mutex != TX_SUCCESS)
		return;

	UARTCharPut(UART0_BASE, elevador);
	UARTCharPut(UART0_BASE, 'a');
	UARTCharPut(UART0_BASE, '\n');
	UARTCharPut(UART0_BASE, '\r');

	status_mutex = tx_mutex_put(&mutex);
	if (status_mutex != TX_SUCCESS)
		return;
	
	tx_thread_sleep(1000);
	
	status_mutex = tx_mutex_get(&mutex, TX_WAIT_FOREVER);
	if (status_mutex != TX_SUCCESS)
		return;
	
	UARTCharPut(UART0_BASE, elevador);
	UARTCharPut(UART0_BASE, 'f');
	UARTCharPut(UART0_BASE, '\n');
	UARTCharPut(UART0_BASE, '\r');
	
	status_mutex = tx_mutex_put(&mutex);
	if (status_mutex != TX_SUCCESS)
		return;
}

void movimentacao_elevador(char elevador, int status_movimentacao)
{
	status_mutex = tx_mutex_get(&mutex, TX_WAIT_FOREVER);

	if (status_mutex != TX_SUCCESS)
		return;
	
	if (status_movimentacao == 1)
	{
		UARTCharPut(UART0_BASE, elevador);
		UARTCharPut(UART0_BASE, 's');
		UARTCharPut(UART0_BASE, '\n');
		UARTCharPut(UART0_BASE, '\r');
	}
	else if (status_movimentacao == -1)
	{
		UARTCharPut(UART0_BASE, elevador);
		UARTCharPut(UART0_BASE, 'd');
		UARTCharPut(UART0_BASE, '\n');
		UARTCharPut(UART0_BASE, '\r');
	}
	else
	{
		UARTCharPut(UART0_BASE, elevador);
		UARTCharPut(UART0_BASE, 'p');
		UARTCharPut(UART0_BASE, '\n');
		UARTCharPut(UART0_BASE, '\r');
	}
	
	status_mutex = tx_mutex_put(&mutex);
	
	if (status_mutex != TX_SUCCESS)
		return;
}   

char qual_andar(char andar_dezena, char andar_unidade)
{
  	int andar = (andar_dezena - '0') * 10 + (andar_unidade - '0');
  	
	switch (andar)
    {
        case 0:
            return 'a';
        case 1:
            return 'b';
        case 2:
            return 'c';
        case 3:
            return 'd';
        case 4:
            return 'e';
        case 5:
            return 'f';
        case 6:
            return 'g';
        case 7:
            return 'h';
        case 8:
            return 'i';
        case 9:
            return 'j';
        case 10:
            return 'k';
        case 11:
            return 'l';
        case 12:
            return 'm';
        case 13:
            return 'n';
        case 14:
            return 'o';
        case 15:
            return 'p';
        default:
            return ' ';
    }
}

void ControladoraThread(ULONG msg)
{
  char chamada[16];  
  char aux_char;
  int iterador = 0;
  int posicaoEntrada = -1;
  
  UINT status;
  
  while (1)
  {
    while (UARTCharsAvail(UART0_BASE))
    {
      //tx_thread_sleep(2);
      //Guarda o char recebido
      aux_char = UARTCharGet(UART0_BASE);  
      if (aux_char != '\n' && aux_char != '\r')
       {
        chamada[iterador] = aux_char;
        iterador++;
	//Caso seja F ou A ignora e limpa o vetor
        if (aux_char == 'F' || aux_char == 'A')
        {
          memset(chamada, 0, sizeof chamada);
          iterador = 0;
        }
      }
      else
        {
        iterador = 0;
        posicaoEntrada = 1;
        }
		
	tx_thread_sleep(2);

    }

    if (posicaoEntrada)
    {
      if (chamada[iterador] == 'e')
      {
        status = tx_queue_send(&esquerda_uart, chamada, TX_WAIT_FOREVER);
        if (status != TX_SUCCESS)
          break;
        status = tx_queue_send(&esquerda_interna, chamada, TX_WAIT_FOREVER);
        if (status != TX_SUCCESS)
          break;
      }
      if (chamada[iterador] == 'c')
      {
        status = tx_queue_send(&centro_uart, chamada, TX_WAIT_FOREVER);
        if (status != TX_SUCCESS)
          break;
        status = tx_queue_send(&centro_interna, chamada, TX_WAIT_FOREVER);
        if (status != TX_SUCCESS)
          break;
      }
      if (chamada[iterador] == 'd')
      {
        status = tx_queue_send(&direita_uart, chamada, TX_WAIT_FOREVER);
        if (status != TX_SUCCESS)
          break;
        status = tx_queue_send(&direita_interna, chamada, TX_WAIT_FOREVER);
        if (status != TX_SUCCESS)
          break;        
      }
      memset(chamada, 0, sizeof chamada);
    }
	tx_thread_sleep(2);
  }
}

void elevador_esq(ULONG elevador)
{
	char andar = 'a';
	char novoAndar = 'a';
	char posicao[16];
	char posicaoAtual[16];
	char aux[16];
	int movendo = -1;
	int tamanho = 0;
    
	
	UINT status;
	
	inicializacao('e');
	
	while (1)
	{
	  //limpa e armazena o comando recebido da fila
	  status = tx_queue_receive(&esquerda_uart, posicao, TX_WAIT_FOREVER);
	  if (status != TX_SUCCESS)
	    break;
		
	  //verifica a posição atual do elevador
	  if (posicao[1] >= 48 && posicao[1] <= 57)
	  {
	    if (posicao[2] >= 48 && posicao[2] <= 57)
	    {
	      andar = qual_andar(posicao[1], posicao[2]);
	    }
	    else
	    {
	      andar = qual_andar('0', posicao[1]);
	    }
	  }
	  
	  if (posicao[1] == 'E' || posicao[1] == 'I')
	  {
	    if(posicao[1] == 'I')
	    Led_On_Off('e', posicao[2], 1);
	    tamanho++;
	  }
	  if (tamanho > 0 && strlen(posicaoAtual) == 0)
	  {
	    status = tx_queue_receive(&esquerda_interna, posicaoAtual, TX_WAIT_FOREVER);
	    if (status != TX_SUCCESS)
	      break;
	  }
	  if (posicaoAtual[1] == 'E')
	  {
	    novoAndar = qual_andar(posicaoAtual[2], posicaoAtual[3]);
	  }
	  else if (posicaoAtual[1] == 'I')
	  {
            novoAndar = posicaoAtual[2];
	  }
	  if (novoAndar > andar && movendo == -1)
	  {
	    movimentacao_elevador('e', 1);
	    movendo = 1;
	  }
	  else if (novoAndar < andar && movendo == -1)
	  {
	    movimentacao_elevador('e', -1);
	    movendo = 1;
	  }
	  if (novoAndar == andar)
	  {
	    movimentacao_elevador('e', 0);
	    Portas('e');
	    Led_On_Off('e', novoAndar, 0);
	    tamanho--;
            memset(posicao, 0, sizeof posicao);
	    memset(posicaoAtual, 0, sizeof posicaoAtual);
            movendo = -1;
	  }
	}
        
}

void elevador_cen(ULONG elevador)
{
	char andar = 'a';
	char novoAndar = 'a';
	char posicao[16];
	char posicaoAtual[16];
	char aux[16];
	int movendo = -1;
	int tamanho = 0;
    
    
	
	UINT status;
	
	inicializacao('c');
	
	while (1)
	{
	  //limpa e armazena o comando recebido da fila
	  status = tx_queue_receive(&centro_uart, posicao, TX_WAIT_FOREVER);
	  if (status != TX_SUCCESS)
	    break;
          
	  //verifica a posição atual do elevador
	  if (posicao[1] >= 48 && posicao[1] <= 57)
	  {
	    if (posicao[2] >= 48 && posicao[2] <= 57)
	    {
	      andar = qual_andar(posicao[1], posicao[2]);
	    }
	    else
	    {
	      andar = qual_andar('0', posicao[1]);
	    }
	  }
	  if (posicao[1] == 'E' || posicao[1] == 'I')
	  {
	    if(posicao[1] == 'I')
	    Led_On_Off('c', posicao[2], 1);
	    tamanho++;
	  }
	  if (tamanho > 0 && strlen(posicaoAtual) == 0)
	  {
	    status = tx_queue_receive(&centro_interna, posicaoAtual, TX_WAIT_FOREVER);
	    if (status != TX_SUCCESS)
	      break;
	  }
	  if (posicaoAtual[1] == 'E')
	  {
	    novoAndar = qual_andar(posicaoAtual[2], posicaoAtual[3]);
	  }
	  else if (posicaoAtual[1] == 'I')
	  {
	    novoAndar = posicaoAtual[2];
	  }
	  if (novoAndar > andar && movendo == -1)
	  {
	    movimentacao_elevador('c', 1);
	    movendo = 1;
	  }
	  else if (novoAndar < andar && movendo == -1)
	  {
	    movimentacao_elevador('c', -1);
	    movendo = 1;
	  }
	  if (novoAndar == andar)
	  {
	    movimentacao_elevador('c', 0);
	    Portas('c');
	    Led_On_Off('c', novoAndar, 0);
	    tamanho--;
            memset(posicao, 0, sizeof posicao);
	    memset(posicaoAtual, 0, sizeof posicaoAtual);
	    movendo = -1;
	  }
	}
}

void elevador_dir(ULONG elevador)
{
	char andar = 'a';
	char novoAndar = 'a';
	char posicao[16];
	char posicaoAtual[16];
	char aux[16];
	int movendo = -1;
	int tamanho = 0;
    
    
	
	UINT status;
	
	inicializacao('d');
	
	while (1)
	{
	  //limpa e armazena o comando recebido da fila
	  status = tx_queue_receive(&direita_uart, posicao, TX_WAIT_FOREVER);
	  if (status != TX_SUCCESS)
	    break;
	
          //verifica a posição atual do elevador
	  if (posicao[1] >= 48 && posicao[1] <= 57)
	  {
	    if (posicao[2] >= 48 && posicao[2] <= 57)
	    {
	      andar = qual_andar(posicao[1], posicao[2]);
	    }
	    else
	    {
	      andar = qual_andar('0', posicao[1]);
	    }
	  }
	  if (posicao[1] == 'E' || posicao[1] == 'I')
	  {
	    if(posicao[1] == 'I')
	    Led_On_Off('d', posicao[2], 1);
	    tamanho++;
	  }
	  if (tamanho > 0 && strlen(posicaoAtual) == 0)
	  {
	    status = tx_queue_receive(&direita_interna, posicaoAtual, TX_WAIT_FOREVER);
	    if (status != TX_SUCCESS)
	      break;
	  }
	  if (posicaoAtual[1] == 'E')
	  {
	    novoAndar = qual_andar(posicaoAtual[2], posicaoAtual[3]);
	  }
	  else if (posicaoAtual[1] == 'I')
	  {
	    novoAndar = posicaoAtual[2];
	  }
	  if (novoAndar > andar && movendo == -1)
	  {
	    movimentacao_elevador('d', 1);
	    movendo = 1;
	  }
	  else if (novoAndar < andar && movendo == -1)
	  {
	    movimentacao_elevador('d', -1);
	    movendo = 1;
	  }
	  if (novoAndar == andar && strlen(posicaoAtual) != 0)
	  {
	    movimentacao_elevador('d', 0);
	    Portas('d');
	    Led_On_Off('d', novoAndar, 0);
	    tamanho--;
            memset(posicao, 0, sizeof posicao);
	    memset(posicaoAtual, 0, sizeof posicaoAtual);
	    movendo = -1;
	  }
	}
}

void tx_application_define(void *first_unused_memory)
{
    CHAR    *pointer = TX_NULL;
	
#ifdef TX_ENABLE_EVENT_TRACE
    tx_trace_enable(trace_buffer, sizeof(trace_buffer), 32);
#endif

    tx_byte_pool_create(&byte_pool, "byte pool", byte_pool_memory,DEMO_BYTE_POOL_SIZE);
    
    tx_byte_allocate(&byte_pool, (VOID **)&pointer, DEMO_STACK_SIZE,TX_NO_WAIT);
    tx_thread_create(&thread1, "thread elevador esquerda", elevador_esq, 2,pointer, DEMO_STACK_SIZE,0, 0, 20, TX_AUTO_START);
    tx_byte_allocate(&byte_pool, (VOID **)&pointer, DEMO_STACK_SIZE,TX_NO_WAIT);
    tx_thread_create(&thread2, "thread elevador centro", elevador_cen, 3,pointer, DEMO_STACK_SIZE,0, 0, 20, TX_AUTO_START);
    tx_byte_allocate(&byte_pool, (VOID **)&pointer, DEMO_STACK_SIZE,TX_NO_WAIT);
    tx_thread_create(&thread3, "thread elevador direita", elevador_dir, 4,pointer, DEMO_STACK_SIZE,0, 0, 20, TX_AUTO_START);
    tx_byte_allocate(&byte_pool, (VOID **)&pointer, DEMO_STACK_SIZE,TX_NO_WAIT);
    tx_thread_create(&thread_controll, "thread controll", ControladoraThread, 1,pointer, DEMO_STACK_SIZE,0, 0, 20, TX_AUTO_START);
    
    tx_mutex_create(&mutex, "mutex", TX_NO_INHERIT);
    
    tx_byte_allocate(&byte_pool, (VOID **)&pointer, DEMO_QUEUE_SIZE * sizeof(ULONG), TX_NO_WAIT);
    tx_queue_create(&esquerda_uart, "queue elevador esquerda", TX_1_ULONG, pointer,DEMO_QUEUE_SIZE * sizeof(ULONG));
    tx_byte_allocate(&byte_pool, (VOID **)&pointer, DEMO_QUEUE_SIZE * sizeof(ULONG), TX_NO_WAIT);
    tx_queue_create(&centro_uart, "queue elevador centro", TX_1_ULONG, pointer,DEMO_QUEUE_SIZE * sizeof(ULONG));
    tx_byte_allocate(&byte_pool, (VOID **)&pointer, DEMO_QUEUE_SIZE * sizeof(ULONG), TX_NO_WAIT);
    tx_queue_create(&direita_uart, "queue elevador direita", TX_1_ULONG, pointer,DEMO_QUEUE_SIZE * sizeof(ULONG));
    tx_byte_allocate(&byte_pool, (VOID **)&pointer, DEMO_QUEUE_SIZE * sizeof(ULONG), TX_NO_WAIT);
    tx_queue_create(&esquerda_interna, "queue interna esquerda", TX_1_ULONG, pointer, DEMO_QUEUE_SIZE * sizeof(ULONG));
    tx_byte_allocate(&byte_pool, (VOID **)&pointer, DEMO_QUEUE_SIZE * sizeof(ULONG), TX_NO_WAIT);
    tx_queue_create(&centro_interna, "queue interna centro", TX_1_ULONG, pointer, DEMO_QUEUE_SIZE * sizeof(ULONG));
    tx_byte_allocate(&byte_pool, (VOID **)&pointer, DEMO_QUEUE_SIZE * sizeof(ULONG), TX_NO_WAIT);
    tx_queue_create(&direita_interna, "queue interna direita", TX_1_ULONG, pointer, DEMO_QUEUE_SIZE * sizeof(ULONG));
    
    tx_block_release(pointer);
}     
        