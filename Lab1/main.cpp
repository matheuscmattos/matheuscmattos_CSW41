#include <iostream>
using namespace std;

int main()
{
	float valor1;
	float valor2;
	float soma;
        
	cout << "Lab1\n";
	cout << "Date:" << __DATE__ << endl;
	cout << "File:" << __FILE__ << endl;
	cout << "STDC:" << __STDC__ << endl;
	cout << "Line:" << __LINE__ << endl;
	cout << "Time:" << __TIME__ << endl;
	cout << "Digite um valor" << endl;
        cin >> valor1;
        cout << "Valor digitado eh:" << valor1 << endl;
        cout << "Digite outro valor" << endl;
        cin >> valor2;
        cout << "Valor digitado eh:" << valor2 << endl;
          
	soma = valor1 + valor2;
        cout << "O valor da soma eh:" << soma << endl;

	return 0;
}