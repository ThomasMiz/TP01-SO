#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include "workerManagerADT.h"
#include "communication.h"
#include "constants.h"
#include "./../shared/memhelper.h"

struct workerManagerCDT {
	/** The amount of workers this manager was created with. */
	unsigned int workerCount;
	
	/** The amount of workers this manager currently has open. */
	unsigned int remainingWorkerCount;
	
	/**
	 * The fds of the pipes to send requests to workers. These are
	 * set to -1 once they are closed.
	 */
	int* requestPipeWriteFds;
	
	/**
	 * Structures used for event polling on the result pipes.
	 * Also contains the result pipe fds. May be out of order.
	 * The length of this array is remainingWorkerCount.
	*/
	struct pollfd* pollFds;
	
	/**
	 * An array that contains for each pollfd in pollFds, the
	 * id of the related worker in said array position.
	 */
	unsigned int* pollFdsWorkerIds;
	
	/** Function pointer to the worker result callback. May be null. */
	TWorkerResultCallback resultCallback;
	void* resultCallbackArg;
	
	/** Function pointer to the worker closed callback. May be null. */
	TWorkerClosedCallback closedCallback;
	void* closedCallbackArg;
	
	/** 
	 * Buffers used to receive the TWorkResult structs sent by workers,
	 * since there's no guarantee it will be read all at once.
	 */
	TWorkerResult* resultBufs;
	
	/** The amount of bytes read for each TWorkerResult in resultBufs. */
	int* resultBufCounts;
};

/**
 * Spawns the specified amount of workers, creates the pipes to
 * send tasks to them and places the pipe fds where required.
 * The write end of the request pipes are written to the manager's
 * requestPipeWriteFds array, while the read ends of the result
 * pipes go to pollFds[i].fd.
 */
static void performWorkerSpawning(workerManagerADT manager) {
	int requestPipe[2];
	int resultPipe[2];
	
	for (unsigned int i=0; i<manager->workerCount; i++) {
		// We attempt to create the request pipe and handle any errors.
		if (pipe(requestPipe)) {
			fprintf(stderr, "Worker manager error: failed to create request pipe for worker %i: ", i);
			perror(NULL);
			exit(EXIT_CODE_CREATE_PIPE_FAILED);
		}
		
		// We attempt to create the result pipe and handle any errors.
		if (pipe(resultPipe)) {
			fprintf(stderr, "Worker manager error: failed to create result pipe for worker %i: ", i);
			perror(NULL);
			exit(EXIT_CODE_CREATE_PIPE_FAILED);
		}
		
		manager->requestPipeWriteFds[i] = requestPipe[1];
		manager->pollFds[i].fd = resultPipe[0];
		
		// We create the child process and handle any errors.
		pid_t forkResult = fork();
		if (forkResult == -1) {
			fprintf(stderr, "Worker manager error: Failed to create fork for worker %i: ", i);
			perror(NULL);
			exit(EXIT_CODE_FORK_FAILED);
		}
		
		// The child process redirects fds and exec-s the worker executable.
		if (forkResult == 0) {
			// Close the write end of the request pipe.
			close(requestPipe[1]);
			// Close the read end of the result pipe.
			close(resultPipe[0]);
			// Redirect STDOUT to the write end of the result pipe.
			dup2(resultPipe[1], STDOUT_FILENO);
			// Redirect STDIN to the read end of the request pipe.
			dup2(requestPipe[0], STDIN_FILENO);
			
			// The only parameter the worker receives is it's worker id.
			char idString[12] = {0};
			sprintf(idString, "%u", i);
			execl(WORKER_EXEC_FILE, WORKER_EXEC_FILE, idString, NULL);
			
			// This only runs if the exec fails.
			fprintf(stderr, "Worker with id %u failed to exec %s: ", i, WORKER_EXEC_FILE);
			perror(NULL);
			exit(-1);
		}
		
		// The parent process now closes the pipe ends it doesn't need.
		// Close the read end of the request pipe.
		close(requestPipe[0]);
		// Close the write end of the result pipe.
		close(resultPipe[1]);
	}
}

