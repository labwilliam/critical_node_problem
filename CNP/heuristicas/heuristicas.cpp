// ./configure
// make
// make check
// make install
// ldconfig

//g++ -std=c++11 heuristicas.cpp -I/usr/local/include/igraph -L/usr/local/include/lib -ligraph

#include <math.h>
#include <vector>
#include <climits>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdio.h>
#include <igraph.h>
#include <iostream>
#include <algorithm>

using namespace std;

//Struct para armazenar informações sobre o grafo
struct infoGrafo {
    int tam;
    int rem;
    int *grau;
    int arestas;
    int **matAdj;
};

//Função para remover no do grafo e atualizar o grau
void removeNo(struct infoGrafo *grafo, int no){
	
	//Remove no do grafo
	for(int i = 0; i < grafo->tam; i++){
		
		//Atualiza o grau dos nós
		if(grafo->matAdj[no][i] == 1){
			grafo->grau[no]--;
			grafo->grau[i]--;
		}
		
		//Tira nó do grafo
		grafo->matAdj[no][i] = 0;
		grafo->matAdj[i][no] = 0;
	}
}

//Funcão para gerar um grafo aleatoriamente, contando o grau e as arestas
void geraGrafo (struct infoGrafo *grafo){
	
    //Declarações
    int tArestas = 0;
    int aresta;
    
    //Gerando números aleatórios com base no tempo
    srand((unsigned)time(NULL));
    
    //Gerando grafo com arestas aleatórias
    for(int i = 0; i < grafo->tam; i++){
    	for(int j = i+1; j < grafo->tam; j++){
    		//Valor aleatorio para aresta, 0 não existe e 1 existe 
    		aresta = rand() % 2;
    		
    		//Marcando a matriz de maneira simétrica
    		grafo->matAdj[i][j] = aresta;
    		grafo->matAdj[j][i] = aresta;

			//Conta o grau de cada nó
			grafo->grau[i] += aresta;
			grafo->grau[j] += aresta;
			
			//Conta as arestas
			if(aresta == 1){
			    tArestas += 2;
			}
    	}
    }
    
    //Quantidade de arestas do grafo
    grafo->arestas = tArestas;
}

//Funcão para salvar o grafo, arestas e nós para remover
void salvaGrafo (struct infoGrafo *grafo){
    
    ofstream arquivo;
    
    arquivo.open("grafo.txt");
    
    arquivo << grafo->tam << "," << grafo->rem << "," << grafo->arestas << endl;
    
    for(int i = 0; i < grafo->tam; i++){
	arquivo << i+1 << ":";
    	for(int j = 0; j < grafo->tam; j++){
    		if(grafo->matAdj[i][j] == 1){
    			arquivo << j+1 << ",";
    		}
    	}
	arquivo << endl;
    }
}

//Funcão para imprimir o grau dos nós e o grafo
void imprimeGrafo (struct infoGrafo *grafo){
	
	//Grau dos nós
    cout << "Grau" << endl;
    for(int i = 0; i < grafo->tam; i++){
    	cout << grafo->grau[i] << " ";
    }
    cout << endl;
    
    //Grafo gerado
    cout << "Grafo" << endl;
    for(int i = 0; i < grafo->tam; i++){
    	for(int j = 0; j < grafo->tam; j++){
    		cout << grafo->matAdj[i][j] << " ";
    	}
    	cout << endl;
    }
    cout << endl;
}

//Função para imprimir as arestas para testar no Gephi
void imprimeGephi(struct infoGrafo *grafo){
	
	//Impressão das arestas no formato de entrada do Gephi
	cout << "Source;Target;Type;Weight;" << endl;
    for(int i = 0; i < grafo->tam; i++){
    	for(int j = i+1; j < grafo->tam; j++){
    		if(grafo->matAdj[i][j] == 1){
    			//cout << (char)('A' + i) << ";" << (char)('A' + j) << ";Undirected;1;" << endl;
			cout << i << ";" << j << ";Undirected;1;" << endl;
		}
    	}
    }
	cout << endl;
}

//Função para clonar o vetor com o grau dos nós
void clonaGrau (int tam, int *A, int *B){
	
	//Copia o vetor B para o vetor A
	for (int i = 0; i < tam; i++) {
		memcpy((void *) A, (void *) B, tam * sizeof(int));
	}
}

