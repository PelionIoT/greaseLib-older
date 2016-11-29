/*
 * GreaseLogger.h
 *
 * greaselib bindings
 * launch the grease logging process via these library calls
 *
 *  Created on: Nov 23, 2016
 *      Author: ed
 * (c) 2016, WigWag Inc
 */


#ifndef GreaseLib_H_
#define GreaseLib_H_
#include <gperftools/tcmalloc.h>
#include <stdint.h>

#define GREASE_LIB_OK 0

#define LIB_METHOD( name, ... ) int GreaseLib_##name( GreaseLibCallback libCB, ## __VA_ARGS__ )
#define LIB_METHOD_FRIEND( name, ... ) friend int ::GreaseLib_##name( GreaseLibCallback libCB, ## __VA_ARGS__ )

#ifdef __cplusplus
extern "C" {
#endif

#define GREASE_LIB_MAX_ERR_STRING 256

typedef struct {
	char errstr[GREASE_LIB_MAX_ERR_STRING];
	int _errno;
} GreaseLibError;

typedef void (*GreaseLibCallback) (GreaseLibError *, void *);

typedef struct {
	char *data;
	size_t size;
	int own;    // if > 0 then cleanup call will free this memory
} GreaseLibBuf;

void GreaseLib_init_GreaseLibBuf(GreaseLibBuf *b);
GreaseLibBuf *GreaseLib_new_GreaseLibBuf(size_t l);
void GreaseLib_cleanup_GreaseLibBuf(GreaseLibBuf *b);


typedef struct {
	uint32_t LevelFilterOut;
	bool defaultFilterOut;
	bool show_filters;
	bool callback_errors;
} GreaseLibOpts;

LIB_METHOD(setGlobalOpts, GreaseLibOpts *opts);
LIB_METHOD(Start);


#ifdef __cplusplus
}
#endif

#endif
