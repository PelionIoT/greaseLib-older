#include <stdio.h>

#include "logger.h"
#include "grease_lib.h"


void loggerStartedCB(GreaseLibError *, void *) {
	printf("Logger started.");
}

int main() {
	GreaseLib_Start(loggerStartedCB);


}
