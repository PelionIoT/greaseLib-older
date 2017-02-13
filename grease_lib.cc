/*
 * grease_lib.c
 *
 *  Created on: Nov 26, 2016
 *      Author: ed
 * (c) 2016, WigWag Inc.
 */

#include <uv.h>
#include <stdlib.h>
#include <string.h>
#include "grease_lib.h"
#include "grease_client.h"
#include "logger.h"

using namespace Grease;

void GreaseLib_init_GreaseLibBuf(GreaseLibBuf *b)
{
	b->data = NULL;
	b->size = 0;
	b->own = 0;
}

GreaseLibBuf *GreaseLib_new_GreaseLibBuf(size_t size) {
	GreaseLibBuf *b = (GreaseLibBuf *) LMALLOC((int ) sizeof(GreaseLibBuf) + size);
	b->data = (char *) b + sizeof(GreaseLibBuf);
	b->size = 0;
	b->own = 1;
	return b;
}

void GreaseLib_cleanup_GreaseLibBuf(GreaseLibBuf *b) {
	if(b && b->own > 0) {
		LFREE(b);
	}
}

uv_thread_t libThread;
uv_loop_t libLoop;
uv_timer_t idleTimer;
bool libStarted = false;

//void libTimerCB(uv_timer_t* handle) {
//    // Compute extra-terrestrial life
//    // fold proteins
//    // computer another digit of PI
//    // or similar
//    printf("LibTimer\n");
////    // just to avoid overwhelming your terminal emulator
////    uv_idle_stop(handle);
//}
///**
// * This is main thread for the library. It does not do the loggin processing, which is handled by
// * the logger's threads. Since the greaseLogger depends on an lib_uv event loop, this starts that loop
// */
//void libMainThread(void *arg) {
//	libLoop = (uv_loop_t *) ::malloc(sizeof(uv_loop_t));
//	uv_loop_init(libLoop);
//
//    uv_idle_t idler;
//
////    uv_idle_init(libLoop, &idler);
////    uv_idle_start(&idler, lib_idle);
//
//
//
//    uv_timer_init(libLoop, &libMainTimer);
//    uv_timer_start(&libMainTimer, libTimerCB, 2000, 2000);
//
//
//
//	// if(info.Length() > 0 && info[0]->IsFunction()) {
//	// 	startinfo->targetStartCB = new Nan::Callback(Local<Function>::Cast(info[0]));
//	// }
//
//	uv_run(libLoop, UV_RUN_DEFAULT);
//}


//static int N = 0;
//void lib_idle(uv_idle_t* handle) {
//
//	printf("IDLE %d\n",N);
//	N++;
//
//	if(N > 100) {
//		uv_idle_stop(handle);
//	}
//}

int observationCounter;

void libraryMain(void *arg) {
	libStarted = true;
// Callbacks will occur in this thread.
	uv_run(&libLoop, UV_RUN_DEFAULT);
}

void heartbeat(uv_timer_t* handle) {
	observationCounter++;
	//	printf("\nheartbeat.\n\n");
}

LIB_METHOD(start) {
	uv_loop_init(&libLoop);

	observationCounter = 0;
	// spawn thread
	uv_thread_create(&libThread, libraryMain, NULL);
	// timer keeps uv_run up
	uv_timer_init(&libLoop, &idleTimer);
	uv_timer_start(&idleTimer, heartbeat, 5000, 2000);
	GreaseLogger *l = GreaseLogger::setupClass(DEFAULT_BUFFER_SIZE,LOGGER_DEFAULT_CHUNK_SIZE,&libLoop);
	GreaseLogger::target_start_info *startinfo = new GreaseLogger::target_start_info();

	if(libCB) startinfo->targetStartCB = libCB;
	l->start(GreaseLogger::start_logger_cb, startinfo);

	return GREASE_LIB_OK;
}

LIB_METHOD_SYNC(refLoop) {

}

LIB_METHOD(shutdown) {
	if(libStarted) {
		printf("got shutdown");
		uv_timer_stop(&idleTimer);
		uv_loop_close(&libLoop);
//		uv_timer_stop(&libMainTimer);
	}
	return GREASE_LIB_OK;
}


LIB_METHOD(setGlobalOpts, GreaseLibOpts *opts) {
//	NAN_METHOD(GreaseLogger::SetGlobalOpts) {
//
//		GreaseLogger *l = GreaseLogger::setupClass();
//		Local<Function> cb;
//
//		if(info.Length() < 1 || !info[0]->IsObject()) {
//			Nan::ThrowTypeError("setGlobalOpts: bad parameters");
//			return;
//		}
//
//		Local<Object> jsObj = info[0]->ToObject();
//	//	l->levelFilterOutMask
//		Local<Value> jsVal = jsObj->Get(Nan::New("levelFilterOutMask").ToLocalChecked());
//
//		if(jsVal->Uint32Value()) {
//			l->Opts.levelFilterOutMask = jsVal->Uint32Value();
//		}
//
//		jsVal = jsObj->Get(Nan::New("defaultFilterOut").ToLocalChecked());
//		if(jsVal->IsBoolean()) {
//			bool v = jsVal->ToBoolean()->BooleanValue();
//			uv_mutex_lock(&l->modifyFilters);
//			l->Opts.defaultFilterOut = v;
//			uv_mutex_unlock(&l->modifyFilters);
//		}
//
//		jsVal = jsObj->Get(Nan::New("show_errors").ToLocalChecked());
//		if(jsVal->IsBoolean()) {
//			bool v = jsVal->ToBoolean()->BooleanValue();
//			l->Opts.show_errors = v;
//		}
//		jsVal = jsObj->Get(Nan::New("callback_errors").ToLocalChecked());
//		if(jsVal->IsBoolean()) {
//			bool v = jsVal->ToBoolean()->BooleanValue();
//			l->Opts.callback_errors = v;
//		}
//	}

	if(opts) {
		GreaseLogger *l = GreaseLogger::setupClass();
		l->Opts.levelFilterOutMask = opts->LevelFilterOut;
		uv_mutex_lock(&l->modifyFilters);
		l->Opts.defaultFilterOut = opts->defaultFilterOut;
		uv_mutex_unlock(&l->modifyFilters);


	} else {
		return GREASE_INVALID_PARAMS;
	}

	return GREASE_LIB_OK;
}


