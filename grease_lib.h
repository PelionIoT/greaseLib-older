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


#ifdef __cplusplus
extern "C" {
#endif

typedef void (*GreaseLibCallback) (int err, void *);



#ifdef __cplusplus
}
#endif


#define LIB_METHOD( name, ... ) void name( GreaseLibCallback libCB, __VA_ARGS__ )
	