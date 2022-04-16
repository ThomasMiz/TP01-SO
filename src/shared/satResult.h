#ifndef _SATRESULT_H_
#define _SATRESULT_H_

/**
 * Represents the possible result statuses a worker can return
 * to the parent. Includes errors.
 */
enum SatResult { Sat, Unsat, Error };

const char* satResultToString(enum SatResult satResult);

#endif