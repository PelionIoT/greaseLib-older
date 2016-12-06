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
#include "grease_client.h"
#include <gperftools/tcmalloc.h>
#include <stdint.h>

// typically defined in <linux/limits.h>
#define GREASE_PATH_MAX        4096

#define GREASE_LIB_OK 0
#define GREASE_LIB_NOT_FOUND 0x01E00000

#define LIB_METHOD( name, ... ) int GreaseLib_##name( GreaseLibCallback libCB, ## __VA_ARGS__ )
#define LIB_METHOD_FRIEND( name, ... ) friend int ::GreaseLib_##name( GreaseLibCallback libCB, ## __VA_ARGS__ )

#define LIB_METHOD_SYNC( name, ... ) int GreaseLib_##name( __VA_ARGS__ )
#define LIB_METHOD_SYNC_FRIEND( name, ... ) friend int ::GreaseLib_##name( __VA_ARGS__ )

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
LIB_METHOD(start);
LIB_METHOD(shutdown);
LIB_METHOD_SYNC(addTagLabel, uint32_t val, char *utf8, int len);
LIB_METHOD_SYNC(addOriginLabel, uint32_t val, char *utf8, int len);
LIB_METHOD_SYNC(addLevelLabel, uint32_t val, char *utf8, int len);
LIB_METHOD_SYNC(maskOutByLevel, uint32_t val);
LIB_METHOD_SYNC(unmaskOutByLevel, uint32_t val);


#define GREASE_LIB_SET_FILEOPTS_MODE           0x10000000
#define GREASE_LIB_SET_FILEOPTS_FLAGS          0x20000000
#define GREASE_LIB_SET_FILEOPTS_MAXFILES       0x40000000
#define GREASE_LIB_SET_FILEOPTS_MAXFILESIZE    0x80000000
#define GREASE_LIB_SET_FILEOPTS_MAXTOTALSIZE   0x01000000
#define GREASE_LIB_SET_FILEOPTS_ROTATEONSTART  0x02000000  // set if you want files to rotate on start
#define GREASE_LIB_SET_FILEOPTS_ROTATE         0x04000000  // set if you want files to rotate, if not set all other rotate options are skipped

typedef struct {
	uint32_t _enabledFlags;
	uint32_t mode;           // permissions for file (recommend default)
	uint32_t flags;          // file flags (recommend default)
	uint32_t max_files;      // max # of files to maintain (rotation)
	uint32_t max_file_size;  // max size for any one file
	uint64_t max_total_size; // max total size to maintain in rotation
} GreaseLibTargetFileOpts;

typedef struct {
	char *delim;             // points to delimetter UTF8, does not need NULL termination, NULL pointer will use default value
	int len_delim;           // length of above buffer
	char *delim_output;
	int len_delim_output;
	char *tty;
	char *file;
	GreaseLibCallback targetCB; // used if this is target is a callback
	GreaseLibTargetFileOpts *fileOpts; // NULL if not needed
	char *format_pre;
	int format_pre_len;
	char *format_time;
	int format_time_len;
	char *format_level;
	int format_level_len;
	char *format_tag;
	int format_tag_len;
	char *format_origin;
	int format_origin_len;
	char *format_post;
	int format_post_len;
	char *format_pre_msg;
	int format_pre_msg_len;
} GreaseLibTargetOpts;

GreaseLibTargetFileOpts *GreaseLib_new_GreaseLibTargetFileOpts();
void GreaseLib_cleanup_GreaseLibTargetFileOpts(GreaseLibTargetFileOpts *opts);
void GreaseLib_set_flag_GreaseLibTargetFileOpts(GreaseLibTargetFileOpts *opts,uint32_t flag);

GreaseLibTargetOpts *GreaseLib_new_GreaseLibTargetOpts();
void GreaseLib_cleanup_GreaseLibTargetOpts(GreaseLibTargetOpts *opts);
void GreaseLib_set_string_GreaseLibTargetFileOpts(GreaseLibTargetFileOpts *opts,uint32_t flag,const char *s);

LIB_METHOD_SYNC(modifyDefaultTarget,GreaseLibTargetOpts *opts);
LIB_METHOD(addTarget,GreaseLibTargetOpts *opts);

//* addFilter(obj) where
//* obj = {
//*      // at least origin and/or tag must be present
//*      origin: 0x33,    // any positive number > 0
//*      tag: 0x25        // any positive number > 0,
//*      target: 3,       // mandatory
//*      mask:  0x4000000 // optional (default, show everything: 0xFFFFFFF),
//*      format: {        // optional (formatting settings)
//*      	pre: 'targ-pre>', // optional pre string
//*      	post: '<targ-post;
//*      }
//* }

#define GREASE_LIB_SET_FILTER_ORIGIN  0x1
#define GREASE_LIB_SET_FILTER_TAG     0x2
#define GREASE_LIB_SET_FILTER_TARGET  0x4
#define GREASE_LIB_SET_FILTER_MASK    0x8

typedef struct {
	uint32_t _enabledFlags;
	uint32_t origin;
	uint32_t tag;
	uint32_t target;
	uint32_t mask;
	FilterId id;
	char *format_pre;
	int format_pre_len;
	char *format_post;
	int format_post_len;
	char *format_post_pre_msg;
	int format_post_pre_msg_len;
} GreaseLibFilter;

GreaseLibFilter *GreaseLib_new_GreaseLibFilter();
void GreaseLib_cleanup_GreaseLibFilter(GreaseLibFilter *filter);
void GreaseLib_setvalue_GreaseLibFilter(GreaseLibFilter *opts,uint32_t flag,uint32_t val);

LIB_METHOD_SYNC(addFilter,GreaseLibFilter *filter);
LIB_METHOD_SYNC(disableFilter,GreaseLibFilter *filter);
LIB_METHOD_SYNC(enableFilter,GreaseLibFilter *filter);
//LIB_METHOD_SYNC(modifyDefaultTarget,GreaseLibTargetOpts *opts);

#define GREASE_LIB_SINK_UNIXDGRAM 0x1
#define GREASE_LIB_SINK_PIPE 0x2

typedef struct {
	char path[GREASE_PATH_MAX];
	uint32_t sink_type;
	SinkId id;
} GreaseLibSink;

GreaseLibSink *GreaseLib_new_GreaseLibSink(uint32_t sink_type, const char *path);
void GreaseLib_cleanup_GreaseLibSink(GreaseLibSink *sink);

LIB_METHOD_SYNC(addSink,GreaseLibSink *sink);

LIB_METHOD_SYNC(disableTarget, TargetId id);
LIB_METHOD_SYNC(enableTarget, TargetId id);
LIB_METHOD_SYNC(flush, TargetId id);

#ifdef __cplusplus
}
#endif

#endif
