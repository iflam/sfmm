#include "cream.h"
#include "utils.h"
#include "queue.h"
#include "stdio.h"

int main(int argc, char *argv[]) {
    queue_t *test = create_queue();
    void* i = "hello";
    void* k = "goodbye";
    enqueue(test,i);
    enqueue(test,k);
    dequeue(test);
    dequeue(test);
    dequeue(test);
    printf("%p",test);
    exit(0);
}
