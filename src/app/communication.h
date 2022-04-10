#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_

#include <stdint.h>

/**
 * Represents a request sent from the APP process to a WORKER
 * process. This does not include the filepath, which is sent
 * separately since it's of variable size.
 */
typedef struct {
	uint32_t taskId;
	uint32_t filepathLength;
} TWorkerRequest;

/**
 * Represents the possible result statuses a worker can return
 * to the parent. Includes errors.
 */
enum SatResult { Sat, Unsat, FileOpenFailed };

/**
 * Represents a result sent from a WORKER process to the APP
 * process.
 */
typedef struct {
	uint32_t taskId;
	enum SatResult status;
	uint32_t cantidadClausulas;
	uint32_t cantidadVariables;
	uint64_t timeNanoseconds;
} TWorkerResult;

#endif