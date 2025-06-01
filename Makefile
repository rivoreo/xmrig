# I still don't want to use cmake(1)

DEFINES += -D _GNU_SOURCE=1 -D __STDC_FORMAT_MACROS=1
# Linking to OpenSSL creates license issues, disable it for now
DEFINES += -D XMRIG_NO_TLS=1
DEFINES += -D XMRIG_NO_API=1 -D XMRIG_NO_HTTPD=1
#DEFINES += -D RAPIDJSON_SSE2=1
DEFINES += -D NDEBUG=1
INCLUDE_PATHS += -I src -I src/3rdparty
CFLAGS += $(DEFINES) $(INCLUDE_PATHS) -Wall -std=gnu99
ifeq ($(filter -O%,$(CFLAGS)),)
CFLAGS += -O3
endif
CXXFLAGS += $(DEFINES) $(INCLUDE_PATHS) -D _GLIBCXX_USE_NANOSLEEP=1 -D _GLIBCXX_USE_SCHED_YIELD=1 -Wall -std=gnu++0x -funroll-loops -fmerge-all-constants
ifeq ($(filter -O%,$(CXXFLAGS)),)
CXXFLAGS += -O3
endif
#CXXFLAGS += -Dnullptr=__null -Dconstexpr=const -Doverride= -fpermissive
CXXFLAGS += -D "alignas(b)=__attribute__((__aligned__))"
LIBS += -l uv -l pthread

# Need for FreeBSD
#LIBS += -lkvm

ifndef ARCH
GET_ARCH_COMMAND := arch=`uname -m` && case $$arch in i?86|i86*|amd64|x86_64) echo x86; ;; arm*|aarch64) echo arm; ;; *) echo $$arch; ;; esac
ARCH := $(shell $(GET_ARCH_COMMAND))
endif

ifdef WITH_ASM
WITH_CRYPTONIGHT_ASM := 1
WITH_RANDOMX_ASM := 1
endif
ifdef WITH_RANDOMX_JIT
WITH_RANDOMX_ASM := 1
endif

SOURCES = \
    src/base/io/json/Json.cpp \
    src/base/io/json/Json_unix.cpp \
    src/base/io/json/JsonChain.cpp \
    src/base/io/json/JsonRequest.cpp \
    src/base/io/log/backends/ConsoleLog.cpp \
    src/base/io/log/backends/FileLog.cpp \
    src/base/io/log/Log.cpp \
    src/base/io/Watcher.cpp \
    src/base/kernel/Base.cpp \
    src/base/kernel/config/BaseConfig.cpp \
    src/base/kernel/config/BaseTransform.cpp \
    src/base/kernel/Entry.cpp \
    src/base/kernel/Platform.cpp \
    src/base/kernel/Platform_unix.cpp \
    src/base/kernel/Process.cpp \
    src/base/kernel/Signals.cpp \
    src/base/net/dns/Dns.cpp \
    src/base/net/dns/DnsRecord.cpp \
    src/base/net/http/Http.cpp \
    src/base/net/stratum/BaseClient.cpp \
    src/base/net/stratum/Client.cpp \
    src/base/net/stratum/Job.cpp \
    src/base/net/stratum/Pool.cpp \
    src/base/net/stratum/Pools.cpp \
    src/base/net/stratum/Url.cpp \
    src/base/net/stratum/strategies/FailoverStrategy.cpp \
    src/base/net/stratum/strategies/SinglePoolStrategy.cpp \
    src/base/tools/Arguments.cpp \
    src/base/tools/Buffer.cpp \
    src/base/tools/String.cpp \
    src/base/tools/Timer.cpp

DEFINES += -D HAVE_SYSLOG_H=1
SOURCES += src/base/io/log/backends/SysLog.cpp

