/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
 * Copyright 2019      Spudz76     <https://github.com/Spudz76>
 * Copyright 2018-2019 SChernykh   <https://github.com/SChernykh>
 * Copyright 2016-2019 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
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


#include <syslog.h>
#include "base/io/log/Log.h"
#include "base/io/log/backends/SysLog.h"
#include "version.h"


xmrig::SysLog::SysLog()
{
    openlog(APP_ID, LOG_PID, LOG_USER);
}


xmrig::SysLog::~SysLog() noexcept
{
    closelog();
}


void xmrig::SysLog::print(int level, const char *line, size_t offset, size_t, bool colors)
{
	if(colors) return;
	int syslog_priority = LOG_CRIT;
	switch(level) {
		case Log::ERR:
			syslog_priority = LOG_ERR;
			break;
		case Log::WARNING:
			syslog_priority = LOG_WARNING;
			break;
		case Log::NOTICE:
			syslog_priority = LOG_NOTICE;
			break;
		case Log::NONE:
		case Log::INFO:
			syslog_priority = LOG_INFO;
			break;
		case Log::DEBUG:
			syslog_priority = LOG_DEBUG;
			break;
	}
	syslog(syslog_priority, "%s", line + offset);
}
