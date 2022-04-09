#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_

/** Represents a request sent from the APP process to a WORKER
 * process. This does not include the filepath, which is sent
 * separately since it's of variable size. */
typedef struct {
	
} TWorkerRequest;

/** Represents a result sent from a WORKER process to the APP
 * process. This is a fixed-size struct small enough to guarantee
 * that writing it to a pipe can be done atomically. */
typedef struct {
	
} TWorkerResult;

#endif