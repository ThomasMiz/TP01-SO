#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include "app.h"

/** Spawns the specified amount of workers, creates the pipes to
 * send tasks to them and places the write end fd of those pipes
 * in the specified array. */
void spawnWorkers(unsigned int workerCount, int requestPipeWriteFds[], int resultPipeReadFds[]);

int main(int argc, const char* argv[]) {
	if (argc <= 1) {
		fprintf(stderr, "[APP] Error: no input files.\n");
		return EXIT_CODE_NOT_ENOUGH_PARAMS;
	}
	
	// The amount of workers to spawn will be the amount of files
	// we have to process, clamped to the maximum allowed.
	unsigned int workerCount = argc - 1;
	if (workerCount > MAX_WORKERS)
		workerCount = MAX_WORKERS;
	
	int resultPipeReadFds[workerCount];
	int requestPipeWriteFds[workerCount];
	spawnWorkers(workerCount, requestPipeWriteFds, resultPipeReadFds);
	
	fprintf(stderr, "App process sending some shit to workers\n");
	
	// Pipe test: send a simple string to all workers.
	for (int i=0; i<workerCount; i++) {
		write(requestPipeWriteFds[i], "hola! :D", 8);
		close(requestPipeWriteFds[i]);
	}
	
	// Pipe test: send to STDOUT what comes in through results.
	
	// Make a list of the fds and events we want to monitor.
	struct pollfd fds[workerCount];
	for (int i=0; i<workerCount; i++) {
		fds[i].fd = resultPipeReadFds[i];
		fds[i].events = POLLIN;
		fds[i].revents = 0;
	}
	
	int remainingWorkers = workerCount;
	while (remainingWorkers > 0) {
		// We wait until something happens in one of our fds.
		int pollres = poll(fds, remainingWorkers, -1);
		
		if (pollres <= 0) {
			fprintf(stderr, "[APP] poll() returned value <= 0. Ignoring."); // TODO: remove print?
			continue;
		}
		
		for (int i=0; pollres != 0 && i < remainingWorkers; i++) {
			if (fds[i].revents) {
				pollres--;
				if (fds[i].revents & POLLIN) {
					char c;
					write(STDOUT_FILENO, &c, read(fds[i].fd, &c, 1));
				} else {
					// Remove this fd from the list.
					remainingWorkers--;
					if (remainingWorkers != 0)
						fds[i] = fds[remainingWorkers]; // Remove via swap
					break;
				}
			}
		}
	}
	
	printf("\nApp process closing.\n");
	return 0;
}

void spawnWorkers(unsigned int workerCount, int requestPipeWriteFds[], int resultPipeReadFds[]) {
	int requestPipe[2];
	int resultPipe[2];
	
	for (unsigned int i=0; i<workerCount; i++) {
		// We attempt to create the request pipe and handle any errors.
		if (pipe(requestPipe)) {
			fprintf(stderr, "[APP] Error: Failed to create request pipe for worker %i: ", i);
			perror(NULL);
			exit(EXIT_CODE_CREATE_PIPE_FAILED);
		}
		
		// We attempt to create the result pipe and handle any errors.
		if (pipe(resultPipe)) {
			fprintf(stderr, "[APP] Error: Failed to create result pipe for worker %i: ", i);
			perror(NULL);
			exit(EXIT_CODE_CREATE_PIPE_FAILED);
		}
		
		requestPipeWriteFds[i] = requestPipe[1];
		resultPipeReadFds[i] = resultPipe[0];
		
		// We create the child process and handle any errors.
		pid_t forkResult = fork();
		if (forkResult == -1) {
			fprintf(stderr, "[APP] Error: Failed to create fork for worker %i: ", i);
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
			fprintf(stderr, "[WORKER %u] Failed to exec %s: ", i, WORKER_EXEC_FILE);
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