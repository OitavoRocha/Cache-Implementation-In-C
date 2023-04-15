#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// declaration of global variables for offset, tag and index bits
int nBitsOffset;
int nBitsIndex;
int nBitsTag;

struct cache {
    int * val;
    int * tag;
};

void readCommandLine( int * nSets, int * bSize, int * assoc, char * subst, int * flagOut, char * arquivoEntrada, int argc, char const ** argv ); // function to read from the command line

int main( int argc, char const ** argv ) {
    int nSets, bSize, assoc, flagOut;
    char subst[2], arquivoEntrada[50];

    readCommandLine( &nSets, &bSize, &assoc, subst, &flagOut, arquivoEntrada, argc, argv );

    printf("nsets = %d\n", nSets);
	printf("bsize = %d\n", bSize);
	printf("assoc = %d\n", assoc);
	printf("subst = %s\n", subst);
	printf("flagOut = %d\n", flagOut);
	printf("arquivo = %s\n", arquivoEntrada);

    return 0;
}


void readCommandLine( int * nSets, int * bSize, int * assoc, char * subst, int * flagOut, char * arquivoEntrada, int argc, char const ** argv ) {
    if (argc != 7){
		printf("Numero de argumentos incorreto. Utilize:\n");
		printf("./cache_simulator <nsets> <bsize> <assoc> <substituição> <flag_saida> arquivo_de_entrada\n");
		exit(EXIT_FAILURE);
	}

    *nSets = atoi( argv[1] );
    *bSize = atoi( argv[2] );
    *assoc = atoi( argv[3] );
    strcpy( subst, argv[4] );
    *flagOut = atoi( argv[5] );
    strcpy( arquivoEntrada, argv[6] );
}