//Função para clonar o grafo
void clonaGrafo (int tam, int **A, int **B){
	
	//Copia os vetores de B para os vetores de A
	for (int i = 0; i < tam; i++) {
		memcpy((void *) A[i], (void *) B[i], tam * sizeof(int));
	}
}

//Heurística 1: Remoção dos vértices de maior betweenness (igraph)
vector<float> heuristica1(struct infoGrafo *grafo){
	
	//Declarações
	vector<float> betweenness;
	igraph_t graph;
	igraph_vector_t res;
	igraph_matrix_t adjmatrix;
	
    	//Inicializando
	igraph_vector_init(&res, grafo->tam);
	igraph_matrix_init(&adjmatrix, grafo->tam, grafo->tam);
	
	for(int i = 0; i < grafo->tam; i++){
		for(int j = 0; j < grafo->tam; j++){
			igraph_matrix_set(&adjmatrix, i, j, (long int) grafo->matAdj[i][j]);
		}
	}

	//Criando igraph
	igraph_adjacency(&graph, &adjmatrix, IGRAPH_ADJ_UNDIRECTED);
	

    	//Calculando betweenness
	igraph_betweenness(&graph, &res, igraph_vss_all(), 0, 0, 1);

	for(int j = 0; j < grafo->tam; j++){
		betweenness.push_back(igraph_vector_e(&res, j));
	}
	
	igraph_vector_destroy(&res);
	igraph_matrix_destroy(&adjmatrix);

	return betweenness;
}

//Heurística 2: Encontrar os vertices aos quais os vertices que tem menor grau estão ligados
vector<int> heuristica2(struct infoGrafo *grafo){
	
	//Declarações
	int no;
	int grauMin;
	int grauMax;
	vector<int> freq;

	//Inicializando
	grauMin = INT_MAX;
	for(int i = 0; i < grafo->tam; i++){
		if(grafo->grau[i] > 0 && grafo->grau[i] < grauMin){
			grauMin = grafo->grau[i]; //menor grau do grafo e maior que zero
		}
	}
	freq.assign(grafo->tam, 0);

	//Fazendo a contagem da frequência das ligações dos nós de grau menor
	for(int i = 0; i < grafo->tam; i++){
		if(grafo->grau[i] == grauMin){
			for(int j = 0; j < grafo->tam; j++){
				freq[j] = freq[j] + grafo->matAdj[i][j]; //como a matriz de adjacência é binária, podemos somar direto
			}
		}
	}
	
	return freq;
}

//Heurística 3: Remoção dos vértices de maior closeness (igraph)
vector<float> heuristica3(struct infoGrafo *grafo){
	
	//Declarações
	vector<float> closeness;
	igraph_t graph;
	igraph_vector_t res;
	igraph_matrix_t adjmatrix;
	
    	//Inicializando
	igraph_vector_init(&res, grafo->tam);
	igraph_matrix_init(&adjmatrix, grafo->tam, grafo->tam);
	
	for(int i = 0; i < grafo->tam; i++){
		for(int j = 0; j < grafo->tam; j++){
			igraph_matrix_set(&adjmatrix, i, j, (long int) grafo->matAdj[i][j]);
		}
	}

	//Criando igraph
	igraph_adjacency(&graph, &adjmatrix, IGRAPH_ADJ_UNDIRECTED);
	

    	//Calculando closeness
	igraph_closeness(&graph, &res, igraph_vss_all(), IGRAPH_ALL, 0, 1);

	for(int j = 0; j < grafo->tam; j++){
		closeness.push_back(igraph_vector_e(&res, j));
	}
	
	igraph_vector_destroy(&res);
	igraph_matrix_destroy(&adjmatrix);

	return closeness;
}