/**
 * addTagLabel(id,label,label_len)
 * @param id is a number - which is the tag #
 * @param label is a UTF8 string (does not need to be \0 terminated)
 * @param len is the length in bytes of that string
 * @return GREASE_OK if all good
 */
LIB_METHOD_SYNC(addTagLabel, uint32_t val, const char *utf8, int len) {
	GreaseLogger *l = GreaseLogger::setupClass();
	if(utf8 && len > 0) {
//		Nan::Utf8String v8str(info[1]->ToString());
		GreaseLogger::logLabel *label = GreaseLogger::logLabel::fromUTF8(utf8,len);
//		l->tagLabels.addReplace(info[0]->Uint32Value(),label);
		l->tagLabels.addReplace(val,label);
		return GREASE_OK;
	} else {
		return GREASE_INVALID_PARAMS;
	}
};



/**
 * addOriginLabel(id,label)
 * @param args id is a number, label a string
 *
 * @return v8::Undefined
 */
LIB_METHOD_SYNC(addOriginLabel, uint32_t val, const char *utf8, int len) {
	GreaseLogger *l = GreaseLogger::setupClass();
	if(utf8 && len > 0) {
		GreaseLogger::logLabel *label = GreaseLogger::logLabel::fromUTF8(utf8,len);
		l->originLabels.addReplace(val,label);
		return GREASE_OK;
	} else {
		return GREASE_INVALID_PARAMS;
	}
};

// these match the levels in
// grease_client.h
// "Standard levels"
const char *GREASE_STD_LABEL_LOG = "LOG";
const char *GREASE_STD_LABEL_ERROR = "ERROR";
const char *GREASE_STD_LABEL_WARN = "WARN";
const char *GREASE_STD_LABEL_DEBUG = "DEBUG";
const char *GREASE_STD_LABEL_DEBUG2 = "DEBUG2";
const char *GREASE_STD_LABEL_DEBUG3 = "DEBUG3";
const char *GREASE_STD_LABEL_USER1 = "USER1";
const char *GREASE_STD_LABEL_USER2 = "USER2";
const char *GREASE_STD_LABEL_SUCCESS = "SUCCESS";
const char *GREASE_STD_LABEL_INFO = "INFO";
const char *GREASE_STD_LABEL_TRACE = "TRACE";

/**
 * defined in grease_client.h
#define GREASE_LEVEL_LOG     0x01
#define GREASE_LEVEL_ERROR   0x02
#define GREASE_LEVEL_WARN    0x04
#define GREASE_LEVEL_DEBUG   0x08
#define GREASE_LEVEL_DEBUG2  0x10
#define GREASE_LEVEL_DEBUG3  0x20
#define GREASE_LEVEL_USER1   0x40
#define GREASE_LEVEL_USER2   0x80
#define GREASE_LEVEL_SUCCESS 0x100
#define GREASE_LEVEL_INFO    0x0100
#define GREASE_LEVEL_TRACE   0x200
 *
 */

const char *GREASE_STD_LABEL_SYSLOG = "syslog";
const char *GREASE_STD_LABEL_STDOUT = "stdout";
const char *GREASE_STD_LABEL_STDERR = "stderr";

LIB_METHOD_SYNC(setupStandardLevels) {
	GreaseLib_addLevelLabel(GREASE_LEVEL_LOG,GREASE_STD_LABEL_LOG,strlen(GREASE_STD_LABEL_LOG));
	GreaseLib_addLevelLabel(GREASE_LEVEL_ERROR,GREASE_STD_LABEL_ERROR,strlen(GREASE_STD_LABEL_ERROR));
	GreaseLib_addLevelLabel(GREASE_LEVEL_WARN,GREASE_STD_LABEL_WARN,strlen(GREASE_STD_LABEL_WARN));
	GreaseLib_addLevelLabel(GREASE_LEVEL_DEBUG,GREASE_STD_LABEL_DEBUG,strlen(GREASE_STD_LABEL_DEBUG));
	GreaseLib_addLevelLabel(GREASE_LEVEL_DEBUG2,GREASE_STD_LABEL_DEBUG2,strlen(GREASE_STD_LABEL_DEBUG2));
	GreaseLib_addLevelLabel(GREASE_LEVEL_DEBUG3,GREASE_STD_LABEL_DEBUG3,strlen(GREASE_STD_LABEL_DEBUG3));
	GreaseLib_addLevelLabel(GREASE_LEVEL_USER1,GREASE_STD_LABEL_USER1,strlen(GREASE_STD_LABEL_USER1));
	GreaseLib_addLevelLabel(GREASE_LEVEL_USER2,GREASE_STD_LABEL_USER2,strlen(GREASE_STD_LABEL_USER2));
	GreaseLib_addLevelLabel(GREASE_LEVEL_SUCCESS,GREASE_STD_LABEL_SUCCESS,strlen(GREASE_STD_LABEL_SUCCESS));
	GreaseLib_addLevelLabel(GREASE_LEVEL_INFO,GREASE_STD_LABEL_INFO,strlen(GREASE_STD_LABEL_INFO));
	GreaseLib_addLevelLabel(GREASE_LEVEL_TRACE,GREASE_STD_LABEL_TRACE,strlen(GREASE_STD_LABEL_TRACE));
	return GREASE_OK;
}

