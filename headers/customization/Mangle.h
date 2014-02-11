#ifndef MANGLE_H
#define MANGLE_H


#include <typeinfo>

#include <SupportDefs.h>
#include <String.h>


status_t demangle_class_name(const char* name, BString& out);


#endif // MANGLE_H

