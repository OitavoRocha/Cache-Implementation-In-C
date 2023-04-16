/*
        ERROS ENCONTRADOS:
    nao esta abrindo o arquivo na função mapeada diretamente

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// declaration of global variables for offset, tag and index bits
int nBitsOffset;
int nBitsIndex;
int nBitsTag;

struct _cache {
    int val;
    int tag;
};
typedef struct _cache * Cache;

void readCommandLine( int * nSets, int * bSize, int * assoc, char * subst, int * flagOut, char * arquivoEntrada, int argc, char const ** argv ); // function to read from the command line
void fullyAssociative( int nSets, int bSize, int assoc, char * subst, char * arquivoEntrada, int * accesses, int * compulsory, int * capacity );
void directMapped( int nSets, int bSize, int assoc, char * arquivoEntrada, int * accesses, int * compulsory, int * conflict );
void setAssociative( int nSets, int bSize, int assoc, char * subst, char * arquivoEntrada, int * accesses, int * compulsory, int * capacity, int * conflict );

int main( int argc, char const ** argv ) {
    int nSets = 256, bSize = 4, assoc = 1, flagOut = 0;
    int accesses = 0, hit, misses, compulsory = 0, capacity = 0, conflict = 0;
    char subst[2], arquivoEntrada[] = "bin_100.bin";

    //readCommandLine( &nSets, &bSize, &assoc, subst, &flagOut, arquivoEntrada, argc, argv );

    // calculates the numbers of bits for each one
    nBitsOffset = ( int ) log2( (double)bSize );
    nBitsIndex = ( int ) log2( (double)nSets );
    nBitsTag = 32 - nBitsOffset - nBitsIndex;

    // one function for each mapping
    if ( nSets == 1 && assoc != 1 )
        return 0;
        //fullyAssociative( nSets, bSize, assoc, subst, arquivoEntrada, &accesses, &compulsory, &capacity );
    else if ( assoc == 1 )
        directMapped( nSets, bSize, assoc, arquivoEntrada, &accesses, &compulsory, &conflict );
    else
        return 0;
        //setAssociative( nSets, bSize, assoc, subst, arquivoEntrada, &accesses, &compulsory, &capacity, &conflict );

    // calculating miss and hit rates
    misses = compulsory + conflict + capacity;
    hit = accesses - misses;

    // printing the results, based on flag
    if ( flagOut == 0 ) {
        printf("+---Resultados---+\n");
        printf("\tNumero de Acessos: %d\n", accesses);
        printf("\tTaxa de Hit: %.2f\n", ((float)hit / (float)accesses) );
        printf("\tTaxa de Misses: %.2f\n", ((float)misses / (float)accesses) );
        printf("\tTaxa de Misses Compulsorios: %.2f\n", ((float)compulsory / (float)accesses) );
        printf("\tTaxa de Misses de Conflito: %.2f\n", ((float)conflict / (float)accesses) );
        printf("\tTaxa de Misses de Capacidade: %.2f\n", ((float)capacity / (float)accesses) );
        return 0;
    }

    printf("\n%d %.2f %.2f %.2f %.2f %.2f\n", accesses, ((float)hit / (float)accesses), ((float)misses / (float)accesses), ((float)compulsory / (float)accesses), ((float)conflict / (float)accesses), ((float)capacity / (float)accesses) );

    return 0;
}

/*
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
*/

void directMapped( int nSets, int bSize, int assoc, char * arquivoEntrada, int * accesses, int * compulsory, int * conflict ) {
    Cache c1 = (Cache)malloc( sizeof( struct _cache ) * ( nSets * assoc ) );
    FILE * p = fopen( arquivoEntrada, "rb" );
    int end;
    int read; // variable to read from file
    int tag;
    int index;

    if ( !p ) {
        printf("Impossivel abrir o arquivo\n");
        exit(1);
    }

    while( fscanf(p, "%d", &read) == 1 ) {
        end = read;
        tag = end >> (nBitsOffset + nBitsIndex);
        index = ( end >> (nBitsTag) ) & ( (int)pow(2, nBitsIndex) - 1 );

        (*accesses)++;

        if( c1[index].val != 1 ) {
            (*compulsory)++;
            c1[index].val = 1;
            c1[index].tag = tag;
        } else {
            (*conflict)++; // in direct mapped architecture there are only conflict and compulsory misses
            c1[index].tag = tag;
        }
        // else if ( c1.tag == tag) hit++ --> when its not a miss, its a hit
    }

    fclose( p );
    free( c1 ); // liberates allocated memory
}