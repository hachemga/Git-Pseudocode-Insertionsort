#include "hash_table.h"

void htable_set(htable **ht, const unsigned char *key, size_t key_len,
                const unsigned char *value, size_t value_len) {
    /* TODO IMPLEMENT */
    struct htable* s;
    HASH_FIND(hh, *ht, key, key_len, s);
    if (s) {
        free(s->value);
        s->value = malloc(sizeof(unsigned char)*value_len);
        s->value_len = value_len;
        memcpy(s->value, value, value_len);
    }else {
        struct htable* new_el = malloc(sizeof(struct htable));
        new_el->value = malloc(sizeof(unsigned char)*value_len);
        new_el->key = malloc(sizeof(unsigned char)*key_len);
        new_el->key_len = key_len;
        new_el->value_len = value_len;
        memcpy(new_el->key, key, key_len);
        memcpy(new_el->value, value, value_len);

        HASH_ADD_KEYPTR(hh, *ht, new_el->key, new_el->key_len, new_el);
    }
}

htable *htable_get(htable **ht, const unsigned char *key, size_t key_len) {
    /* TODO IMPLEMENT */
    struct htable* s;
    HASH_FIND(hh, ht[0], key, key_len, s);
    return s;
}

int htable_delete(htable **ht, const unsigned char *key, size_t key_len) {
    /* TODO IMPLEMENT */
    struct htable* s;
    HASH_FIND(hh, ht[0], key, key_len, s);
    if (s){
        HASH_DEL(ht[0], s);
        free(s);
        return 0;
    }else{
        return -1;
    }

}
