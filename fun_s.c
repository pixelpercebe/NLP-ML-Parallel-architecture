/***************************************************
 AC - OpenMP -- SERIE
 fun_s.c
 rutinas que se utilizan en el modulo grupopal_s.c
****************************************************/
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include "defineg.h" // definiciones

/*******************************************************************
 1 - Funcion para calcular el maximo entre dos floats.
 Entrada: 2 elementos.
 Salida:  El mayor elemento.
********************************************************************/
#define MAX(i, j) (((i) > (j)) ? (i) : (j))


void swap(int *a, int *b) {
    int t = *a;
    *a = *b;
    *b = t;
}

// function to find the partition position
int partition(int array[], int low, int high) {
    // select the rightmost element as pivot
    int pivot = array[high];
    // pointer for greater element
    int i = (low - 1);
    // traverse each element of the array
    // compare them with the pivot
    for (int j = low; j < high; j++) {
        if (array[j] <= pivot) {
            // if element smaller than pivot is found
            // swap it with the greater element pointed by i
            i++;
            // swap element at i with element at j
            swap(&array[i], &array[j]);
        }
    }
    // swap the pivot element with the greater element at i
    swap(&array[i + 1], &array[high]);
    // return the partition point
    return (i + 1);
}

//metodo qiucksort gracias a https://www.programiz.com/dsa/quick-sort
void quickSort(int array[], int low, int high) {
    if (low < high) {

        // find the pivot element such that
        // elements smaller than pivot are on left of pivot
        // elements greater than pivot are on right of pivot
        int pi = partition(array, low, high);

        // recursive call on the left of pivot
        quickSort(array, low, pi - 1);

        // recursive call on the right of pivot
        quickSort(array, pi + 1, high);
    }
}
/*******************************************************************
 1 - Funcion para calcular la distancia euclidea entre dos vectores
 Entrada: 2 elementos con NDIM caracteristicas (por referencia)
 Salida:  distancia (double)
********************************************************************/
double distpal(float *vec1, float *vec2){
	// PARA COMPLETAR
	// calcular la distancia euclidea entre dos vectores
    int i;
    double res = 0.0f;
    for (i=0;i<NDIM;i++)
        res += (double)pow((vec1[i] - vec2[2]),2);
	return sqrt(res);
}

/***********************************************************************************
 2 - Funcion para calcular el grupo (cluster) mas cercano (centroide mas cercano)
 Entrada: nvec  numero de vectores, int
          mvec  vectores, una matriz de tamanno MAXV x NDIM, por referencia
          cent  centroides, una matriz de tamanno ngrupos x NDIM, por referencia
 Salida:  popul grupo mas cercano a cada elemento, vector de tamanno MAXV, por ref.
************************************************************************************/
void grupo_cercano (int nvec, float mvec[][NDIM], float cent[NDIM],
		int *popul){
	// PARA COMPLETAR
    //por cada centroide en cent comparar con un vector de mvec (un total de nvec) y guardar los que menos distancia
    //tengan en popul, teniendo un grupo de X vectores por cada centroide.
    double dist,distmin;
    int i, k;
    for (i = 0; i < nvec; i++) {
        distmin = DBL_MAX; //cada vector se compara con la distancia maxima de un double (muy pequeño)
        for (k = 0; k < ngrupos; k++) { //hay un total de ngrupos de centroides de NDIM dimensiones.
            dist = distpal(mvec[i], &cent[k]);
            if (dist<distmin)
            {
                distmin = dist;
                popul[i] = k; //guardamos el INDICE DEL CENTROIDE mas cercano, es decir e indice del grupo. Eso para
                //cada palabra/vector (nvec veces) cada palabra se asigna a un centro, al que este mas cerca.
            }
        }
    }
	// popul: grupo mas cercano a cada elemento
}