ifdef WITH_HTTP
SOURCES += \
	src/3rdparty/http-parser/http_parser.c \
	src/base/api/Api.cpp \
	src/base/api/Httpd.cpp \
	src/base/api/requests/ApiRequest.cpp \
	src/base/api/requests/HttpApiRequest.cpp \
	src/base/net/http/HttpApiResponse.cpp \
	src/base/net/http/HttpClient.cpp \
	src/base/net/http/HttpContext.cpp \
	src/base/net/http/HttpResponse.cpp \
	src/base/net/http/HttpServer.cpp \
	src/base/net/stratum/DaemonClient.cpp \
	src/base/net/tools/TcpServer.cpp

DEFINES += -D XMRIG_FEATURE_HTTP=1 -D XMRIG_FEATURE_API=1
endif

SOURCES += \
    src/backend/cpu/Cpu.cpp \
    src/backend/cpu/CpuBackend.cpp \
    src/backend/cpu/CpuConfig.cpp \
    src/backend/cpu/CpuLaunchData.h \
    src/backend/cpu/CpuThread.cpp \
    src/backend/cpu/CpuThreads.cpp \
    src/backend/cpu/CpuWorker.cpp

SOURCES += \
    src/backend/common/Hashrate.cpp \
    src/backend/common/Threads.cpp \
    src/backend/common/Worker.cpp \
    src/backend/common/Workers.cpp

SOURCES += \
    src/App.cpp \
    src/core/config/Config.cpp \
    src/core/config/ConfigTransform.cpp \
    src/core/Controller.cpp \
    src/core/Miner.cpp \
    src/net/JobResults.cpp \
    src/net/Network.cpp \
    src/net/NetworkState.cpp \
    src/net/strategies/DonateStrategy.cpp \
    src/Summary.cpp \
    src/xmrig.cpp

SOURCES += \
    src/crypto/cn/c_blake256.c \
    src/crypto/cn/c_groestl.c \
    src/crypto/cn/c_jh.c \
    src/crypto/cn/c_skein.c \
    src/crypto/cn/CnCtx.cpp \
    src/crypto/cn/CnHash.cpp \
    src/crypto/common/Algorithm.cpp \
    src/crypto/common/Coin.cpp \
    src/crypto/common/keccak.cpp \
    src/crypto/common/MemoryPool.cpp \
    src/crypto/common/Nonce.cpp \
    src/crypto/common/VirtualMemory.cpp

SOURCES += \
	src/App_unix.cpp \
	src/crypto/common/VirtualMemory_unix.cpp

ifdef WITH_RANDOMX
DEFINES += -D XMRIG_ALGO_RANDOMX=1
SOURCES += \
	src/crypto/randomx/aes_hash.cpp \
	src/crypto/randomx/allocator.cpp \
	src/crypto/randomx/argon2_core.c \
	src/crypto/randomx/argon2_ref.c \
	src/crypto/randomx/blake2_generator.cpp \
	src/crypto/randomx/blake2/blake2b.c \
	src/crypto/randomx/bytecode_machine.cpp \
	src/crypto/randomx/dataset.cpp \
	src/crypto/randomx/instructions_portable.cpp \
	src/crypto/randomx/randomx.cpp \
	src/crypto/randomx/reciprocal.c \
	src/crypto/randomx/soft_aes.cpp \
	src/crypto/randomx/superscalar.cpp \
	src/crypto/randomx/virtual_machine.cpp \
	src/crypto/randomx/virtual_memory.cpp \
	src/crypto/randomx/vm_compiled_light.cpp \
	src/crypto/randomx/vm_compiled.cpp \
	src/crypto/randomx/vm_interpreted_light.cpp \
	src/crypto/randomx/vm_interpreted.cpp \
	src/crypto/rx/Rx.cpp \
	src/crypto/rx/RxAlgo.cpp \
	src/crypto/rx/RxBasicStorage.cpp \
	src/crypto/rx/RxCache.cpp \
	src/crypto/rx/RxConfig.cpp \
	src/crypto/rx/RxDataset.cpp \
	src/crypto/rx/RxQueue.cpp \
	src/crypto/rx/RxVm.cpp
