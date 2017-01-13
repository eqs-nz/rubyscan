/* queue.h
 * threadsafe event queue for Ruby
 * written by Alex Fulton
 * Sept 2016
 */
#ifndef rscan_queue_H
#define rscan_queue_H

typedef struct event_queue_member {
    struct event_queue_member *next;
    rscan_match_event_t *event;
} rscan_event_queue_member_t;

typedef struct event_queue {
    pthread_mutex_t lock;
    pthread_cond_t ready;
    pthread_cond_t empty;
    int open;
    rscan_event_queue_member_t *head;
    rscan_event_queue_member_t *tail;
} rscan_event_queue_t;

typedef unsigned int rscan_queue_err_t;

typedef struct queue_gvl_arg {
    unsigned int interrupted;
    rscan_event_queue_t *queue;
    rscan_match_event_t **out;
} queue_gvl_arg_t;

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

VALUE rscan_queue_define(VALUE root);
VALUE rscan_class_queue();

void rscan_queue_close(rscan_event_queue_t *pQueue);
rscan_queue_err_t rscan_queue_closed_p(rscan_event_queue_t *pQueue);
void rscan_queue_open(rscan_event_queue_t *pQueue);
rscan_queue_err_t rscan_queue_open_p(rscan_event_queue_t *pQueue);
void rscan_queue_drop(rscan_event_queue_t *pQueue);
void rscan_queue_drop_all(rscan_event_queue_t *pQueue);
void rscan_queue_put(rscan_event_queue_t *pQueue, rscan_match_event_t *pMatch);
rscan_match_event_t *rscan_queue_shift(rscan_event_queue_t *pQueue);
rscan_queue_err_t rscan_queue_shift_ptr(rscan_event_queue_t *pQueue, rscan_match_event_t **val);
rscan_queue_err_t rscan_queue_shift_nogvlwait(rscan_event_queue_t *queue, rscan_match_event_t **out);
#define rscan_queue_shift_nonblock(pQueue,val) \
    rscan_queue_shift_nonblock_opt((pQueue),rscan_queue_LOCKWAIT_DEFAULT,(val))
rscan_queue_err_t rscan_queue_shift_nonblock_opt(rscan_event_queue_t *pQueue, int bWaitForLock, rscan_match_event_t **val);
void rscan_queue_destroy(rscan_event_queue_t *pQueue);
void rscan_queue_wait_until_empty(rscan_event_queue_t *pQueue);

#define Get_Queue(value,pointer) \
        Data_Get_Struct(value,rscan_event_queue_t,pointer)
#endif
