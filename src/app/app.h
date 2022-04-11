#ifndef _APP_H_
#define _APP_H_

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
} TAppContext;

#endif