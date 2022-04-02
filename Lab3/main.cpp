#include <cstdlib>
#include <iostream>
#include <stdint.h>

#include "images.c"

extern "C" uint16_t EightBitHistogram(uint16_t width, uint16_t height, uint8_t * p_image, uint16_t * p_histogram);

using namespace std;

int main()
{
  //Inicializa o vetor
  uint16_t vetor[256] = {0};
  
  //Tamanho da imagem
  //uint16_t tamanho_imagem = EightBitHistogram(width0, height0, (uint8_t*) *image0, vetor);
  uint16_t tamanho_imagem = EightBitHistogram(width1, height1, (uint8_t*) *image1, vetor);
  int contador = 0;
  
  cout << "Inicializando\n";
  cout << "Tamanho da imagem:" << tamanho_imagem << '\n';
  
  cout << "Histograma gerado: \n" ;
  
  for (int i=0;i<256;i++){
    cout << vetor[i] << ' ';
    contador ++;
  }
  
  cout << "\nQuantidade total de pixel lidos:" << contador; 
  
  return 0;
}