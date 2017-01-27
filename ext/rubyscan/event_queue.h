/* event_queue.h
 * queue type which handles events
 * written by Alex Fulton
 * Jan 2017
 */
#ifndef RSCAN_EVENT_QUEUE_H
#define RSCAN_EVENT_QUEUE_H

struct event_queue_member {
    struct event_queue_member *next;
    struct event *event;
};

struct event_queue {
    pthread_mutex_t lock;
    pthread_cond_t ready;
    pthread_cond_t empty;
    int open;
    unsigned int length;
    struct event_queue_member *head;
    struct event_queue_member *tail;
};

VALUE rscan_event_queue_define(VALUE root);
VALUE rscan_class_event_queue();

#define rscan_event_queue_close(event_queue) \
            rscan_queue_close((struct queue*)(event_queue))
            
#define rscan_event_queue_closed_p(event_queue) \
            rscan_queue_closed_p((struct queue*)(event_queue))

#define rscan_event_queue_open(event_queue) \
            rscan_queue_open((struct queue*)(event_queue))
            
#define rscan_event_queue_open_p(event_queue) \
            rscan_queue_open_p((struct queue*)(event_queue))

#define rscan_event_queue_drop(event_queue) \
            rscan_queue_drop((struct queue*)(event_queue))
            
#define rscan_event_queue_drop_all(event_queue) \
            rscan_queue_drop_all((struct queue*)(event_queue))

#define rscan_event_queue_put(event_queue, event) \
            rscan_queue_put((struct queue *)(event_queue), (void *)(event))

#define rscan_event_queue_shift(event_queue) \
            (struct event *)rscan_queue_shift((struct queue *)(event_queue))

#define rscan_event_queue_shift_ptr(event_queue, out) \
            rscan_queue_shift_ptr((struct queue *)(event_queue), (void **)(out))

#define rscan_event_queue_shift_nogvlwait(event_queue, out) \
            rscan_queue_shift_nogvlwait((struct queue *)(event_queue), (void **)(out))

#define rscan_event_queue_shift_nonblock(event_queue, out) \
            rscan_event_queue_shift_nonblock_opt((event_queue), RSCAN_QUEUE_LOCKWAIT_DEFAULT, (out))

#define rscan_event_queue_shift_nonblock_opt(event_queue, lock_wait, out) \
            rscan_queue_shift_nonblock_opt((struct queue *)(event_queue), (lock_wait), (void **)(out))

#define rscan_event_queue_peek(event_queue, i) \
            (struct event *)rscan_queue_peek((struct queue *)(event_queue), (i))

#define rscan_event_queue_peek_ptr(event_queue, i, out) \
            rscan_queue_peek_ptr((struct queue *)(event_queue), (i), (void **)(out))

#define rscan_event_queue_peek_nogvlwait(event_queue, i, out) \
            rscan_queue_peek_nogvlwait((struct queue *)(event_queue), (i), (void **)(out))

#define rscan_event_queue_peek_nonblock(event_queue, i, out) \
            rscan_event_queue_peek_nonblock_opt((event_queue), RSCAN_QUEUE_LOCKWAIT_DEFAULT, (i), (out))

#define rscan_event_queue_peek_nonblock_opt(event_queue, lock_wait, i, out) \
            rscan_queue_peek_nonblock_opt((struct queue *)(event_queue), (lock_wait), (i), (struct event **)(out))

#define Get_Event_Queue(value,pointer) \
        Data_Get_Struct(value,struct event_queue,pointer)
#endif