ifdef WITH_RANDOMX_ASM
DEFINES += -D XMRIG_FEATURE_RANDOMX_ASM=1
ifeq ($(ARCH),x86)
SOURCES += \
	src/crypto/randomx/jit_compiler_x86_static.S \
	src/crypto/randomx/jit_compiler_x86.cpp
else ifeq ($(ARCH),riscv64)
SOURCES += \
	src/crypto/randomx/jit_compiler_rv64_static.S \
	src/crypto/randomx/jit_compiler_rv64.cpp
endif
endif	# WITH_RANDOMX_ASM
ifdef WITH_HWLOC
SOURCES += \
	src/crypto/rx/RxNUMAStorage.cpp \
	src/crypto/rx/RxConfig_hwloc.cpp
else
SOURCES += src/crypto/rx/RxConfig_basic.cpp
endif	# WITH_HWLOC
endif	# WITH_RANDOMX

ifdef WITH_ARGON2
DEFINES += -D XMRIG_ALGO_ARGON2=1
SOURCES += \
	src/crypto/argon2/Impl.cpp \
	src/3rdparty/argon2/lib/argon2.c \
	src/3rdparty/argon2/lib/core.c \
	src/3rdparty/argon2/lib/encoding.c \
	src/3rdparty/argon2/lib/genkat.c \
	src/3rdparty/argon2/lib/impl-select.c \
	src/3rdparty/argon2/lib/blake2/blake2.c
endif

ifdef WITH_CRYPTONIGHT_ASM
DEFINES += -D XMRIG_FEATURE_CRYPTONIGHT_ASM=1
SOURCES += \
	src/crypto/common/Assembly.cpp \
	src/crypto/cn/r/CryptonightR_gen.cpp
ifdef WIN32
SOURCES += \
	src/crypto/cn/asm/win64/cn_main_loop.S \
	src/crypto/cn/asm/CryptonightR_template.S
else
SOURCES += \
	src/crypto/cn/asm/cn_main_loop.S \
	src/crypto/cn/asm/CryptonightR_template.S
endif
else
#DEFINES += -D XMRIG_NO_ASM=1
endif	# WITH_CRYPTONIGHT_ASM

ifdef WITH_HWLOC
DEFINES += -D XMRIG_FEATURE_HWLOC=1
LIBS += -l hwloc
SOURCES += \
	src/base/kernel/Platform_hwloc.cpp \
	src/crypto/common/NUMAMemoryPool.cpp \
	src/crypto/common/VirtualMemory_hwloc.cpp \
	src/backend/cpu/platform/HwlocCpuInfo.cpp
endif

ifdef WITH_LIBCPUID
DEFINES += -D XMRIG_FEATURE_LIBCPUID=1
INCLUDE_PATHS += -I src/3rdparty/libcpuid
#LIBS += -l cpuid
DEPENDS += src/3rdparty/libcpuid/libcpuid.a
LIBS += src/3rdparty/libcpuid/libcpuid.a
SOURCES += src/backend/cpu/platform/AdvancedCpuInfo.cpp
ifdef WITH_HWLOC
SOURCES += src/backend/cpu/platform/BasicCpuInfo.cpp
endif
else
DEFINES += -D XMRIG_NO_LIBCPUID=1
#SOURCES += src/common/cpu/Cpu.cpp
#ifeq ($(ARCH),arm)
#SOURCES += src/backend/cpu/platform/BasicCpuInfo_arm.cpp
#else
SOURCES += src/backend/cpu/platform/BasicCpuInfo.cpp
#endif
endif

OBJECTS = $(addsuffix .o,$(basename $(SOURCES)))

xmrig:	$(OBJECTS) $(DEPENDS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

clean:
	$(MAKE) -C src/3rdparty/libcpuid/ $@
	rm -f $(OBJECTS)

src/3rdparty/libcpuid/libcpuid.a:
	$(MAKE) -C src/3rdparty/libcpuid/
