#ifndef _APP_HELPER_H_
#define _APP_HELPER_H_

#include "app.h"
#include "workerManagerADT.h"

/**
 * Decides the amount of workers to spawn based on constants and
 * the configuration of the running system.
 *
 * Will always return a value of at least 1, unless fileCount is 0.
 */
unsigned int decideWorkerCount(unsigned int fileCount);

/**
 * Decides how many initial tasks to send to each worker, then sends
 * said amount of tasks to all workers.
 *
 * Will always send at least one initial task to each worker.
 */
void sendInitialWorkerTasks(workerManagerADT workerManager, TAppContext* appContext);

#endif