LIB_METHOD_SYNC(setupStandardTags) {
	GreaseLib_addTagLabel(GREASE_TAG_SYSLOG,GREASE_STD_LABEL_SYSLOG,strlen(GREASE_STD_LABEL_SYSLOG));
	GreaseLib_addTagLabel(GREASE_TAG_STDOUT,GREASE_STD_LABEL_STDOUT,strlen(GREASE_STD_LABEL_STDOUT));
	GreaseLib_addTagLabel(GREASE_TAG_STDERR,GREASE_STD_LABEL_STDERR,strlen(GREASE_STD_LABEL_STDERR));
	return GREASE_OK;
}


/**
 * addLevelLabel(id,label)
 * @param args id is a number, label a string
 *
 * @return v8::Undefined
 */
LIB_METHOD_SYNC(addLevelLabel, uint32_t val, const char *utf8, int len) {
	GreaseLogger *l = GreaseLogger::setupClass();
	if(utf8 && len > 0) {
		GreaseLogger::logLabel *label = GreaseLogger::logLabel::fromUTF8(utf8,len);
		l->levelLabels.addReplace(val,label);
		return GREASE_OK;
	} else {
		return GREASE_INVALID_PARAMS;
	}
};


GreaseLibTargetFileOpts *GreaseLib_new_GreaseLibTargetFileOpts() {
	GreaseLibTargetFileOpts *ret = (GreaseLibTargetFileOpts *) ::malloc(sizeof(GreaseLibTargetFileOpts));
	::memset(ret,0,sizeof(GreaseLibTargetFileOpts));
	return ret;
}
GreaseLibTargetFileOpts *GreaseLib_init_GreaseLibTargetFileOpts(GreaseLibTargetFileOpts *ret) {
	::memset(ret,0,sizeof(GreaseLibTargetFileOpts));
	return ret;
}
void GreaseLib_cleanup_GreaseLibTargetFileOpts(GreaseLibTargetFileOpts *opts) {
	if(opts) ::free(opts);
}
void GreaseLib_set_flag_GreaseLibTargetFileOpts(GreaseLibTargetFileOpts *opts,uint32_t flag) {
	opts->_enabledFlags |= flag;
}

static int nextTargetOptsId = 0;

GreaseLibTargetOpts *GreaseLib_new_GreaseLibTargetOpts(void) {
	GreaseLibTargetOpts *ret = (GreaseLibTargetOpts *) ::malloc(sizeof(GreaseLibTargetOpts));
	::memset(ret,0,sizeof(GreaseLibTargetOpts));
	ret->optsId = nextTargetOptsId++;
	return ret;
}

GreaseLibTargetOpts *GreaseLib_init_GreaseLibTargetOpts(GreaseLibTargetOpts *ret) {
	::memset(ret,0,sizeof(GreaseLibTargetOpts));
	ret->optsId = nextTargetOptsId++;
	return ret;
}



void GreaseLib_cleanup_GreaseLibTargetOpts(GreaseLibTargetOpts *opts) {
	if(opts) ::free(opts);
}

void GreaseLib_set_string_GreaseLibTargetFileOpts(GreaseLibTargetFileOpts *opts,uint32_t flag,const char *s) {

}



