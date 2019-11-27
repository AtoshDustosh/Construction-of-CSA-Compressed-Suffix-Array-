#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "SimpleTest.h"
#include "HelperFunction.h"
#include "SABuildFunc.h"
#include "FileOperation.h"
#include "BasicStep.h"
#include "MergeStep.h"
#define LINELENGTH 70

/*
 * Global variables.
 */
char* FILEPATH = "testdata_2.txt";   // file path
long ARRAYLENGTH = 0; // length of T ~ n
long PARTLENGTH = 0; // part length of T ~ l
long PARTNUM = 0; // number of parts ~ ceil(n/l)

char* T = NULL; // DNA sequence of (A,C,G,T) plus a '$'
long* SA = NULL; // SA of T
long* SA_inverse = NULL; // inverse of SA
long* Psi = NULL; // Psi of T - the compressed suffix array

/*
 * Functions.
 */
void directlyConstruction();
void testSet();
void readAndPrint();

int main() {
    long i = 0;

    ARRAYLENGTH = fnaDataSize(FILEPATH);    // get length of DNA sequence in the fnaFile
    ARRAYLENGTH = ARRAYLENGTH + 1; // get ready to add character '$' to the end of the DNA sequence
    printf("DNA (plus a \'$\') sequence length: %ld\n", ARRAYLENGTH);
    PARTLENGTH = ARRAYLENGTH / log2(ARRAYLENGTH);   // length of a part (an increment)
    PARTNUM = (long)ceil((double)ARRAYLENGTH / PARTLENGTH);
    printf("PartLength: %ld, PartNum: %ld\n", PARTLENGTH, PARTNUM);

    // cannot apply for memory in a function's stack, because memory applied there will be recycled.
    T = (char*)malloc(sizeof(char) * ARRAYLENGTH);
    SA = (long*)malloc(sizeof(long) * ARRAYLENGTH);
    SA_inverse = (long*)malloc(sizeof(long) * ARRAYLENGTH);
    Psi = (long*)malloc(sizeof(long) * ARRAYLENGTH);

    i = PARTNUM;
    baseStep(FILEPATH, T, SA, SA_inverse, Psi, ARRAYLENGTH, PARTLENGTH, PARTNUM);

    for(i = PARTNUM - 1; i > 0; i--) {
        printf("\nincrement part (%ld)\n", i);
        long partIndex = i; // T_i and T_apostrophe is stored using partIndex
        long* order = (long*)malloc(sizeof(long) * PARTLENGTH);
        // sorted suffixes are stored in SA[startIndex_i]...[startIndex_apostrophe]

        mergeStepA(T, SA, ARRAYLENGTH, PARTLENGTH, PARTNUM, partIndex);
        mergeStepB(T, SA, Psi, ARRAYLENGTH, PARTLENGTH, PARTNUM, partIndex, order);
        mergeStepC(T, SA, Psi, ARRAYLENGTH, PARTLENGTH, PARTNUM, partIndex, order);
    }

    /**
     * \TODO strange bug - if I put this func before the first func in main() which is fnaDataSize(...)
     *  the result of func g[] in mergeStepC(...) will get wrong ....www('A')www
     */
    testSet();

    return 0;
}



////////////////////////////////// Actually Working Funcs //////////////////////////////////




/**
 * This is simply a debug function, mainly used when developing this program.
 * Or if you just want to see the effect or progress of some function, you can disable the comment mark,
 * and execute this function.
 */
void testSet() {

    int i = 0;
    for(i = 0; i < 20; i++) {
        printf("\n");
    }

//    _CLanguageReview();
//    _mathematicalFuncsTest();
//    _mathematicalFuncsTest();
//    _mathematicalFuncsTest();
//    _quickSortTest();
//    _myStrLengthTest();
    _suffixArrayQuickSortTest();
//    _compareSuffixTest();
//    _inverseSAWholeTest();
//    _psiArrayBuildWholeTest();
//    _lowerCaseTest();
//    _binarySearchBoundTest();
//    _CSABinaryBoundSearchTest();
    _CSABinarySearchOrderValueTest();
    _fgpsiFuncTest();
//
//    readAndPrint();
    directlyConstruction();

    for(i = 0; i < 20; i++) {
        printf("\n");
    }
}

