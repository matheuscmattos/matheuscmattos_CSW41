#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <ctype.h>
#include "utils/uartstdio.h"
#include "tx_api.h"
#include "system_TM4C1294.h"
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"
#include "inc/hw_memmap.h"

#define GPIO_PA1_U0TX 0x00000401
#define GPIO_PA0_U0RX 0x00000001

#define BaudRate 115200
#define BUFFER_SIZE 16

#define ELEVATOR_BLOCK_POOL_SIZE 100
#define ELEVATOR_BYTE_POOL_SIZE 9120
#define ELEVATOR_QUEUE_SIZE 100
#define ELEVATOR_STACK_SIZE 1024

#define ELEVATOR_TYPE_1 'e'
#define ELEVATOR_TYPE_2 'c'
#define ELEVATOR_TYPE_3 'd'

#define INTERNAL_SIGN 'I'
#define EXTERNAL_SIGN 'E'

#define MOVE_UP 1
#define MOVE_DOWN -1
#define STOP_MOVE 0

uint8_t BufferPos;
uint8_t LastPos;
uint8_t buffer[BUFFER_SIZE];

TX_THREAD threadMain;
TX_THREAD threadElevator1;
TX_THREAD threadElevator2;
TX_THREAD threadElevator3;
TX_QUEUE queueElevator1;
TX_QUEUE internalQueue1;
TX_QUEUE queueElevator2;
TX_QUEUE internalQueue2;
TX_QUEUE queueElevator3;
TX_QUEUE internalQueue3;
TX_MUTEX mutex;
TX_BYTE_POOL bytePool;
TX_BLOCK_POOL block_pool_0;
UCHAR bytePoolMemory[ELEVATOR_BYTE_POOL_SIZE];


void UARTInit(void){
  //Initialize peripherals
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  while (!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));
  //Config GPIO
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinTypeUART(GPIO_PORTA_BASE,(GPIO_PIN_0|GPIO_PIN_1));
  //Config UART
  UARTConfigSetExpClk(UART0_BASE,SystemCoreClock,(uint32_t)BaudRate,(UART_CONFIG_WLEN_8| UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE));
  UARTFIFOEnable(UART0_BASE);
  //Buffer
  BufferPos = 0;
  LastPos = 0;
}


int main(){
  IntMasterEnable();
  SysTickPeriodSet(2500000);
  SysTickIntEnable();
  SysTickEnable();
  UARTInit();
  tx_kernel_enter();
}

//Função para obter Mutex
bool sendGetMutexAndcheckStatus(){
  return ((UINT) tx_mutex_get(&mutex, TX_WAIT_FOREVER)) == TX_SUCCESS;
}

//Função para devolver Mutex 
bool sendPutMutexAndcheckStatus(){
  return ((UINT) tx_mutex_put(&mutex)) == TX_SUCCESS;
}

//envia comandos para final de linha
void commandFinalizer(bool finalizer){
  if (finalizer){
    UARTCharPut(UART0_BASE, '\n');
    UARTCharPut(UART0_BASE, '\r');
  }
}

//envia um unico comando
void sendCommand(char comm, bool finalizer){
  UARTCharPut(UART0_BASE, comm);
  commandFinalizer(finalizer);
}

//envia varios comandos
void sendSingleCommand(char elevator, char comm, bool finalizer){
  sendCommand(elevator, false);
  sendCommand(comm, false);
  commandFinalizer(finalizer);
}

//Verifica o status do Led ON ou OFF
void ledStatus(char elevator, char floor, int turnon){
  UINT status = tx_mutex_get(&mutex, TX_WAIT_FOREVER);
  if (status != TX_SUCCESS)
    return;

  UARTCharPut(UART0_BASE, elevator);
  if(turnon == 1)
    UARTCharPut(UART0_BASE, 'L');
  else
    UARTCharPut(UART0_BASE, 'D');
	UARTCharPut(UART0_BASE, floor);
	UARTCharPut(UART0_BASE, '\r');

  status = tx_mutex_put(&mutex);

  if (status != TX_SUCCESS)
    return;
}

