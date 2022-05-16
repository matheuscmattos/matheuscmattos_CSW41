# Sistemas Embarcados CSW41

# LAB6

__Objetivo__

Este laboratório consiste em criar um conjunto de 3 tarefas com temporizações conhecidas. Experimentando variações de algoritmos de escalonamento disponível no ThreadX.

__Resultados__

a) Escalonamento por time-slice de 50 ms. Todas as tarefas com mesma prioridade.

O time slice significa que as threads alternam entre si a cada 50 ms, se estão prontas para executar. Caso só uma thread esteja pronta, esta executa sem interrupções. O efeito disso são os leds piscando alternadamente de maneira rápida com time-slice de 50ms.
	
b) Escalonamento sem time-slice e sem preempção. Prioridades estabelecidas no passo 3. A preempção pode ser evitada com o “preemption threshold” do ThreadX.

Nesse caso, como as threads não são interrompidas por conta da desabilitação da preempção, então a thread possuir uma prioridade maior não significa que a thread será executada assim que pronta, e sim dependendo da liberação do processador. Nesse modo, o efeito visto é que o leds piscam alternadamente entre si, mas não se interrompem, só começam a piscar quando o anterior parou.
	
c) Escalonamento preemptivo por prioridade.

As 3 threads podem se interromper. Leds com maior prioridade podem interromper a execução de qualquer tarefa. Quando a tarefa é interrompida. ela aguarda até poder voltar a executar, o que pode tornar o seu tempo de execução bem maior do que o previsto.

d) Implemente um Mutex compartilhado entre T1 e T3. No início de cada job estas tarefas devem solicitar este mutex e liberá-lo no final. Use mutex sem herança de prioridade. Observe o efeito na temporização das tarefas.

Os períodos das 3 threads podem ser alterados por conta do mutex. Isso faz com que os leds alternem entre si, e o tempo entre um led desligar e outro ligar é tambem variável.	
	
e) Idem acima, mas com herança de prioridade.

Idem ao caso acima, porém T2 pode aumentar seu período caso T3 esteja executando, pela herança de prioridade do mutex. O efeito visual é muito parecido.
