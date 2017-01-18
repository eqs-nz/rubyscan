/* event_queue.h
 * queue type which handles events
 * written by Alex Fulton
 * Jan 2017
 */
#ifndef RSCAN_EVENT_QUEUE_H
#define RSCAN_EVENT_QUEUE_H

struct event_queue_member {
    struct event_queue_member *next;
    rscan_event_t *event;
}

struct event_queue {
    pthread_mutex_t lock;
    pthread_cond_t ready;
    pthread_cond_t empty;
    int open;
    struct event_queue_member *head;
    struct event_queue_member *tail;
}

VALUE rscan_queue_define(VALUE root);
VALUE rscan_class_queue();

#define rscan_event_queue_put(queue, event) \
            rscan_queue_put((rscan_queue_t *)(queue), (rscan_event_t *)event)

#define rscan_event_queue_shift(queue) \
            (rscan_event_t *)rscan_queue_shift((rscan_queue_t *)(queue))

#define rscan_event_queue_shift_ptr(queue, out) \
            rscan_queue_shift_ptr((rscan_queue_t *)(queue), (rscan_event_t **)(out))

#define rscan_event_queue_shift_nogvlwait(queue, out) \
            rscan_queue_shift_nogvlwait((rscan_queue_t *)(queue), (rscan_event_t **)(out))

#define rscan_event_queue_shift_nonblock(queue, out) \
            rscan_event_queue_shift_nonblock_opt((queue), RSCAN_QUEUE_LOCKWAIT_DEFAULT, (out))

#define rscan_event_queue_shift_nonblock_opt(queue, lock_wait, out) \
            rscan_queue_shift_nonblock_opt((rscan_queue_t *)(queue), (lock_wait), (rscan_event_t **)(out))

#define rscan_event_queue_peek(queue, i) \
            (rscan_event_t *)rscan_queue_peek((rscan_queue_t *)(queue), (i))

#define rscan_event_queue_peek_ptr(queue, i, out) \
            rscan_queue_peek_ptr((rscan_queue_t *)(queue), (i), (rscan_event_t **)(out))

#define rscan_event_queue_peek_nogvlwait(queue, i, out) \
            rscan_queue_peek_nogvlwait((rscan_queue_t *)(queue), (i), (rscan_event_t **)(out))

#define rscan_event_queue_peek_nonblock(queue, i, out) \
            rscan_event_queue_peek_nonblock_opt((queue), RSCAN_QUEUE_LOCKWAIT_DEFAULT, (i), (out))

#define rscan_event_queue_peek_nonblock_opt(queue, lock_wait, i, out) \
            rscan_queue_peek_nonblock_opt((rscan_queue_t *)(queue), (lock_wait), (i), (rscan_event_t **)(out))

#define Get_Event_Queue(value,pointer) \
        Data_Get_Struct(value,rscan_event_queue_t,pointer)
#endif
