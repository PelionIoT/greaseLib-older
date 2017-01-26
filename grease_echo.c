/*
 * send-multiple-test.c
 *
 *  Created on: Sep 3, 2015
 *      Author: ed
 * (c) 2015, WigWag Inc.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

//#define GREASE_DEBUG_MODE 1
#include "grease_common_tags.h"
#define GREASE_NO_DEFAULT_NATIVE_ORIGIN 1    // we dont need to specify an origin with this client
#define GLOG_DEFAULT_TAG GREASE_TAG_ECHO
#include "grease_client.h"

#define ERROR_OUT(s,...) fprintf(stderr, "**ERROR** " s, ##__VA_ARGS__ )//#define ERROR_PERROR(s,...) fprintf(stderr, "*****ERROR***** " s, ##__VA_ARGS__ );
#define ERROR_PERROR(s,...) { perror("**ERROR** [ %s ] " s, ##__VA_ARGS__ ); }
#define DBG_OUT(s,...) fprintf(stderr, "**DEBUG** " s, ##__VA_ARGS__ )
#define IF_DBG( x ) { x }


void bye(int e) {
	SHUTDOWN_GLOG;
	exit(e);
}

int main(int argc, char *argv[]) {
	if(argc < 2) {
		exit(1);
	}

	int n = 1;
	char *socket_path = NULL;
	int opt_n = 0;

	if(argv[n][0] == '-' && argv[n][1] == '-') {
		if(!strcmp(argv[1]+2,"help")) {
			printf("Usage: grease_echo [--check] [--socket PATH] [--[LEVEL]] \"string here\"\n"
				   "            --socket PATH  use a custom path to the grease socket. Must be first argument.\n"
				   "                           Needs an absolute path.\n"
				   "            --check        will check to see if logger is live. (Use with no other args)\n"
				   "LEVELs:     --error\n"
				   "            --warn\n"
				   "            --success\n"
				   "            --info\n"
				   "            --debug\n"
				   "            --debug2\n"
				   "            --debug3\n"
				   "            --user1\n"
				   "            --user2\n");
			exit(1);
		}
		if(!strcmp(argv[1]+2,"check")) {
			if(grease_initLogger() != GREASE_OK) {
				printf("Grease not running.\n");
				bye(1);
			} else {
				printf("Grease running & OK.\n");
				bye(0);
			}
		}


		if(!strcmp(argv[1]+2,"socket")){
			socket_path = argv[2];
			opt_n += 2;
		}

		if(grease_fastInitLogger_extended(socket_path) != GREASE_OK) {
			fprintf(stderr,"    Error: Grease not running.\n");
		}

		if(argc > opt_n+2 && argv[opt_n+2][0] != '\0') {
			if(!strcmp(argv[n+1]+2,"info")) {
				GLOG_INFO(argv[2]);
				bye(0);
			} else
			if(!strcmp(argv[opt_n+1]+2,"error")) {
				GLOG_ERROR(argv[opt_n+2]);
				bye(0);
			} else
			if(!strcmp(argv[opt_n+1]+2,"warn")) {
				GLOG_WARN(argv[opt_n+2]);
				bye(0);
			} else
			if(!strcmp(argv[opt_n+1]+2,"success")) {
				GLOG_SUCCESS(argv[opt_n+2]);
				bye(0);
			} else
			if(!strcmp(argv[opt_n+1]+2,"debug")) {
				GLOG_DEBUG(argv[opt_n+2]);
				bye(0);
			} else
			if(!strcmp(argv[opt_n+1]+2,"debug2")) {
				GLOG_DEBUG2(argv[opt_n+2]);
				bye(0);
			} else
			if(!strcmp(argv[opt_n+1]+2,"debug3")) {
				GLOG_DEBUG3(argv[opt_n+2]);
				bye(0);
			} else
			if(!strcmp(argv[opt_n+1]+2,"user1")) {
				GLOG_USER1(argv[opt_n+2]);
				bye(0);
			} else
			if(!strcmp(argv[opt_n+1]+2,"user2")) {
				GLOG_USER2(argv[opt_n+2]);
				bye(0);
			} else {
				fprintf(stderr,"grease_echo: Unknown LEVEL.\n");
				GLOG(argv[opt_n+2]);
				bye(1);
			}
		}
	}

	if(grease_fastInitLogger_extended(socket_path) != GREASE_OK) {
		fprintf(stderr,"    Error: Grease not running.\n");
	}
	if(argv[opt_n+1][0] != '\0') {
		GLOG(argv[opt_n+1]);
	}
	bye(0);

	return 0;
}


/**
 * BUILD:  gcc grease_echo.c  grease_client.c -std=c99 -o grease_echo -ldl
 */
