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
#endif
#include "backend/cpu/platform/BasicCpuInfo.h"
#include <thread>
#include <string.h>

xmrig::BasicCpuInfo::BasicCpuInfo() :
    m_brand(),
    m_threads(std::thread::hardware_concurrency()),
    m_aes(false),
    m_avx2(false) {
#ifdef BSD
	int mib[] = { CTL_HW, HW_MODEL };
	size_t model_len = sizeof m_brand;
	if(sysctl(mib, 2, m_brand, &model_len, NULL, 0) < 0 && errno != ENOMEM)
#endif
	memcpy(m_brand, "Unknown", 7);
}


const char *xmrig::BasicCpuInfo::backend() const {
	return "basic_unknown";
}


xmrig::CpuThreads xmrig::BasicCpuInfo::threads(const Algorithm &, uint32_t) const {
	return CpuThreads(threads());
}
