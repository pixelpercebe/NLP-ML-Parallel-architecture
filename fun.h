/***************************************************************
    fun.h
    cabeceras de las funciones utilizadas en el modulo gengrupos
****************************************************************/

extern double gendist (float *vec1, float *vec2);
extern void grupo_cercano (int nvec, float mvec[][NDIM], float cent[][NDIM], int *popul);
extern double silhouette_simple(float mvec[][NDIM], struct lista_grupos *listag, float cent[][NDIM], float a[]);
extern void analisis_campos(struct lista_grupos *listag, float mcam[][NCAM], struct analisis *info_cam);

extern void inicializar_centroides(float cent[][NDIM]);
extern int nuevos_centroides(float mvec[][NDIM], float cent[][NDIM], int popul[], int nvec);