workerManagerADT newWorkerManager(unsigned int workerCount) {
	if (workerCount == 0) {
		fprintf(stderr, "Worker manager warning: new worker manager created with workerCount = 0.\n");
	}
	
	workerManagerADT manager = callocOrExit(1, sizeof(struct workerManagerCDT));
	manager->workerCount = workerCount;
	manager->remainingWorkerCount = workerCount;
	manager->requestPipeWriteFds = mallocOrExit(sizeof(int) * workerCount);
	manager->pollFds = mallocOrExit(sizeof(struct pollfd) * workerCount);
	manager->pollFdsWorkerIds = mallocOrExit(sizeof(unsigned int) * workerCount);
	manager->resultBufs = mallocOrExit(sizeof(TWorkerResult) * workerCount);
	manager->resultBufCounts = callocOrExit(workerCount, sizeof(int));
	
	for (int i=0; i<workerCount; i++) {
		struct pollfd* p = &manager->pollFds[i];
		p->events = POLLIN;
		p->revents = 0;
		manager->pollFdsWorkerIds[i] = i;
	}
	
	performWorkerSpawning(manager);
	
	return manager;
}

void freeWorkerManager(workerManagerADT manager) {
	free(manager->requestPipeWriteFds);
	free(manager->pollFds);
	free(manager->pollFdsWorkerIds);
	free(manager->resultBufs);
	free(manager->resultBufCounts);
	free(manager);
}

unsigned int getWorkerCount(workerManagerADT manager) {
	return manager->workerCount;
}

unsigned int getRemainingWorkerCount(workerManagerADT manager) {
	return manager->remainingWorkerCount;
}

int isWorkerOpen(workerManagerADT manager, unsigned int workerId) {
	return workerId < manager->workerCount && manager->requestPipeWriteFds[workerId] >= 0;
}

void sendWorkerTask(workerManagerADT manager, unsigned int workerId, unsigned int taskId, const char* filepath) {
	if (workerId >= manager->workerCount) {
		fprintf(stderr, "Worker manager error: sendWorkerTask to id %u, but max worker id is %u.\n", workerId, manager->workerCount-1);
		return;
	}
	
	TWorkerRequest request;
	request.taskId = taskId;
	request.filepathLength = strlen(filepath);
	write(manager->requestPipeWriteFds[workerId], &request, sizeof(TWorkerRequest));
	write(manager->requestPipeWriteFds[workerId], filepath, request.filepathLength);
	// TODO: Instead of using write(), wrap it in a function that ensures ALL the
	// bytes were written? Only the int returned amount of bytes are written!
}

void setWorkerResultCallback(workerManagerADT manager, TWorkerResultCallback callback, void* arg) {
	manager->resultCallback = callback;
	manager->resultCallbackArg = arg;
}

void setWorkerClosedCallback(workerManagerADT manager, TWorkerClosedCallback callback, void* arg) {
	manager->closedCallback = callback;
	manager->closedCallbackArg = arg;
}

int closeWorker(workerManagerADT manager, unsigned int workerId) {
	if (workerId >= manager->workerCount) {
		fprintf(stderr, "Worker manager error: closeWorker with id %u, but max worker id is %u.\n", workerId, manager->workerCount-1);
		return 0;
	}
	
	if (manager->requestPipeWriteFds[workerId] < 0) {
		fprintf(stderr, "Worker manager warning: attempted to close already closed worker id %u.\n", workerId);
		return 0;
	}
	
	// TODO: change all close() calls to use a wrapper that ensures close() runs even if interrupted by a signal!
	if (close(manager->requestPipeWriteFds[workerId])) {
		fprintf(stderr, "Worker manager warning: failed to close request pipe write end for worker %u.\n", workerId);
	}
	
	manager->requestPipeWriteFds[workerId] = -1;
	return 1;
}

int closeRemainingWorkers(workerManagerADT manager) {
	int r = 0;
	
	for (int i=0; i<manager->workerCount; i++) {
		// If the worker is still open, then close it.
		if (manager->requestPipeWriteFds[i] >= 0)
			r = closeWorker(manager, i) || r;
	}
	
	return r;
}

