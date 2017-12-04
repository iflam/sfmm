#include "utils.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>

#define MAP_KEY(base, len) (map_key_t) {.key_base = base, .key_len = len}
#define MAP_VAL(base, len) (map_val_t) {.val_base = base, .val_len = len}
#define MAP_NODE(key_arg, val_arg, tombstone_arg) (map_node_t) {.key = key_arg, .val = val_arg, .tombstone = tombstone_arg}

/*
typedef struct map_node_t {
    map_key_t key;
    map_val_t val;
    bool tombstone;
} map_node_t;

typedef struct hashmap_t {
    uint32_t capacity;
    uint32_t size;
    map_node_t *nodes;
    hash_func_f hash_function;
    destructor_f destroy_function;
    int num_readers;
    pthread_mutex_t write_lock;
    pthread_mutex_t fields_lock;
    bool invalid;
} hashmap_t;
*/

bool canPut(map_node_t node, map_node_t compare){
    if(node.tombstone){
        return true;
    }
    if((node.key).key_len == 0 && (node.val).val_len == 0){
        return true;
    }
    if(node.key.key_len == compare.key.key_len){
        if(memcmp(node.key.key_base, compare.key.key_base, node.key.key_len) == 0){
            return true;
        }
    }
    return false;
}

bool isFull(hashmap_t *map){
    for(int i = 0; i < map->capacity; i++){
        if(map->nodes[i].key.key_len == 0){
            return false;
        }
    }
    return true;
}

bool hasVal(map_node_t node){
    if(node.val.val_base == NULL){
        return false;
    }
    return true;
}
hashmap_t *create_map(uint32_t capacity, hash_func_f hash_function, destructor_f destroy_function) {
    hashmap_t *new_map;
    if((new_map = (hashmap_t*)calloc(1,sizeof(hashmap_t))) == NULL){
        return NULL;
    }
    new_map->capacity = capacity;
    new_map->nodes = (map_node_t*)calloc(capacity,sizeof(map_node_t));
    new_map->destroy_function = destroy_function;
    new_map->hash_function = hash_function;
    if(pthread_mutex_init(&new_map->write_lock,NULL) != 0 || pthread_mutex_init(&new_map->fields_lock,NULL) != 0){
        return NULL;
    }
    return new_map;
}

bool put(hashmap_t *self, map_key_t key, map_val_t val, bool force) {
    // pthread_mutex_lock(&self->write_lock);
    if(!self ||!key.key_base||
        !val.val_base|| key.key_len == 0 || val.val_len == 0){
        errno = EINVAL;
        return false;
    }
    map_node_t new_node;
    new_node.key = key;
    new_node.val = val;
    new_node.tombstone = false;
    pthread_mutex_lock(&self->write_lock);
    if(self->invalid){
        errno = EINVAL;
        pthread_mutex_unlock(&self->write_lock);
        return false;
    }
    int hash_index = get_index(self,key);
    int initial_index = hash_index;
    if(!isFull(self)){
        while(!canPut(self->nodes[hash_index],new_node)){
            //while self->nodes[hash_index] is not empty...
            if(hash_index == self->capacity-1){
                hash_index = -1;
            }
            hash_index++;
            if(hash_index == initial_index){
                pthread_mutex_unlock(&self->write_lock);
                return false;
            }
        }
    }
    else{
        if(!force){
            errno = ENOMEM;
            pthread_mutex_unlock(&self->write_lock);
            return false;
        }
    }
    if(hasVal(self->nodes[hash_index])){
        self->destroy_function(self->nodes[hash_index].key,self->nodes[hash_index].val);
        self->size--;
    }
    self->nodes[hash_index].key = key;
    self->nodes[hash_index].val = val;
    self->size++;
    pthread_mutex_unlock(&self->write_lock);
    return true;
}

