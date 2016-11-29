/*
 * local_strdup.c
 *
 *  Created on: May 22, 2015
 *      Author: ed
 * (c) 2015, WigWag Inc.

	A simple work around for strdup() when also using XSI compatible functions - strerror_r
 */

#include <string.h>
#include <stdio.h>


#define MAX_STRDUP 4096

char *local_strdup_safe(const char *s) {
	char *ret = NULL;
	if(s) {
		int n = strlen(s);
		if(n > MAX_STRDUP) {
			printf("Overflow on local_strdup_safe() %d\n",n);
			n = MAX_STRDUP;
		}
		ret = (char *) malloc(n + 1);
		memcpy(ret,s,n);
		*(ret+n) = '\0';
	}
	return ret;
}
