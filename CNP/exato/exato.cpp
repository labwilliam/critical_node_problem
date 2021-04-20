#include <algorithm>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <list>
#include <iostream>
#include <climits> // para usar INT_MAX
#include <vector>
#include <string>
#include <math.h>
#include <iomanip>


using namespace std;

#define maxVert 51 // constante que define o número máximo de vértices

vector <int> escolhidos;

typedef struct
{
    list <int> Adj;	// armazena adjacências do vértice
    bool visitado; 	// indica se o vértice já foi visitado na DFS
    int p;			// indica o pai/predecessor do vértice na DFS
    int d;			// número DFS
} VertexType;

// **************************** FUNÇÕES GERAIS *******************************

// Convert string to integer
int GetStr2Int(string strConvert)
{
    int intReturn;
    intReturn = atoi(strConvert.c_str());
    return(intReturn);
}

// Convert integer to string
string GetInt2Str(int intConvert)
{
    stringstream out;
    out << intConvert;
    return(out.str());
}

// **************************** LEITURA DO ARQUIVO DE ENTRADA *******************************

// lê o arquivo de entrada e "carrega" na lista de adjacências Vet
// variável n é passada por referência (não por valor), porque será modificada
void readFileLista(string arquivo,int *n, int *k, VertexType Vet[])
{
    string line;
    short v,u,i,temp;
    ifstream myfile; // abre arquivo e aponta para próximo elemento a ser lido
    myfile.open (arquivo.c_str(),ios::in);
    if (myfile.is_open())
    {
        i=0;
        while (myfile.good())
        {
            i++;
            getline(myfile,line);
            if (i==1)
            {
                temp=line.find_first_of(",");
                *n = GetStr2Int(line.substr(0,temp)); // número de vértices do grafo, converte string para inteiro
                line = line.substr(temp+1);

                temp=line.find_first_of(",");
                *k = GetStr2Int(line.substr(0,temp));
            }
            else if (line.length()>0)  // demais vértices do grafo
            {
                temp=line.find_first_of(":");
                v=GetStr2Int(line.substr(0,temp));
                line=line.substr(temp+1);
                while (line.length()>1)
                {
                    temp=line.find_first_of(",");
                    u=GetStr2Int(line.substr(0,temp));
                    line=line.substr(temp+1);
                    Vet[v].Adj.push_back(u);
                }
            }
        }
        myfile.close();
    }
    else cout << "Unable to open file";
}

// **************************** FUNÇÕES SOBRE A LISTA DE ADJACÊNCIAS *******************************

//lê a lista de adjacências Vet e "imprime" o conteúdo para o usuário
void writeGraphLista(int n,VertexType Vet[])
{
    //int u;
    list<int>:: iterator it;
    for(int i=1; i<=n; i++)
    {
        printf("Adj[%d]: ",i); //<< " é adjacente a: ";
        for (it=Vet[i].Adj.begin(); it!=Vet[i].Adj.end(); it++)
        {
            //u=*it;
            std::cout << *it << ", ";
        }
        std::cout << "\n";
    }
}

// **************************** BUSCA EM PROFUNDIDADE *******************************

// DFS começando a partir do vértice s
void dfs(int u,int n,VertexType Vet[],int *cont)
{
    int v;

    Vet[u].visitado=true;
    Vet[u].d=*cont;
    //cout << "Vértice entrando na dfs: " << u <<" - Valor de cont atribuído: " << Vet[u].d << "\n";

    // it é iterador usado para obter todos os vértices adjacentes de um vértice
    list<int>:: iterator it;

    // Obtém todos os vértices adjacentes do vértice u
    for (it=Vet[u].Adj.begin(); it!=Vet[u].Adj.end(); it++)
    {
        v=*it;
        // se vértice adjacente ainda não foi visitado, marca como visitado e chama recursão
        if (!Vet[v].visitado)
        {
            Vet[v].p=u; // marca como visitado
            (*cont)++;
            dfs(v,n,Vet,cont);
        }
    } // dfs
}
//*****************************conversores de numeros**************
//converte um numero interio para binário
vector<int> decToBin(int n, int maximo)
{
    vector <int> array;
    for(int i=0; i<n; i++)
    {
        array.push_back(0);
    }

    for(int i=0; i<n; i++)
    {
        array[n-i-1]= maximo &((long) 1 << i) ? 1:0;

    }
    return array;

}
//*****************************finalmente, a resposta***************

