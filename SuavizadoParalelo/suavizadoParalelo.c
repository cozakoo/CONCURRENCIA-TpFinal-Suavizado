#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mpi.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

#define DETENER 0
#define REALIZAR_CICLO_SMOOTH 1
#define CLONACION_MATRIZ 2
#define CONTROL_PARADA 3

int controlIteracion = 0; 
int N;
int filas;
int size, rank, workers, filas;
bool condicion_parada;
int control_exeso = 0;
bool iteracionInfinita = false;


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

void preparar_arreglo(int grilla[filas][N])
{
    int val;
    if (rank < workers)
    {   
        for (int j = 0; j < N; j++)
        {
            val = grilla[filas - 1][j];         //Filas = 2, menos 1 me posciciono en la ultima fila de la grila
            MPI_Send(&val, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
        }
    }

    if (rank > 1)
    {
        for (int j = 0; j < N; j++)
        {
            val = grilla[0][j];                 
            MPI_Send(&val, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD);
        }
    }
}


int mirarVecinos(int grilla[][N], int i, int j, int arreglo_abajo[N], int arreglo_arriba[N])
{
    int contProvisorio = 0;    

    if ((i - 1 == -1) && (rank != 1) && (workers > 1))
    {
        if ((j - 1 != -1) && (arreglo_arriba[j - 1] == 1)) // Pregunto si necesito acceder al arreglo que me envia mi rank - 1 y si la cantidad de procesos es mayor a 1 por que si no, estaria
            contProvisorio++;                              // trabajando con una matriz de igual tamaño a la original y no necesito acceder a otros arreglos
 
        if (arreglo_arriba[j] == 1)
            contProvisorio++;
        
        if ((j + 1 != N) && (arreglo_arriba[j + 1] == 1))
            contProvisorio++;
    }

    if ((i + 1 == filas) && (rank != workers))
    { // Pregunto si necesito acceder al arreglo que me envia mi rank + 1
        if ((j - 1 != -1) && (arreglo_abajo[j - 1] == 1))
            contProvisorio++;
        
        if (arreglo_abajo[j] == 1)
            contProvisorio++;
        
        if ((j + 1 != N) && (arreglo_abajo[j + 1] == 1))
            contProvisorio++;
    }

    if ((i - 1 != -1) && (j - 1 != -1) && (grilla[i - 1][j - 1] == 1))      //Pregunto si existe mi vecino del noroeste
        contProvisorio++;
    
    if ((i - 1 != -1) && (grilla[i - 1][j] == 1))                           //Pregunto si existe mi vecino del norte
        contProvisorio++;
    
    if ((i - 1 != -1) && (j + 1 != N) && (grilla[i - 1][j + 1] == 1))       //Pregunto si existe mi vecino del noreste
        contProvisorio++;
 
    if ((j - 1 != -1) && (grilla[i][j - 1] == 1))                           //Pregunto si existe mi vecino del oeste
        contProvisorio++;
    
    if ((j + 1 != N) && (grilla[i][j + 1] == 1))                            //Pregunto si existe mi vecino del este
        contProvisorio++;
    
    if ((i + 1 != filas) && (j - 1 != -1) && (grilla[i + 1][j - 1] == 1))   //Pregunto si existe mi vecino del suroeste
        contProvisorio++;
    
    if ((i + 1 != filas) && (grilla[i + 1][j] == 1))                        //Pregunto si existe mi vecino del sur
        contProvisorio++;
    
    if ((i + 1 != filas) && (j + 1 != N) && (grilla[i + 1][j + 1] == 1))    //Pregunto si existe mi veciono del sureste
        contProvisorio++;

    return contProvisorio;
}

//***********************************************************

void print_matriz(int matriz[][N], int len)
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

void llenar_matriz(int g[][N], int len)
{
    int i, j;

    srand(getpid());

    for (i = 0; i < len; ++i)
    {
        for (j = 0; j < len; ++j)
        {
            if (rand() % 10000 >= 5000)
                g[i][j] = 0;
            else
                g[i][j] = 1;
        }
    }
}

//************************************

int main(int argc, char *argv[])
{
    int vecinos, iteraciones, mostrar_cada;

    int mensaje;

    int iniManual = 0;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    sscanf(argv[1], "%d", &N);            //tomo el tamaño de la matriz
    sscanf(argv[2], "%d", &vecinos);      //tomo la cantidad de vecinos para saber apagar un pixel
    sscanf(argv[3], "%d", &iteraciones);  //tomo la cantidad de veces que debe iterar

    workers = size - 1;

    if ((argc != 5 && argc != 4) || (N % workers != 0) || (workers > N) || (N < 2) || (vecinos <= 0 || vecinos > 9) || (argc == 5 && !(iniManual = !strncmp(argv[4], "-i", 2))))
    {
        if (rank == 0)
        {
            printf("\n");
            printf("1. Los workers deben dividir a la matriz\n");
            printf("\n");
            printf("2. Los workers son todos los procesos menos el coordinador\n");       
            printf("\n");
            printf("3. La cantidad de procesos no debe superar el tamaño del arreglo\n");
            printf("\n");
            printf("4. La cantidad de vecinos no debe ser menor a 1 ni mayor a 9\n");       
            printf("\n");
        }
        MPI_Barrier(MPI_COMM_WORLD);
        return 0;
    }    
    
    filas = N / workers;
    int matriz_original[N][N];
    int matriz_nueva[N][N];

    int grilla[filas][N];
    int grilla_temporal[filas][N];

    int arreglo_abajo[N];  //Del worker 1 hasta workers - 1 (todos menos el ultimo worker) obtienen este arreglo
    int arreglo_arriba[N]; //Del worker 2 hasta workers (todos menos el primer worker) obtienen este arreglo

    int cant_celdas_grilla = N * filas;
    time_t comienzo, final;
    
    
    if (rank == 0)
    {
        printf("\n");
        printf("Matriz: %d x %d\n", N, N);
        printf("\n");
        printf("Workers: %d\n", workers);
        printf("\n");
        printf("Cantidad vecinos: %d\n", vecinos);
        
        if (iteraciones == 0){
            printf("Iteraciones: infinitas\n\n");
            iteraciones = 6;    // le asignamos 1 para que while sea infinito
            iteracionInfinita = true;
        }
        else
            printf("Iteraciones: %d\n\n", iteraciones);

        if(iniManual)
            inicializarMatrizManual(matriz_original,N);
        else
            llenar_matriz(matriz_original, N);

        printf("\nMatriz Inicial:\n\n");
        print_matriz(matriz_original,N);

        int i = 0;
        int tope = filas;
        clock_t start, end;
        start = clock(); 

        //DIVIDE LA MATRIZ
        for (int k = 1; k < workers + 1; k++)
        {
            int m = 0;
            for (i; i < tope; i++)
            {
                for (int j = 0; j < N; j++)
                    grilla[m][j] = matriz_original[i][j];
                
                m++;
            }
            i = tope;
            tope = filas + tope;
            MPI_Send(&grilla, cant_celdas_grilla, MPI_INT, k, 0, MPI_COMM_WORLD);
        }

        while (control_exeso <= iteraciones)
        {
            mensaje = REALIZAR_CICLO_SMOOTH;
            
            for (int k = 1; k < workers + 1; k++)
                MPI_Send(&mensaje, 1, MPI_INT, k, 0, MPI_COMM_WORLD);
            
            mensaje = CLONACION_MATRIZ;

            for (int k = 1; k < workers + 1; k++)
                MPI_Send(&mensaje, 1, MPI_INT, k, 0, MPI_COMM_WORLD);
            
            int i = 0;
            int tope = filas;
            
            condicion_parada = true;

            for (int k = 1; k < workers + 1; k++)
            {       
                MPI_Recv(&grilla, cant_celdas_grilla, MPI_INT, k, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                int m = 0;
                for (i; i < tope; i++)
                {
                    for (int j = 0; j < N; j++)
                    {
                        if (matriz_nueva[i][j] != grilla[m][j])
                        {
                            matriz_nueva[i][j] = grilla[m][j];
                            condicion_parada = false;
                        }else
                            matriz_nueva[i][j] = grilla[m][j];
                    }
                    m++;
                }
                i = tope;
                tope = filas + tope;
            }

            if ((control_exeso == iteraciones) || (condicion_parada))
            {
                mensaje = DETENER;
                for (int i = 1; i < workers + 1; i++) // Les digo a todos los procesos que terminen su ejecución
                    MPI_Send(&mensaje, 1, MPI_INT, i, 0, MPI_COMM_WORLD);

                if (condicion_parada)
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

                print_matriz(matriz_nueva, N);

                end = clock();
                double total = (double)(end - start) * 100 / CLOCKS_PER_SEC;
                printf("El tiempo fue de: %fs\n", total);
                printf("\n***********************Finaliza el programa**************************\n\n");
                break;
            }
            else
            {
                if (!(iteracionInfinita))
                    ++control_exeso;

                ++controlIteracion;
            }

        }
    } // end rank 0

    if (rank != 0)
    {
        
        MPI_Recv(&grilla, cant_celdas_grilla, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        for (int i = 0; i < filas; i++)
        {
            for (int j = 0; j < N; j++)
                grilla_temporal[i][j] = grilla[i][j];
        }
        
        while (1)
        {
            int cantVecinosVivos = 0;
            int valor;

            MPI_Recv(&mensaje, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            if (mensaje == REALIZAR_CICLO_SMOOTH || mensaje == CLONACION_MATRIZ)
            {   

                if (mensaje == CLONACION_MATRIZ)
                {
                    for (int i = 0; i < filas; i++)
                    {
                        for (int j = 0; j < N; j++)
                            grilla[i][j] = grilla_temporal[i][j];
                    }

                    MPI_Send(&grilla, cant_celdas_grilla, MPI_INT, 0, 0, MPI_COMM_WORLD);
                }
                
                preparar_arreglo(grilla);

                if (rank < workers)
                {
                    for (int i = 0; i < N; i++){
                        MPI_Recv(&valor, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        arreglo_abajo[i] = valor;
                    }
                }
                if (rank > 1)
                {
                    for (int i = 0; i < N; i++){
                        MPI_Recv(&valor, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        arreglo_arriba[i] = valor;
                    }
                }
                
                for (int i = 0; i < filas; i++)
                {
                    for (int j = 0; j < N; j++)
                    {
                        cantVecinosVivos = mirarVecinos(grilla, i, j, arreglo_abajo, arreglo_arriba);
                        
                        if ((cantVecinosVivos == vecinos) && (grilla_temporal[i][j] == 1))
                            grilla_temporal[i][j] = 0;             
                    }
                }
            }
            if (mensaje == DETENER)
                break;
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    
}