map_val_t get(hashmap_t *self, map_key_t key) {
    pthread_mutex_lock(&self->fields_lock);
    self->num_readers++;
    if(self->num_readers == 1){
        pthread_mutex_lock(&self->write_lock);
    }
    pthread_mutex_unlock(&self->fields_lock);

    if(!self || !key.key_base || key.key_len == 0){
        errno = EINVAL;
        pthread_mutex_lock(&self->fields_lock);
        self->num_readers--;
        if(self->num_readers == 0){
            pthread_mutex_unlock(&self->write_lock);
        }
        pthread_mutex_unlock(&self->fields_lock);
        return MAP_VAL(NULL,0);
    }
    int hash_index = get_index(self,key);
    int initial_index = hash_index;
    if(self->invalid){
        errno = EINVAL;
        pthread_mutex_lock(&self->fields_lock);
        self->num_readers--;
        if(self->num_readers == 0){
            pthread_mutex_unlock(&self->write_lock);
        }
        pthread_mutex_unlock(&self->fields_lock);
        return MAP_VAL(NULL,0);
    }
    while(self->nodes[hash_index].key.key_len != 0){
        if(!self->nodes[hash_index].tombstone){
            if(self->nodes[hash_index].key.key_len == key.key_len){
                if(memcmp(self->nodes[hash_index].key.key_base,key.key_base,key.key_len) == 0){
                    pthread_mutex_lock(&self->fields_lock);
                    self->num_readers--;
                    if(self->num_readers == 0){
                        pthread_mutex_unlock(&self->write_lock);
                    }
                    pthread_mutex_unlock(&self->fields_lock);
                    return self->nodes[hash_index].val;
                }
            }
        }
        if(hash_index == self->capacity-1){
                hash_index = -1;
        }
        hash_index++;
        if(initial_index == hash_index)
            break;
    }
    pthread_mutex_lock(&self->fields_lock);
    self->num_readers--;
    if(self->num_readers == 0){
        pthread_mutex_unlock(&self->write_lock);
    }
    pthread_mutex_unlock(&self->fields_lock);
    return MAP_VAL(NULL, 0);
}

map_node_t delete(hashmap_t *self, map_key_t key) {
    if(!self || !key.key_base || key.key_len == 0){
        errno = EINVAL;
        return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
    }
    int hash_index = get_index(self,key);
    int initial_index = hash_index;
    pthread_mutex_lock(&self->write_lock);
    while(self->nodes[hash_index].key.key_base){
        if(!self->nodes[hash_index].tombstone && self->nodes[hash_index].key.key_len == key.key_len){
            if(memcmp(self->nodes[hash_index].key.key_base,key.key_base,key.key_len) == 0){
                self->nodes[hash_index].tombstone = true;
                self->size--;
                // printf("%i\n",*(int*)key.key_base);
                pthread_mutex_unlock(&self->write_lock);
                return self->nodes[hash_index];
            }
        }
        hash_index = (hash_index + 1)%self->capacity;
        if(initial_index == hash_index)
            break;
    }
    pthread_mutex_unlock(&self->write_lock);
    errno = EINVAL;
    return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
}

bool clear_map(hashmap_t *self) {
    if(!self){
        errno = EINVAL;
        return false;
    }
    pthread_mutex_lock(&self->write_lock);
    if(self->invalid){
        errno = EINVAL;
        pthread_mutex_unlock(&self->write_lock);
        return false;
    }
    for(int i = 0; i < self->capacity; i++){
        if(self->nodes[i].key.key_base){
            self->destroy_function(self->nodes[i].key,self->nodes[i].val);
        }
    }
    memset(self->nodes,0,self->capacity*sizeof(map_node_t));
    self->size = 0;
    pthread_mutex_unlock(&self->write_lock);
	return true;
}

bool invalidate_map(hashmap_t *self) {
    if(!self){
        errno = EINVAL;
        return false;
    }
    pthread_mutex_lock(&self->write_lock);
    if(self->invalid){
        errno = EINVAL;
        pthread_mutex_unlock(&self->write_lock);
        return false;
    }
    for(int i = 0; i < self->capacity-1; i++){
        if(self->nodes[i].key.key_base){
            self->destroy_function(self->nodes[i].key,self->nodes[i].val);
        }
    }
    free(self->nodes);
    self->invalid = true;
    pthread_mutex_unlock(&self->write_lock);
    return true;
}
