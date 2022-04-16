#ifndef _WORKER_MANAGER_ADT_
#define _WORKER_MANAGER_ADT_

#include "communication.h"

typedef struct workerManagerCDT* workerManagerADT;

/**
 * Represents a pointer to a function that will handle the event of a
 * worker returning results.
 */
typedef void (*TWorkerResultCallback)(workerManagerADT sender, unsigned int workerId, const TWorkerResult* result, void* arg);

/**
 * Represents a pointer to a function that will handle the event of a
 * worker having closed. The "result" pointer is only valid until your
 * callback function returns.
 */
typedef void (*TWorkerClosedCallback)(workerManagerADT sender, unsigned int workerId, void* arg);

/**
 * Creates a new workerManagerADT and spawns the specified amount of
 * workers.
 *
 * Returns NULL on failure.
 */
workerManagerADT newWorkerManager(unsigned int workerCount);

/**
 * Frees all resources used by a workerManagerADT.
 */
void freeWorkerManager(workerManagerADT manager);

/**
 * Gets the amount of remaining workers.
 */
unsigned int getRemainingWorkerCount(workerManagerADT manager);

/**
 * Gets the amount of sent tasks the worker with the specified id has
 * with pending results.
 */
unsigned int getWorkerRemainingTasks(workerManagerADT manager, unsigned int workerId);

/**
 * Sends a task to the specified worker.
 */
void sendWorkerTask(workerManagerADT manager, unsigned int workerId, unsigned int taskId, const char* filepath);

/**
 * Sets the function that should be called when a worker returns a
 * result.
 */
void setWorkerResultCallback(workerManagerADT manager, TWorkerResultCallback callback, void* arg);

/**
 * Sets the function that should be called when a worker closes.
 */
void setWorkerClosedCallback(workerManagerADT manager, TWorkerClosedCallback callback, void* arg);

/**
 * Instructs the worker with the specified id that no more tasks will be
 * sent to it, and that it should close once its done with any remaining
 * tasks previously sent. This does not decrement the value returned by
 * getRemainingWorkerCount() until the worker actually closes.
 * 
 * Returns 1 if the operation succeeded, 0 otherwise.
 */
int closeWorker(workerManagerADT manager, unsigned int workerId);

/**
 * Instructs all remaining workers that no more tasks will be sent to
 * them, and that they should close once they're done with any remaining
 * tasks previously sent. Does not modify the value returned by
 * getRemainingWorkerCount() until the workers actually start closing.
 * 
 * Returns 1 if the operation succeeded, 0 if errors were encountered.
 * 
 * Same as calling closeWorker() on all workers.
 */
int closeRemainingWorkers(workerManagerADT manager);

/**
 * Waits until events are available to be reported and invokes the
 * appropiate event callbacks. If events were already available,
 * returns without waiting. Otherwise, blocks until an event occurs
 * for up to timeoutMillis milliseconds.
 *
 * A timeoutMillis of zero forces no wait, while a negative value
 * will be an infinite timeout, i.e. block until an event occurs.
 *
 * Returns 1 if any events were handled.
 */
int pollEvents(workerManagerADT manager, int timeoutMillis);

/**
 * Runs pollEvents() on a loop until all workers have exited.
 */
void pollUntilFinished(workerManagerADT manager);

#endif