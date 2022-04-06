#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "introprog_input_merge_sort.h"

/*
 * Diese Funktion fügt zwei bereits sortierte Arrays zu einem
 * sortierten Array zusammen
 *
 * array : Pointer auf das Array
 * first : Index des ersten Elements (Beginn) des (Teil-)Array
 * middle: Index des mittleren Elements des (Teil-)Array
 * last  : Index des letzten Elements(Ende) des (Teil-)Array
 */
void merge(int* array, int first, int middle, int last)
{
  // HIER Funktion merge() implementieren
  int* halfarray = calloc( last-first+1, sizeof(int) );
  int k = first;
  int m = middle + 1;
  int i = 0;
  while (k <= middle && m <= last) {
    if (array[k] <= array[m]) {
      halfarray[i] = array[k];
      k++;
    } else {
      halfarray[i] = array[m];
      m++;
    }
    i++;
  }

  while (k <= middle) {
    halfarray[i] = array[k];
    k++;
    i++;
  }

  while (m <= last) {
    halfarray[i] = array[m];
    m++;
    i++;
  }

  int j = 0;
  while (j < i) {
    array[first + j] = halfarray[j];
    j++;
  }
  free(halfarray);
}

/*
 * Diese Funktion implementiert den rekursiven Mergesort
 * Algorithmus auf einem Array. Sie soll analog zum Pseudocode
 * implementiert werden.
 *
 * array: Pointer auf das Array
 * first: Index des ersten Elements des (Teil-)Array
 * last:  Index des letzten Elements des (Teil-)Array
 */
void merge_sort(int* array, int first, int last)
{
  // HIER Funktion merge_sort() implementieren
  if (first < last) {
    int q = (int) ((first+last)/2);
    merge_sort(array, first, q);
    merge_sort(array, q+1, last);
    merge(array, first, q, last);
  }
}

/*
 * Hauptprogramm.
 *
 * Liest Integerwerte aus einer Datei und gibt diese sortiert im
 * selben Format über die Standardausgabe wieder aus.
 *
 * Aufruf: ./introprog_merge_sort_rekursiv <maximale anzahl> \
 *         <dateipfad>
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
