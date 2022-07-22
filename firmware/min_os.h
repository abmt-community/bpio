/**
 * Author: Hendrik van Arragon, 2021
 * SPDX-License-Identifier: MIT
 */

#ifndef SHARED_MIN_OS_H_
#define SHARED_MIN_OS_H_

struct min_os_t{
	void die(const char* msg){};
};

extern min_os_t min_os;



#endif /* SHARED_MIN_OS_H_ */
