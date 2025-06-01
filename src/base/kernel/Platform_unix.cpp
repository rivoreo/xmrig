/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
 * Copyright 2016-2018 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
 * Copyright 2018-2025 Rivoreo
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/param.h>
#if defined __FreeBSD__ && !defined __FreeBSD_kernel__
#define __FreeBSD_kernel__
#endif
#if defined __GLIBC__ || defined __gnu_hurd__
// GNU is not BSD
#undef BSD
#endif

#if defined __APPLE__ && defined __MACH__
# include <mach/thread_act.h>
# include <mach/thread_policy.h>
#elif defined __FreeBSD_kernel__
# include <sys/cpuset.h>
# ifdef __FreeBSD__
#  include <pthread_np.h>
# endif
#endif

#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>
#include <uv.h>


#include "base/kernel/Platform.h"
#include "version.h"

#ifdef XMRIG_NVIDIA_PROJECT
#   include "nvidia/cryptonight.h"
#endif


#ifdef __FreeBSD_kernel__
#define cpu_set_t cpuset_t
#if !defined __FreeBSD__ && defined __GLIBC__
// GNU C Library didn't provide a wrapper for cpuset_setaffinity(2)
#include <sys/syscall.h>
#define cpuset_setaffinity(level,which,id,size,mask) syscall(SYS_cpuset_setaffinity,(level),(which),(id),(size),(mask))
#endif
#endif

#if defined __UCLIBC__
 #ifdef __linux__
 #define OS_NAME "uC/Linux"
 #else
 #define OS_NAME "uClibc"
 #endif
#elif defined __GLIBC__
 #ifdef __linux__
 #define OS_NAME "GNU/Linux"
 #elif (defined __GNU__ && defined __MACH__) || defined __gnu_hurd__
 #define OS_NAME "GNU/Hurd"
 #elif defined __FreeBSD_kernel__
 #define OS_NAME "GNU/kFreeBSD"
 #elif defined __sun || defined __sun_kernel
 #define OS_NAME "GNU/kOpenSolaris"
 #else
 #define OS_NAME "GNU"
 #endif
#elif defined __FreeBSD__
#define OS_NAME "FreeBSD"
#elif defined __NetBSD__
#define OS_NAME "NetBSD"
#elif defined __APPLE__
 #ifdef __MACH__
 //#define OS_NAME "Darwin"
 #define OS_NAME "Mac OS X"
 #else
 #define OS_NAME "A/UX"
 #endif
#elif defined BSD
#define OS_NAME "BSD"
#elif defined __sun
 #ifdef __SVR4
 #define OS_NAME "Solaris"
 #else
 #define OS_NAME "SunOS"
 #endif
#elif defined __ANDROID__
 #ifdef __linux__
 #define OS_NAME "Android/Linux"
 #else
 #define OS_NAME "Android"
 #endif
#else
 #define OS_NAME "unknown"
#endif

#ifdef __x86_64__
#define ARCH_NAME "x86_64"
#elif defined __aarch64__
#define ARCH_NAME "aarch64"
#elif defined __arm__
#define ARCH_NAME "arm"
#elif defined __i386__
#define ARCH_NAME "i386"
#elif defined __riscv && __riscv_xlen == 32
#define ARCH_NAME "riscv32"
#elif defined __riscv && __riscv_xlen == 64
#define ARCH_NAME "riscv64"
#elif defined __powerpc64__
#define ARCH_NAME "powerpc64"
#elif defined __powerpc__
#define ARCH_NAME "powerpc"
#else
#define ARCH_NAME "unknown"
#endif

char *xmrig::Platform::createUserAgent()
{
    constexpr const size_t max = 256;

    char *buf = new char[max]();
    int length = snprintf(buf, max, "%s/%s (%s %s) libuv/%s", APP_NAME, APP_VERSION, OS_NAME, ARCH_NAME, uv_version_string());

#   ifdef XMRIG_NVIDIA_PROJECT
    const int cudaVersion = cuda_get_runtime_version();
    length += snprintf(buf + length, max - length, " CUDA/%d.%d", cudaVersion / 1000, cudaVersion % 100);
#   endif

# ifdef __clang__
    length += snprintf(buf + length, max - length, " clang/%d.%d.%d", __clang_major__, __clang_minor__, __clang_patchlevel__);
# elif defined __INTEL_COMPILER
    length += snprintf(buf + length, max - length, " ICC/%d.%d", __INTEL_COMPILER / 100, __INTEL_COMPILER % 100 / 10);
# elif defined(__GNUC__)
    length += snprintf(buf + length, max - length, " gcc/%d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
# endif

    return buf;
}


#ifndef XMRIG_FEATURE_HWLOC
bool xmrig::Platform::setThreadAffinity(uint64_t cpu_id)
{
#if defined __APPLE__ && defined __MACH__
    thread_port_t mach_thread;
    thread_affinity_policy_data_t policy = { static_cast<integer_t>(cpu_id) };
    mach_thread = pthread_mach_thread_np(pthread_self());

    return thread_policy_set(mach_thread, THREAD_AFFINITY_POLICY, (thread_policy_t)&policy, 1) == KERN_SUCCESS;
#else
    cpu_set_t mn;
    CPU_ZERO(&mn);
    CPU_SET(cpu_id, &mn);
# ifdef __ANDROID__
    return sched_setaffinity(gettid(), sizeof(cpu_set_t), &mn) == 0;
# elif defined __FreeBSD_kernel__ && !defined __FreeBSD__
    return cpuset_setaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID, -1, sizeof mn, &mn) == 0;
# else
    return pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &mn) == 0;
# endif
#endif
}
#endif


uint32_t xmrig::Platform::setTimerResolution(uint32_t resolution)
{
    return resolution;
}


void xmrig::Platform::restoreTimerResolution()
{
}


void xmrig::Platform::setProcessPriority(int priority)
{
    if (priority == -1) {
        return;
    }

    int prio = 19;
    switch (priority)
    {
    case 1:
        prio = 5;
        break;

    case 2:
        prio = 0;
        break;

    case 3:
        prio = -5;
        break;

    case 4:
        prio = -10;
        break;

    case 5:
        prio = -15;
        break;

    default:
        break;
    }

    setpriority(PRIO_PROCESS, 0, prio);

#   ifdef SCHED_IDLE
    if (priority == 0) {
        sched_param param;
        param.sched_priority = 0;

        if (sched_setscheduler(0, SCHED_IDLE, &param) != 0) {
            sched_setscheduler(0, SCHED_BATCH, &param);
        }
    }
#   endif
}


void xmrig::Platform::setThreadPriority(int priority)
{
#ifdef __linux__
	setProcessPriority(priority);
#endif
}
