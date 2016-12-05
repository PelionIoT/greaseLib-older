#include <stdio.h>

#include "logger.h"
#include "grease_lib.h"
#include <unistd.h>

void loggerStartedCB(GreaseLibError *, void *) {
	printf("Logger started.\n");
}

int main() {
	GreaseLib_start(loggerStartedCB);

	printf("before sleep ... 10 seconds\n");
	sleep(10);
	printf("after sleep\n");

	GreaseLib_shutdown(NULL);

}
