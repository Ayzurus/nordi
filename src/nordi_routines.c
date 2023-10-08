/**
 * Copyright (c) 2023 Ayzurus
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include <stdlib.h>
#include <time.h>
#include "nordi_routines.h"

static int
_immediate_routine(nordi_routine_ptr routine) {
    routine->function(routine->context);
    routine->state = FINISH;
    return thrd_success;
}

static int
_delayed_routine(nordi_routine_ptr routine) {
    struct timespec delay = {};
    timespec_get(&delay, TIME_UTC);
    delay.tv_sec += routine->delay; // add delay to current time
    int mutex_state = mtx_timedlock(&routine->mutex, &delay);
    if (mutex_state == thrd_timedout) {
        routine->function(routine->context);
        routine->state = FINISH;
    } else {
        routine->state = CANCEL;
    }
    return thrd_success;
}

nordi_routine_ptr
nordi_routine_new(nordi_routine_func_t function, void* context, int delay) {
    nordi_routine_ptr routine = (nordi_routine_ptr)malloc(sizeof(nordi_routine_t));
    if (routine == NULL) {
        return NULL;
    }
    int result = mtx_init(&routine->mutex, mtx_timed);
    if (result != thrd_success) {
        free(routine);
        return NULL;
    }
    routine->function = function;
    routine->context = context;
    routine->state = BUSY;
    routine->delay = delay;
    if (delay > 0) {
        mtx_lock(&routine->mutex); // lock thread to wait for the delay
        result = thrd_create(&routine->thread, (int (*)(void*))_delayed_routine, (void*)routine);
    } else {
        result = thrd_create(&routine->thread, (int (*)(void*))_immediate_routine, (void*)routine);
    }
    if (result != thrd_success) {
        mtx_destroy(&routine->mutex);
        free(routine);
        return NULL;
    }
    return routine;
}

void
nordi_routine_join(nordi_routine_ptr routine) {
    if (routine == NULL) {
        return;
    }
    int result = thrd_join(routine->thread, NULL);
    if (result == thrd_success && routine->state != BUSY) {
        mtx_destroy(&routine->mutex);
        free(routine);
    }
}

void
nordi_routine_cancel(nordi_routine_ptr routine) {
    if (routine == NULL) {
        return;
    }
    mtx_unlock(&routine->mutex);
    nordi_routine_join(routine);
}