LIB_METHOD_SYNC(modifyDefaultTarget,GreaseLibTargetOpts *opts) {
	GreaseLogger *l = GreaseLogger::setupClass();
	GreaseLogger::logTarget *targ = l->defaultTarget;

	GreaseLogger::target_start_info *i = new GreaseLogger::target_start_info();

	if(opts->tty) {

		i->cb = NULL;
		i->targId = DEFAULT_TARGET;

		GreaseLogger::delim_data defaultdelim;
		if(opts->delim) {
			defaultdelim.setDelim(opts->delim, opts->len_delim);
		} else {
			defaultdelim.delim.sprintf("\n");
		}
		GreaseLogger::LOGGER->Opts.lock();
		int size = GreaseLogger::LOGGER->Opts.bufferSize;
		GreaseLogger::LOGGER->Opts.unlock();

		// replaces existing default target with this one:
		targ = new GreaseLogger::ttyTarget(size, DEFAULT_TARGET, GreaseLogger::LOGGER, GreaseLogger::targetReady,std::move(defaultdelim), i);

	} else if(opts->file) {
		i->cb = NULL;
		i->targId = DEFAULT_TARGET;

		GreaseLogger::delim_data defaultdelim;
		if(opts->delim) {
			defaultdelim.setDelim(opts->delim, opts->len_delim);
		} else {
			defaultdelim.delim.sprintf("\n");
		}
		GreaseLogger::LOGGER->Opts.lock();
		int size = GreaseLogger::LOGGER->Opts.bufferSize;
		GreaseLogger::LOGGER->Opts.unlock();

		int mode = DEFAULT_MODE_FILE_TARGET;
		int flags = DEFAULT_FLAGS_FILE_TARGET;
		GreaseLogger::fileTarget::rotationOpts rotateOpts;

		if(opts->fileOpts) {
			if(opts->fileOpts->_enabledFlags & GREASE_LIB_SET_FILEOPTS_MODE) {
				mode = opts->fileOpts->mode;
			}

			if(opts->fileOpts->_enabledFlags & GREASE_LIB_SET_FILEOPTS_FLAGS) {
				flags = opts->fileOpts->flags;
			}

			if(opts->fileOpts->_enabledFlags & GREASE_LIB_SET_FILEOPTS_ROTATE) {
				if(opts->fileOpts->_enabledFlags & GREASE_LIB_SET_FILEOPTS_MAXFILES) rotateOpts.max_files = opts->fileOpts->max_files;
				if(opts->fileOpts->_enabledFlags & GREASE_LIB_SET_FILEOPTS_MAXFILESIZE) rotateOpts.max_file_size = opts->fileOpts->max_file_size;
				if(opts->fileOpts->_enabledFlags & GREASE_LIB_SET_FILEOPTS_MAXTOTALSIZE) rotateOpts.max_total_size = opts->fileOpts->max_total_size;
				if(opts->fileOpts->_enabledFlags & GREASE_LIB_SET_FILEOPTS_ROTATEONSTART) rotateOpts.rotate_on_start = true;
			}

			targ = new GreaseLogger::fileTarget(size, DEFAULT_TARGET, GreaseLogger::LOGGER, flags, mode, opts->file,
					std::move(defaultdelim), i, GreaseLogger::targetReady, rotateOpts);
		} else {
			targ = new GreaseLogger::fileTarget(size, DEFAULT_TARGET, GreaseLogger::LOGGER, flags, mode, opts->file,
					std::move(defaultdelim), i, GreaseLogger::targetReady);
		}
	} else {
		if(opts->delim) {
			targ->delim.setDelim(opts->delim,opts->len_delim);
		}
	}

	if(opts->targetCB) {
		targ->setCallback(opts->targetCB);
	}

	if(opts->format_level) {
		targ->setLevelFormat(opts->format_level,opts->format_level_len);
	}
	if(opts->format_origin) {
		targ->setOriginFormat(opts->format_origin,opts->format_origin_len);
	}
	if(opts->format_tag) {
		targ->setTagFormat(opts->format_tag,opts->format_tag_len);
	}
	if(opts->format_post) {
		targ->setPostFormat(opts->format_post,opts->format_post_len);
	}
	if(opts->format_pre) {
		targ->setPreFormat(opts->format_pre,opts->format_pre_len);
	}
	if(opts->format_pre_msg) {
		targ->setPreMsgFormat(opts->format_pre_msg,opts->format_pre_msg_len);
	}
	if(opts->format_time) {
		targ->setTimeFormat(opts->format_time,opts->format_time_len);
	}

	return GREASE_LIB_OK;

}

// implements the actionCB call
void addTarget_actionCB(GreaseLogger *, _errcmn::err_ev &err, void *data) {
	GreaseLogger::target_start_info *info = (GreaseLogger::target_start_info *)	data;
	if(info && info->targetStartCB) {
		if(err.hasErr()) {
			// pass error - convert to C style error:
			GreaseLibError errout;
			errout._errno = err._errno;
			::strncpy(err.errstr,errout.errstr,sizeof(errout.errstr));
			info->targetStartCB(&errout,NULL);
		} else {
			GreaseLibStartedTargetInfo outinfo;
			outinfo.targId = info->targId;
			outinfo.optsId = info->optsId;
			info->targetStartCB(NULL,&outinfo);
		}
	}
}


