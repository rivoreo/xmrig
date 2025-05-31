/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2019 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
 * Copyright 2018-2019 SChernykh   <https://github.com/SChernykh>
 * Copyright 2016-2019 XMRig       <support@xmrig.com>
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
#ifdef BSD
#include <sys/sysctl.h>
#include <errno.h>
#elif defined __linux__
#include <fstream>
#include <ctype.h>
#endif
#include "backend/cpu/platform/BasicCpuInfo.h"
#include <thread>
#include <string.h>

#if defined __arm__ || defined __ARMEL__ || defined __arm64__ || defined __aarch64__ || defined __AARCH64EL__
#define XMRIG_ARM
#endif

#if defined XMRIG_ARM && __ARM_FEATURE_CRYPTO && !defined(__APPLE__)
#   include <sys/auxv.h>
#   include <asm/hwcap.h>
#endif

xmrig::BasicCpuInfo::BasicCpuInfo() :
    m_brand(),
    m_threads(std::thread::hardware_concurrency()),
    m_aes(false),
    m_avx2(false) {
#ifdef BSD
	int mib[] = { CTL_HW, HW_MODEL };
	size_t model_len = sizeof m_brand;
	if(sysctl(mib, 2, m_brand, &model_len, NULL, 0) < 0 && errno != ENOMEM) *m_brand = 0;
#elif defined __linux__
	std::ifstream stream("/proc/cpuinfo", std::ios_base::in);
	if(stream.is_open()) {
#if defined __sparc__ || defined __mips__ || defined __riscv
		std::string value;
#else
		std::string value1, value2;
#endif
		std::string line;
		while(std::getline(stream, line)) {
			size_t colon_i = line.find(':');
			if(colon_i == std::string::npos || !colon_i) continue;
			size_t value_i = line.find_first_not_of(' ', colon_i + 1);
			if(value_i == std::string::npos) continue;
			size_t i = line.find_last_not_of(" 	", colon_i - 1);
			i = (i == std::string::npos) ? colon_i : i + 1;
#ifdef XMRIG_ARM
			if(line.compare(0, i, "Hardware") == 0) value1.assign(line, value_i);
			else if(line.compare(0, i, "Processor") == 0) {
				if(isdigit(line[value_i])) continue;
				value2.assign(line, value_i);
			} else if(line.compare(0, i, "model name") == 0) value2.assign(line, value_i);
#elif defined __sh__
			if(line.compare(0, i, "machine") == 0) {
				value1.assign(line, value_i);
				value2.erase();
				break;
			}
			if(line.compare(0, i, "cpu family") == 0) value1.assign(line, value_i);
			else if(line.compare(0, i, "cpu type") == 0) value2.assign(line, value_i);
#elif defined __powerpc__
			if(line.compare(0, i, "vendor") == 0) value1.assign(line, value_i);
			else if(line.compare(0, i, "cpu") == 0) value2.assign(line, value_i);
			else continue;
			if(!value1.empty() && !value2.empty()) break;
#elif defined __sparc__
			if(line.compare(0, i, "cpu") == 0) {
				value.assign(line, value_i);
				break;
			}
#elif defined __mips__
			if(line.compare(0, i, "cpu model") == 0) {
				value.assign(line, value_i);
				break;
			}
#elif defined __riscv
			if(line.compare(0, i, "uarch") == 0) {
				value.assign(line, value_i);
				break;
			}
#else
			if(line.compare(0, i, "vendor_id") == 0) value1.assign(line, value_i);
			else if(line.compare(0, i, "model name") == 0) value2.assign(line, value_i);
			else continue;
			if(!value1.empty() && !value2.empty()) break;
#endif
		}
		stream.close();
#if defined __sparc__ || defined __mips__ || defined __riscv
		if(!value.empty()) value.copy(m_brand, sizeof m_brand - 1);
#else
		size_t len = value1.length();
		if(len) value1.copy(m_brand, sizeof m_brand - 1);
		if(!value2.empty() && len < sizeof m_brand - 1) {
			if(len) m_brand[len] = ' ';
			int offset = len ? len + 1 : 0;
			value2.copy(m_brand + offset, sizeof m_brand - 1 - offset);
		}
#endif
	}
#endif

	if(!*m_brand) {
#ifdef XMRIG_ARM
#if defined __aarch64__ || defined __AARCH64EL__
		memcpy(m_brand, "Unknown AArch64 processor", 23);
#else
		memcpy(m_brand, "Unknown ARM processor", 21);
#endif
#else
		memcpy(m_brand, "Unknown", 7);
#endif
	}

#if defined XMRIG_ARM && __ARM_FEATURE_CRYPTO && 0
# if !defined(__APPLE__)
    m_aes = getauxval(AT_HWCAP) & HWCAP_AES;
# else
    m_aes = true;
# endif
#endif
}


const char *xmrig::BasicCpuInfo::backend() const {
	return "basic_generic";
}


xmrig::CpuThreads xmrig::BasicCpuInfo::threads(const Algorithm &, uint32_t) const {
	return CpuThreads(threads());
}
