#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "introprog_input_merge_sort.h"

/*
 * Diese Funktion fügt zwei bereits sortierte Arrays zu einem
 * sortierten Array zusammen
 *
 * array : Pointer auf das Array
 * first : Index des ersten Elemements (Beginn) des (Teil-)Array
 * middle: Index des mittleren Elements des (Teil-)Array
 * last  : Index des Letzten Elements (Ende) des (Teil-)Array
 */
void merge(int* array, int first, int middle, int last)
{
    // HIER Funktion merge() implementieren
    int* halfarray = calloc( last-first+1, sizeof(int) );
    int f = first;
    int m = middle + 1;
    int j = 0;
    while (f <= middle && m <= last) {
        if (array[f] <= array[m]) {
            halfarray[j] = array[f];
            f++;
        } else {
            halfarray[j] = array[m];
            m++;
        }
        j++;
    }
    while (f <= middle) {
        halfarray[j] = array[f];
        f++;
        j++;
    }
    while (m <= last) {
        halfarray[j] = array[m];
        m++;
        j++;
    }

    int i = 0;
    while (i < j) {
        array[first + i] = halfarray[i];
        i++;
    }
    free(halfarray);
}

/*
 * Diese Funktion implementiert den iterativen Mergesort
 * Algorithmus auf einem Array. Sie soll analog zum Pseudocode
 * implementiert werden.
 *
 * array:  Pointer auf das Array
 * first:  Index des ersten Elements
 * last :  Index des letzten Elements
 */
void merge_sort(int* array, int first, int last)
{
    // HIER Funktion merge_sort() implementieren
    int s = 1;
    while (s <= last) {
        int l = 0;
        while (l <= last-s) {
            int m = l + s - 1;
            if (m > last) {
                m = last;
            }
            int r = l + 2*s - 1;
            if (r > last) {
                r = last;
            }
            merge(array, l, m, r);
            l = l + 2*s;
        }
        s = s*2;
    }
}

/*
 * Hauptprogramm.
 *
 * Liest Integerwerte aus einer Datei und gibt diese sortiert im
 * selben Format über die Standardausgabe wieder aus.
 *
 * Aufruf: ./introprog_merge_sort_iterativ <maximale anzahl> \
 * 	   <dateipfad>
 */
int main (int argc, char *argv[])
{
    if (argc!=3){
        printf ("usage: %s <maximale anzahl>  <dateipfad>\n", argv[0]);
        exit(2);
    }
    
    char *filename = argv[2];
    
    // Hier array initialisieren
    int length = atoi(argv[1]);
    int* array = calloc(length, sizeof(int));
    
    int len = read_array_from_file(array, atoi(argv[1]), filename);

    printf("Eingabe:\n");
    print_array(array, len);

    // HIER Aufruf von "merge_sort()"
    merge_sort(array, 0, len-1);

    printf("Sortiert:\n");
    print_array(array, len);

    free(array);

    return 0;
}
