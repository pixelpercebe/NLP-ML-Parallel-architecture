/***************************************************
 AC - OpenMP -- PARALELO
 fun_p.c
 rutinas que se utilizan en el modulo grupopal_p.c

 gcc -O2 -o grupopal_p grupopal_p.c fun_p.c -lm -fopenmp
 ./grupopal_p ../../ARQ/ppalabras/vecpal.dat ../../ARQ/ppalabras/campos.dat 1000
****************************************************/
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include "defineg.h" // definiciones

/*******************************************************************
 Funcion para calcular el maximo entre dos floats.
 Entrada: 2 elementos.
 Salida:  El mayor elemento.
********************************************************************/
#define MAX(i, j) (((i) > (j)) ? (i) : (j))

/*******************************************************************
 metodo qiucksort gracias a https://www.programiz.com/dsa/quick-sort
 Entrada: 1 array de floats y dos integers que son el indice mayor y meno del array
 Salida:  Array con los elementos ordenados de menor a mayor.
********************************************************************/
void quickSort(float array[], int low, int high) {
    int pi;
    if (low < high) {
        pi = partition(array,low,high);
        quickSort(array, low, pi - 1);
        quickSort(array, pi + 1, high);
    }
}
/*******************************************************************
 metodo que intercambia la posicion a la que señalas dos punteros entre si.
 Entrada: 2 punteros que señalan a dos valores float.
 Salida:  2 punteros a y b con los valores de entrada intercambiados.
********************************************************************/

void swap(float *a, float *b) {
    float t = *a;
    *a = *b;
    *b = t;
}