LIB_METHOD(addTarget,GreaseLibTargetOpts *opts) {
	GreaseLogger *l = GreaseLogger::setupClass();
	GreaseLogger::target_start_info *i = new GreaseLogger::target_start_info();
	i->cb = addTarget_actionCB;
//	if(opts->)
	i->targetStartCB = libCB; // this will be called when target starts
	assert(opts);
	i->optsId = opts->optsId;

	TargetId id;
	GreaseLogger::logTarget *targ = NULL;
	uv_mutex_lock(&l->nextIdMutex);
	id = l->nextTargetId++;
	uv_mutex_unlock(&l->nextIdMutex);

	GreaseLogger::delim_data defaultdelim;
	if(opts->delim) {
		defaultdelim.setDelim(opts->delim, opts->len_delim);
	} else {
		defaultdelim.delim.sprintf("\n");
	}

	GreaseLogger::LOGGER->Opts.lock();
	int size = GreaseLogger::LOGGER->Opts.bufferSize;
	GreaseLogger::LOGGER->Opts.unlock();


	if(opts->tty) {

		i->targId = id;

		// replaces existing default target with this one:
		targ = new GreaseLogger::ttyTarget(size, id, GreaseLogger::LOGGER, GreaseLogger::targetReady,std::move(defaultdelim), i);

	} else if(opts->file) {
		i->targId = id;


		int mode = DEFAULT_MODE_FILE_TARGET;
		int flags = DEFAULT_FLAGS_FILE_TARGET;
		GreaseLogger::fileTarget::rotationOpts rotateOpts;

		if(opts->fileOpts) {
			if(opts->fileOpts->_enabledFlags & GREASE_LIB_SET_FILEOPTS_MODE) {
				mode = opts->fileOpts->mode;
			}

			if(opts->fileOpts->_enabledFlags & GREASE_LIB_SET_FILEOPTS_FLAGS) {
				flags = opts->fileOpts->flags;
			}

			if(opts->fileOpts->_enabledFlags & GREASE_LIB_SET_FILEOPTS_ROTATE) {
				if(opts->fileOpts->_enabledFlags & GREASE_LIB_SET_FILEOPTS_MAXFILES) rotateOpts.max_files = opts->fileOpts->max_files;
				if(opts->fileOpts->_enabledFlags & GREASE_LIB_SET_FILEOPTS_MAXFILESIZE) rotateOpts.max_file_size = opts->fileOpts->max_file_size;
				if(opts->fileOpts->_enabledFlags & GREASE_LIB_SET_FILEOPTS_MAXTOTALSIZE) rotateOpts.max_total_size = opts->fileOpts->max_total_size;
				if(opts->fileOpts->_enabledFlags & GREASE_LIB_SET_FILEOPTS_ROTATEONSTART) rotateOpts.rotate_on_start = true;
			}

			targ = new GreaseLogger::fileTarget(size, id, GreaseLogger::LOGGER, flags, mode, opts->file,
					std::move(defaultdelim), i, GreaseLogger::targetReady, rotateOpts);
		} else {
			targ = new GreaseLogger::fileTarget(size, id, GreaseLogger::LOGGER, flags, mode, opts->file,
					std::move(defaultdelim), i, GreaseLogger::targetReady);
		}
	} else {
// 		callbackTarget(int buffer_size, uint32_t id, GreaseLogger *o,
//		targetReadyCB cb, delim_data _delim, target_start_info *readydata) :
//			logTarget(buffer_size, id, o, cb, std::move(_delim), readydata) {}

		i->targId = id;

		targ = new GreaseLogger::callbackTarget(size,id, GreaseLogger::LOGGER, GreaseLogger::targetReady, std::move(defaultdelim), i);

		if(opts->delim) {
			targ->delim.setDelim(opts->delim,opts->len_delim);
		}
	}

	if(opts->targetCB) {
		targ->setCallback(opts->targetCB);
	}

	if(opts->format_level) {
		targ->setLevelFormat(opts->format_level,opts->format_level_len);
	}
	if(opts->format_origin) {
		targ->setOriginFormat(opts->format_origin,opts->format_origin_len);
	}
	if(opts->format_tag) {
		targ->setTagFormat(opts->format_tag,opts->format_tag_len);
	}
	if(opts->format_post) {
		targ->setPostFormat(opts->format_post,opts->format_post_len);
	}
	if(opts->format_pre) {
		targ->setPreFormat(opts->format_pre,opts->format_pre_len);
	}
	if(opts->format_pre_msg) {
		targ->setPreMsgFormat(opts->format_pre_msg,opts->format_pre_msg_len);
	}
	if(opts->format_time) {
		targ->setTimeFormat(opts->format_time,opts->format_time_len);
	}

	return GREASE_LIB_OK;

}



LIB_METHOD_SYNC(maskOutByLevel, uint32_t val) {
	GreaseLogger *l = GreaseLogger::setupClass();
	uv_mutex_lock(&l->modifyFilters);
	l->Opts.levelFilterOutMask |= val;
	uv_mutex_unlock(&l->modifyFilters);
	return GREASE_LIB_OK;
}

LIB_METHOD_SYNC(unmaskOutByLevel, uint32_t val) {
	GreaseLogger *l = GreaseLogger::setupClass();
	uv_mutex_lock(&l->modifyFilters);
	l->Opts.levelFilterOutMask = !val | l->Opts.levelFilterOutMask;
	uv_mutex_unlock(&l->modifyFilters);
	return GREASE_LIB_OK;
}

GreaseLibFilter *GreaseLib_new_GreaseLibFilter() {
	GreaseLibFilter *ret = (GreaseLibFilter *) ::malloc(sizeof(GreaseLibFilter));
	::memset(ret,0,sizeof(GreaseLibFilter));
	ret->mask = GREASE_ALL_LEVELS;
	return ret;
}
GreaseLibFilter *GreaseLib_init_GreaseLibFilter(GreaseLibFilter *ret) {
	::memset(ret,0,sizeof(GreaseLibFilter));
	ret->mask = GREASE_ALL_LEVELS;
	return ret;
}

void GreaseLib_cleanup_GreaseLibFilter(GreaseLibFilter *opts) {
	if(opts) ::free(opts);
}
void GreaseLib_setvalue_GreaseLibFilter(GreaseLibFilter *opts,uint32_t flag,uint32_t val) {
	if(flag && opts) {
//#define GREASE_LIB_SET_FILTER_ORIGIN  0x1
//#define GREASE_LIB_SET_FILTER_TAG     0x2
//#define GREASE_LIB_SET_FILTER_TARGET  0x4
//#define GREASE_LIB_SET_FILTER_MASK    0x8
		switch(flag) {
			case GREASE_LIB_SET_FILTER_ORIGIN:
				opts->origin = val;
				opts->_enabledFlags |= GREASE_LIB_SET_FILTER_ORIGIN;
				break;
			case GREASE_LIB_SET_FILTER_TAG:
				opts->tag = val;
				opts->_enabledFlags |= GREASE_LIB_SET_FILTER_TAG;
				break;
			case GREASE_LIB_SET_FILTER_TARGET:
				opts->target = val;
				opts->_enabledFlags |= GREASE_LIB_SET_FILTER_TARGET;
				break;
			case GREASE_LIB_SET_FILTER_MASK:
				opts->mask = val;
				opts->_enabledFlags |= GREASE_LIB_SET_FILTER_MASK;
				break;
//			default:
				// ERROR!
		}
	}
}

