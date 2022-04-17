#ifndef _FILE_OUTPUT_H_
#define _FILE_OUTPUT_H_

#include "app.h"
#include "communication.h"

/**
 * Called when the app starts. Prepares anything needed to
 * output results to a file.
 *
 * Returns 1 if file output is enabled, 0 otherwise (if,
 * for example, the output file couldn't be opened).
 */
int fileOutputBegin(TAppContext* appContext);

/**
 * Called whenever the app receives a result. Outputs said
 * result to a file if required.
 */
void fileOutputResult(TAppContext* appContext, unsigned int workerId, const TWorkerResult* result, const char* filepath);

/**
 * Called when the app has no more results. Disposes of
 * any resources created for outputing to a file.
 */
void fileOutputEnd(TAppContext* appContext);

#endif