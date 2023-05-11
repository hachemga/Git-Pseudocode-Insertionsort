#ifndef QUEUE_H
#define QUEUE_H

typedef struct _queue_object{
    void* object;
    struct _queue_object* next;
}queue_object;

/**
 * adds a new queue object right behind the very first element in the queue (@param queue remains the
 * head of the queue).
 * 
 * Remember to allocate space for a new queue object and to adjust the pointers of the affected objects.
 * 
 * @param new_object pointer to some object you want to add into the FIFO queue
 * @param queue the pointer to a queue you want to add the obejct to 
 * @return 0 if everything was fine, else return 1
 */
int queue_add(void* new_obejct, queue_object* queue);


/**
 * Gets and deletes the oldest item from the queue.
 * 
 * Remember to properly check for NULL-values before dereferencing pointers and to free removed queue object.
 * Also make sure to adjust pointers correctly.
 * 
 * @param queue The pointer to a queue handler you want to poll from
 * @result pointer to an object that was previously added using function int queue_add or NULL if the queue is empty
 */ 
void* queue_poll(queue_object* queue);


/**
 * Creates and returns a new queue. Should be called before working with the queue.
 * 
 * A new queue consists of a single queue object which is used as a head element.
 * Remember to allocate enough space for any object you need to create.
 * 
 * @result a pointer to a new queue handler.
 */
queue_object* new_queue();


/**
 * Deletes and frees all queue items.
 * 
 * @param queue The pointer to the queue you want to delete
 */
void free_queue(queue_object* queue);


/**
 * Gets the oldest item in the queue, but doesn't delete it.
 * 
 * Remember to properly check for NULL-values before dereferencing pointers.
 * 
 * @param queue Pointer to the queue you want to peek from
 * @result pointer to an object that was previously added using queue add or NULL if the queue is empty
 */
void* queue_peek(queue_object* queue);
#endif //QUEUE_H