/*******************************************************************
 Metodo para encontrar la particiones de la posicion,
 Entrada: 1 array de floats y dos integers que son el indice mayor y menor a de la particion a ordenar
 Salida:  Array con los elementos de un segmento ordenados de menor a mayor.
********************************************************************/
// function to find the partition position
int partition(float array[], int low, int high) {

    // select the rightmost element as pivot
    float pivot = array[high];
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

/*******************************************************************
 Metodo que ordenar un array y devuelve la media.
 Entrada: 1 integer, tamaño del array y un array de floats que representa los datos  datos a tratar.
 Salida:  float que guarda la medaiana de los datos del array.
********************************************************************/
double sort_and_median(int tam, float* datos){

    quickSort(datos,0,tam-1);
    return datos[tam/2];
}
/*******************************************************************
 1 - Funcion para calcular la distancia euclidea entre dos vectores
 Entrada: 2 elementos con NDIM caracteristicas (por referencia)
 Salida:  distancia (double)
********************************************************************/
double distpal(float *vec1, float *vec2){
    int i;
    double res = 0.0f;
    for (i=0;i<NDIM;i++)
        res += pow((double)(vec1[i] - vec2[i]),2);
	return sqrt(res);
}

/***********************************************************************************
 2 - Funcion para calcular el grupo (cluster) mas cercano (centroide mas cercano)
 Entrada: nvec  numero de vectores, int
          mvec  vectores, una matriz de tamanno MAXV x NDIM, por referencia
          cent  centroides, una matriz de tamanno ngrupos x NDIM, por referencia
 Salida:  popul grupo mas cercano a cada elemento, vector de tamanno MAXV, por ref.
************************************************************************************/
void grupo_cercano (int nvec, float mvec[][NDIM], float cent[][NDIM], int *popul){
    //por cada centroide en cent comparar con un vector de mvec (un total de nvec) y guardar los que menos distancia
    //tengan en popul, teniendo un grupo de X vectores por cada centroide.
    double dist,distmin;
    int i, k;
#pragma omp parallel for default(none) shared(nvec,ngrupos,mvec,cent,popul) private(distmin,dist,i,k)
        for (i = 0; i < nvec; i++) {
            distmin = DBL_MAX; //cada vector se compara con la distancia maxima de un double (muy pequeño)
            for (k = 0; k < ngrupos; k++) { //hay un total de ngrupos de centroides de NDIM dimensiones.
                dist = distpal(mvec[i], cent[k]);
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
    max = 0.0f;
#pragma omp parallel default(none) shared(ngrupos,mvec,listag, cent,a,b, sumdist) private(i,k,j)
    {
#pragma omp for
        for (i = 0; i < ngrupos; i++)b[i] = 0.0f;
#pragma omp for
        for (i = 0; i < ngrupos; i++)a[i] = 0.0f;

        //aproximar a[i] de cada cluster: calcular la densidad de los grupos;
        //media de las distancia entre todos los elementos del grupo;
        //si el numero de elementos del grupo es 0 o 1, densidad = 0
#pragma omp for nowait reduction(+:sumdist)
        for (k = 0; k < ngrupos; k++) {
            sumdist = 0;
            for (i = 0; i < listag[k].nvecg; i++) {
                for (j = i + 1; j < listag[k].nvecg; j++) {
                    sumdist += distpal(mvec[listag[k].vecg[i]], mvec[listag[k].vecg[j]]);
                }
            }
            if (listag[k].nvecg > 1) {
                //el vector esta inicializado a 0 no hace falta hacer nada si el cluster contiene <= 1 vector.
                a[k] = sumdist / (float) (listag[k].nvecg);
            }
        }

        //aproximar b[k] de cada cluster
#pragma omp for nowait reduction(+:sumdist)
        for (k = 0; k < ngrupos; k++) {
            sumdist = 0;
            for (j = k + 1; j < ngrupos; j++) {
                sumdist += distpal(cent[k], cent[j]);
            }
            b[k] = sumdist / ngrupos;
        }
    }

    // calcular el ratio s[k] de cada cluster
    s= 0.0f;
    for (k = 0; k < ngrupos; k++) {
        max = MAX(a[k], b[k]);
        if (max != 0.0f) {
            s += (float) (b[k] - a[k] / max); //se suma cada ratio, sin necesidad de guardarlo.
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
void analisis_campos(struct lista_grupos *listag, float mcam[][NCAM],struct analisis *info_cam) {

    int i, cam, k; //i : palabras ; k : cluster; cam : Campo UNESCO.
    float mediana; //mediana del los valores de la relaciones al campo en una columna de todas las palabras de un grupo.
    float *relacion; //apuntador al primer valor de las relaciones de las palabras de un cluster con un campo.
#pragma omp parallel default(none) private(i, k, cam,mediana,relacion) shared(ngrupos,listag, mcam, info_cam)
{
    #pragma omp for nowait
        for (i = 0; i < NCAM; i++) {
            info_cam[i].mmin = FLT_MAX;
            info_cam[i].mmax = 0.0f;
        }
    #pragma omp for nowait schedule(dynamic)
        for (k = 0; k < ngrupos; k++) {
            for (cam = 0; cam < NCAM; cam++)
            {
                /**en vez de hacer un array con el numero maximo de palabras como tamaño maximo,
                * reservamos espacion en memoria por cada cluster
                * y solo para los elementos de ese cluster.*/

                relacion = malloc(sizeof(float) * listag[k].nvecg);
                for (i = 0; i < listag[k].nvecg; i++) {
                    relacion[i] = 0.0f;
                }
                for (i = 0; i < listag[k].nvecg; i++) {
                    //guardamos las relaciones de cada palabra cone el campo.
                    relacion[i] = mcam[listag[k].vecg[i]][cam];
                }
                //ordenar y calcular la media
                mediana = sort_and_median(listag[k].nvecg, relacion);
                //ahora tenemos la media de las relaciones de las palbras de este cluster con repsecto a este campo.

                //comprobamos si es minimo o maximo y actalizamos si lo es.
                if (mediana > 0 && mediana < info_cam[cam].mmin) {
                    info_cam[cam].mmin = mediana;
                    info_cam[cam].gmin = k;
                }
                if (mediana > info_cam[cam].mmax) {
                    info_cam[cam].mmax = mediana;
                    info_cam[cam].gmax = k;
                }
                free(relacion);
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
    #pragma omp parallel default(none) shared(ngrupos,nvec,mvec,cent,popul,newcent,additions,fin) private(discent,i,j)
        {
    #pragma omp for
            for (i = 0; i < ngrupos; i++)
                for (j = 0; j < NDIM + 1; j++)
                    additions[i][j] = 0.0;

            // acumular los valores de cada caracteristica; numero de elementos al final
    #pragma omp for nowait reduction(+:additions[:ngrupos][:NDIM+1])
            for (i = 0; i < nvec; i++) {
                for (j = 0; j < NDIM; j++) additions[popul[i]][j] += mvec[i][j];
                additions[popul[i]][NDIM]++;
            }

            // calcular los nuevos centroides y decidir si el proceso ha finalizado o no (en funcion de DELTA
    #pragma omp single
            fin = 1;
    #pragma omp for
            for (i = 0; i < ngrupos; i++) {
                if (additions[i][NDIM] > 0) { // ese grupo (cluster) no esta vacio
                    // media de cada caracteristica
                    for (j = 0; j < NDIM; j++)
                        newcent[i][j] = (float) (additions[i][j] / additions[i][NDIM]);

                    // decidir si el proceso ha finalizado
                    discent = distpal(&newcent[i][0], &cent[i][0]);
                    if (discent > DELTA1) {
                    fin = 0;  // en alguna centroide hay cambios; continuar
                    }

                    // copiar los nuevos centroides
                    for (j = 0; j < NDIM; j++)
                        cent[i][j] = newcent[i][j];
                }
            }
        }
	return fin;
}
