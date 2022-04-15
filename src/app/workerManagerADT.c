// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include "workerManagerADT.h"
#include "./../shared/constants.h"
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
	 * An array with the amount of remaining tasks sent each worker
	 * has.
	 */
	unsigned int* remainingTaskCounts;
	
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
 *
 * Returns 1 if the operation succeeded, 0 otherwise.
 */
static int performWorkerSpawning(workerManagerADT manager) {
	int requestPipe[2];
	int resultPipe[2];
	
	for (unsigned int i=0; i<manager->workerCount; i++) {
		// We attempt to create the request pipe and handle any errors.
		if (pipe(requestPipe)) {
			fprintf(stderr, "[Master] Worker manager error: Failed to create request pipe for worker %i: ", i);
			perror(NULL);
			return 0;
		}
		
		// We attempt to create the result pipe and handle any errors.
		if (pipe(resultPipe)) {
			fprintf(stderr, "[Master] Worker manager error: Failed to create result pipe for worker %i: ", i);
			perror(NULL);
			close(requestPipe[0]);
			close(requestPipe[1]);
			return 0;
		}
		
		// We create the child process and handle any errors.
		pid_t forkResult = fork();
		if (forkResult == -1) {
			fprintf(stderr, "[Master] Worker manager error: Failed to create fork for worker %i: ", i);
			perror(NULL);
			close(requestPipe[0]);
			close(requestPipe[1]);
			close(resultPipe[0]);
			close(resultPipe[1]);
			return 0;
		}
		
		// The child process redirects fds and exec-s the worker executable.
		if (forkResult == 0) {
			// Close the write end of the request pipe.
			close(requestPipe[1]);
			// Close the read end of the result pipe.
			close(resultPipe[0]);
			
			// Close all open fds the parent had for previous workers.
			for (int j=0; j<i; j++) {
				close(manager->requestPipeWriteFds[j]);
				close(manager->pollFds[j].fd);
			}
			
			// Redirect STDOUT to the write end of the result pipe.
			dup2(resultPipe[1], STDOUT_FILENO);
			// Redirect STDIN to the read end of the request pipe.
			dup2(requestPipe[0], STDIN_FILENO);
			
			execl(WORKER_EXEC_FILE, WORKER_EXEC_FILE, NULL);
			
			// This only runs if the exec fails.
			fprintf(stderr, "[Worker] Worker with id %u failed to exec %s: ", i, WORKER_EXEC_FILE);
			perror(NULL);
			close(requestPipe[0]);
			close(resultPipe[1]);
			exit(EXIT_CODE_EXEC_FAILED);
		}
		
		// The parent process now stores the fds it wants to keep
		// and closes the ones it doesn't.
		manager->requestPipeWriteFds[i] = requestPipe[1];
		manager->pollFds[i].fd = resultPipe[0];
		
		// Close the read end of the request pipe.
		close(requestPipe[0]);
		// Close the write end of the result pipe.
		close(resultPipe[1]);
	}
	
	return 1;
}

workerManagerADT newWorkerManager(unsigned int workerCount) {
	if (workerCount == 0) {
		fprintf(stderr, "[Master] Worker manager warning: New worker manager created with workerCount = 0.\n");
		return NULL;
	}
	
	// We attempt to allocate memory for the CDT struct. If this fails, we return NULL.
	workerManagerADT manager;
	if (!tryCalloc((void**)&manager, 1, sizeof(struct workerManagerCDT)))
		return NULL;
	
	// We attempt to allocate the memory needed for the workerManagerADT. If any
	// of the allocs fail, we free them all and return NULL.
	if (!tryCalloc((void**)&manager->requestPipeWriteFds, workerCount, sizeof(int))
		|| !tryCalloc((void**)&manager->remainingTaskCounts, workerCount, sizeof(unsigned int))
		|| !tryCalloc((void**)&manager->pollFds, workerCount, sizeof(struct pollfd))
		|| !tryMalloc((void**)&manager->pollFdsWorkerIds, sizeof(unsigned int) * workerCount)
		|| !tryMalloc((void**)&manager->resultBufs, sizeof(TWorkerResult) * workerCount)
		|| !tryCalloc((void**)&manager->resultBufCounts, workerCount, sizeof(int))) {
		
		fprintf(stderr, "[Master] Worker manager error: failed to allocate enough memory for manager with %u workers.\n", workerCount);
		
		// It's ok to call free() on all of them because if they weren't allocated,
		// they will have NULL (note that the struct workerManaerCDT is zeroed out
		// by calloc), and free() ignores calls with NULL.
		freeWorkerManager(manager);
		return NULL;
	}
	
	// Set initial values on the manager struct.
	manager->workerCount = workerCount;
	manager->remainingWorkerCount = workerCount;
	
	for (int i=0; i<workerCount; i++) {
		struct pollfd* p = &manager->pollFds[i];
		p->events = POLLIN;
		p->revents = 0;
		manager->pollFdsWorkerIds[i] = i;
	}
	
	// Attempt to spawn the workers. If this fails, close all open pipe fds,
	// free the manager and return NULL.
	if (!performWorkerSpawning(manager)) {
		// Both of these arrays are zeroed on allocation, so if performWorkerSpawning()
		// didn't create a fd yet it's left as 0. We close all the non-zero ones.
		for (int i=0; i<workerCount; i++) {
			if (manager->requestPipeWriteFds[i]) close(manager->requestPipeWriteFds[i]);
			if (manager->pollFds[i].fd) close(manager->pollFds[i].fd);
		}
		freeWorkerManager(manager);
		return NULL;
	}
	
	return manager;
}

