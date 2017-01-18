/* queue.h
 * threadsafe queue for Ruby
 * written by Alex Fulton
 * Sept 2016
 */
#ifndef RSCAN_QUEUE_H
#define RSCAN_QUEUE_H

struct queue_member {
    struct queue_member *next;
    void *data;
}

typedef struct queue {
    pthread_mutex_t lock;
    pthread_cond_t ready;
    pthread_cond_t empty;
    int open;
    struct queue_member *head;
    struct queue_member *tail;
} rscan_queue_t;

typedef unsigned int rscan_queue_err_t;

#define RSCAN_QUEUE_SUCCESS 0
#define RSCAN_QUEUE_FAIL_LOCKBUSY 1
#define RSCAN_QUEUE_FAIL_EMPTY 2
#define RSCAN_QUEUE_FAIL_CLOSED 3
#define RSCAN_QUEUE_FAIL_INTERRUPT 4
#define RSCAN_QUEUE_FAIL_UNDEF 5

/* non-blocking queue requests still wait for the object lock by default */
#define RSCAN_QUEUE_LOCKWAIT_DEFAULT 1

#define Queue_Empty(q) ((q)->head == NULL)
#define Queue_Nonempty(q) (!Queue_Empty((q)))

            VALUE   rscan_queue_define(VALUE root);
            VALUE   rscan_class_queue();

            void    rscan_queue_close(rscan_queue_t *queue);
rscan_queue_err_t   rscan_queue_closed_p(rscan_queue_t *queue);
            void    rscan_queue_open(rscan_queue_t *queue);
rscan_queue_err_t   rscan_queue_open_p(rscan_queue_t *queue);
            void    rscan_queue_drop(rscan_queue_t *queue);
            void    rscan_queue_drop_all(rscan_queue_t *queue);
            void    rscan_queue_put(rscan_queue_t *queue, void *pMatch);
            void*   rscan_queue_shift(rscan_queue_t *queue);
rscan_queue_err_t   rscan_queue_shift_ptr(rscan_queue_t *queue, void **val);
rscan_queue_err_t   rscan_queue_shift_nogvlwait(rscan_queue_t *queue, void **out);
#define             rscan_queue_shift_nonblock(queue,val) \
            rscan_queue_shift_nonblock_opt((queue),RSCAN_QUEUE_LOCKWAIT_DEFAULT,(val))
rscan_queue_err_t   rscan_queue_shift_nonblock_opt(rscan_queue_t *queue, int bWaitForLock, void **val);
            void*   rscan_queue_peek(rscan_queue_t *queue, int i);
rscan_queue_err_t   rscan_queue_peek_ptr(rscan_queue_t *queue, int i, void **val);
rscan_queue_err_t   rscan_queue_peek_nogvlwait(rscan_queue_t *queue, int i, void **out);
#define             rscan_queue_peek_nonblock(queue, val, i) \
            rscan_queue_peek_nonblock_opt((queue),RSCAN_QUEUE_LOCKWAIT_DEFAULT, (i), (val))
rscan_queue_err_t   rscan_queue_peek_nonblock_opt(rscan_queue_t *queue, int bWaitForLock, int i, void **val);
            void    rscan_queue_destroy(rscan_queue_t *queue);
            void    rscan_queue_wait_until_empty(rscan_queue_t *queue);

#define Get_Queue(value,pointer) \
        Data_Get_Struct(value,rscan_queue_t,pointer)
#endif
