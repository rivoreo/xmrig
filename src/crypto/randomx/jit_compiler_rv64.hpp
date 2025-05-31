/*
Copyright (c) 2023 tevador <tevador@gmail.com>

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
	* Redistributions of source code must retain the above copyright
	  notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright
	  notice, this list of conditions and the following disclaimer in the
	  documentation and/or other materials provided with the distribution.
	* Neither the name of the copyright holder nor the
	  names of its contributors may be used to endorse or promote products
	  derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include "jit_compiler.hpp"

#define ALIGN_SIZE(N,A) ((((N)-1)/(A)+1)*(A))

#define HANDLER_ARGS randomx::CompilerState& state, randomx::Instruction isn, int i

namespace randomx {

	class Program;
	struct ProgramConfiguration;
	class SuperscalarProgram;
	class Instruction;

	class RV64JitCompiler {
	public:
		static constexpr size_t MaxRandomXInstrCodeSize = 56;     //FDIV_M requires 56 bytes of rv64 code
		static constexpr size_t MaxSuperscalarInstrSize = 12;     //IXOR_C requires 12 bytes of rv64 code
		static constexpr size_t SuperscalarProgramHeader = 136;   //overhead per superscalar program
		static constexpr size_t CodeAlign = 4096;                 //align code size to a multiple of 4 KiB
		static constexpr size_t LiteralPoolSize = CodeAlign;
		const size_t SuperscalarLiteraPoolSize = RandomX_CurrentConfig.CacheAccesses * CodeAlign;
		static constexpr size_t ReserveCodeSize = CodeAlign;  //prologue, epilogue + reserve

		const size_t RandomXCodeSize = ALIGN_SIZE(LiteralPoolSize + ReserveCodeSize + MaxRandomXInstrCodeSize * RandomX_CurrentConfig.ProgramSize, CodeAlign);
		const size_t SuperscalarSize = ALIGN_SIZE(SuperscalarLiteraPoolSize + ReserveCodeSize + (SuperscalarProgramHeader + MaxSuperscalarInstrSize * (3 * RandomX_CurrentConfig.SuperscalarLatency + 2)) * RandomX_CurrentConfig.CacheAccesses, CodeAlign);

		const uint32_t CodeSize = RandomXCodeSize + SuperscalarSize;
		const uint32_t ExecutableSize = CodeSize - LiteralPoolSize;

		static constexpr int32_t LiteralPoolOffset = LiteralPoolSize / 2;
		const int32_t SuperScalarLiteralPoolOffset = RandomXCodeSize;
		const int32_t SuperScalarLiteralPoolRefOffset = RandomXCodeSize + (RandomX_CurrentConfig.CacheAccesses - 1) * LiteralPoolSize + LiteralPoolOffset;
		const int32_t SuperScalarHashOffset = SuperScalarLiteralPoolOffset + SuperscalarLiteraPoolSize;

		RV64JitCompiler();
		~RV64JitCompiler();

		void generateProgram(Program&, ProgramConfiguration&);
		void generateProgramLight(Program&, ProgramConfiguration&, uint32_t);
		void generateSuperscalarHash(SuperscalarProgram programs[RANDOMX_CACHE_MAX_ACCESSES], std::vector<uint64_t>&);
		void generateDatasetInitCode() {}
		ProgramFunc* getProgramFunc() {
			return (ProgramFunc*)entryProgram;
		}
		DatasetInitFunc* getDatasetInitFunc() {
			return (DatasetInitFunc*)entryDataInit;
		}
		uint8_t* getCode() {
			return state.code;
		}
		size_t getCodeSize();
		using InstructionHandler = void(HANDLER_ARGS);
		static InstructionHandler *opcodeMap1[256];
		static void v1_IADD_RS(HANDLER_ARGS);
		static void v1_IADD_M(HANDLER_ARGS);
		static void v1_ISUB_R(HANDLER_ARGS);
		static void v1_ISUB_M(HANDLER_ARGS);
		static void v1_IMUL_R(HANDLER_ARGS);
		static void v1_IMUL_M(HANDLER_ARGS);
		static void v1_IMULH_R(HANDLER_ARGS);
		static void v1_IMULH_M(HANDLER_ARGS);
		static void v1_ISMULH_R(HANDLER_ARGS);
		static void v1_ISMULH_M(HANDLER_ARGS);
		static void v1_IMUL_RCP(HANDLER_ARGS);
		static void v1_INEG_R(HANDLER_ARGS);
		static void v1_IXOR_R(HANDLER_ARGS);
		static void v1_IXOR_M(HANDLER_ARGS);
		static void v1_IROR_R(HANDLER_ARGS);
		static void v1_IROL_R(HANDLER_ARGS);
		static void v1_ISWAP_R(HANDLER_ARGS);
		static void v1_FSWAP_R(HANDLER_ARGS);
		static void v1_FADD_R(HANDLER_ARGS);
		static void v1_FADD_M(HANDLER_ARGS);
		static void v1_FSUB_R(HANDLER_ARGS);
		static void v1_FSUB_M(HANDLER_ARGS);
		static void v1_FSCAL_R(HANDLER_ARGS);
		static void v1_FMUL_R(HANDLER_ARGS);
		static void v1_FDIV_M(HANDLER_ARGS);
		static void v1_FSQRT_R(HANDLER_ARGS);
		static void v1_CBRANCH(HANDLER_ARGS);
		static void v1_CFROUND(HANDLER_ARGS);
		static void v1_ISTORE(HANDLER_ARGS);
		static void v1_NOP(HANDLER_ARGS);
	private:
		CompilerState state;
		void* entryDataInit;
		void* entryProgram;
		void clearCache(CodeBuffer &);
	};
}
