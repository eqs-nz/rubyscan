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
};

struct thread_queue {
    pthread_mutex_t lock;
    pthread_cond_t ready;
    pthread_cond_t empty;
    int open;
    unsigned int length;
    struct thread_queue_member *head;
    struct thread_queue_member *tail;
};

VALUE rscan_thread_queue_define(VALUE root);
VALUE rscan_class_thread_queue();


#define rscan_thread_queue_close(queue) \
            rscan_queue_close((struct queue *)(queue))
            
#define rscan_thread_queue_closed_p(queue) \
            rscan_queue_closed_p((struct queue *)(queue))

#define rscan_thread_queue_open(queue) \
            rscan_queue_open((struct queue *)(queue))
            
#define rscan_thread_queue_open_p(queue) \
            rscan_queue_open_p((struct queue *)(queue))

#define rscan_thread_queue_drop(queue) \
            rscan_queue_drop((struct queue *)(queue))
            
#define rscan_thread_queue_drop_all(queue) \
            rscan_queue_drop_all((struct queue *)(queue))

#define rscan_thread_queue_put(queue, thread) \
            rscan_queue_put((struct queue *)(queue), (void *)(thread))

#define rscan_thread_queue_shift(queue) \
            (rb_nativethread_id_t *)rscan_queue_shift((struct queue *)(queue))

#define rscan_thread_queue_shift_ptr(queue, out) \
            rscan_queue_shift_ptr((struct queue *)(queue), (rb_nativethread_id_t **)(out))

#define rscan_thread_queue_shift_nogvlwait(queue, out) \
            rscan_queue_shift_nogvlwait((struct queue *)(queue), (rb_nativethread_id_t **)(out))

#define rscan_thread_queue_shift_nonblock(queue, out) \
            rscan_thread_queue_shift_nonblock_opt((queue), RSCAN_QUEUE_LOCKWAIT_DEFAULT, (out))

#define rscan_thread_queue_shift_nonblock_opt(queue, lock_wait, out) \
            rscan_queue_shift_nonblock_opt((struct queue *)(queue), (lock_wait), (rb_nativethread_id_t **)(out))

#define rscan_thread_queue_peek(queue, i) \
            (rb_nativethread_id_t*)rscan_queue_peek((struct queue *)(queue), (i))

#define rscan_thread_queue_peek_ptr(queue, i, out) \
            rscan_queue_peek_ptr((struct queue *)(queue), (i), (rb_nativethread_id_t **)(out))

#define rscan_thread_queue_peek_nogvlwait(queue, i, out) \
            rscan_queue_peek_nogvlwait((struct queue *)(queue), (i), (rb_nativethread_id_t **)(out))

#define rscan_thread_queue_peek_nonblock(queue, i, out) \
            rscan_thread_queue_peek_nonblock_opt((queue), RSCAN_QUEUE_LOCKWAIT_DEFAULT, (i), (out))

#define rscan_thread_queue_peek_nonblock_opt(queue, lock_wait, i, out) \
            rscan_queue_peek_nonblock_opt((struct queue *)(queue), (lock_wait), (i), (rb_nativethread_id_t **)(out))

#define Get_Thread_Queue(value,pointer) \
        Data_Get_Struct(value,struct thread_queue,pointer)
#endif
