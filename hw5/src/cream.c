#include "cream.h"
#include "utils.h"
#include "queue.h"
#include "stdio.h"
#include "hashmap.h"
void map_free_function(map_key_t key, map_val_t val) {
    free(key.key_base);
    free(val.val_base);
}

uint32_t jenkins_hash(map_key_t map_key) {
    const uint8_t *key = map_key.key_base;
    size_t length = map_key.key_len;
    size_t i = 0;
    uint32_t hash = 0;

    while (i != length) {
        hash += key[i++];
        hash += hash << 10;
        hash ^= hash >> 6;
    }

    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

int main(int argc, char *argv[]) {
    hashmap_t *map = create_map(101,(void*)jenkins_hash,(void*)map_free_function);
    map_key_t key;
    key.key_base = malloc(sizeof(char)*5);
    *(int*)key.key_base = 1;
    key.key_len = 5*sizeof(char);
    map_val_t val;
    val.val_base = malloc(sizeof(char)*5);
    *(int*)val.val_base = 2;
    val.val_len = 5*sizeof(char);
    put(map,key,val,false);
    delete(map,key);
    invalidate_map(map);
    exit(0);
}
