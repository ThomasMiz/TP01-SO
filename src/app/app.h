#ifndef _APP_H_
#define _APP_H_

#include <stdio.h>
#include <time.h>
#include "./../shared/shmHandler.h"

/** Stores context information about the app process. */
typedef struct {
	/** An array with the files to process, of length fileCount. */
	const const char** files;
	
	/** The total amount of files the app process was given. */
	unsigned int fileCount;
	
	/**
	 * The amount of files that have already been sent to workers,
	 * but not necessarily gotten results for.
	 */
	unsigned int filesSent;
	
	/** The amount of results received from workers. */
	unsigned int resultsReceived;
	
	/**
	 * The handle of the file to which result outputs are written.
	 * Should only be used in output.c.
	 */
	FILE* resultOutputFile;
	
	/**
	* Structure where all the information of the shm is saved
	*/
	TSharedMem ptrInfo;
	
	/**
	* Pointer to the start of the shm where we have 2 semph and
	* the length of the buffer sent
	**/
	TSharedMemContext* sharedMemContext;
	
	/**
	* Structure used to trigger sem_timewait
	*/
	struct timespec ts;
	
	} TAppContext;

#endif