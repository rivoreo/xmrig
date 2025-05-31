#include <stddef.h>

#if defined __i386__ || defined __amd64__ || defined _M_IX86 || defined _M_AMD64 || defined __x86_64 || defined __x86_64__
#include "BasicCpuInfo_x86.cpp"
#else
#include "BasicCpuInfo_generic.cpp"
#endif
