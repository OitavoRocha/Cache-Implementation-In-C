/*
        ERROS ENCONTRADOS:
    os resultados do mapeamento direto estao incorretos

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <time.h>

// declaration of global variables for offset, tag and index bits
int nBitsOffset;
int nBitsIndex;
int nBitsTag;

struct _cache {
    int val;
    uint32_t tag;
};
typedef struct _cache * Cache;

void readCommandLine( int * nSets, int * bSize, int * assoc, char * subst, int * flagOut, char * arquivoEntrada, int argc, char const ** argv ); // function to read from the command line
uint32_t reverseAddress( int address );
int randomSwap( int assoc );
void fullyAssociative( int nSets, int bSize, int assoc, char * subst, char * arquivoEntrada, int * accesses, int * hit, int * misses, int * compulsory, int * capacity );
void directMapped( int nSets, int bSize, int assoc, char * arquivoEntrada, int * accesses, int * hit, int * misses, int * compulsory, int * conflict );
void setAssociative( int nSets, int bSize, int assoc, char * subst, char * arquivoEntrada, int * accesses, int * hit, int * misses, int * compulsory, int * capacity, int * conflict );

int main( int argc, char const ** argv ) {
    int nSets = 256, bSize = 4, assoc = 1, flagOut = 1;
    int accesses = 0, hit = 0, misses = 0, compulsory = 0, capacity = 0, conflict = 0;
    float hitRate, missRate, compRate, capRate, confRate;
    char subst[2], arquivoEntrada[50];

    //readCommandLine( &nSets, &bSize, &assoc, subst, &flagOut, arquivoEntrada, argc, argv );

    // calculates the numbers of bits for each one
    nBitsOffset = ( int ) log2( bSize );
    nBitsIndex = ( int ) log2( nSets );
    nBitsTag = 32 - nBitsOffset - nBitsIndex;


    // one function for each mapping
    if ( nSets == 1 && assoc != 1 ) {
        fullyAssociative( nSets, bSize, assoc, subst, arquivoEntrada, &accesses, &hit, &misses, &compulsory, &capacity );
    } else if ( assoc == 1 ) {
        directMapped( nSets, bSize, assoc, arquivoEntrada, &accesses, &hit, &misses, &compulsory, &conflict );
    } else {
        setAssociative( nSets, bSize, assoc, subst, arquivoEntrada, &accesses, &hit, &misses, &compulsory, &capacity, &conflict );
    }


    hitRate = ((float)hit/(float)accesses);
    missRate = ((float)misses/(float)accesses);
    compRate = ((float)compulsory/(float)misses);
    capRate = ((float)capacity/(float)misses);
    confRate = ((float)conflict/(float)misses);


    // printing the results, based on flag
    if ( flagOut == 0 ) {
        printf("+---Resultados---+\n");
        printf("\tNumero de Acessos: %d\n", accesses );
        printf("\tTaxa de Hit: %.2f\n", hitRate );
        printf("\tTaxa de Misses: %.2f\n", missRate );
        printf("\tTaxa de Misses Compulsorios: %.2f\n", compRate );
        printf("\tTaxa de Misses de Conflito: %.2f\n", confRate );
        printf("\tTaxa de Misses de Capacidade: %.2f\n", capRate );
        return 0;
    }

    printf("\n%d, %.4f, %.4f, %.2f, %.2f, %.2f\n", accesses, hitRate, missRate, compRate, capRate, confRate );

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

uint32_t reverseAddress( int address ) {
    /*
    uint32_t mask = 0xf0000000;
    uint32_t reversed = 0;

    reversed |= ((address<<(7*4))&mask);
    reversed |= ((address<<(5*4))&(mask>>(1*4)));
    reversed |= ((address<<(3*4))&(mask>>(2*4)));
    reversed |= ((address<<(1*4))&(mask>>(3*4)));
    reversed |= ((address>>(1*4))&(mask>>(4*4)));
    reversed |= ((address>>(3*4))&(mask>>(5*4)));
    reversed |= ((address>>(5*4))&(mask>>(6*4)));
    reversed |= ((address>>(7*4))&(mask>>(7*4)));

    return reversed;*/
    /**/
    uint32_t mask = 0xff000000;
    uint32_t reversed = 0;

    reversed |= ((address<<(6*4))&mask);
    reversed |= ((address<<(2*4))&(mask>>(2*4)));
    reversed |= ((address>>(2*4))&(mask>>(4*4)));
    reversed |= ((address>>(6*4))&(mask>>(6*4)));

    return reversed;
}

int randomSwap( int assoc ) {
    srand(time(NULL));

    return rand() % assoc;
} 

