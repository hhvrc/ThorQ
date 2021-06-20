#ifndef CPUID_H
#define CPUID_H

#ifdef _MSC_VER
#include <intrin.h>
#endif

#ifdef __GNUC__

void __cpuid(int* cpuinfo, int info)
{
    __asm__ __volatile__(
        "xchg %%ebx, %%edi;"
        "cpuid;"
        "xchg %%ebx, %%edi;"
        :"=a" (cpuinfo[0]), "=D" (cpuinfo[1]), "=c" (cpuinfo[2]), "=d" (cpuinfo[3])
        :"0" (info)
    );
}

#endif

#endif // CPUID_H
