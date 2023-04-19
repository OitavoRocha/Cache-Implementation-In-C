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

void readCommandLine( int * nSets, int * bSize, int * assoc, int * flagOut, char * arquivoEntrada, int argc, char const ** argv ); // function to read from the command line
uint32_t reverseAddress( int address );
void fullyAssociative( int nSets, int bSize, int assoc, char * arquivoEntrada, int * accesses, int * hit, int * misses, int * compulsory, int * capacity );
void directMapped( int nSets, int bSize, int assoc, char * arquivoEntrada, int * accesses, int * hit, int * misses, int * compulsory, int * conflict );
void setAssociative( int nSets, int bSize, int assoc, char * arquivoEntrada, int * accesses, int * hit, int * misses, int * compulsory, int * capacity, int * conflict );

int main( int argc, char const ** argv ) {
    int nSets, bSize, assoc, flagOut;
    int accesses = 0, hit = 0, misses = 0, compulsory = 0, capacity = 0, conflict = 0;
    float hitRate, missRate, compRate, capRate, confRate;
    char arquivoEntrada[50];


    readCommandLine( &nSets, &bSize, &assoc, &flagOut, arquivoEntrada, argc, argv );


    // calculates the numbers of bits for each one
    nBitsOffset = ( int ) log2( bSize );
    nBitsIndex = ( int ) log2( nSets );
    nBitsTag = 32 - nBitsOffset - nBitsIndex;


    // one function for each mapping
    if ( nSets == 1 && assoc != 1 ) {
        fullyAssociative( nSets, bSize, assoc, arquivoEntrada, &accesses, &hit, &misses, &compulsory, &capacity );
    } else if ( assoc == 1 ) {
        directMapped( nSets, bSize, assoc, arquivoEntrada, &accesses, &hit, &misses, &compulsory, &conflict );
    } else {
        setAssociative( nSets, bSize, assoc, arquivoEntrada, &accesses, &hit, &misses, &compulsory, &capacity, &conflict );
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
        printf("\tTaxa de Hit: %.4f\n", hitRate );
        printf("\tTaxa de Misses: %.4f\n", missRate );
        printf("\tTaxa de Misses Compulsorios: %.4f\n", compRate );
        printf("\tTaxa de Misses de Conflito: %.4f\n", confRate );
        printf("\tTaxa de Misses de Capacidade: %.4f\n", capRate );
        return 0;
    }

    printf("\n%d, %.4f, %.4f, %.4f, %.4f, %.4f\n", accesses, hitRate, missRate, compRate, capRate, confRate );

    return 0;
}

void readCommandLine( int * nSets, int * bSize, int * assoc, int * flagOut, char * arquivoEntrada, int argc, char const ** argv ) {
    if (argc != 7){
        printf("Numero de argumentos incorreto. Utilize:\n");
        printf("./cache_simulator <nsets> <bsize> <assoc> <substituição> <flag_saida> arquivo_de_entrada\n");
        exit(EXIT_FAILURE);
    }

    if ( strcmp( argv[4], "R" ) != 0 ) {
        printf("O programa nao aceita politicas de substiuicao diferentes da Random.\nPor favor, tente Novamente.\n");
        exit(EXIT_FAILURE);
    }

    *nSets = atoi( argv[1] );
    *bSize = atoi( argv[2] );
    *assoc = atoi( argv[3] );
    *flagOut = atoi( argv[5] );
    strcpy( arquivoEntrada, argv[6] );
}

uint32_t reverseAddress( int address ) { // OK
    uint32_t mask = 0xff000000;
    uint32_t reversed = 0;

    reversed |= ((address<<(24))&mask);
    reversed |= ((address<<(8))&(mask>>(2*4)));
    reversed |= ((address>>(8))&(mask>>(4*4)));
    reversed |= ((address>>(24))&(mask>>(6*4)));

    return reversed;
}