//Liga e desliga botão
void switchButton(char elevator, char floor, bool turnon){
  if (!sendGetMutexAndcheckStatus()) return;
  sendSingleCommand(elevator, (turnon) ? 'L' : 'D', false);
  sendCommand(floor, false);
  sendCommand('\r', false);
  if (!sendPutMutexAndcheckStatus()) return;
}

//Desliga todos os botões do elevador
void turnOffAllButtons(char elevator){
  char floors[] = "abcdefghijklmnop";
  for (int i=0; i<strlen(floors); i++){
    switchButton(elevator, floors[i], false);
    tx_thread_sleep(2);
  }
}

//Inicializa o elevador
void initElevator(char elevator){
  if (!sendGetMutexAndcheckStatus()) return;
  sendSingleCommand(elevator, 'r', false);
  sendCommand('\r', false);
  if (!sendPutMutexAndcheckStatus()) return;
  if (!sendGetMutexAndcheckStatus()) return;
  sendSingleCommand(elevator, 'f', true);
  if (!sendPutMutexAndcheckStatus()) return;
  turnOffAllButtons(elevator);
}

//Abre e fecha as portas
void doorStatus(char elevator){
  if (!sendGetMutexAndcheckStatus()) return;
  sendSingleCommand(elevator, 'a', true);
  if (!sendPutMutexAndcheckStatus()) return;
  tx_thread_sleep(1000);
  if (!sendGetMutexAndcheckStatus()) return;
  sendSingleCommand(elevator, 'f', true);
  if (!sendPutMutexAndcheckStatus()) return;
}

//Movimentação do elevador
bool moveOrStopElevator(char elevator, int movement){
  bool ret = true;
  if (!sendGetMutexAndcheckStatus()) return false;
  if (movement == MOVE_UP)
    sendSingleCommand(elevator, 's', true);
  else if (movement == MOVE_DOWN)
    sendSingleCommand(elevator, 'd', true);
  else {
    sendSingleCommand(elevator, 'p', true);
    ret = false;
  }
  if (!sendPutMutexAndcheckStatus()) return false;
  return ret;
}

//Converte char para string
char * convertCharToString(char c){
    char str1[2] = {c , '\0'};
    char *str = malloc(5);
    strcpy(str,str1);
    return str;
}

//Busca o index da string
int indexOf(char * src, char search){
    char * found = strstr( src, convertCharToString(search) );
    int index = -1;
    if (found != NULL)
      index = found - src;
	return index;
}

//Converte char para int
int charToInt(char value){
    if (isdigit(value)){
        char numbersChar[] = "0123456789";
        int numbers[] = {0,1,2,3,4,5,6,7,8,9};
        int pos = indexOf(numbersChar, value); 
        if (pos >= 0) return numbers[pos];
    }
    return -1;
}

//Retorna o andar correspondente
char getFloor(char command1, char command2){
  if (!isdigit(command1) || !isdigit(command2)) return ' ';
  int ret1 = (int) charToInt(command1);
  int ret2 = (int) charToInt(command2);
  int floor =  (ret1*10) + ret2;
  if (floor < 0 ) return ' ';
  char floors[] = "abcdefghijklmnop";
  if (floor >= 0 && floor <= 15) return floors[floor];
  return ' ';
}

//Verifica se o elevador chegou no andar desejado
bool checkIfCanStopElevator(char elevator, char currentFloor, char targetFloor){
  if (targetFloor == currentFloor){
    moveOrStopElevator(elevator, STOP_MOVE);
    doorStatus(elevator);
    switchButton(elevator, targetFloor, false);
    return true;
  }
  return false;
}

//Verifica o próximo andar
char checkFloor(char command1, char command2){
  if (command1 >= 48 && command1 <= 57){ // 0 - 9
    if (command2 >= 48 && command2 <= 57) // 0 - 9
      return getFloor(command1, command2);
    else
      return getFloor('0', command1);
  }
  return ' ';
}

