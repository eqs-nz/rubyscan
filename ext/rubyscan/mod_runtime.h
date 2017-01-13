/* scanner.h
 * pattern scanner object class
 * written by Alex Fulton
 * Sept 2016
 */
#ifndef RSCAN_MOD_SCAN_H
#define RSCAN_MOD_SCAN_H

#include "mod_common.h"
#include <ruby/thread.h>

#include "scan_unit.h"
#include "event.h"
#include "queue.h"
#include "scanner.h"

extern VALUE rscan_module_scan_define(VALUE root);
extern VALUE rscan_module_scan();

#endif
