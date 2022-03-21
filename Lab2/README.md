# Sistemas Embarcados CSW41

# LAB2

__Porque a variável ui32Loop é declarada como volatile dentro de main() ?__


Variáveis volatile indicam ao compilador que a variável pode ser modificada sem o conhecimento do main. A variável ui32Loop é declarada como volatile dentro do main () para sinalizar que o compilador não deve otimizar nada dessa variável, facilitando o acesso a HW, mapeamento de memória I/O e uso de threads.