void mainThread(ULONG msg){
	
  char bufferRequest[16];
  int pos = 0;
  char aux;
  bool processed = false;
  while (1) {
    while (UARTCharsAvail(UART0_BASE)){
      tx_thread_sleep(2);
      aux = UARTCharGet(UART0_BASE);
      if (aux != '\n' && aux != '\r'){
        bufferRequest[pos] = aux;
        pos++;
        if (aux == 'F'){
          memset(bufferRequest, 0, sizeof bufferRequest);
          pos = 0;
        }
      }
      else {
        pos = 0;
        processed = true;
      }
    }

    if (processed){
      if (bufferRequest[0] == ELEVATOR_TYPE_1)
        if ((UINT) tx_queue_send(&queueElevator1, bufferRequest, TX_WAIT_FOREVER) != TX_SUCCESS) break;
      if (bufferRequest[0] == ELEVATOR_TYPE_2)
        if ((UINT) tx_queue_send(&queueElevator2, bufferRequest, TX_WAIT_FOREVER) != TX_SUCCESS) break;
      if (bufferRequest[0] == ELEVATOR_TYPE_3)
        if ((UINT) tx_queue_send(&queueElevator3, bufferRequest, TX_WAIT_FOREVER) != TX_SUCCESS) break;

      memset(bufferRequest, 0, sizeof bufferRequest);
      processed = false;
    }
  }
}

//Elevador esquerda
void elevator1(ULONG elevator){
	
  char elevator_type = ELEVATOR_TYPE_1;
  char floor = 'a';
  char targetFloor = 'a';
  char command[16];
  char lastCommand[16];
  bool inMovement = false;
  bool buttonPressed = false;
  
  initElevator(elevator_type);

  while (1){
    memset(command, 0, sizeof command);
    if ((UINT) tx_queue_receive(&queueElevator1, command, TX_WAIT_FOREVER) != TX_SUCCESS) break;

   
    char retFloor = checkFloor(command[1], command[2]);
    if(retFloor != ' ') floor = retFloor;

    
    if (command[1] == EXTERNAL_SIGN || command[1] == INTERNAL_SIGN){
      if(command[1] == INTERNAL_SIGN) switchButton(elevator_type, command[2], true);
      buttonPressed = true;
      memset(lastCommand, 0, sizeof lastCommand);
      if ((UINT) tx_queue_send(&internalQueue1, command, TX_WAIT_FOREVER) != TX_SUCCESS) break;
    }
    
    if (buttonPressed && strlen(lastCommand) == 0) 
      if ((UINT) tx_queue_receive(&internalQueue1, lastCommand, TX_WAIT_FOREVER) != TX_SUCCESS) break;
    
    if (lastCommand[1] == EXTERNAL_SIGN) 
      targetFloor = getFloor(lastCommand[2], lastCommand[3]);
    else if (lastCommand[1] == INTERNAL_SIGN)
      targetFloor = lastCommand[2];
    
    if (targetFloor > floor && !inMovement) 
      inMovement = moveOrStopElevator(elevator_type, MOVE_UP);
    else if (targetFloor < floor && !inMovement) 
      inMovement = moveOrStopElevator(elevator_type, MOVE_DOWN);
    
    if (strlen(lastCommand) != 0 && checkIfCanStopElevator(elevator_type, floor, targetFloor)){ 
      switchButton(elevator_type, targetFloor, false);
      buttonPressed = false;
      inMovement = false;
      memset(lastCommand, 0, sizeof lastCommand);
    }
  }
}

