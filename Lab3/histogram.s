THUMB

  PUBLIC EightBitHistogram
  SECTION .text: CODE
	
Zerar
    STR R8, [R3], #2  
    ADD R7, #1        ;Incrementa o iterador
    CMP R7, #256      ;Compara com o tamanho do vetor, para verificar se chegou no fim
	
    BNE Zerar
	BX LR
	
;Criando o histograma
GerarHistograma
    MOV R3, R9         ;pega a posição inicial
    LDRB R8, [R2], #1  ;Lê o valor do pixel e acrescenta 1
    ADD R8, R8	       ;Dobra o valor para ficar de 16bit 	
    LDR R11, [R3, R8]   ;Lê o valor atual do vetor e soma 1
    ADD R11, #1
    ADD R3, R8	       ;Guarda o valor lido do pixel em R3
    STR R11, [R3]	
    ADD R7, #1
    CMP R7, R6	       ;Compara se chegou no final do vetor

    BNE GerarHistograma
    BX LR

EightBitHistogram
    MUL R6, R0, R1     ;Armazena o tamanho da imagem
    MOV R9, R3	       ;Posição inicial do vetor    
    CMP R6, #65536     ;Compara o tamanho da imagem com 64k
    ITT GT             ;Verifica se a imagem é maior que 64k
      MOVGT R0, #0
      BXGT LR          ;Retorna zero
    MOV R7, #0	       ;Chamada aux para zerar o vetor
    MOV R8, #0	       ;Chamada aux para zerar o vetor
    PUSH {LR}          ;Inserir na pilha
    BL Zerar
    POP {LR}           ;Desempilhar    
    MOV R7, #0	       ;Chamada aux para zerar o vetor
    PUSH {LR}
    BL GerarHistograma
    POP {LR}    
    MOV R0, R6	       ;Recebe o tamanho da imagem       
	BX LR
	
 END