LIB_METHOD_SYNC(addFilter,GreaseLibFilter *filter) {
	FilterId id;

	if(!filter) {
		return GREASE_INVALID_PARAMS;
	}

	GreaseLogger *l = GreaseLogger::setupClass();
	GreaseLogger::logTarget *targ = l->defaultTarget;

	GreaseLogger::logLabel *preFormat = NULL;
	GreaseLogger::logLabel *postFormat = NULL;
	GreaseLogger::logLabel *postFormatPreMsg = NULL;

	if(filter->format_post) {
		postFormat = GreaseLogger::logLabel::fromUTF8(filter->format_post,filter->format_post_len);
	}
	if(filter->format_pre) {
		preFormat = GreaseLogger::logLabel::fromUTF8(filter->format_pre,filter->format_pre_len);
	}
	if(filter->format_post_pre_msg) {
		postFormatPreMsg = GreaseLogger::logLabel::fromUTF8(filter->format_post_pre_msg,filter->format_post_pre_msg_len);
	}

	if(l->_addFilter(filter->target,filter->origin,filter->tag,filter->mask,id,preFormat,postFormatPreMsg,postFormat)) {
		filter->id = id;
		return GREASE_LIB_OK;
	} else {
		return GREASE_FAILED;
	}

}

LIB_METHOD_SYNC(disableFilter,GreaseLibFilter *filter) {
	GreaseLogger *l = GreaseLogger::setupClass();
	GreaseLogger::Filter *found = NULL;

	if(l->_lookupFilter(filter->origin,filter->tag,filter->id,found)) {
		found->_disabled = true;
		return GREASE_LIB_OK;
	} else {
		return GREASE_LIB_NOT_FOUND;
	}

}

LIB_METHOD_SYNC(enableFilter,GreaseLibFilter *filter) {
	GreaseLogger *l = GreaseLogger::setupClass();
	GreaseLogger::Filter *found = NULL;

	if(l->_lookupFilter(filter->origin,filter->tag,filter->id,found)) {
		found->_disabled = false;
		return GREASE_LIB_OK;
	} else {
		return GREASE_LIB_NOT_FOUND;
	}
}

GreaseLibSink *GreaseLib_new_GreaseLibSink(uint32_t sink_type, const char *path) {
	GreaseLibSink *ret = (GreaseLibSink *) ::malloc(sizeof(GreaseLibSink));
	::memset(ret,0,sizeof(GreaseLibSink));
	ret->sink_type = sink_type;
	::strncpy(ret->path,path,GREASE_PATH_MAX);
	return ret;
}

GreaseLibSink *GreaseLib_init_GreaseLibSink(GreaseLibSink *ret, uint32_t sink_type, const char *path) {
	::memset(ret,0,sizeof(GreaseLibSink));
	ret->sink_type = sink_type;
	::strncpy(ret->path,path,GREASE_PATH_MAX);
	return ret;
}

void GreaseLib_cleanup_GreaseLibSink(GreaseLibSink *sink) {
	if(sink) {
		::free(sink);
	}
}


LIB_METHOD_SYNC(addSink,GreaseLibSink *sink) {
	GreaseLogger *l = GreaseLogger::setupClass();
	if(sink->sink_type == GREASE_LIB_SINK_PIPE) {
		uv_mutex_lock(&l->nextIdMutex);
		sink->id = l->nextSinkId++;
		uv_mutex_unlock(&l->nextIdMutex);

		GreaseLogger::PipeSink *newsink = new GreaseLogger::PipeSink(l, sink->path, sink->id, l->loggerLoop);
		GreaseLogger::Sink *base = dynamic_cast<GreaseLogger::Sink *>(newsink);

		newsink->bind();
		newsink->start();

		l->sinks.addReplace(sink->id,base);
	} else if (sink->sink_type == GREASE_LIB_SINK_UNIXDGRAM) {
		uv_mutex_lock(&l->nextIdMutex);
		sink->id = l->nextSinkId++;
		uv_mutex_unlock(&l->nextIdMutex);

		GreaseLogger::UnixDgramSink *newsink = new GreaseLogger::UnixDgramSink(l, sink->path, sink->id, l->loggerLoop);
		GreaseLogger::Sink *base = dynamic_cast<GreaseLogger::Sink *>(newsink);

		newsink->bind();
		newsink->start();

		l->sinks.addReplace(sink->id,base);
	} else if (sink->sink_type == GREASE_LIB_SINK_SYSLOGDGRAM) {
		uv_mutex_lock(&l->nextIdMutex);
		sink->id = l->nextSinkId++;
		uv_mutex_unlock(&l->nextIdMutex);

		GreaseLogger::SyslogDgramSink *newsink = new GreaseLogger::SyslogDgramSink(l, sink->path, sink->id, l->loggerLoop);
		GreaseLogger::Sink *base = dynamic_cast<GreaseLogger::Sink *>(newsink);

		newsink->bind();
		newsink->start();

		l->sinks.addReplace(sink->id,base);
	} else {
		return GREASE_INVALID_PARAMS;
	}
	return GREASE_LIB_OK;
}




