// ec.h - the commandline interface to EZC's header
//   biggest notes are that it requires to be linked to EZC built with the
//   `std module`

#ifndef EC_H_
#define EC_H_

#include "ezc.h"

// force include the standard module
#define EZC_MODULE_NAME std
#include "ezc-module.h"

int main(int argc, char** argv);

#endif /* EC_H_ */

