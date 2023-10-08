// Copyright (c) 2023 Ayzurus
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <threads.h>

typedef enum {
    FINISH = 0, // routine ran as expected
    BUSY,       // routine is active
    CANCEL      // routine was canceled
} nordi_routine_result_t;

typedef struct {
    thrd_t thread;
    mtx_t mutex;
    void (*function)(void*);
    void* context;
    int delay;
    int state;
} nordi_routine_t;

typedef nordi_routine_t* nordi_routine_ptr;
typedef void (*nordi_routine_func_t)(void*);

/**
 * @brief Creates a thread that will run the given function once. If the provided delay is > `0`, the routine will
 * wait that amount in seconds before calling the function. If the routine is canceled with `nordi_routine_cancel`
 * before the delay finishes, the routine will not call the function.
 * @param function The function to be called.
 * @param context The data argument to be passed onto the function.
 * @param delay The delay in seconds until the function is actually called.
 */
nordi_routine_ptr nordi_routine_new(nordi_routine_func_t, void*, int);

/**
 * @brief Waits for the given routine to finish and then frees its resources. Blocks while waiting.
 * @param routine The routine to join.
 */
void nordi_routine_join(nordi_routine_ptr);

/**
 * @brief Cancels a routine execution and then waits for the routine to finish. Blocks while waiting.
 * Canceling will only work for delayed routines, if the delay has not finished yet. Immediate routines
 * won't be affected by the cancelling, so if the objective is to stop the function logic, other flags
 * and variables have to be used for that. Calls `nordi_routine_join` so no joining is needed after this.
 * @param routine The routine to cancel.
 */
void nordi_routine_cancel(nordi_routine_ptr);
