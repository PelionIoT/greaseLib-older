#include <stdio.h>

#include "logger.h"
#include "grease_lib.h"
#include <unistd.h>
#include <string.h>

void loggerStartedCB(GreaseLibError *, void *d) {
	printf("Logger started.\n");
}

GreaseLibFilter *f1, *f2, *f3;

void filterAddCB(GreaseLibError *err, void *d) {
	if(!err) {
		printf("Filter added\n" );
	} else {
		printf("Filter add failure: %d %s\n",err->_errno, err->errstr);
	}
}

char output_buf[5128];

void targetCallback(GreaseLibError *err, void *d, uint32_t targetId) {
	printf("**** in targetCallback - targId %d\n", targetId);
	GreaseLibBuf *buf = (GreaseLibBuf *)d;
	if(buf->size < 5127) {
		memcpy(output_buf,buf->data,buf->size);
		*(buf->data+buf->size+1) = '\0';
		printf("CALLBACK TARGET>>>>>>>>>%s<<<<<<<<<<<<<<<<\n",output_buf);
	} else {
		printf("OOOPS. Overflow on test output. size was %lu\n",buf->size);
	}
	GreaseLib_cleanup_GreaseLibBuf(buf);
}

#define CALLBACK_TARG_OPTID 99

void targetAddCB(GreaseLibError *err, void *d) {
	if(!err) {
		GreaseLibStartedTargetInfo *info = 	(GreaseLibStartedTargetInfo *) d;
		printf("Target added - ID:%d (optsID:%d)\n",info->targId,info->optsId );
		if(info->optsId == 0) {
			printf("adding Filter\n");
			GreaseLib_setvalue_GreaseLibFilter(f1,GREASE_LIB_SET_FILTER_TARGET,info->targId);
			GreaseLib_addFilter(f1);
		}
		if(info->optsId == CALLBACK_TARG_OPTID) {
			printf("adding Filter for callback target\n");
			GreaseLib_setvalue_GreaseLibFilter(f3,GREASE_LIB_SET_FILTER_MASK,GREASE_ALL_LEVELS);
			GreaseLib_setvalue_GreaseLibFilter(f3,GREASE_LIB_SET_FILTER_TARGET,info->targId);
			GreaseLib_addFilter(f3);
		}
	} else {
		printf("Target Failure: %d %s\n",err->_errno, err->errstr);
	}
}

const char *level_format = "%-10s ";
const char *tag_format = " [%-15s] ";
const char *origin_format = " (%-15s) ";
const int WAITSECS = 90;

int main() {
	GreaseLib_start(loggerStartedCB);

	printf("before sleep ... 5 seconds\n");
	sleep(5);
	printf("after sleep - setup sink\n");

	GreaseLib_setupStandardLevels();
	GreaseLib_setupStandardTags();
	// test setting up a sink

	GreaseLibSink *sink = GreaseLib_new_GreaseLibSink(GREASE_LIB_SINK_UNIXDGRAM,"/tmp/testsocket");

	// setup a file destination

	GreaseLibTargetOpts *target = GreaseLib_new_GreaseLibTargetOpts();

	char outFile[] = "/tmp/output.log";

	target->file = outFile;
	target->format_level = (char *) level_format;
	target->format_level_len = strlen(level_format);
	target->format_tag = (char *) tag_format;
	target->format_tag_len = strlen(tag_format);
	target->format_origin = (char *) origin_format;
	target->format_origin_len = strlen(origin_format);

	target->fileOpts = GreaseLib_new_GreaseLibTargetFileOpts();
	f1 = GreaseLib_new_GreaseLibFilter();

	GreaseLib_addTarget(targetAddCB, target);

	f3 = GreaseLib_new_GreaseLibFilter();

	// another target... will be ignored - since nothing if directed to it yet
	GreaseLibTargetOpts *target2 = GreaseLib_new_GreaseLibTargetOpts();
//	target2->file = outFile;
//	target2->fileOpts = GreaseLib_new_GreaseLibTargetFileOpts();
	target2->optsId = CALLBACK_TARG_OPTID;
	target2->targetCB = targetCallback;
	target2->num_banks = 10; // default is 4, let's make it
	GreaseLib_set_flag_GreaseLibTargetOpts(target2,GREASE_JSON_ESCAPE_STRINGS);
	GreaseLib_addTarget(targetAddCB, target2);

	int ret;

	if((ret = GreaseLib_addSink(sink)) != GREASE_LIB_OK) {
		printf("ERROR on addSink(): %d",ret);
	}
	printf("after setup sink\n");


	GreaseLibSink *sink2 = GreaseLib_new_GreaseLibSink(GREASE_LIB_SINK_SYSLOGDGRAM,"/dev/log");

	if((ret = GreaseLib_addSink(sink2)) != GREASE_LIB_OK) {
		printf("ERROR on addSink(): %d",ret);
	}
	printf("after setup sink\n");



	printf("Will shutdown in %d seconds\n",WAITSECS);

	GreaseLib_waitOnGreaseShutdown();
	//	sleep(WAITSECS);
//
//	printf("sleep over\n");
//
//	GreaseLib_shutdown(NULL);

}
;
