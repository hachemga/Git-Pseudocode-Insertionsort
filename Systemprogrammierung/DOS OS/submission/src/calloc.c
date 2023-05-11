#include "../include/calloc.h"
// Für memset
#include <string.h>

static void * MEM;
static size_t MEM_SIZE;

/* Die aktuelle Position für das Next Fit Verfahren */
static mem_block * last_allocation = NULL;

void my_calloc_init(void * mem, size_t mem_size){
	MEM = mem;
	MEM_SIZE = mem_size;

	/* Initial existiert genau ein mem_block direkt am Anfang des Speichers */
	mem_block * beginning = MEM;

	beginning->next = NULL;
	beginning->prev = NULL;

	/* Der verwaltete Speicher des initialen mem_blocks ist die des
	 * gesamten Speichers minus die Größe seiner eigenen Verwaltungsstruktur
	 * Da sowohl MEM_SIZE ein Vielfaches von 8 ist und sizeof(mem_block) auch
	 * mindestens ein vielfaches von 8 oder mehr ist, ist das LSB
	 * auch direkt 0 -> free.
	 */
	beginning->size = MEM_SIZE - sizeof(mem_block);

	/* Unser Zeiger für das Next Fit Verfahren */
	last_allocation = beginning;
}

/* +------------------------------------+ *
 * | Hier ist Platz für Hilfsfunktionen | *
 * +------------------------------------+ */



/* -------------------------------------- */

void * my_calloc(size_t nmemb, size_t size, int c) {
	// TODO
    mem_block * suche = last_allocation;
    size_t sizeToReserve = size * nmemb;
    if (sizeToReserve == 0){
        return NULL;
    }
    while ((sizeToReserve % 8 ) != 0){
        sizeToReserve += 1;
    }
    while (((suche->size % 8) != 0) || (((suche->size % 8 ) == 0) && (suche->size < (sizeToReserve)))) {
        if (suche->next != NULL){
            suche = suche->next;
        }else {
            suche = MEM;
        }
        if (suche == last_allocation){
            return NULL;
        }
    }

    if (suche->size < (sizeToReserve + sizeof(mem_block) + 8)){ // *sizeof(char)
        memset(suche + 1, c, suche->size);
        suche->size += 1;
        last_allocation = suche;
        return (void*)(suche+1);
    } else{

        mem_block  * newNode = (mem_block *)((char *) suche +sizeof (mem_block)+ sizeToReserve);
        newNode->size = suche->size - sizeToReserve - sizeof(mem_block);
        newNode->next = suche->next;
        newNode->prev = suche;
        suche->next = newNode;
        suche->size = sizeToReserve;
        if (newNode->next != NULL){
            newNode->next->prev = newNode;
        }
        memset(suche + 1, c, suche->size);
        suche->size += 1;
        last_allocation = suche;
        return (void*)(suche+1);

    }
}

void my_free(void *ptr){
	// TODO
    if (ptr == NULL ){
        return;
    }

    mem_block * blFree = (mem_block *) ((char*)(ptr) - sizeof(mem_block));
    blFree->size -= 1 ;
    mem_block *after = blFree->next;
    mem_block *previous = blFree->prev;

    if((blFree->prev != NULL ) &&
       (blFree->next != NULL )){

        if(((blFree->prev->size % 8 ) == 1 ) &&
           ((blFree->next->size % 8) == 1 )){

            return;

        }else {
            if ((blFree->next->size % 8) == 0) {

                blFree->size += sizeof(mem_block) + after->size;
                blFree->next = after->next;
                if (after->next != NULL) {
                    after->next->prev = blFree;
                }
                if (after == last_allocation) {
                    last_allocation = blFree;
                }

            }
            if ((blFree->prev->size % 8) == 0) {

                previous->size += sizeof(mem_block) + blFree->size;
                previous->next = blFree->next;
                if (blFree->next != NULL) {
                    blFree->next->prev = previous;
                }
                if (blFree == last_allocation) {
                    last_allocation = previous;
                }
            }
            return;
        }
    }else if((blFree->prev != NULL)&&(blFree->next == NULL) ){
        if((blFree->prev->size % 8) == 0 ){

            previous->size += sizeof(mem_block) + blFree->size;
            blFree->prev->next = NULL;
            if(blFree == last_allocation){
                last_allocation = previous;
            }

        }
        return;

    }else if((blFree->next != NULL) && (blFree->prev == NULL ) ){
        if((blFree->next->size % 8 ) == 0 ) {

            blFree->size += sizeof(mem_block) + after->size;
            blFree->next = after->next;
            if(after->next != NULL){
                after->next->prev = blFree;
            }
            if (after == last_allocation){
                last_allocation = blFree;
            }
        }
        return;

    } else{
        return;

    }
}