/**
 * logstring and level manadatory
 * all else optional
 * log(message(number), level{number},tag{number},origin{number},extras{object})
 *
 * extras = {
 *    .ignores = {number|array}
 * }
 * @method log
 *
 */
//LIB_METHOD_SYNC(Log) {
//	static extra_logMeta meta; // static - this call is single threaded from node.
//	ZERO_LOGMETA(meta.m);
//	uint32_t target = DEFAULT_TARGET;
//	if(info.Length() > 1 && info[0]->IsString() && info[1]->IsInt32()){
//		GreaseLogger *l = GreaseLogger::setupClass();
//		v8::String::Utf8Value v8str(info[0]->ToString());
//		meta.m.level = (uint32_t) info[1]->Int32Value(); // level
//
//		if(info.Length() > 2 && info[2]->IsInt32()) // tag
//			meta.m.tag = (uint32_t) info[2]->Int32Value();
//		else
//			meta.m.tag = 0;
//
//		if(info.Length() > 3 && info[3]->IsInt32()) // origin
//			meta.m.origin = (uint32_t) info[3]->Int32Value();
//		else
//			meta.m.origin = 0;
//
//		if(l->sift(meta.m)) {
//			if(info.Length() > 4 && info[4]->IsObject()) {
//				Local<Object> jsObj = info[4]->ToObject();
//				Local<Value> val = jsObj->Get(Nan::New("ignores").ToLocalChecked());
//				if(val->IsArray()) {
//					Local<Array> array = v8::Local<v8::Array>::Cast(val);
//					uint32_t i = 0;
//					for (i=0 ; i < array->Length() ; ++i) {
//					  const Local<Value> value = array->Get(i);
//					  if(i >= MAX_IGNORE_LIST) {
//						  break;
//					  } else {
//						  meta.ignore_list[i] = value->Uint32Value();
//					  }
//					}
//					meta.ignore_list[i] = 0;
//				} else if(val->IsUint32()) {
//					meta.ignore_list[0] = val->Uint32Value();
//					meta.ignore_list[1] = 0;
//				}
//				meta.m.extras = 1;
//			}
//			FilterList *list = NULL;
//			l->_log(meta.m,v8str.operator *(),v8str.length());
//		}
//	}
//}
//
//LIB_METHOD_SYNC(GreaseLogger::LogSync) {
//	static logMeta meta; // static - this call is single threaded from node.
//	uint32_t target = DEFAULT_TARGET;
//	if(info.Length() > 1 && info[0]->IsString() && info[1]->IsInt32()){
//		GreaseLogger *l = GreaseLogger::setupClass();
//		v8::String::Utf8Value v8str(info[0]->ToString());
//		meta.level = (uint32_t) info[1]->Int32Value();
//
//		if(info.Length() > 2 && info[2]->IsInt32()) // tag
//			meta.tag = (uint32_t) info[2]->Int32Value();
//		else
//			meta.tag = 0;
//
//		if(info.Length() > 3 && info[3]->IsInt32()) // tag
//			meta.origin = (uint32_t) info[3]->Int32Value();
//		else
//			meta.origin = 0;
//
//		FilterList *list = NULL;
//		if(l->sift(meta)) {
//			l->_logSync(meta,v8str.operator *(),v8str.length());
//		}
//	}
//}


LIB_METHOD_SYNC(disableTarget, TargetId id) {
	GreaseLogger *l = GreaseLogger::setupClass();

	GreaseLogger::logTarget *t = NULL;
	if(l->targets.find(id,t)) {
		t->disableWrites(true);
		return GREASE_LIB_OK;
	} else {
		return GREASE_LIB_NOT_FOUND;
	}
}

LIB_METHOD_SYNC(enableTarget, TargetId id) {
	GreaseLogger *l = GreaseLogger::setupClass();
	GreaseLogger::logTarget *t = NULL;
	if(l->targets.find(id,t)) {
		t->disableWrites(false);
		return GREASE_LIB_OK;
	} else {
		return GREASE_LIB_NOT_FOUND;
	}
}


LIB_METHOD_SYNC(flush, TargetId id) {
	GreaseLogger *l = GreaseLogger::setupClass();
	GreaseLogger::logTarget *t = NULL;
	if(l->targets.find(id,t)) {
		t->flushAll();
		return GREASE_LIB_OK;
	} else {
		return GREASE_LIB_NOT_FOUND;
	}
}






/**
 * obj = {
 *    pipe: "/var/mysink"   // currently our only option is a named socket / pipe
 *    OR
 *    unixDgram: "/var/mydgramsink"
 * }
 */
