/****************************************************************************************
    grupopal_s   SERIE

    Entrada: vecpal.dat    fichero con las representaciones vectoriales de palabras
             campos.dat    fichero con la carcania de cada palabra a cada campo UNESCO
    Salida:  vecpal_s.out  centroides, densidad, analisis

    compilar con el modulo fun_s.c y la opcion -lm
******************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "defineg.h"
#include "fun.h"

float  mvec[MAXV][NDIM];                 // vectores (palabras) a procesar
struct lista_grupos listag[MAX_GRUPOS];  // lista de vectores de los grupos
float  mcam[MAXV][NCAM];                 // campos UNESCO asociados a los vectores (palabras)
struct  analisis info_cam[NCAM];         // analisis de los campos UNESCO
int ngrupos = 35;

// programa principal
// ==================
int main (int argc, char *argv[]) {
	float   a[MAX_GRUPOS]; // densidad de cada cluster
	int     popul[MAXV]; // grupo de cada vector
	float   cent[MAX_GRUPOS][NDIM]; // centroides
	int     i, j, nvec, grupo, num, ind;
	int     fin = 0, num_ite = 0;
	int     convergencia_cont;
	double  sil, sil_old, diff;

	FILE   *fd;
	struct timespec t1, t2, t11, t12, t17, t20, t21;
	double texe, t_lec, t_clust, t_camp, t_escri;

	if ((argc < 3)  || (argc > 4)) {
	  printf ("[!] ERROR: %s bd_vectores bd_campos [num_vec]\n", argv[0]);
		exit (-1);
	}

	setbuf(stdout, NULL);
	printf ("\n*** Ejecucion serie ***\n\n");
	clock_gettime (CLOCK_REALTIME, &t1);

	// lectura de datos (palabras): mvec[i][j]
	// =======================================
	clock_gettime (CLOCK_REALTIME, &t11);
	fd = fopen (argv[1], "r");
	if (fd == NULL) {
		printf ("[!] Error al abrir el fichero %s\n", argv[1]);
		exit (-1);
	}

	fscanf (fd, "%d", &nvec);
	if (argc == 4) nvec = atoi(argv[3]); // 4. parametro: numero de vectores

	for (i=0; i<nvec; i++)
		for (j=0; j<NDIM; j++)
			fscanf (fd, "%f", &(mvec[i][j]));

	fclose (fd);


	// lectura de datos (campos): mcam[i][j]
	// =====================================
	fd = fopen (argv[2], "r");
	if (fd == NULL) {
		printf ("[!] Error al abrir el fichero %s\n", argv[2]);
		exit (-1);
	}

	for (i=0; i<nvec; i++) {
		for (j=0; j<NCAM; j++)
			fscanf (fd, "%f", &(mcam[i][j]));
	}
	fclose (fd);
	clock_gettime (CLOCK_REALTIME, &t12);
	t_lec = (t12.tv_sec-t11.tv_sec) + (t12.tv_nsec-t11.tv_nsec)/(double)1e9;

	convergencia_cont = 0;
	sil_old = -1;
	while(ngrupos<MAX_GRUPOS && convergencia_cont<1){
		// generacion de los primeros centroides de forma aleatoria
		// ========================================================
		inicializar_centroides(cent);

		// A: agrupar los vectores y calcular los nuevos centroides
		// ========================================================
		num_ite = 0;
		fin = 0;
		while ((fin == 0) && (num_ite < MAXIT)) {
			// calcular el grupo mas cercano
			grupo_cercano(nvec, mvec, cent, popul);
			// calcular los nuevos centroides de los grupos
			fin = nuevos_centroides(mvec, cent, popul, nvec);
			num_ite++;
		}

		// B: Calcular la "calidad" del agrupamiento
		// =========================================
		// lista de clusters: numero de elementos y su clasificacion
		for (i=0; i<ngrupos; i++) listag[i].nvecg = 0;
		for (i=0; i<nvec; i++){
			grupo = popul[i];
			num=listag[grupo].nvecg;
			listag[grupo].vecg[num] = i; // vectores de cada grupo (cluster)
			listag[grupo].nvecg++;
		}
		
		// silhouette: calidad de la particion de clusters
		sil = silhouette_simple(mvec, listag, cent, a);

		// calcular la diferencia: medida de la estabilidad
		diff = sil - sil_old;
		if(diff < DELTA2) convergencia_cont++;
		else convergencia_cont = 0;
		sil_old = sil;

		ngrupos=ngrupos+10;
	}
	ngrupos=ngrupos-10;
	clock_gettime (CLOCK_REALTIME, &t17);
	t_clust = (t17.tv_sec-t12.tv_sec) + (t17.tv_nsec-t12.tv_nsec)/(double)1e9;

	// 2. fase: numero de vectores de cada grupo; analisis campos UNESCO
	// =================================================================
	clock_gettime (CLOCK_REALTIME, &t20);
	analisis_campos(listag, mcam, info_cam);
	clock_gettime (CLOCK_REALTIME, &t21);
	t_camp = (t21.tv_sec-t20.tv_sec) + (t21.tv_nsec-t20.tv_nsec)/(double)1e9;

	// escritura de resultados en el fichero de salida
	// ===============================================
	fd = fopen ("vecpal_s.out", "w");
	if (fd == NULL) {
		printf ("Error al abrir el fichero dbgen_out.s\n");
		exit (-1);
	}

	fprintf (fd,">> Centroides de los clusters\n\n");
	for (i=0; i<ngrupos; i++) {
		for (j=0; j<NDIM; j++) fprintf (fd, "%7.3f", cent[i][j]);
		fprintf (fd,"\n");
	}

	fprintf (fd,"\n\n>> Numero de clusteres: %d. Numero de vectores en cada cluster:\n\n", ngrupos);
	ind = 0;
	for (i=0; i<ngrupos/10; i++) {
		for (j=0; j<10; j++){
			fprintf(fd, "%6d", listag[ind].nvecg);
			ind++;
		}
		fprintf(fd, "\n");
	}
	for(i=ind; i<ngrupos; i++) fprintf(fd, "%6d", listag[i].nvecg);
	fprintf(fd, "\n");

	fprintf (fd,"\n\n>> Densidad de los clusters: a[i]\n\n");
	ind = 0;
	for (i=0; i<ngrupos/10; i++) {
		for (j=0; j<10; j++){
			fprintf (fd, "%9.2f", a[ind]);
			ind++;
		}
		fprintf (fd, "\n");
	}
	for(i=ind; i<ngrupos; i++) fprintf(fd, "%9.2f", a[i]);
	fprintf(fd, "\n");

	fprintf (fd,"\n\n>> Analisis de campos (medianas) en los grupos\n\n");
	for (i=0; i<NCAM; i++)
		fprintf (fd,"Campo: %2d - mmax: %4.2f (grupo %2d) - mmin: %4.2f (grupo %2d)\n",
				i, info_cam[i].mmax, info_cam[i].gmax, info_cam[i].mmin, info_cam[i].gmin);

	fprintf (fd,"\n\n");
	fclose (fd);

	clock_gettime (CLOCK_REALTIME, &t2);
	t_escri = (t2.tv_sec-t21.tv_sec) + (t2.tv_nsec-t21.tv_nsec)/(double)1e9;
	texe = (t2.tv_sec-t1.tv_sec) + (t2.tv_nsec-t1.tv_nsec)/(double)1e9;

	// mostrar por pantalla algunos resultados
	// =======================================
	printf ("\n>> Centroides 0, 20, 40...\n");
	for (i=0; i<ngrupos; i+=20) {
		printf ("\n  cent%2d -- ", i);
		for (j=0; j<NDIM; j++) printf ("%5.1f", cent[i][j]);
		printf("\n");
	}

	printf ("\n>> Numero de clusteres: %d. Tamanno de los grupos:\n\n", ngrupos);
	ind = 0;
	for (i=0; i<ngrupos/10; i++) {
		for (j=0; j<10; j++){
			printf ("%6d", listag[ind].nvecg);
			ind++;
		}
		printf ("\n");
	}
	for(i=ind; i<ngrupos; i++) printf("%6d", listag[i].nvecg);
	printf("\n");

	printf("\n>> Densidad de los clusters: a[i]\n\n");
	ind = 0;
	for (i=0; i<ngrupos/10; i++) {
		for (j=0; j<10; j++){
			printf("%9.2f", a[ind]);
			ind++;
		}
		printf("\n");
	}
	for(i=ind; i<ngrupos; i++) printf("%9.2f", a[i]);
	printf("\n");

	printf ("\n>> Analisis de campos en los grupos\n\n");
	for (i=0; i<NCAM; i++)
		printf ("Campo: %2d - max: %4.2f (grupo %2d) - min: %4.2f (grupo %2d)\n",
				i, info_cam[i].mmax, info_cam[i].gmax, info_cam[i].mmin, info_cam[i].gmin);

	printf("\n");
	printf("t_lec,%f\n", t_lec);
	printf("t_clust,%f\n", t_clust);
	printf("t_camp,%f\n", t_camp);
	printf("Texe_s,%f\n", texe);
}
