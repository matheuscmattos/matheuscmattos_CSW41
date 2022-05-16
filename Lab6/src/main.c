#include "tx_api.h"
#include <stdint.h>
#include <stdbool.h>
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"

#define DEMO_BYTE_POOL_SIZE     9120
#define DEMO_STACK_SIZE         1024

/* Define the ThreadX object control blocks...  */
TX_THREAD               thread_1;
TX_THREAD               thread_2;
TX_THREAD               thread_3;
TX_BYTE_POOL            byte_pool_0;
TX_MUTEX                mutex_0;

/* Define byte pool memory.  */
UCHAR                   byte_pool_memory[DEMO_BYTE_POOL_SIZE];


// função para apagar os leds
void turnOff(){
  GPIOPinWrite(GPIO_PORTN_BASE,(GPIO_PIN_0 | GPIO_PIN_1), (GPIO_PIN_0 | GPIO_PIN_1) & (0));
  GPIOPinWrite(GPIO_PORTF_BASE,(GPIO_PIN_0 | GPIO_PIN_4), (GPIO_PIN_0 & 0) | (GPIO_PIN_4 & (0)));
}

// função para acender os leds
void turnOn(uint8_t led){
  GPIOPinWrite(GPIO_PORTN_BASE,(GPIO_PIN_0 | GPIO_PIN_1), (GPIO_PIN_0 | GPIO_PIN_1) & (led>>2));
  GPIOPinWrite(GPIO_PORTF_BASE,(GPIO_PIN_0 | GPIO_PIN_4), (GPIO_PIN_0 & led) | (GPIO_PIN_4 & (led<<3)));
}

uint8_t sel_escalonamento(){
	//a) Escalonamento por time-slice de 50 ms. Todas as tarefas com mesma prioridade.
	//return 0;
	
	//b) Escalonamento sem time-slice e sem preempção. Prioridades estabelecidas no passo 3. A preempção pode ser evitada com o “preemption threshold” do ThreadX.
	return 1;
	
	//c) Escalonamento preemptivo por prioridade.
	// return 2;	
	
	//d) Implemente um Mutex compartilhado entre T1 e T3. No início de cada job estas tarefas devem solicitar este mutex e liberá-lo no final. Use mutex sem herança de prioridade. Observe o efeito na temporização das tarefas.
	// return 3;	
	
	//e) Idem acima, mas com herança de prioridade.
	// return 4;	
}

uint8_t loop_Delay_led(uint8_t led, uint32_t numLoops){
    for(uint32_t delay_count = 0; delay_count < numLoops; delay_count++) {
     turnOn(led);
     turnOff();  
    }
    return 0;
}

void thread_entry(ULONG thread_entrada){

//tempo medido = 1,6us
//T1 300ms - 187500
//T2 500ms - 312500
//T3 800ms - 500000

  uint32_t t1 = 187500;
  uint32_t t2 = 312500;
  uint32_t t3 = 500000;
  
  if(thread_entrada==1){
    while(1){
      //D e E
      if(sel_escalonamento()== 3 | sel_escalonamento()== 4){
        UINT status = tx_mutex_get(&mutex_0,TX_WAIT_FOREVER);
        if (status != TX_SUCCESS)
          break;
        loop_Delay_led(8, t1);
        tx_thread_sleep(500);
      
        status = tx_mutex_put(&mutex_0);
        if (status != TX_SUCCESS)
          break;
      }
      //A, B e C
      else{
        loop_Delay_led(8, t1);
        tx_thread_sleep(500);
      }

    }
  }

  else if(thread_entrada==2){
    while(1){
      loop_Delay_led(4, t2);
      tx_thread_sleep(750);
    }
  }

  else if(thread_entrada==3){
    while(1){
      //D e E
      if(sel_escalonamento()== 3 | sel_escalonamento()== 4){
        UINT status = tx_mutex_get(&mutex_0,TX_WAIT_FOREVER);
        if (status != TX_SUCCESS)
          break;
        
        loop_Delay_led(2,t3);
        tx_thread_sleep(2000);

        status = tx_mutex_put(&mutex_0);
        if (status != TX_SUCCESS)
          break;

      //A, B e C
      }else{
        loop_Delay_led(2, t3);
        tx_thread_sleep(2000);
      }

    }
  }

}

int main(){
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));

  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));

  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1 );
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4 );
  
  turnOff();
  
  tx_kernel_enter();
}

void    tx_application_define(void *first_unused_memory)
{

CHAR    *pointer = TX_NULL;


#ifdef TX_ENABLE_EVENT_TRACE
    tx_trace_enable(trace_buffer, sizeof(trace_buffer), 32);
#endif
    UINT time_slice[3];
    UINT priority[3];
    UINT preemption[3];

      //A
      if(sel_escalonamento() == 0){
        for(int i=0;i<3;i++){
          priority[i]=0;
          preemption[i]=0;
          time_slice[i]=25;
        } 
      }

      //C, D e E
      else{
        for(int i=0;i<3;i++){
           priority[i]=i;
           preemption[i]=i;
           time_slice[i] = TX_NO_TIME_SLICE;
        }
      }

    //B
    if(sel_escalonamento() == 1){
        preemption[0]=0;
        preemption[1]=0;
        preemption[2]=0;
    }
	
    //D
    if(sel_escalonamento()== 3){
      tx_mutex_create(&mutex_0, "mutex 0", TX_NO_INHERIT);
    }
    else if(sel_escalonamento()== 4){
      tx_mutex_create(&mutex_0, "mutex 0", TX_INHERIT);
    }

    /* Create a byte memory pool from which to allocate the thread stacks.  */
    tx_byte_pool_create(&byte_pool_0, "byte pool 0", byte_pool_memory, DEMO_BYTE_POOL_SIZE);

    /* Put system definition stuff in here, e.g. thread creates and other assorted
       create information.  */


    /* Allocate the stack for thread 1.  */
    tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);

    /* Create threads 1 and 2. These threads pass information through a ThreadX 
       message queue.  It is also interesting to note that these threads have a time
       slice.  */
    tx_thread_create(&thread_1, "thread 1", thread_entry, 1,  
            pointer, DEMO_STACK_SIZE, 
            priority[0], preemption[0], time_slice[0], TX_AUTO_START);

    /* Allocate the stack for thread 2.  */
    tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);

    tx_thread_create(&thread_2, "thread 2", thread_entry, 2,  
            pointer, DEMO_STACK_SIZE, 
            priority[1], preemption[1], time_slice[1], TX_AUTO_START);

    // /* Allocate the stack for thread 3.  */
    tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);

    // /* Create threads 3 and 4.  These threads compete for a ThreadX counting semaphore.  
    //    An interesting thing here is that both threads share the same instruction area.  */
    tx_thread_create(&thread_3, "thread 3", thread_entry, 3,  
            pointer, DEMO_STACK_SIZE, 
            priority[2], preemption[2], time_slice[2], TX_AUTO_START);
}