int resposta(int k, int n, VertexType Vet[], int *cont)
{
    int acc=0;
    int resp=0;
    int minimo_de_vertices = INT_MAX;
    int qtd;
    int connected_component = 0;
    int copiados = 0;
    long long int maximo = (long long int )pow(2,n)-1;
    VertexType copiaVertex[maxVert];
    vector <int> excluidosVertex;

    for(long long int i = maximo; i>0; i--)
    {
        vector <int> r = decToBin(n,i);

        acc = 0;

        for(int j = 0; j < r.size(); j++)
        {
            if (r[j] == 1)
            {
                acc++;

            }

        }

        if( n-acc > k )
        {
            continue;
        }

        qtd=n-acc;

        for(int u = 1; u <= n; u++)
        {
            if(r[u-1] == 1)
            {
                copiaVertex[u].Adj=Vet[u].Adj;
                copiaVertex[u].d=Vet[u].d;
                copiaVertex[u].p=Vet[u].p;
                copiaVertex[u].visitado=Vet[u].visitado;
                copiados++;
            }
            else
            {

                excluidosVertex.push_back(u);
            }
        }

        for(int j = 1; j <= n; j++)
        {
            if(r[j-1]==1)
            {
                for(int k = 0; k < (int) excluidosVertex.size(); k++)
                {
                    copiaVertex[j].Adj.remove(excluidosVertex[k]);
                }
            }
        }


        *(cont) = 1;
        for(int u = 1; u <= n; u++)
        {
            if(r[u-1]==1 && copiaVertex[u].visitado== false)
            {

                connected_component+=1;
                dfs(u, n, copiaVertex,cont);
            }
        }

	if(qtd > 0){
		if(connected_component == resp && qtd <= minimo_de_vertices){
		    minimo_de_vertices = qtd;
		    resp = connected_component;
		    escolhidos.clear();
		    escolhidos = excluidosVertex;
		}
		else if(connected_component > resp)
		{
		    minimo_de_vertices= qtd;
		    resp = connected_component;
		    escolhidos.clear();
		    escolhidos = excluidosVertex;
		}
	}

        excluidosVertex.clear();
        connected_component=0;
        copiados=0;
    }

    return resp;
} // resposta


// **************************** MAIN *******************************
int main( int argc, char * argv[ ])
{
    int n; // número de vértices do grafo
    int k;
    VertexType Vet[maxVert]; // cria lista de adjacências com tamanho maxVert
    string arquivo="grafo.txt";

    readFileLista(arquivo,&n,&k,Vet);

    //writeGraphLista(n,Vet);

    // marca todos os vértices como não visitados e sem predecessor
    for(int i = 1; i <= n; i++)
    {
        Vet[i].visitado=false;
        Vet[i].p=-1;
        Vet[i].d=INT_MAX;
    }

    int digitos = ceil(log10(n));

    const clock_t begin_time = clock();

    int resp;
    int cont =1;
    
    resp = resposta(k, n, Vet, &cont);

    if(resp == 1)
    {
        cout<<"não ha resposta para este k"<<endl;
        resp=0;
        cout << "Tempo: " << float( clock () - begin_time ) /  CLOCKS_PER_SEC << " s" << endl;
	return 0;
    }

    cout << "metodo exato: ";

    for(int i = 0; i < escolhidos.size(); i++)
    {
        cout << setw(digitos) << setfill('0') << escolhidos[i] << " ";
    }
    cout << endl;

    cout << "componentes: " << resp << endl;

    cout << "Tempo: " << float( clock () - begin_time ) /  CLOCKS_PER_SEC << " s" << endl;

    return 0;
}