//LIB_METHOD(GreaseLogger::AddSink) {
//	GreaseLogger *l = GreaseLogger::setupClass();
//
//	if(info.Length() > 0 && info[0]->IsObject()) {
//		Local<Object> jsSink = info[0]->ToObject();
//		Local<Value> isPipe = jsSink->Get(Nan::New("pipe").ToLocalChecked());
//		Local<Value> isUnixDgram = jsSink->Get(Nan::New("unixDgram").ToLocalChecked());
//		Local<Value> newConnCB = jsSink->Get(Nan::New("newConnCB").ToLocalChecked()); // called when a new connection is made on the callback
//
//		if(isPipe->IsString()) {
//			v8::String::Utf8Value v8str(isPipe);
//
//			uv_mutex_lock(&l->nextIdMutex);
//			SinkId id = l->nextSinkId++;
//			uv_mutex_unlock(&l->nextIdMutex);
//
//			PipeSink *sink = new PipeSink(l, v8str.operator *(), id, l->loggerLoop);
//			Sink *base = dynamic_cast<Sink *>(sink);
//
//			sink->bind();
//			sink->start();
//
//			l->sinks.addReplace(id,base);
//		} else if(isUnixDgram->IsString()) {
//			v8::String::Utf8Value v8str(isUnixDgram);
//
////			DBG_OUT("Opening socket unix dgram: %s\n",);
//
//			uv_mutex_lock(&l->nextIdMutex);
//			SinkId id = l->nextSinkId++;
//			uv_mutex_unlock(&l->nextIdMutex);
//
//			UnixDgramSink *sink = new UnixDgramSink(l, v8str.operator *(), id, l->loggerLoop);
//			Sink *base = dynamic_cast<Sink *>(sink);
//
//			sink->bind();
//			sink->start();
//
//			l->sinks.addReplace(id,base);
//		} else {
//			Nan::ThrowError("addSink: unsupported Sink type");
//		}
//	} else
//		Nan::ThrowTypeError("addSink: bad parameters");
//};



/**
 * addFilter(obj) where
 * obj = {
 *      // at least origin and/or tag must be present
 *      origin: 0x33,    // any positive number > 0
 *      tag: 0x25        // any positive number > 0,
 *      target: 3,       // mandatory
 *      mask:  0x4000000 // optional (default, show everything: 0xFFFFFFF),
 *      format: {        // optional (formatting settings)
 *      	pre: 'targ-pre>', // optional pre string
 *      	post: '<targ-post;
 *      }
 * }
 */
//LIB_METHOD(GreaseLogger::AddFilter) {
//	GreaseLogger *l = GreaseLogger::setupClass();
//	FilterId id;
//	TagId tagId = 0;
//	OriginId originId = 0;
//	TargetId targetId = 0;
//	LevelMask mask = ALL_LEVELS;
//	Handle<Value> ret = Nan::New(false);
//	if(info.Length() > 0 && info[0]->IsObject()) {
//		Local<Object> jsObj = info[0]->ToObject();
//
//		bool ok = false;
//		bool set_disable = false;
//		Local<Value> jsTag = jsObj->Get(Nan::New("tag").ToLocalChecked());
//		Local<Value> jsOrigin = jsObj->Get(Nan::New("origin").ToLocalChecked());
//		Local<Value> jsTarget = jsObj->Get(Nan::New("target").ToLocalChecked());
//		Local<Value> jsMask = jsObj->Get(Nan::New("mask").ToLocalChecked());
//		Local<Value> jsDisable = jsObj->Get(Nan::New("disable").ToLocalChecked());
//
//		if(jsTag->IsUint32()) {
//			tagId = (TagId) jsTag->Uint32Value();
//			ok = true;
//		}
//		if(jsOrigin->IsUint32()) {
//			originId = (OriginId) jsOrigin->Uint32Value();
//			ok = true;
//		}
//		if(jsMask->IsUint32()) {
//			mask = jsMask->Uint32Value();
//			ok = true;
//		} else if (!jsMask->IsUndefined()) {
//			Nan::ThrowTypeError("addFilter: bad parameters (mask)");
//			return;
//		}
//		if(jsTarget->IsUint32()) {
//			targetId = (OriginId) jsTarget->Uint32Value();
//		} else {
////			ok = false;
//			targetId = 0;
//		}
//
//		if((!jsDisable.IsEmpty() && !jsDisable->IsUndefined()) && jsDisable->IsBoolean()) {
//			if(jsDisable->IsTrue()) {
//				set_disable = true;
//			}
//		}
//
//		if(!ok) {
//			Nan::ThrowTypeError("addFilter: bad parameters");
//			return;
//		}
//
//		logLabel *preFormat = NULL;
//		logLabel *postFormat = NULL;
//		logLabel *postFormatPreMsg = NULL;
//
//		Local<Value> jsKey = jsObj->Get(Nan::New("pre").ToLocalChecked());
//		if(jsKey->IsString()) {
//			v8::String::Utf8Value v8str(jsKey);
//			preFormat = logLabel::fromUTF8(v8str.operator *(),v8str.length());
//		}
//		jsKey = jsObj->Get(Nan::New("post").ToLocalChecked());
//		if(jsKey->IsString()) {
//			v8::String::Utf8Value v8str(jsKey);
//			postFormat= logLabel::fromUTF8(v8str.operator *(),v8str.length());
//		}
//		jsKey = jsObj->Get(Nan::New("post_fmt_pre_msg").ToLocalChecked());
//		if(jsKey->IsString()) {
//			v8::String::Utf8Value v8str(jsKey);
//			postFormatPreMsg= logLabel::fromUTF8(v8str.operator *(),v8str.length());
//		}
//
//		if(l->_addFilter(targetId,originId,tagId,mask,id,preFormat,postFormatPreMsg,postFormat))
//			ret = Nan::New((uint32_t) id);
//		else
//			ret = Nan::New(false);
//
//		if(set_disable) {
//			Filter *found;
//			if(l->_lookupFilter(originId,tagId,id,found)) {
//				found->_disabled = true;
//			}
//		}
//	} else {
//		Nan::ThrowTypeError("addFilter: bad parameters");
//		return;
//	}
//	info.GetReturnValue().Set(v8::Local<Value>(ret));
//}
//


