#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

/** The name of the executable file containing the worker process. */
#define WORKER_EXEC_FILE "worker.out"

#define RESULT_OUTPUT_FILE "output.csv"

/**
 * A hard limit on the maximum amount of workers that can be spawned
 * by an app process. Set to -1 to disable this limit (this is ok
 * because the app process will use the amount of CPUs on the system
 * as a limit anyways).
*/
#define MAX_WORKERS -1

/**
 * Define this to 1 to ensure the app process will spawn less than
 * one worker per system processor. Otherwise, it may spawn one
 * worker per system processor.
 *
 * Spawning one worker per system processor might mean that work
 * can be done faster, but on some operating systems can impact
 * system responsiveness, specially if other processes are
 * running at the same time. 
 */
#define LEAVE_ONE_PROCESSOR 1

#define EXIT_CODE_NOT_ENOUGH_PARAMS 1
#define EXIT_CODE_CREATE_PIPE_FAILED 2
#define EXIT_CODE_FORK_FAILED 3
#define EXIT_CODE_NOT_ENOUGH_MEMORY 4

#endif