/***************************************************************************************
 3 - Funcion para calcular la calidad de la particion de clusteres.
     Ratio entre a y b.
     El termino a corresponde a la distancia intra-cluster.
     El termino b corresponde a la distancia inter-cluster.
 Entrada: mvec    vectores, una matriz de tamanno MAXV x NDIM, por referencia
          listag  vector de ngrupos structs (informacion de grupos generados), por ref.
          cent    centroides, una matriz de tamanno ngrupos x NDIM, por referencia
 Salida:  valor del CVI (double): calidad/bondad de la particion de clusters
****************************************************************************************/
double silhouette_simple(float mvec[][NDIM], struct lista_grupos *listag, float cent[][NDIM], float a[]){

    float b[ngrupos]; //medias distancia inter - cluster para cada cluster.
    float s, max; //s: suma de los ratios ; max : el maximo entre la media intercluster e intra cluster de un cluster.
    int i,j,k; //k : cluster ; i : palabras ; j : mismo tipo que su bucle anterior, solo en anidados.
    double sumdist; //suma de las distancias, inter e intra cluster.


    for (i = 0; i<ngrupos;i++)b[i] = 0.0f;
    for (i = 0; i<MAX_GRUPOS;i++)a[i] = 0.0f;
    max = 0.0f;

    //aproximar a[i] de cada cluster: calcular la densidad de los grupos;
    //media de las distancia entre todos los elementos del grupo;
    //si el numero de elementos del grupo es 0 o 1, densidad = 0
    for (k = 0; k < ngrupos ; k++) {
        sumdist = 0;
        for (i = 0; i < listag[k].nvecg; i++) {
            for (j = i + 1; j < listag[k].nvecg; j++) {
                sumdist += distpal((float*)&listag[k].vecg[i],(float*)&listag[k].vecg[j]);
            }
        }
        if (listag[k].nvecg > 1) {
            //el vector esta inicializado a 0 no hace falta hacer nada si el cluster contiene <= 1 vector.
            a[k] = sumdist / (float) listag[k].nvecg;}
    }

    //aproximar b[k] de cada cluster
    for(k = 0;k<ngrupos;k++){
        sumdist = 0;
        for(j = k+1; j<ngrupos;j++)
        {
            sumdist += distpal(cent[k], cent[j]);
        }
        b[k] = sumdist / ngrupos;
    }

    // calcular el ratio s[k] de cada cluster
    for (k= 0;k < ngrupos; k++)
    {
        max = MAX(a[k],b[k]);
        if( max != 0.0f){
            s += (float)(b[k] - a[k] / max); //se suma cada ratio, sin necesidad de guardarlo.
        }
    }

    //se devuelve la media que indica la calidad de la particion (la suma de los ratios s[k] entre el num. de centroides.)
    // promedio y devolver
    return (double)(s/(float)ngrupos);
}



/********************************************************************************************
 4 - Funcion para relizar el analisis de campos UNESCO
 Entrada:  listag   vector de ngrupos structs (informacion de grupos generados), por ref.
           mcam     campos, una matriz de tamaño MAXV x NCAM, por referencia
 Salida:   info_cam vector de NCAM structs (informacion del analisis realizado), por ref.
*****************************************************************************************/
void analisis_campos(struct lista_grupos *listag, float mcam[][NCAM],
		struct analisis *info_cam){
	// PARA COMPLETAR
	// Realizar el analisis de campos UNESCO en los grupos:
	//    mediana maxima y el grupo en el que se da este maximo (para cada campo)
	//    mediana minima y su grupo en el que se da este minimo (para cada campo)

    // que campo de la UNESCO esta mas cerca y lejos de cada cluster.
    int i,cam,k; //i : palabras ; k : cluster; cam : Campo UNESCO
    float mediana;
    float relacion[MAXV];

    for (cam = 0; cam < NCAM; cam ++)
    {
        for(k=0; k < ngrupos; k++)
        {
            for ()
            {
            //limpiar relacion. 0.0 todo 
            }
            for (i = 0; i < listag[k].nvecg; i++ )
            {
                relacion[i] = mcam[listag[k].vecg[i]][cam]
            }

        }
    }

}

/*************************************
   OTRAS FUNCIONES DE LA APLICACION
**************************************/
void inicializar_centroides(float cent[][NDIM]){
	int i, j;
	float rand_val;
	srand (147);
	for (i=0; i<ngrupos; i++)
		for (j=0; j<NDIM/2; j++){
			rand_val = ((rand() % 10000) / 10000.0)*2-1;
			cent[i][j] = rand_val;
			cent[i][j+(NDIM/2)] = cent[i][j];
		}
}

int nuevos_centroides(float mvec[][NDIM], float cent[][NDIM], int popul[], int nvec){
	int i, j, fin;
	double discent;
	double additions[ngrupos][NDIM+1];
	float newcent[ngrupos][NDIM];

	for (i=0; i<ngrupos; i++)
		for (j=0; j<NDIM+1; j++)
			additions[i][j] = 0.0;

	// acumular los valores de cada caracteristica; numero de elementos al final
	for (i=0; i<nvec; i++){
		for (j=0; j<NDIM; j++) additions[popul[i]][j] += mvec[i][j];
		additions[popul[i]][NDIM]++;
	}

	// calcular los nuevos centroides y decidir si el proceso ha finalizado o no (en funcion de DELTA)
	fin = 1;
	for (i=0; i<ngrupos; i++){
		if (additions[i][NDIM] > 0) { // ese grupo (cluster) no esta vacio
			// media de cada caracteristica
			for (j=0; j<NDIM; j++)
				newcent[i][j] = (float)(additions[i][j] / additions[i][NDIM]);

			// decidir si el proceso ha finalizado
			discent = distpal(&newcent[i][0], &cent[i][0]);
			if (discent > DELTA1){
				fin = 0;  // en alguna centroide hay cambios; continuar
			}

			// copiar los nuevos centroides
			for (j=0; j<NDIM; j++)
				cent[i][j] = newcent[i][j];
		}
	}
	return fin;
}