static void handleWorkerClosing(workerManagerADT manager, unsigned int bufIndex) {
	int remWorkers = --manager->remainingWorkerCount;
	
	unsigned int closingWorkerId = manager->pollFdsWorkerIds[bufIndex];
	
	if (close(manager->pollFds[bufIndex].fd)) {
		fprintf(stderr, "Worker manager warning: worker %u closed, but close() failed on its result pipe fd.\n", closingWorkerId);
	}
	
	if (manager->requestPipeWriteFds[closingWorkerId] != -1) {
		close(manager->requestPipeWriteFds[closingWorkerId]);
		manager->requestPipeWriteFds[closingWorkerId] = -1;
		fprintf(stderr, "Worker manager warning: worker %u closed, but its request pipe was still open.\n", closingWorkerId);
	}
	
	if (manager->resultBufCounts[bufIndex]) {
		fprintf(stderr, "Worker manager warning: worker %u closed, but it still had %i bytes in the result buffer.\n", closingWorkerId, manager->resultBufCounts[bufIndex]);
	}
	
	// If it's not the last element in pollFds, we remove the entry by swapping
	// it with the last element in the array.
	if (bufIndex != remWorkers) {
		manager->pollFds[bufIndex] = manager->pollFds[remWorkers];
		manager->pollFdsWorkerIds[bufIndex] = manager->pollFdsWorkerIds[remWorkers];
		manager->resultBufCounts[bufIndex] = manager->resultBufCounts[remWorkers];
		manager->resultBufs[bufIndex] = manager->resultBufs[remWorkers];
	}
	
	// We invoke the worker closed callback if it's not null.
	if (manager->closedCallback) {
		manager->closedCallback(manager, closingWorkerId, manager->closedCallbackArg);
	}
}

/**
 * Handles reading from a result pipe and taking actions such as removing a worker.
 * Returns 1 if the worker was removed from the list.
 */
static int handleReadResultPipe(workerManagerADT manager, unsigned int bufIndex) {
	// This read is guaranteed to be non-blocking, because this is only called
	// by pollEvents() when an event occurs on the fd.
	
	// We read from the fd the amount of bytes necessary to fill up a TWorkerResult.
	int t = manager->resultBufCounts[bufIndex];
	t = read(manager->pollFds[bufIndex].fd, (void*)&manager->resultBufs[bufIndex] + t, sizeof(TWorkerResult) - t);
	
	if (t == 0) {
		// When read() returns 0, it is to indicate an end of file. This worker has closed.
		handleWorkerClosing(manager, bufIndex);
		return 1;
	}
	
	// If the read fails then we don't try again- if there's stuff to read
	// from this fd it will occur on the next pollEvents() cycle.
	if (t < 0) {
		fprintf(stderr, "Worker manager warning: error while reading from worker %u result pipe: ", manager->pollFdsWorkerIds[bufIndex]);
		perror(NULL);
		return 0;
	}
	
	// We check if total amount of bytes that was read from the result of this worker
	// is enough to form a whole TWorkerResult struct. If so, we invoke the result callback.
	t += manager->resultBufCounts[bufIndex];
	
	if (t >= sizeof(TWorkerResult)) {
		
		// This should never happen, but let's handle it just in case.
		if (t > sizeof(TWorkerResult))
			fprintf(stderr, "Error while reading from worker %u result pipe, too many bytes read?? Your linux is broken lol\n", manager->pollFdsWorkerIds[bufIndex]);
		
		manager->resultBufCounts[bufIndex] = 0;
		
		if (manager->resultCallback) {
			manager->resultCallback(manager, manager->pollFdsWorkerIds[bufIndex], &manager->resultBufs[bufIndex], manager->resultCallbackArg);
		} else {
			fprintf(stderr, "Warning: worker %u returned result got discarded because result callback is null.\n", manager->pollFdsWorkerIds[bufIndex]);
		}
	} else {
		// We haven't read enough bytes to form a full TWorkerResult yet.
		manager->resultBufCounts[bufIndex] = t;
	}
	
	return 0;
}

int pollEvents(workerManagerADT manager, int timeoutMillis) {
	// We poll on all file descriptors and get the amount of fds with events.
	int pollRemaining = poll(manager->pollFds, manager->remainingWorkerCount, timeoutMillis);
	
	if (pollRemaining == 0)
		return 0;
	
	int i = 0;
	while (i < manager->remainingWorkerCount) {
		// If this fd didn't have any events occur, we skip it.
		if (!manager->pollFds[i].revents) {
			i++;
			continue;
		}
		
		// There is data available for reading in this fd! Let's handle it.
		// We only increment i if the worker has NOT been removed from the list.
		if (!handleReadResultPipe(manager, i))
			i++;
		
		// We decrement the remaining amount of fds with events. If 0, we break
		// the loop because there are no more fds with events to handle.
		if (--pollRemaining == 0)
			break;
	}
	
	return 1;
}

void pollUntilFinished(workerManagerADT manager) {
	while (manager->remainingWorkerCount)
		pollEvents(manager, -1);
}