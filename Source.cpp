#include <iostream>
#include <algorithm>    // std::sort,max
#include <iterator>
#include <queue>
#include "List.h"
#include <chrono>       // tiempos 
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <fstream>


using namespace std;

const float PESOMOCHILA = 50;
const int MAXOBJETOS = 10000;

class Objeto {
public:

	float pesoAct;
	float benefAct;
	int k;//nivel del arbol
	List<int> elems; //elementos escogidos
	float cotaOpt;
	
	Objeto(float peso, float benef, int k, List<int> l, float opt);
	bool operator<(const Objeto& rhs) const
	{
		return this->cotaOpt > rhs.cotaOpt;
	}

};

Objeto::Objeto(float peso, float benef, int nivel, List<int> l, float opt) {
	pesoAct = peso;
	benefAct = benef;
	k = nivel;
	elems = l;
	cotaOpt = opt;
}

struct elem {
	float valor;
	float peso;
	float valorReal;
};

void calcularCotasBuenas(float &opt, float &pes, int k, const float &pesoAct, const float & benefAct, const elem lista[], const int & nObjetos) {
	float valor = benefAct;
	float pesoRes = PESOMOCHILA - pesoAct;
	while (pesoRes >= lista[k].peso && k < nObjetos) { // llenar con los que caben
		valor += lista[k].valor;
		pesoRes -= lista[k].peso;
		k++;
	}
	pes = valor; // los que caben
	opt = valor;
	if(k<nObjetos)
		opt += pesoRes*lista[k].valorReal; // mas licuar ultimo
}

void calcularCotasMalas(float &opt, float &pes, int k, const float &pesoAct, const float & benefAct, const elem lista[], const int & nObjetos) {
	opt = benefAct;
	pes = benefAct;
	if (k < nObjetos) { // licuo el primer objeto actual y lleno todo la mochila con el
		float pesoRes = PESOMOCHILA - pesoAct;
		opt+= lista[k].valorReal * pesoRes;
	}
}

struct CustomCompare {
	bool operator()(const Objeto& lhs, const Objeto& rhs) const {
		return lhs.cotaOpt < rhs.cotaOpt; // el objeto con mas prioridad primero
	}
};

struct solucion {
	List<int> sol;
	float ben;
};

void imprimir_sol(const solucion & mejorSol, const int &nodosExplorados, const double & tiempoTotal) {
	double media = tiempoTotal/ nodosExplorados;
	List<int>::ConstIterator it = mejorSol.sol.cbegin();
	std::cout << "La solucion esta formada por estos elementos:";
	while (it != mejorSol.sol.cend()) {
		std::cout << it.elem() << " ";
		it++;
	}
	std::cout << "y tiene un beneficio de: " << mejorSol.ben << endl;
	std::cout << "Se han explorado " << nodosExplorados << " nodos" << endl;
	std::cout << "El tiempo medio por nodo explorado es de: " << media << " ms" << endl;
	std::cout << "Tiempo hasta encontrar solucion:  " << tiempoTotal << " ms";

}

solucion mochila(const elem lista[],const int nObjetos) {
	priority_queue <Objeto, vector<Objeto>, CustomCompare> cola;
	solucion mejorSol;
	mejorSol.ben = 0;
	int nodosExplorados = 0;
	float optNueva,benefAnt,pesNueva, pesoAnt;
	List<int> l;
	Objeto nodo(0, 0, 0, l, 0); //generar raiz

	calcularCotasBuenas(optNueva, pesNueva,nodo.k, nodo.pesoAct, nodo.benefAct, lista, nObjetos);
	nodo.cotaOpt = optNueva;
	mejorSol.ben=pesNueva;
	cola.push(nodo);

	auto start = chrono::high_resolution_clock::now();

	while (cola.size() > 0 && cola.top().cotaOpt>=mejorSol.ben) {
		nodo = cola.top();
		cola.pop();
		nodosExplorados += 2;

		//calcular nodo metiendo objeto
		if (nodo.pesoAct + lista[nodo.k].peso <= PESOMOCHILA) { //si es factible
			// guardar valores antiguos y actualizar nodo
			nodo.elems.push_back(nodo.k); // se recupera haciendo pop_back
			benefAnt = nodo.benefAct;
			pesoAnt = nodo.pesoAct;

			nodo.benefAct += lista[nodo.k].valor;
			nodo.pesoAct += lista[nodo.k].peso;
			//al meter el objeto las cotas son las mismas que la de su padre, por eso no se calculan

			//si es solucion, es mejor que la actual por el while
			if (nodo.k == nObjetos-1 && nodo.benefAct > mejorSol.ben) {
				mejorSol.sol = nodo.elems;
				mejorSol.ben = nodo.benefAct;
			}
			else if(nodo.k != nObjetos - 1) {
				nodo.k++;
				cola.push(nodo);
				nodo.k--; // recuperar k para general el siguiente hijo
			}
			// recuperacion del nodo padre
			nodo.elems.pop_back();
			nodo.benefAct = benefAnt;
			nodo.pesoAct = pesoAnt;
		}
		//calcular nodo sin meter objeto
		calcularCotasBuenas(optNueva, pesNueva, nodo.k+1, nodo.pesoAct, nodo.benefAct, lista, nObjetos);
		if (optNueva >= mejorSol.ben) { //meter en la cola
			if (nodo.k == nObjetos-1) { // si es solucion
				mejorSol.sol = nodo.elems;
				mejorSol.ben = nodo.benefAct;
			}
			else {
				nodo.cotaOpt = optNueva;
				nodo.k++;
				cola.push(nodo);

				mejorSol.ben = max(mejorSol.ben, pesNueva);
			}
		}
	} // fin while
	auto stop = chrono::high_resolution_clock::now();
	chrono::duration<double, std::milli> fp_ms = stop - start;
	imprimir_sol(mejorSol, nodosExplorados, fp_ms.count());
	return mejorSol;
}