void freeWorkerManager(workerManagerADT manager) {
	free(manager->requestPipeWriteFds);
	free(manager->remainingTaskCounts);
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

unsigned int getWorkerRemainingTasks(workerManagerADT manager, unsigned int workerId) {
	if (workerId >= manager->workerCount)
		return 0;
	
	return manager->remainingTaskCounts[workerId];
}

void sendWorkerTask(workerManagerADT manager, unsigned int workerId, unsigned int taskId, const char* filepath) {
	if (workerId >= manager->workerCount) {
		fprintf(stderr, "[Master] Worker manager error: sendWorkerTask() to id %u, but max worker id is %u.\n", workerId, manager->workerCount-1);
		return;
	}
	
	if (manager->requestPipeWriteFds[workerId] < 0) {
		fprintf(stderr, "[Master] Worker manager error: sendWorkerTask() to id %u, but worker is closed.\n", workerId);
		return;
	}
	
	TWorkerRequest request;
	request.taskId = taskId;
	request.filepathLength = strlen(filepath);
	
	// Attempt to do both writes. If the first one fails, the second one is skipped.
	// writeResult will be 1 if both writes succeeded.
	int writeResult = writeFull(manager->requestPipeWriteFds[workerId], &request, sizeof(TWorkerRequest))
		&& writeFull(manager->requestPipeWriteFds[workerId], filepath, request.filepathLength);
	
	// Handle any possible errors during write.
	if (!writeResult) {
		fprintf(stderr, "[Master] Worker manager error: sendWorkerTask() to id %u failed to write on it's request pipe.\n", workerId);
	}
	
	manager->remainingTaskCounts[workerId]++;
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
		fprintf(stderr, "[Master] Worker manager error: closeWorker() with id %u, but max worker id is %u.\n", workerId, manager->workerCount-1);
		return 0;
	}
	
	if (manager->requestPipeWriteFds[workerId] < 0) {
		fprintf(stderr, "[Master] Worker manager warning: Attempted to close already closed worker id %u.\n", workerId);
		return 0;
	}
	
	if (close(manager->requestPipeWriteFds[workerId])) {
		fprintf(stderr, "[Master] Worker manager warning: Failed to close request pipe write end for worker %u.\n", workerId);
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
		fprintf(stderr, "[Master] Worker manager warning: Worker %u closed, but close() failed on its result pipe fd.\n", closingWorkerId);
	}
	
	if (manager->requestPipeWriteFds[closingWorkerId] != -1) {
		close(manager->requestPipeWriteFds[closingWorkerId]);
		manager->requestPipeWriteFds[closingWorkerId] = -1;
		fprintf(stderr, "[Master] Worker manager warning: Worker %u closed, but its request pipe is still open. Did worker crash?\n", closingWorkerId);
	}
	
	if (manager->remainingTaskCounts[closingWorkerId]) {
		fprintf(stderr, "[Master] Worker manager warning: Worker %u closed, but it still had %u task/s awaiting results. Did worker crash?\n", closingWorkerId, manager->remainingTaskCounts[closingWorkerId]);
	}
	
	if (manager->resultBufCounts[bufIndex]) {
		fprintf(stderr, "[Master] Worker manager warning: Worker %u closed, but it still had %i byte/s in the result buffer.\n", closingWorkerId, manager->resultBufCounts[bufIndex]);
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
		fprintf(stderr, "[Master] Worker manager warning: Error while reading from worker %u result pipe: ", manager->pollFdsWorkerIds[bufIndex]);
		perror(NULL);
		return 0;
	}
	
	// We check if total amount of bytes that was read from the result of this worker
	// is enough to form a whole TWorkerResult struct. If so, we invoke the result callback.
	t += manager->resultBufCounts[bufIndex];
	
	if (t >= sizeof(TWorkerResult)) {
		
		// This should never happen, but let's handle it just in case.
		if (t > sizeof(TWorkerResult))
			fprintf(stderr, "[Master] Worker manager error: Failed to read from worker %u result pipe, too many bytes read?? Your linux is broken lololol\n", manager->pollFdsWorkerIds[bufIndex]);
		
		manager->resultBufCounts[bufIndex] = 0;
		manager->remainingTaskCounts[manager->pollFdsWorkerIds[bufIndex]]--;
		
		if (manager->resultCallback) {
			manager->resultCallback(manager, manager->pollFdsWorkerIds[bufIndex], &manager->resultBufs[bufIndex], manager->resultCallbackArg);
		} else {
			fprintf(stderr, "[Master] Worker manager warning: Worker %u returned result got discarded because result callback is null.\n", manager->pollFdsWorkerIds[bufIndex]);
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