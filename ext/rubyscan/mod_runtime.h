/* scanner.h
 * pattern scanner object class
 * written by Alex Fulton
 * Sept 2016
 */
#ifndef RSCAN_MOD_RUNTIME_H
#define RSCAN_MOD_RUNTIME_H

#include "mod_common.h"
#include <ruby/thread.h>

typedef unsigned int rscan_queue_err_t;
#include "scan_op.h"
#include "scan_unit.h"
#include "scanner.h"
#include "event.h"
#include "scan_event.h"
#include "match_event.h"
#include "queue.h"
#include "event_queue.h"
#include "thread_queue.h"

extern VALUE rscan_module_runtime_define(VALUE root);
extern VALUE rscan_module_runtime();

#endif