solucion mochilaSoloFactible(const elem lista[], const int nObjetos) {
	priority_queue <Objeto, vector<Objeto>, CustomCompare> cola;
	solucion mejorSol;
	mejorSol.ben = 0;
	int nodosExplorados = 0;
	float  benefAnt, pesoAnt;
	List<int> l;
	Objeto nodo(0, 0, 0, l, 0); //generar raiz

	cola.push(nodo);

	auto start = chrono::high_resolution_clock::now();

	while (cola.size() > 0) {
		nodo = cola.top();
		cola.pop();
		nodosExplorados += 2;

		//calcular nodo metiendo objeto
		if (nodo.pesoAct + lista[nodo.k].peso <= PESOMOCHILA) { //si es factible
																// guardar valores antiguos y actualizar nodo
			nodo.elems.push_back(nodo.k); // se recupera haciendo pop_back
			benefAnt = nodo.benefAct;
			pesoAnt = nodo.pesoAct;

			nodo.benefAct += lista[nodo.k].valor;
			nodo.pesoAct += lista[nodo.k].peso;
			//al meter el objeto las cotas son las mismas que la de su padre, por eso no se calculan

			//si es solucion, es mejor que la actual por el while
			if (nodo.k == nObjetos - 1 && nodo.benefAct > mejorSol.ben) {
				mejorSol.sol = nodo.elems;
				mejorSol.ben = nodo.benefAct;
			}
			else if (nodo.k != nObjetos - 1) {
				nodo.k++;
				if (nodo.k < nObjetos - 1) {
					cola.push(nodo);
					if (nodo.benefAct > mejorSol.ben) {
						mejorSol.sol = nodo.elems;
						mejorSol.ben = nodo.benefAct;
					}
				}
				nodo.k--; // recuperar k para general el siguiente hijo
			}
			// recuperacion del nodo padre
			nodo.elems.pop_back();
			nodo.benefAct = benefAnt;
			nodo.pesoAct = pesoAnt;
		}
		//calcular nodo sin meter objeto
			if (nodo.k == nObjetos - 1 && nodo.benefAct>mejorSol.ben) { // si es solucion
				mejorSol.sol = nodo.elems;
				mejorSol.ben = nodo.benefAct;
			}
			else {
				//nodo.cotaOpt = optNueva;
				nodo.k++;
				if(nodo.k < nObjetos-1)
					cola.push(nodo);

			}
		
	} // fin while
	auto stop = chrono::high_resolution_clock::now();
	chrono::duration<double, std::milli> fp_ms = stop - start;
	imprimir_sol(mejorSol, nodosExplorados, fp_ms.count());
	return mejorSol;
}


//deberia indicar el menor pero eso lo ordena de menor a mayor 
//le digo cual es mayor para que lo ordene de mayor a menor
bool elems_sorter(elem const& lhs, elem const& rhs) {
	return lhs.valorReal > rhs.valorReal;
}

void generar_datos(elem lista[], const int &nObjetos) {
	srand(time(NULL));
	const int TAM = 10000;
	for (int i = 0; i < TAM; i++) {
		lista[i].peso= rand() % 1000 + 1.000000;
		lista[i].valor = rand() % 1000 + 1.000000;
		lista[i].valorReal = lista[i].valor / lista[i].peso ;
	}
	sort(lista, lista + TAM, &elems_sorter);
	for(int i=0;i<TAM;i++)
		std::cout << lista[i].valorReal << " ";

	ofstream myfile("datos1000.txt");
	if (myfile.is_open())
	{
		myfile << TAM << endl;
		for (int i = 0; i<TAM; i++)
			myfile << lista[i].valor << " " << lista[i].peso << " " <<  lista[i].valorReal << endl;
	}
	else std::cout << "Unable to open file";
	myfile.close();
}

void leerDatos(elem lista[], int & N) {
	ifstream myfile("datos40.txt");
	if (myfile.is_open())
	{
		myfile >> N;
		for (int i = 0; i < N; i++) {
			myfile >> lista[i].valor;
			myfile >> lista[i].peso;
			myfile >> lista[i].valorReal;
		}
		myfile.close();
	}
	else std::cout << "Unable to open file";
}

int main() {
	int nObjetos;
	elem lista[MAXOBJETOS];
	leerDatos(lista,nObjetos);
	mochila(lista, nObjetos);
	//mochilaSoloFactible(lista, nObjetos);
	return 0;
}