/* Introdución a la Concurrencia
 * Tp Final : Smoothing 
 * 
 * Integrantes :    Arcos Martin
 *                  Argañaras Florencia
 *                  Montes Adriel   
 */

/* Suavizado version secuencial
 * 
 * Dado que cuenta con archivo Makefile, 
 * Para compilar : 
 * 
 * make 
 * 
 * Para ejecutar, el modo de uso es:
 *
 * ./suavizadoSecuencial <tamaño_matriz> <vecinos> <iteraciones> [-i]
 * 
 * tamanio_matriz:      Tamaño de la matriz
 * vecinos:             Cantidad de vecinos que se va a tener
 * iteraciones:         Cantidad de iteraciones 
 *                      (si no se utiliza esta opcion, seran infinitas as iteraciones, al igual que 0)
 * -i:                  Indica que se cargarán los valores iniciales manualmente 
 *                      (si no se utiliza esta opción, se inicializará aleatoriamente)
 * 
 * para medir leer un archivo txt:
 *      cat <nombreArchivo>.txt | ./suavizadoSecuencial <tamaño_matriz> <vecinos> <iteraciones> [-i]
 * 
 * para medir el tiempo:
 *       time ./suavizadoSecuencial <tamaño_matriz> <vecinos> <iteraciones> [-i]
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

int N = 0;
int vecinos = 0;
bool controlParada = true;
int control_exeso = 0;
clock_t start, end;     //medida de tiempo
int iteraciones;
bool iteracionInfinita = false;
int controlIteracion = 0; 

void imprimirMatriz(int matriz[][N])
{	    
    int i, j;
	
    printf("\n");
            
	for (i=0; i<N; i++)
		printf("___");

	printf("\n");

	for (i=0; i<N; i++) {
	    
        printf("|");
        for (j=0; j<N; j++)
		    if (matriz[i][j])
				printf("%2d ", matriz[i][j]);
			else
				printf(" - ");

		printf("\b|\n");
	}

	for (i=0; i<N; i++)
		printf("---");

	printf("-\n");
}

int analizarVecinos(int FILAS, int COLUMNAS, int matriz[][N]){
    int vecinos = 0;

    int i = FILAS-1;
    for(i; i <= FILAS+1; i++){
        int j = COLUMNAS-1;

        for(j; j <= COLUMNAS+1; j++){
            if((i > -1 && j > -1) && (i < N && j < N) && !(i == FILAS && j == COLUMNAS))
                if(matriz[i][j] == 1)
                    vecinos++;
        }
    }

    return vecinos;
}

void cicloSmooth(int matriz[][N], int matriz_aux[][N])
{    
    int i,j,n_vecinos;

    //ciclo principal
    for(i = 0; i < N; i++){
        for(j = 0; j < N; j++){
            n_vecinos = analizarVecinos(i,j,matriz_aux);

            //condiciones para que una celula no muera
            if ((n_vecinos == vecinos) && (matriz[i][j] == 1)){
                matriz[i][j] = 0; 
            }
            /*
            if(n_vecinos >= vecinos){
                matriz[i][j] = (matriz[i][j] == 1)? 0:1;
                controlParada = false; //controlar que si se genera una cambio se debe volver a iterar
            }
            */
        }
    }
}

void clonar(int matriz[][N], int matriz_aux[][N]){

    controlParada = true;
    for(int i = 0; i < N; ++i)
        for(int j = 0; j < N; ++j)
            
            if ((matriz_aux[i][j] != matriz[i][j]))
            {
                matriz_aux[i][j] = matriz[i][j];
                controlParada = false; 
            }
            else
                matriz_aux[i][j] = matriz[i][j];
}

void inicializarMatrizRandom(int matriz[][N])
{
    int i, j;
    
    srand(getpid());

    for (i = 0; i < N; ++i)
        for (j = 0; j < N; ++j)
            matriz[i][j] = rand() % 2;
}

