#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_

#include "./../shared/satResult.h"

/**
 * Represents a request sent from the APP process to a WORKER
 * process. This does not include the filepath, which is sent
 * separately since it's of variable size.
 */
typedef struct {
	unsigned int taskId;
	unsigned int filepathLength;
} TWorkerRequest;

/**
 * Represents a result sent from a WORKER process to the APP
 * process.
 */
typedef struct {
	unsigned int taskId;
	enum SatResult status;
	unsigned int cantidadClausulas;
	unsigned int cantidadVariables;
	double timeSeconds;
} TWorkerResult;

#endif