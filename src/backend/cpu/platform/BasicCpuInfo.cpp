#include <stddef.h>

#if defined __i386__ || defined __amd64__ || defined _M_IX86 || defined _M_AMD64 || defined __x86_64 || defined __x86_64__
#include "BasicCpuInfo_x86.cpp"
#elif defined __arm__ || defined __ARMEL__ || defined __arm64__ || defined __aarch64__ || defined __AARCH64EL__
#include "BasicCpuInfo_arm.cpp"
#else
#include "BasicCpuInfo_unknown.cpp"
#endif