//Heurística 4: Remoção dos vértices de maior eigenvector (igraph)
vector<float> heuristica4(struct infoGrafo *grafo){
	
	//Declarações
	vector<float> eigenvector;
	igraph_t graph;
	igraph_vector_t res;
	igraph_matrix_t adjmatrix;
	igraph_arpack_options_t options;
	
    	//Inicializando
	igraph_vector_init(&res, grafo->tam);
	igraph_matrix_init(&adjmatrix, grafo->tam, grafo->tam);
	igraph_arpack_options_init(&options);
	
	for(int i = 0; i < grafo->tam; i++){
		for(int j = 0; j < grafo->tam; j++){
			igraph_matrix_set(&adjmatrix, i, j, (long int) grafo->matAdj[i][j]);
		}
	}

	//Criando igraph
	igraph_adjacency(&graph, &adjmatrix, IGRAPH_ADJ_UNDIRECTED);
	

	//Calculando eigenvector
	igraph_eigenvector_centrality(&graph, &res, 0, 0, 1, 0, &options);

	for(int j = 0; j < grafo->tam; j++){
		eigenvector.push_back(igraph_vector_e(&res, j));
	}
	
	igraph_vector_destroy(&res);
	igraph_matrix_destroy(&adjmatrix);

	return eigenvector;
}

//Heurística 5: Remoção dos vértices de maior Kleinberg Hub Score(igraph)
vector<float> heuristica5(struct infoGrafo *grafo){
	
	//Declarações
	vector<float> kleinbergHub;
	igraph_t graph;
	igraph_vector_t res;
	igraph_matrix_t adjmatrix;
	igraph_arpack_options_t options;
	
	//Inicializando
	igraph_vector_init(&res, grafo->tam);
	igraph_matrix_init(&adjmatrix, grafo->tam, grafo->tam);
	igraph_arpack_options_init(&options);
	
	for(int i = 0; i < grafo->tam; i++){
		for(int j = 0; j < grafo->tam; j++){
			igraph_matrix_set(&adjmatrix, i, j, (long int) grafo->matAdj[i][j]);
		}
	}

	//Criando igraph
	igraph_adjacency(&graph, &adjmatrix, IGRAPH_ADJ_UNDIRECTED);
	

	//Calculando kleinberg hub score
	igraph_hub_score(&graph, &res, 0, 1, 0, &options);

	for(int j = 0; j < grafo->tam; j++){
		kleinbergHub.push_back(igraph_vector_e(&res, j));
	}
	
	igraph_vector_destroy(&res);
	igraph_matrix_destroy(&adjmatrix);

	return kleinbergHub;
}

//Heurística 6: Remoção dos vértices de maior Kleinberg Authority Score(igraph)
vector<float> heuristica6(struct infoGrafo *grafo){
	
	//Declarações
	vector<float> kleinbergScore;
	igraph_t graph;
	igraph_vector_t res;
	igraph_matrix_t adjmatrix;
	igraph_arpack_options_t options;
	
	//Inicializando
	igraph_vector_init(&res, grafo->tam);
	igraph_matrix_init(&adjmatrix, grafo->tam, grafo->tam);
	igraph_arpack_options_init(&options);
	
	for(int i = 0; i < grafo->tam; i++){
		for(int j = 0; j < grafo->tam; j++){
			igraph_matrix_set(&adjmatrix, i, j, (long int) grafo->matAdj[i][j]);
		}
	}

	//Criando igraph
	igraph_adjacency(&graph, &adjmatrix, IGRAPH_ADJ_UNDIRECTED);
	

	//Calculando kleinberg hub score
	igraph_authority_score(&graph, &res, 0, 1, 0, &options);

	for(int j = 0; j < grafo->tam; j++){
		kleinbergScore.push_back(igraph_vector_e(&res, j));
	}
	
	igraph_vector_destroy(&res);
	igraph_matrix_destroy(&adjmatrix);

	return kleinbergScore;
}

void inicializa(int tam, int **grau, int ***adj){
	
	//Criando o vetor para guardar o grau de cada nó
	(*grau) = (int *) malloc (tam * sizeof(int));
	
	//Criando a matriz de adjacência, a copia da matriz de adjacência e zerando a diagonal principal
	(*adj) = (int **) malloc (tam * sizeof(int *));
	for (int i = 0; i < tam; i++) {
		(*adj)[i] = (int *) malloc (tam * sizeof(int));
		(*adj)[i][i] = 0;
		(*grau)[i] = 0;
	}
}

void finaliza(struct infoGrafo *grafo, int *Cgrau, int **Cadj){
    
	//Liberando memória do vetor grau
	free(grafo->grau);
	free(Cgrau);
    
	//Liberando memória da matriz de adjacência
	int* currentIntPtr;
	for (int i = 0; i < grafo->tam; i++)
	{
		currentIntPtr = grafo->matAdj[i];
		free(currentIntPtr);
        
		currentIntPtr = Cadj[i];
		free(currentIntPtr);
	}
}

