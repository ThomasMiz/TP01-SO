#include <stdio.h>
#include "./../shared/shared.h"

int main(int argc, const char* argv[]) {
	printf("Yo soy el proceso vista!\n");
	printf("Shared string: %s\n", getSomeSharedString());
	return 0;
}