/////////////////////////////////// Test Functions Below ///////////////////////////////////

/**
 * Test steps of construction of CSA - directly build.
 *
 * <note> bug detected - array directly defined cannot be too big.
 * To solve this problem, use (long*)malloc(sizeof(long)*ARRAYLENGTH).
 */
void directlyConstruction() {
    ARRAYLENGTH = fnaDataSize(FILEPATH);
    printf("data length: %ld\n", ARRAYLENGTH);

    // build T[] - DNA sequence array
    ARRAYLENGTH++; // get ready to add character '$' to the end of the DNA sequence
    char* temp = (char*)malloc(sizeof(char) * ARRAYLENGTH);
    T = temp;
    loadFnaData(FILEPATH, ARRAYLENGTH, temp);

    printf("DNA sequence - T[]: \n");
    long i = 0;
    for(i = 0; i < ARRAYLENGTH; i++) {
        printf("%c", T[i]);
    }
    printf("\n");

    // build SA[] - suffix array
    long* SA = (long*)malloc(sizeof(long) * ARRAYLENGTH);
    for(i = 0; i < ARRAYLENGTH; i++) {
        SA[i] = i;
    }
    suffixArrayQuickSort(SA, T, 0, ARRAYLENGTH - 1);
    printf("Suffix array - SA[]: \n");
    for(i = 0; i < ARRAYLENGTH; i++) {
        printf("%ld\t%ld\t%c\n", i, SA[i], T[SA[i]]);
    }
    printf("\n");

    // build Psi[] - ... Psi array (I don't know how to describe it)
    long* SA_inverse = (long*)malloc(sizeof(long) * ARRAYLENGTH);
    long* Psi = (long*)malloc(sizeof(long) * ARRAYLENGTH);
    printf("Inverse suffix array - SA_inverse[]\n");
    inverseSAWhole(SA, SA_inverse, ARRAYLENGTH);
    printf("Psi array - Psi[]: \n");
    psiArrayBuildWhole(SA, SA_inverse, Psi, ARRAYLENGTH);
    printf("i\tPsi[]\tT[SA[]]\n");
    for(i = 0; i < ARRAYLENGTH; i++) {
        printf("%ld\t%ld\t%c\n", i, Psi[i], T[SA[i]]);
    }
    printf("\n");

    free(T);
    free(SA);
    free(SA_inverse);
    free(Psi);
}

/**
 * Read all data in ?.fna file and print to console. (.fna file with specific data line length)
 */
void readAndPrint() {
    FILE* fp = fopen(FILEPATH, "r");
    int dataPart = 0;
    ARRAYLENGTH = 0;
    if(fp != NULL) {
        char ch = fgetc(fp);
        char buffer[LINELENGTH];
        int bufCount = 0;
        while(ch != EOF) {
            if(ch == '\n' && !dataPart) {
                dataPart = 1;
                printf(" #(... get into data part)");
            }
            if(dataPart && ch != '\n') {
//                printf("store character: %c\n", ch);
                buffer[bufCount++] = ch;
                ARRAYLENGTH++;
            } else {
                printf("%c", ch);
            }
            if(bufCount != 0 && (bufCount % LINELENGTH) == 0) {
                // print buffer content when buffer is full and reset the bufPointer
                printf("%s", buffer);
                bufCount = 0;
            }
            ch = fgetc(fp);
        }
    } else {
        printf("failed to open file %s", FILEPATH);
    }
    printf("dataLength: %ld\n", ARRAYLENGTH);
}