void criarGrafo(struct infoGrafo *grafo){
	
	//Leitura do tamanho da matriz de adjacência (quantidade de nós do grafo)
	cout << "Digite a quantidade de nós do grafo" << endl;
	cin >> grafo->tam;
    
	cout << "Digite a quantidade de nós para remover do grafo" << endl;
	cin >> grafo->rem;
    
	inicializa(grafo->tam, &grafo->grau, &grafo->matAdj);
    
	//Gerando grafo
	geraGrafo(grafo);
}

void clonar(struct infoGrafo *grafo, int *Cgrau, int **Cadj){

	//Clona grau para ser alterado
	clonaGrau(grafo->tam, grafo->grau, Cgrau);
    
	//Clona grafo para ser alterado
    	clonaGrafo(grafo->tam, grafo->matAdj, Cadj);
}

//Funcão para ler o grafo, arestas e nós para remover
void lerGrafo (struct infoGrafo *grafo){

	grafo->tam = 12;
	grafo->rem = 1;

	inicializa(grafo->tam, &grafo->grau, &grafo->matAdj);
	
	grafo->matAdj[0][2] = grafo->matAdj[2][0] = 1;
	grafo->matAdj[1][2] = grafo->matAdj[2][1] = 1;
	grafo->matAdj[2][3] = grafo->matAdj[3][2] = 1;
	grafo->matAdj[3][4] = grafo->matAdj[4][3] = 1;
	grafo->matAdj[4][5] = grafo->matAdj[5][4] = 1;
	grafo->matAdj[4][6] = grafo->matAdj[6][4] = 1;
	grafo->matAdj[6][7] = grafo->matAdj[7][6] = 1;
	grafo->matAdj[6][8] = grafo->matAdj[8][6] = 1;
	grafo->matAdj[6][9] = grafo->matAdj[9][6] = 1;
	grafo->matAdj[7][8] = grafo->matAdj[8][7] = 1;
	grafo->matAdj[8][9] = grafo->matAdj[9][8] = 1;
	grafo->matAdj[2][10] = grafo->matAdj[10][2] = 1;
	grafo->matAdj[2][11] = grafo->matAdj[11][2] = 1;
	grafo->matAdj[11][10] = grafo->matAdj[10][11] = 1;
	


	grafo->grau[0] = 1;
	grafo->grau[1] = 1;
	grafo->grau[2] = 5;
	grafo->grau[3] = 2;
	grafo->grau[4] = 3;
	grafo->grau[5] = 1;
	grafo->grau[6] = 4;
	grafo->grau[7] = 2;
	grafo->grau[8] = 3;
	grafo->grau[9] = 2;
	grafo->grau[10] = 2;
	grafo->grau[11] = 2;

	grafo->arestas = 28;
	
}

void dfsVisit(struct infoGrafo *grafo, int *vertices, int no){
	
	vertices[no] == 1;
	for(int i = 0; i < grafo->tam; i++){
		if(grafo->matAdj[no][i] == 1 && vertices[i] == 0){
			vertices[i] = 1;
			dfsVisit(grafo, vertices, i);
		}
	}
}

int dfs(struct infoGrafo *grafo){
	
	//Declarações
	int componentes;
	int *vertices = (int *) malloc (grafo->tam * sizeof(int));

	componentes = 0;

	for(int i = 0; i < grafo->tam; i++){	
		vertices[i] = 0;
	}

	for(int i = 0; i < grafo->tam; i++){
		if(vertices[i] == 0){
			componentes++;
			dfsVisit(grafo, vertices, i);
		}
	}

	return componentes;
}

