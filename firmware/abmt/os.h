/**
 * Author: Hendrik van Arragon, 2021
 * SPDX-License-Identifier: MIT
 */

#ifndef SHARED_MIN_OS_H_
#define SHARED_MIN_OS_H_

#include <string>

namespace abmt{
namespace os{

void die(std::string msg);
void log(std::string msg);
void log_err(std::string msg);

}; // namespace os
}; // namespace abmt

#endif /* SHARED_MIN_OS_H_ */
