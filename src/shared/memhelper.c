#include <stdlib.h>
#include <stdio.h>
#include "memhelper.h"

void* mallocOrExit(size_t size) {
	void* ptr = malloc(size);
	
	if (ptr == NULL && size != 0) {
		fprintf(stderr, "Not enough memory: failed to allocate %lu bytes.\n", size);
		exit(EXIT_CODE_NOT_ENOUGH_MEMORY);
	}
	
	return ptr;
}

void* reallocOrExit(void* ptr, size_t size) {
	ptr = realloc(ptr, size);
	
	if (ptr == NULL && size != 0) {
		fprintf(stderr, "Not enough memory: failed to allocate %lu bytes.\n", size);
		exit(EXIT_CODE_NOT_ENOUGH_MEMORY);
	}
	
	return ptr;
}

void* callocOrExit(size_t nmemb, size_t size) {
	void* ptr = calloc(nmemb, size);
	
	if (ptr == NULL && size != 0) {
		fprintf(stderr, "Not enough memory: failed to allocate %lu bytes.\n", size);
		exit(EXIT_CODE_NOT_ENOUGH_MEMORY);
	}
	
	return ptr;
}