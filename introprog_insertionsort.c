#include <stdio.h>
#include <stdlib.h>
#include "arrayio.h"

int MAX_LAENGE = 1000;

void insertion_sort(int array[], int len) {
    int key;
    int i;
    for (int j=1; j<len; j++){
        key = array[j];
        i = j-1;
        while (i>=0 && array[i]>key) {
            array [i+1] = array [i];
            i = i-1;
        }
        array [i+1] = key;
    }

    /*
     * Hier Insertionsort implementieren!
     * Diese Funktion soll das Eingabearray nach dem
     * Insertionsort-Verfahren sortieren.
     * Hinweis: Verwende die "in place"-Variante! D.h. der
     * Sortiervorgang soll auf dem originalen Array stattfinden und
     * kein zweites verwendet werden.
     */
}

int main(int argc, char *argv[]) {

    if (argc < 2){
        printf("Aufruf: %s <Dateiname>\n", argv[0]);
        printf("Beispiel: %s zahlen.txt\n", argv[0]);
        exit(1);
    }

    char *filename = argv[1];

    int array[MAX_LAENGE];
    int len = read_array_from_file(array, MAX_LAENGE, filename);

    printf("Unsortiertes Array:");
    print_array(array, len);

    insertion_sort(array, len); 
    /* Aufruf Insertionsort */

    printf("Sortiertes Array:");
    print_array(array, len);

    return 0;
}