//Elevador central
void elevator2(ULONG elevator){
	
  char elevator_type = ELEVATOR_TYPE_2;
  char floor = 'a';
  char targetFloor = 'a';
  char command[16];
  char lastCommand[16];
  bool inMovement = false;
  bool buttonPressed = false;
  
  initElevator(elevator_type);

  while (1){
    memset(command, 0, sizeof command);
    if ((UINT) tx_queue_receive(&queueElevator2, command, TX_WAIT_FOREVER) != TX_SUCCESS) break;

    
    char retFloor = checkFloor(command[1], command[2]);
    if(retFloor != ' ') floor = retFloor;

    
    if (command[1] == EXTERNAL_SIGN || command[1] == INTERNAL_SIGN){
      if(command[1] == INTERNAL_SIGN) switchButton(elevator_type, command[2], true);
      buttonPressed = true;
      memset(lastCommand, 0, sizeof lastCommand);
      if ((UINT) tx_queue_send(&internalQueue2, command, TX_WAIT_FOREVER) != TX_SUCCESS) break;
    }
    
    if (buttonPressed && strlen(lastCommand) == 0) 
      if ((UINT) tx_queue_receive(&internalQueue2, lastCommand, TX_WAIT_FOREVER) != TX_SUCCESS) break;
    
    if (lastCommand[1] == EXTERNAL_SIGN) 
      targetFloor = getFloor(lastCommand[2], lastCommand[3]);
    else if (lastCommand[1] == INTERNAL_SIGN)
      targetFloor = lastCommand[2];
    
    if (targetFloor > floor && !inMovement) 
      inMovement = moveOrStopElevator(elevator_type, MOVE_UP);
    else if (targetFloor < floor && !inMovement) 
      inMovement = moveOrStopElevator(elevator_type, MOVE_DOWN);
    
    if (strlen(lastCommand) != 0 && checkIfCanStopElevator(elevator_type, floor, targetFloor)){ 
      switchButton(elevator_type, targetFloor, false);
      buttonPressed = false;
      inMovement = false;
      memset(lastCommand, 0, sizeof lastCommand);
    }
  }
}

//Elevador direita
void elevator3(ULONG elevator){
  char elevator_type = ELEVATOR_TYPE_3;
  char floor = 'a';
  char targetFloor = 'a';
  char command[16];
  char lastCommand[16];
  bool inMovement = false;
  bool buttonPressed = false;
  
  initElevator(elevator_type);

  while (1){
    memset(command, 0, sizeof command);
    if ((UINT) tx_queue_receive(&queueElevator3, command, TX_WAIT_FOREVER) != TX_SUCCESS) break;

    
    char retFloor = checkFloor(command[1], command[2]);
    if(retFloor != ' ') floor = retFloor;

    
    if (command[1] == EXTERNAL_SIGN || command[1] == INTERNAL_SIGN){
      if(command[1] == INTERNAL_SIGN) switchButton(elevator_type, command[2], true);
      buttonPressed = true;
      memset(lastCommand, 0, sizeof lastCommand);
      if ((UINT) tx_queue_send(&internalQueue3, command, TX_WAIT_FOREVER) != TX_SUCCESS) break;
    }
    
    if (buttonPressed && strlen(lastCommand) == 0) 
      if ((UINT) tx_queue_receive(&internalQueue3, lastCommand, TX_WAIT_FOREVER) != TX_SUCCESS) break;
    
    if (lastCommand[1] == EXTERNAL_SIGN)
      targetFloor = getFloor(lastCommand[2], lastCommand[3]);
    else if (lastCommand[1] == INTERNAL_SIGN)
      targetFloor = lastCommand[2];
    
    if (targetFloor > floor && !inMovement) 
      inMovement = moveOrStopElevator(elevator_type, MOVE_UP);
    else if (targetFloor < floor && !inMovement) 
      inMovement = moveOrStopElevator(elevator_type, MOVE_DOWN);
    
    if (strlen(lastCommand) != 0 && checkIfCanStopElevator(elevator_type, floor, targetFloor)){
      switchButton(elevator_type, targetFloor, false);
      buttonPressed = false;
      inMovement = false;
      memset(lastCommand, 0, sizeof lastCommand);
    }
  }
}