void directMapped( int nSets, int bSize, int assoc, char * arquivoEntrada, int * accesses, int * hit, int * misses, int * compulsory, int * conflict ) // OK
{
    Cache c1 = (Cache)malloc( sizeof( struct _cache ) * (nSets) );
    FILE *p;
    uint32_t end;
    uint32_t read; // variable to read from file
    uint32_t tag;
    uint32_t index;


    for ( int i = 0 ; i < nSets ; i++ ) 
         c1[i].val = 0;
         

    p = fopen( arquivoEntrada, "rb" );
    if ( !p ) {
        perror("Impossivel abrir o arquivo: ");
        exit(1);
    }
    while( fread(&read, sizeof(int), 1, p ) == 1 ) {
        end = reverseAddress( read );
        tag = end >> (nBitsOffset + nBitsIndex);
        index = ( end >> (nBitsOffset) ) & ( (int)pow(2, nBitsIndex) - 1 );


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

void setAssociative( int nSets, int bSize, int assoc, char * arquivoEntrada, int * accesses, int * hit, int * misses, int * compulsory, int * capacity, int * conflict ) // OK
{
    FILE *p;
    uint32_t end;
    uint32_t read; // variable to read from file
    uint32_t tag;
    uint32_t index;
    Cache * c1 = (Cache *)malloc( sizeof( Cache ) * nSets );
    int hitFlag;
    int isFull = 0;


    srand(time(NULL)); // sets seed for rand to generate random numbers


    for ( int i = 0 ; i < nSets ; i++ )
        c1[i] = (Cache)malloc( sizeof( struct _cache ) * assoc );
    

    for ( int i = 0 ; i < nSets ; i++ )
        for ( int j = 0 ; j < assoc ; j++ ) 
            c1[i][j].val = 0;


    p = fopen( arquivoEntrada , "rb" );
    if ( !p ) {
        perror("Impossivel abrir o arquivo: ");
        exit(1);
    }


    while( fread(&read, sizeof(int), 1, p ) == 1 ) {
        end = reverseAddress( read );
        tag = end >> (nBitsOffset + nBitsIndex);
        index = ( end >> (nBitsOffset) ) & ( (int)pow(2, nBitsIndex) - 1 );
        hitFlag = 0;

        (*accesses)++;

        if ( isFull == 0 ) {
            for( int i = 0 ; i < assoc ; i++ ) {
                if ( c1[(int)index][i].val == 0 ) {
                    (*misses)++;
                    (*compulsory)++;
                    c1[(int)index][i].val = 1;
                    c1[(int)index][i].tag = tag;
                    hitFlag++;
                    i = assoc; // to end loop
                } else if ( c1[(int)index][i].tag == tag ) {
                    (*hit)++;
                    hitFlag++;
                    i = assoc; // to end loop
                }
            }
        
            if ( hitFlag == 0 ) {
                (*misses)++;
                (*conflict)++;
                c1[(int)index][rand() % assoc].tag = tag;
            }
        } else {
            for( int i = 0 ; i < assoc ; i++ ) {
                if ( c1[(int)index][i].tag == tag ) {
                    (*hit)++;
                    hitFlag++;
                    i = assoc; // to end loop
                }
            }
            if ( hitFlag == 0 ){
                (*misses)++;
                (*capacity)++;
                c1[(int)index][rand() % assoc].tag = tag;
            }
        }


        if ( isFull == 0 ) {
            for( int i = 0 ; i<nSets ; i++ )
                for( int j = 0 ; j<assoc ; j++ )
                    if( c1[i][j].val != 0 )
                        isFull++;
        }


        if( isFull != (nSets)*(assoc) ) // while isFull isn`t equal to assoc*nsets, the cache isnt full
            isFull = 0;

    }

    // closing file and liberating allocated memory
    fclose( p );
    for ( int i = 0 ; i<nSets ; i++ )
        free( c1[i] );
    free( c1 );
}

void fullyAssociative( int nSets, int bSize, int assoc, char * arquivoEntrada, int * accesses, int * hit, int * misses, int * compulsory, int * capacity )
{
    Cache c1 = (Cache)malloc( sizeof( struct _cache ) * (assoc) );
    FILE *p;
    uint32_t end;
    uint32_t read; // variable to read from file
    uint32_t tag;
    int isIn;
    int isFull = 0;


    srand(time(NULL)); // sets seed for rand to generate random numbers


    for ( int i = 0 ; i < assoc ; i++ ) 
         c1[i].val = 0;
         

    p = fopen( arquivoEntrada , "rb" );
    if ( !p ) {
        perror("Impossivel abrir o arquivo: ");
        exit(1);
    }


    while( fread(&read, sizeof(int), 1, p ) == 1 ) {
        end = reverseAddress( read );
        tag = end >> (nBitsOffset);
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
            }        
        } else {
            for ( int i = 0 ; i < assoc ; i++ ) {
                if ( c1[i].tag == tag ) {
                    (*hit)++;
                    i = assoc; // to end loop
                    isIn = 1; // to know if the address was already threated
                }
            }

            if ( isIn == 0 ) {
                (*capacity)++;  // in fully associative mapping there are only capacity misses
                (*misses)++;
                c1[rand() % assoc].tag = tag;
            }
        }

        
        if ( isFull == 0 ) 
            for( int i = 0 ; i<assoc ; i++ )
                if ( c1[i].val != 0 )
                    isFull++;
        

        if ( isFull != assoc ) // while isFull isn`t equal to assoc, the cache isnt full
            isFull = 0;
    }

    // closing file and liberating allocated memory
        fclose( p );
        free( c1 );
}
