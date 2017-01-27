/* mod_common.h
 * module containing common types and routines for rubyscan.rb
 * written by Alex Fulton
 * Sept 2016
 */
#ifndef RSCAN_MOD_COMMON_H
#define RSCAN_MOD_COMMON_H

#include "extconf.h"

#include <errno.h>
#include <pthread.h>
#include <ruby.h>
#include <ruby/thread.h>
#include <ruby/thread_native.h>
#include <hs/hs.h>

#include "platform.h"
#include "scratch.h"
#include "db.h"
#include "hs_error.h"
#include "thread_error.h"

extern VALUE rscan_module_common_define(VALUE root);

extern VALUE rscan_module_common();

#endif
