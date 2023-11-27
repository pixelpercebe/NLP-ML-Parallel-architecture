/**********************************************************
    defineg.h
    definiciones utilizadas en los modulos de la aplicacion
***********************************************************/

#define MAXV 211640     // numero de vectores (palabras)
#define MAX_GRUPOS 100  // numero de clusters
#define NDIM 40         // dimensiones de cada vector
#define NCAM 23         // numero de campos UNESCO

#define DELTA1 0.01
#define DELTA2 0.01
#define MAXIT 10000    // convergencia: numero de iteraciones maximo

extern int ngrupos;

// estructuras de datos

struct lista_grupos // informacion de los clusters
{
 int vecg[MAXV];    // indices de los vectores
 int nvecg;         // numero de vectores en el grupo
};

struct analisis     // resultados del analisis de grupos
{
 float mmax, mmin;  // maximo y minimo de las medianas de las probabilidad de cada campo
 int gmax, gmin;    // grupos con los valores maximo y minimo de las medianas
};