void directMapped( int nSets, int bSize, int assoc, char * arquivoEntrada, int * accesses, int * hit, int * misses, int * compulsory, int * conflict ) {
    Cache c1 = (Cache)malloc( sizeof( struct _cache ) * (nSets) );
    FILE *p;
    uint32_t end;
    uint32_t read; // variable to read from file
    uint32_t tag;
    uint32_t index;

    for ( int i = 0 ; i < nSets ; i++ ) 
         c1[i].val = 0;
         

    p = fopen( "bin_100.txt" , "r" );
    if ( !p ) {
        perror("Impossivel abrir o arquivo: ");
        exit(1);
    }


    printf("reading file...\n");
    while( fscanf(p, "%008x", &read) == 1 ) {
        end = reverseAddress( read );
        tag = end >> (nBitsOffset + nBitsIndex);
        index = ( end >> (nBitsOffset) ) & ( (int)pow(2, nBitsIndex) - 1 );

        printf("%d %d %d\n", nBitsOffset, nBitsIndex, nBitsTag);

        (*accesses)++;

        if( c1[index].val == 0 ) {
            (*misses)++;
            (*compulsory)++;
            c1[index].val = 1;
            c1[index].tag = tag;
        } else if ( c1[index].tag == tag ) {
            (*hit)++;
        } else {
            (*misses)++;
            (*conflict)++; // in direct mapped architecture there are only conflict and compulsory misses
            c1[index].tag = tag;
        }
    }


    fclose( p );
    free( c1 ); // liberates allocated memory
}

void setAssociative( int nSets, int bSize, int assoc, char * subst, char * arquivoEntrada, int * accesses, int * hit, int * misses, int * compulsory, int * capacity, int * conflict )
{
    FILE *p;
    uint32_t end;
    uint32_t read; // variable to read from file
    uint32_t tag;
    uint32_t index;
    Cache * c1 = (Cache *)malloc( sizeof( Cache ) * nSets );
    int hitFlag;

    for ( int i = 0 ; i < nSets ; i++ )
        c1[i] = (Cache)malloc( sizeof( struct _cache ) * assoc );
    
    for ( int i = 0 ; i < nSets ; i++ )
        for ( int j = 0 ; j < assoc ; j++ )
            c1[i][j].val = 0;


    p = fopen( "bin_10000.txt" , "r" );
    if ( !p ) {
        perror("Impossivel abrir o arquivo: ");
        exit(1);
    }

    printf("reading file...\n");
    while( fscanf(p, "%008x", &read) == 1 ) {
        end = reverseAddress( read );
        tag = end >> (nBitsOffset + nBitsIndex);
        index = ( end >> (nBitsOffset) ) & ( (int)pow(2, nBitsIndex) - 1 );
        hitFlag = 0;

        (*accesses)++;

        for( int i = 0 ; i < assoc ; i++ ) {
            if ( c1[index][i].val != 1 ) {
                (*misses)++;
                (*compulsory)++;
                c1[index][i].val = 1;
                c1[index][i].tag = tag;
                i = assoc; // to end loop
                hitFlag++;
            } else if ( c1[index][i].tag == tag ) {
                (*hit)++;
                i = assoc;
                hitFlag++;
            }
        }

        if ( hitFlag == 0 ) {
            (*misses)++;
            (*conflict)++;
            c1[index][randomSwap( assoc )].tag = tag;
        }
    }
}

void fullyAssociative( int nSets, int bSize, int assoc, char * subst, char * arquivoEntrada, int * accesses, int * hit, int * misses, int * compulsory, int * capacity ){
    Cache c1 = (Cache)malloc( sizeof( struct _cache ) * (assoc) );
    FILE *p;
    uint32_t end;
    uint32_t read; // variable to read from file
    uint32_t tag;
    int isFull = 0;
    int isIn;

    for ( int i = 0 ; i < assoc ; i++ ) 
         c1[i].val = 0;
         

    p = fopen( "vortex.in.sem.persons.txt" , "r" );
    if ( !p ) {
        perror("Impossivel abrir o arquivo: ");
        exit(1);
    }


    printf("reading file...\n");
    while( fscanf(p, "%008x", &read) == 1 ) {
        end = reverseAddress( read );
        tag = end >> (nBitsOffset + nBitsIndex);
        //index = ( end >> (nBitsOffset) ) & ( (int)pow(2, nBitsIndex) - 1 );
        isIn = 0;

        (*accesses)++;

        if ( isFull == 0 ) {
            for ( int i = 0 ; i < assoc ; i++ ) {
                if ( c1[i].val == 0 ) {
                    (*compulsory)++;
                    (*misses)++;
                    c1[i].val = 1;
                    c1[i].tag = tag;
                    i = assoc; // to end loop
                } else if ( c1[i].tag == tag ) {
                    (*hit)++;
                    i = assoc; // to end loop
                }
                if ( i == assoc-1 )
                    isFull = 1;
            }
        } else {
            for ( int i = 0 ; i < assoc ; i++ ) {
                if ( c1[i].tag == tag ) {
                    (*hit)++;
                    isIn++;
                    i = assoc;
                }
            }
            if ( isIn == 0 ) {
                (*capacity)++;  // in fully associative mapping there are only capacity misses
                (*misses)++;
                c1[randomSwap(assoc)].tag = tag;
            }
        }
    }
}