void tx_application_define(void *first_unused_memory){
  CHAR *pointer;

#ifdef TX_ENABLE_EVENT_TRACE
  tx_trace_enable(trace_buffer, sizeof(trace_buffer), 32);
#endif

  tx_mutex_create(&mutex, "mutex", TX_NO_INHERIT);
  tx_byte_pool_create(&bytePool, "byte pool", bytePoolMemory, ELEVATOR_BYTE_POOL_SIZE);

  tx_byte_allocate(&bytePool, (VOID **)&pointer, ELEVATOR_STACK_SIZE, TX_NO_WAIT);
  tx_thread_create(&threadMain, "Main Thread" , mainThread, 1, pointer, ELEVATOR_STACK_SIZE, 0, 0, 20, TX_AUTO_START);
  
  tx_byte_allocate(&bytePool, (VOID **)&pointer, ELEVATOR_STACK_SIZE, TX_NO_WAIT);
  tx_thread_create(&threadElevator1, "Elevator Thread 1", elevator1, 2, pointer , ELEVATOR_STACK_SIZE, 0, 0, 20, TX_AUTO_START);
  
  tx_byte_allocate(&bytePool, (VOID **)&pointer, ELEVATOR_STACK_SIZE, TX_NO_WAIT);
  tx_thread_create(&threadElevator2, "Elevator Thread 2", elevator2, 3, pointer , ELEVATOR_STACK_SIZE, 0, 0, 20, TX_AUTO_START);
  
  tx_byte_allocate(&bytePool, (VOID **)&pointer, ELEVATOR_STACK_SIZE, TX_NO_WAIT);
  tx_thread_create(&threadElevator3, "Elevator Thread 3", elevator3, 4, pointer , ELEVATOR_STACK_SIZE, 0, 0, 20, TX_AUTO_START);
  
  tx_byte_allocate(&bytePool, (VOID **)&pointer, ELEVATOR_QUEUE_SIZE * sizeof(ULONG), TX_NO_WAIT);
  tx_queue_create(&queueElevator1, "Elevator Queue 1", TX_1_ULONG, pointer, ELEVATOR_QUEUE_SIZE * sizeof(ULONG));
  
  tx_byte_allocate(&bytePool, (VOID **)&pointer, ELEVATOR_QUEUE_SIZE * sizeof(ULONG), TX_NO_WAIT);
  tx_queue_create(&queueElevator2, "Elevator Queue 2", TX_1_ULONG, pointer, ELEVATOR_QUEUE_SIZE * sizeof(ULONG));
  
  tx_byte_allocate(&bytePool, (VOID **)&pointer, ELEVATOR_QUEUE_SIZE * sizeof(ULONG), TX_NO_WAIT);
  tx_queue_create(&queueElevator3, "Elevator Queue 3", TX_1_ULONG, pointer, ELEVATOR_QUEUE_SIZE * sizeof(ULONG));
  
  tx_byte_allocate(&bytePool, (VOID **)&pointer, ELEVATOR_QUEUE_SIZE * sizeof(ULONG), TX_NO_WAIT);
  tx_queue_create(&internalQueue1, "Internal Queue 1", TX_1_ULONG, pointer, ELEVATOR_QUEUE_SIZE * sizeof(ULONG));
  
  tx_byte_allocate(&bytePool, (VOID **)&pointer, ELEVATOR_QUEUE_SIZE * sizeof(ULONG), TX_NO_WAIT);
  tx_queue_create(&internalQueue2, "Internal Queue 2", TX_1_ULONG, pointer, ELEVATOR_QUEUE_SIZE * sizeof(ULONG));
  
  tx_byte_allocate(&bytePool, (VOID **)&pointer, ELEVATOR_QUEUE_SIZE * sizeof(ULONG), TX_NO_WAIT);
  tx_queue_create(&internalQueue3, "Internal Queue 3", TX_1_ULONG, pointer, ELEVATOR_QUEUE_SIZE * sizeof(ULONG));
 
  tx_byte_allocate(&bytePool, (VOID **)&pointer, ELEVATOR_BLOCK_POOL_SIZE, TX_NO_WAIT);
  tx_block_pool_create(&block_pool_0, "block pool 0", sizeof(ULONG), pointer, ELEVATOR_BLOCK_POOL_SIZE);
  tx_block_allocate(&block_pool_0, (VOID **)&pointer, TX_NO_WAIT);

  tx_block_release(pointer);
}