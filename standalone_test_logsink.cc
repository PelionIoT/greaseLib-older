#include <stdio.h>

#include "logger.h"
#include "grease_lib.h"
#include <unistd.h>

void loggerStartedCB(GreaseLibError *, void *) {
	printf("Logger started.\n");
}

int main() {
	GreaseLib_start(loggerStartedCB);

	printf("before sleep ... 5 seconds\n");
	sleep(5);
	printf("after sleep - setup sink\n");

	// test setting up a sink

	GreaseLibSink *sink = GreaseLib_new_GreaseLibSink(GREASE_LIB_SINK_UNIXDGRAM,"/tmp/testsocket");

	int ret;

	if((ret = GreaseLib_addSink(sink)) != GREASE_LIB_OK) {
		printf("ERROR on addSink(): %d",ret);
	}
	printf("after setup sink\n");




	sleep(5);

	printf("sleep over\n");

	GreaseLib_shutdown(NULL);

}
