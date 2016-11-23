/*
 * error-common.h
 *
 *  Created on: Oct 31, 2014
 *      Author: ed
 * (c) 2015, Framez Inc
 */
#ifndef ERROR_COMMON_H_
#define ERROR_COMMON_H_

#include "nan.h"
#include <string.h>
#include <stdlib.h>

#include <v8.h>


#include <errno.h>
#if !defined(_MSC_VER)
#include <unistd.h>
#endif
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>


// https://gcc.gnu.org/onlinedocs/cpp/Stringification.html
#define xstr(s) str(s)
#define str(s) #s

// concept from node.js src/node_constants.cc
//#define _ERRCMN_DEFINE_CONSTANT(target, constant)
//  (target)->Set(v8::String::NewSymbol(#constant),
//                v8::Number::New(constant),
//                static_cast<v8::PropertyAttribute>(
//                    v8::ReadOnly|v8::DontDelete))

#define _ERRCMN_DEFINE_CONSTANT(target, constant)                         \
  Nan::ForceSet(target,Nan::New(#constant).ToLocalChecked(),               \
                Nan::New((uint32_t)constant),                                \
                static_cast<v8::PropertyAttribute>(                       \
                    v8::ReadOnly|v8::DontDelete))

//// our mode - this is the same thing, with a reverse lookup key also
//#define _ERRCMN_DEFINE_CONSTANT_WREV(target, constant)
//  (target)->Set(v8::String::NewSymbol(#constant),
//                v8::Number::New(constant),
//                static_cast<v8::PropertyAttribute>(
//                    v8::ReadOnly|v8::DontDelete));
//  (target)->Set(v8::String::New( xstr(constant) ),
//                v8::String::New(#constant),
//                static_cast<v8::PropertyAttribute>(
//                    v8::ReadOnly|v8::DontDelete));

#define _ERRCMN_DEFINE_CONSTANT_WREV(target, constant)                    \
  Nan::ForceSet(target,Nan::New(#constant).ToLocalChecked(),                 \
                Nan::New((uint32_t)constant),                                \
                static_cast<v8::PropertyAttribute>(                       \
                    v8::ReadOnly|v8::DontDelete));                        \
  Nan::ForceSet(target, Nan::New( xstr(constant) ).ToLocalChecked(),        \
                Nan::New(#constant).ToLocalChecked(),                         \
                static_cast<v8::PropertyAttribute>(                       \
                    v8::ReadOnly|v8::DontDelete));                        \

// custom error codes should be above this value
#define _ERRCMN_CUSTOM_ERROR_CUTOFF  4000

#define GREASE_UNKNOWN_FAILURE 4001
#define GREASE_UNKNOWN_TTY 4002
#define GREASE_UNKNOWN_NO_PATH 4003

namespace _errcmn {

	void DefineConstants(v8::Handle<v8::Object> target);

	char *get_error_str(int _errno);
	void free_error_str(char *b);
	v8::Local<v8::Value> errno_to_JS(int _errno, const char *prefix = NULL);
	v8::Local<v8::Value> errno_to_JSError(int _errno, const char *prefix = NULL);
	struct err_ev {
		char *errstr;
		int _errno;
		err_ev(void) : errstr(NULL), _errno(0) {};
		void setError(int e,const char *m=NULL);
		err_ev(int e) : err_ev() {
			setError(e);
		}
		err_ev(const err_ev &o) = delete;
		inline err_ev &operator=(const err_ev &o) = delete;
		inline err_ev &operator=(err_ev &&o) {
			this->errstr = o.errstr;  // transfer string to other guy...
			this->_errno = o._errno;
			o.errstr = NULL; o._errno = 0;
			return *this;
		}
		inline void clear() {
			if(errstr) ::free(errstr); errstr = NULL;
			_errno = 0;
		}
		~err_ev() {
			if(errstr) ::free(errstr);
		}
		bool hasErr() { return (_errno != 0); }
	};
	v8::Handle<v8::Value> err_ev_to_JS(err_ev &e, const char *prefix);
}

#ifndef NO_ERROR_CMN_OUTPUT  // if define this, you must define these below yourself


// ensure we get the XSI compliant strerror_r():
// see: http://man7.org/linux/man-pages/man3/strerror.3.html

#ifdef __cplusplus
extern "C" {
#endif
extern int __xpg_strerror_r (int __errnum, char *__buf, size_t __buflen);
#ifdef __cplusplus
};
#endif


#define ERR_STRERROR_R(ernum,b,len) __xpg_strerror_r(ernum, b, len)

#ifdef ERRCMN_DEBUG_BUILD
#pragma message "Build is Debug"
// confused? here: https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
#define ERROR_OUT(s,...) fprintf(stderr, "**ERROR** " s "\n", ##__VA_ARGS__ )
//#define ERROR_PERROR(s,...) fprintf(stderr, "*****ERROR***** " s, ##__VA_ARGS__ );
#define ERROR_PERROR(s,E,...) { char *__S=_errcmn::get_error_str(E); fprintf(stderr, "**ERROR** [ %s ] " s "\n", __S, ##__VA_ARGS__ ); _errcmn::free_error_str(__S); }

#define DBG_OUT(s,...) fprintf(stderr, "**DEBUG** " s "\n", ##__VA_ARGS__ )
#define IF_DBG( x ) { x }
#else
#define ERROR_OUT(s,...) fprintf(stderr, "**ERROR** " s, ##__VA_ARGS__ )//#define ERROR_PERROR(s,...) fprintf(stderr, "*****ERROR***** " s, ##__VA_ARGS__ );
#define ERROR_PERROR(s,E,...) { char *__S=_errcmn::get_error_str(E); fprintf(stderr, "**ERROR** [ %s ] " s, __S, ##__VA_ARGS__ ); _errcmn::free_error_str(__S); }
#define DBG_OUT(s,...) {}
#define IF_DBG( x ) {}
#endif

#else

#define ERROR_OUT(s,...) {} //#define ERROR_PERROR(s,...) fprintf(stderr, "*****ERROR***** " s, ##__VA_ARGS__ );
#define ERROR_PERROR(s,E,...) {}
#define DBG_OUT(s,...) {}
#define IF_DBG( x ) {}


#endif // NO_ERROR_CMN_OUTPUT



#endif /* ERROR_COMMON_H_ */
