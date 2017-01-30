/*
 * test-re.cc
 *
 *  Created on: Jan 30, 2017
 *      Author: ed
 * (c) 2017, WigWag Inc.
 */

#include <string>
#include <iostream>
#include <re2/re2.h>
#include <assert.h>

int main() {
	RE2 re_1("([0-9]*)\\s+([a-zA-z]*)");
	int d;
	std::string s;

	assert(RE2::FullMatch("09811 ksajd",re_1, &d, &s));

	printf("capture: %d\n",d);
	printf("cap2: %s\n",s.c_str());

}



/**
 * Build:
 *
 * g++ -std=c++11 -Wall -Ideps/build/include -pthread  test-re.cc -o test-re deps/build/lib/libre2.a
 *
 */
