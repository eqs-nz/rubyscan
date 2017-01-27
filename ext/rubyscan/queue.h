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
};

struct queue {
    pthread_mutex_t lock;
    pthread_cond_t ready;
    pthread_cond_t empty;
    int open;
    unsigned int length;
    struct queue_member *head;
    struct queue_member *tail;
};

#define RSCAN_QUEUE_SUCCESS 0
#define RSCAN_QUEUE_FAIL_LOCKBUSY 1
#define RSCAN_QUEUE_FAIL_EMPTY 2
#define RSCAN_QUEUE_FAIL_CLOSED 3
#define RSCAN_QUEUE_FAIL_INTERRUPT 4
#define RSCAN_QUEUE_FAIL_BAD_INDEX 5
#define RSCAN_QUEUE_FAIL_UNDEF ~0

/* non-blocking queue requests still wait for the object lock by default */
#define RSCAN_QUEUE_LOCKWAIT_DEFAULT 1

#define Queue_Empty(q) ((q)->head == NULL)
#define Queue_Nonempty(q) (!Queue_Empty((q)))

            VALUE   rscan_queue_define(VALUE root);
            VALUE   rscan_queue_alloc(VALUE self);
            VALUE   rscan_class_queue();

            void    rscan_queue_close(struct queue *queue);
rscan_queue_err_t   rscan_queue_closed_p(struct queue *queue);
            void    rscan_queue_open(struct queue *queue);
rscan_queue_err_t   rscan_queue_open_p(struct queue *queue);
            void    rscan_queue_drop(struct queue *queue);
            void    rscan_queue_drop_all(struct queue *queue);
            void    rscan_queue_put(struct queue *queue, void *data);
            void*   rscan_queue_shift(struct queue *queue);
rscan_queue_err_t   rscan_queue_shift_ptr(struct queue *queue, void **val);
rscan_queue_err_t   rscan_queue_shift_nogvlwait(struct queue *queue, void **out);
#define             rscan_queue_shift_nonblock(queue,val) \
            rscan_queue_shift_nonblock_opt((queue),RSCAN_QUEUE_LOCKWAIT_DEFAULT,(val))
rscan_queue_err_t   rscan_queue_shift_nonblock_opt(struct queue *queue, int bWaitForLock, void **val);
            void*   rscan_queue_peek(struct queue *queue, unsigned int i);
rscan_queue_err_t   rscan_queue_peek_ptr(struct queue *queue, unsigned int i, void **val);
rscan_queue_err_t   rscan_queue_peek_nogvlwait(struct queue *queue, unsigned int i, void **out);
#define             rscan_queue_peek_nonblock(queue, val, i) \
            rscan_queue_peek_nonblock_opt((queue),RSCAN_QUEUE_LOCKWAIT_DEFAULT, (i), (val))
rscan_queue_err_t   rscan_queue_peek_nonblock_opt(struct queue *queue, int bWaitForLock, unsigned int i, void **val);
            void    rscan_queue_destroy(struct queue *queue);
            void    rscan_queue_wait_until_empty(struct queue *queue);

#define Get_Queue(value,pointer) \
        Data_Get_Struct(value,struct queue,pointer)
#endif
