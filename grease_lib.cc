/*
 * grease_lib.c
 *
 *  Created on: Nov 26, 2016
 *      Author: ed
 * (c) 2016, WigWag Inc.
 */

#include <uv.h>
#include <stdlib.h>
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

LIB_METHOD(Start) {
	GreaseLogger *l = GreaseLogger::setupClass();
	GreaseLogger::target_start_info *startinfo = new GreaseLogger::target_start_info();

	// if(info.Length() > 0 && info[0]->IsFunction()) {
	// 	startinfo->targetStartCB = new Nan::Callback(Local<Function>::Cast(info[0]));
	// }
	if(libCB) startinfo->targetStartCB = libCB;
	l->start(GreaseLogger::start_logger_cb, startinfo);
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