void inicializarMatrizManual(int g[][N], int len)
{
	int i, j, val;

	printf("Ingrese valores 1 o 0:\n");
	for(i = 0; i < len; ++i) {
		for(j = 0; j < len; ++j) {
			printf("matriz[%d][%d]: ", i, j);
			scanf("%d", &val);
			// Me aseguro que sean 1 o 0 (0 es 0. Cualquier otro entero es 1)
			val = !val;
			g[i][j] = !val;
		}
		printf("\n");
	}
	printf("\n");
}

int main(int argc, char *argv[])
{
    start = clock();
    
    int iniManual = 0;

    sscanf(argv[1], "%d", &N);            //tomo el tamaño de la matriz
    sscanf(argv[2], "%d", &vecinos);      //tomo la cantidad de vecinos para saber apagar un pixel
    sscanf(argv[3], "%d", &iteraciones);

    if ((argc != 5 && argc != 4) || (N < 2) || (vecinos <= 0 || vecinos > 9) || (argc == 5 && (!(iniManual = !strncmp(argv[4], "-i", 2)))))
    {
        printf("\nUso: %s <tamaño_matriz> <vecinos> <iteraciones> [-i]\n"
                "\ntamaño_matriz:\ttamaño de la matriz \n"
                "vecinos:\tCantidad de vecinos que se va a tener\n"
                "iteraciones:\tCantidad de iteraciones"
                "\t\t(Si no se utiliza esta opcion, seran infinitas as iteraciones, al igual que 0)\n"
                "-i:\t\tIndica que se cargarán los valores iniciales manualmente\n"
                "\t\t(Si no se utiliza esta opción, se inicializará aleatoriamente)\n"
                "\nREGLAS\n"
                "1. El tamaño de la matriz debe de ser mayor que 1\n"
                "2. La cantidad de vecinos no debe ser menor a 1 ni mayor a 9\n\n",
                argv[0]);

        return 0;
    }
    
    int matriz[N][N], matriz_aux[N][N];
    
    printf("\n/**********************************INICIO DEL PROGRAMA**********************************/ \n");
    printf("\nTamaño de grilla: %d x %d\n", N, N);
	printf("Vecinos: %d\n",vecinos);

    if (iteraciones == 0){
        printf("Iteraciones: infinitas\n\n");
        iteraciones = 6;    // le asignamos 1 para que while sea infinito
        iteracionInfinita = true;
    }
    else
        printf("Iteraciones: %d\n\n", iteraciones);

    if(iniManual)
        inicializarMatrizManual(matriz,N);
    else
        inicializarMatrizRandom(matriz);
        
    printf("MATRIZ INICIAL:");
    imprimirMatriz(matriz); //mostrar la matriz inicial

    while(control_exeso <= iteraciones) {

        cicloSmooth(matriz, matriz_aux);
        clonar(matriz, matriz_aux);

        if (controlParada){
            break;
        }else
        {
            if (!(iteracionInfinita))
                ++control_exeso;
            
            ++controlIteracion;
        }
    }
    
    printf("\nResultado final:");
    imprimirMatriz(matriz_aux); //mostrar la matriz final
    
    if (controlParada)
    {
        printf("\nSe termino porque llego a la condicion de parada\n\n");
        printf("Hubo cambios hasta la iteracion %d\n",controlIteracion);
    }
    else
    {
        printf("\nSe realizaron todas las iteraciones\n\n");
        control_exeso--;
        printf ("Se realizaron %d iteraciones\n", control_exeso);
    }
    end = clock();  //obtengo el número de tics de reloj transcurridos desde que se inició el programa
    double tiempoTotal = (double)(end - start) * 100 / CLOCKS_PER_SEC; // calcula el tiempo dividio por el tiempo en segundos (CLOCKS_PER_SEC) 
    printf("\nEl tiempo fue de: %fs \n", tiempoTotal);
    printf("\n/***********************************FIN DEL PROGRAMA*********************************/ \n\n");
    return 0;
}