int main(int argc, char **argv) {
	
	//Declarações
	int no;
	int op;
	int *Cgrau;
	int **Cadj;
	struct infoGrafo grafo;
	vector<int> respostasI;
	vector<float> respostasF;
	int digitos;
	
	//Ler um grafo ou gerar um
	cout << "Digite: 0 para ler um grafo ou 1 para gerar um aleatóriamente: " << endl;
	cin >> op;
    
	if(op){
		criarGrafo(&grafo);
	}
	else{
		lerGrafo(&grafo);
		imprimeGephi(&grafo);
	}

	//Salva grafo
	salvaGrafo(&grafo);
    
	digitos = ceil(log10(grafo.tam));
	//imprimeGrafo(&grafo);
    
	//Cria cópia do vetor com os graus e da matriz de adjacência
	inicializa(grafo.tam, &Cgrau, &Cadj);
	clonaGrau(grafo.tam, Cgrau, grafo.grau);
	clonaGrafo(grafo.tam, Cadj, grafo.matAdj);
    
	//Tempo inicial
	const clock_t begin_time = clock();

	//Execução das Heurísticas:
    
	//Retorna os nós em um único cálculo de betweenness
	cout << "heuristica 1 (betweenness): ";
	for(int i = 0; i < grafo.rem; i++){
		respostasF = heuristica1(&grafo);
		no = distance(begin(respostasF), (max_element(begin(respostasF), end(respostasF))));
		removeNo(&grafo, no);
		cout << setw(digitos) << setfill('0') << no << " ";
	}
	cout << endl;

	cout << "componentes: " << dfs(&grafo) - grafo.rem << endl << endl;

	clonar(&grafo, Cgrau, Cadj);
		
	//Retorna os nós em um único cálculo de frequência
    	cout << "heuristica 2 (frequencia):  ";
	for(int i = 0; i < grafo.rem; i++){
		respostasI = heuristica2(&grafo);
		no = distance(begin(respostasI), (max_element(begin(respostasI), end(respostasI))));
		removeNo(&grafo, no);
		cout << setw(digitos) << setfill('0') << no << " ";
	}
	cout << endl;

	cout << "componentes: " << dfs(&grafo) - grafo.rem << endl << endl;
	
	clonar(&grafo, Cgrau, Cadj);
	
	//Retorna os nós em um único cálculo de closeness (igraph)
    	cout << "heuristica 3 (closeness):   ";
	for(int i = 0; i < grafo.rem; i++){
		respostasF = heuristica3(&grafo);
		no = distance(begin(respostasF), (max_element(begin(respostasF), end(respostasF))));
		respostasF[no] = 0;
		removeNo(&grafo, no);
		cout << setw(digitos) << setfill('0') << no << " ";
	}
	cout << endl;

	cout << "componentes: " << dfs(&grafo) - grafo.rem << endl << endl;
	
	clonar(&grafo, Cgrau, Cadj);

	//Retorna os nós em um único cálculo de betweenness (igraph)
    	cout << "heuristica 4 (eigenvector): ";
	for(int i = 0; i < grafo.rem; i++){
		respostasF = heuristica4(&grafo);
		no = distance(begin(respostasF), (max_element(begin(respostasF), end(respostasF))));
		respostasF[no] = 0;
		removeNo(&grafo, no);
		cout << setw(digitos) << setfill('0') << no << " ";
	}
	cout << endl;

	cout << "componentes: " << dfs(&grafo) - grafo.rem << endl << endl;
	
	clonar(&grafo, Cgrau, Cadj);

	//Retorna os nós em um único cálculo de kleinberg hub (igraph)
    	/*cout << "heuristica 5 (kleinberg h): ";
	respostasF = heuristica5(&grafo);
	for(int i = 0; i < grafo.rem; i++){
		no = distance(begin(respostasF), (max_element(begin(respostasF), end(respostasF))));
		respostasF[no] = 0;
		removeNo(&grafo, no);
		//cout << setw(digitos) << setfill('0') << no << " ";
	}
	cout << endl;

	cout << "componentes: " << dfs(&grafo) - grafo.rem << endl;
	
	clonar(&grafo, Cgrau, Cadj);

	//Retorna os nós em um único cálculo de kleinberg hub (igraph)
    	cout << "heuristica 6 (kleinberg a): ";
	respostasF = heuristica6(&grafo);
	for(int i = 0; i < grafo.rem; i++){
		no = distance(begin(respostasF), (max_element(begin(respostasF), end(respostasF))));
		respostasF[no] = 0;
		removeNo(&grafo, no);
		//cout << setw(digitos) << setfill('0') << no << " ";
	}
	cout << endl;

	cout << "componentes: " << dfs(&grafo) - grafo.rem << endl;*/

	//Tempo final
	cout << "Tempo: " << float( clock () - begin_time ) /  CLOCKS_PER_SEC << " s" << endl;
	
	finaliza(&grafo, Cgrau, Cadj);
	respostasF.clear();
	respostasI.clear();
}
