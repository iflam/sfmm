#include "cream.h"
#include "utils.h"
#include "queue.h"
#include "stdio.h"
#include "hashmap.h"
#include "string.h"
#include <sys/socket.h>
#include "csapp.h"
#include "errno.h"

queue_t *queue;
hashmap_t *hashmap;
static sem_t mutex;

void *thread(void *vargp);
void handle_request(int connfd);
void map_free_function(map_key_t key, map_val_t val);


int main(int argc, char *argv[]) {
    signal(SIGPIPE,SIG_IGN);
    int num_workers;
    int max_items;
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;
    bool badArgs = false;
    if(argc < 2){
        printf("Incorrect number of arguments.\n");
        exit(EXIT_FAILURE);
    }
    if(strcmp(argv[1],"-h") == 0){
        printf("%s\n",help_message);
        exit(EXIT_SUCCESS);
    }
    else if(argc == 4){
        if(strcmp(argv[1],"-h") == 0){
            puts("Incorrect number of arguments.\n");
            exit(EXIT_FAILURE);
        }
        char* end;
        num_workers = strtol(argv[1],&end,10);
        if(strcmp(end,"\0") != 0){
            badArgs = true;
        }
        if(num_workers < 1)
            badArgs = true;
        // strtol(argv[2],&end,10);
        // if(strcmp(end,"\0") != 0){
        //     badArgs = true;
        // }
        max_items = strtol(argv[3],&end,10);
        if(strcmp(end,"\0") != 0){
            badArgs = true;
        }
        if(max_items < 1){
            badArgs = true;
        }
    }
    else{
        badArgs = true;
    }
    if(badArgs){
        printf("Incorrect arguments.\n");
        exit(EXIT_FAILURE);
    }
    listenfd = Open_listenfd(argv[2]);
    queue = create_queue();
    hashmap = create_map(max_items,jenkins_one_at_a_time_hash,map_free_function);
    for(int i = 0; i < num_workers; i++){
        pthread_create(&tid,NULL,thread,NULL);
    }
    while(1){
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd,(SA*)&clientaddr,&clientlen);
        int *item = malloc(sizeof(connfd));
        *item = connfd;
        enqueue(queue, (void*)item);
    }
    exit(0);
}

void *thread(void *vargp){
    pthread_detach(pthread_self());
    while(1){
        int connfd = *(int*)dequeue(queue);
        handle_request(connfd);
        close(connfd);
    }
}

static void init_echo_cnt(void){
    Sem_init(&mutex, 0, 1);
}

void handle_request(int connfd){
    static pthread_once_t once = PTHREAD_ONCE_INIT;
    pthread_once(&once, init_echo_cnt);
    P(&mutex);
    request_header_t *header = malloc(sizeof(request_header_t));
    Rio_readn(connfd,header,sizeof(request_header_t));
    response_header_t *response = malloc(sizeof(response_header_t));
    switch(header->request_code){
        case PUT:;
            if(!(header->key_size >= MIN_KEY_SIZE && header->key_size <= MAX_KEY_SIZE && header->value_size >= MIN_VALUE_SIZE && header->value_size <= MAX_VALUE_SIZE)){
                response->response_code = BAD_REQUEST;
                response->value_size = 0;
                goto write_put;
            }
            map_key_t *key = malloc(sizeof(map_key_t));
            map_val_t *val = malloc(sizeof(map_val_t));
            key->key_base = malloc(sizeof(header->key_size));
            val->val_base = malloc(sizeof(header->value_size));
            Rio_readn(connfd,key->key_base,header->key_size);
            key->key_len = header->key_size;
            Rio_readn(connfd,val->val_base,header->value_size);
            val->val_len = header->value_size;
            if(isFull(hashmap)){
                if(put(hashmap,*key,*val,true)){
                    response->response_code = OK;
                    response->value_size = 0;
                }
                else{
                    response->response_code = BAD_REQUEST;
                    response->value_size = 0;
                }
            }
            else{
                if(put(hashmap,*key,*val,false)){
                    response->response_code = OK;
                    response->value_size = 0;
                }
                else{
                    response->response_code = BAD_REQUEST;
                    response->value_size = 0;
                }
            }
            write_put:
            if(errno == EPIPE)
                goto skip;
            Rio_writen(connfd,response,sizeof(response));
            break;
        case GET:;
            if(header->key_size < MIN_KEY_SIZE || header->key_size > MAX_KEY_SIZE){
                response->response_code = BAD_REQUEST;
                response->value_size = 0;
                if(errno == EPIPE)
                    goto skip;
                Rio_writen(connfd,response,sizeof(response));
                goto end;
            }
            map_key_t *get_key = malloc(sizeof(map_key_t));
            get_key->key_base = malloc(sizeof(header->key_size));
            Rio_readn(connfd,get_key->key_base,header->key_size);
            get_key->key_len = header->key_size;
            map_val_t rval = get(hashmap,*get_key);
            if(!rval.val_base){
                //ERROR
                response->response_code = NOT_FOUND;
                response->value_size = 0;
                if(errno == EPIPE)
                    goto skip;
                Rio_writen(connfd,response,sizeof(response));
            }
            else{
                response->response_code = OK;
                response->value_size = rval.val_len;
                if(errno == EPIPE)
                    goto skip;
                Rio_writen(connfd,response,sizeof(response));
                if(errno == EPIPE)
                    goto skip;
                Rio_writen(connfd,rval.val_base,rval.val_len);
            }
            free(get_key->key_base);
            free(get_key);
            end:
            break;
        case EVICT:
            if(header->key_size < MIN_KEY_SIZE || header->key_size > MAX_KEY_SIZE){
                response->response_code = BAD_REQUEST;
                response->value_size = 0;
                goto write_evict;
            }
            map_key_t *evict_key = malloc(sizeof(map_key_t));
            evict_key->key_base = malloc(sizeof(header->key_size));
            Rio_readn(connfd,evict_key->key_base,header->key_size);
            evict_key->key_len = header->key_size;
            delete(hashmap,*evict_key);
            response->response_code = OK;
            response->value_size = 0;
            write_evict:
            if(errno == EPIPE)
                goto skip;
            Rio_writen(connfd,response,sizeof(response));
            free(evict_key->key_base);
            free(evict_key);
            break;
        case CLEAR:
            clear_map(hashmap);
            response->response_code = OK;
            response->value_size = 0;
            if(errno == EPIPE)
                goto skip;
            Rio_writen(connfd,response,sizeof(response));
            break;
        default:
            response->response_code = UNSUPPORTED;
            response->value_size = 0;
            if(errno == EPIPE)
                goto skip;
            Rio_writen(connfd, response, sizeof(response));
            break;
    }
    skip:
    free(response);
    free(header);
    V(&mutex);
    return;
}

void map_free_function(map_key_t key, map_val_t val) {
    free(key.key_base);
    free(val.val_base);
}
