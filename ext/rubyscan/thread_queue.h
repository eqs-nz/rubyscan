/* thread_queue.h
 * queue type which handles threads
 * written by Alex Fulton
 * Jan 2017
 */
#ifndef RSCAN_THREAD_QUEUE_H
#define RSCAN_THREAD_QUEUE_H

struct thread_queue_member {
    struct thread_queue_member *next;
    pthread_t *thread;
}

struct thread_queue {
    pthread_mutex_t lock;
    pthread_cond_t ready;
    pthread_cond_t empty;
    int open;
    struct thread_queue_member *head;
    struct thread_queue_member *tail;
}

VALUE rscan_thread_queue_define(VALUE root);
VALUE rscan_class_thread_queue();

#define Get_Thread_Queue(value,pointer) \
        Data_Get_Struct(value,rscan_thread_queue_t,pointer)
#endif
