#include <iostream>
#include <map>
#include <string>

#define __CL_ENABLE_EXCEPTIONS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#if defined(__APPLE__) || defined(__MACOSX)
    #include <OpenCL/cl.hpp>
#else
    #include <CL/cl.hpp>
#endif

#include "../generator/isaac.hpp"
#include "../generator/kiss09.hpp"
#include "../generator/kiss99.hpp"
#include "../generator/lcg6432.hpp"
#include "../generator/lcg12864.hpp"
#include "../generator/lfib.hpp"
#include "../generator/mrg31k3p.hpp"
#include "../generator/mrg63k3a.hpp"
#include "../generator/msws.hpp"
#include "../generator/mt19937.hpp"
#include "../generator/mwc64x.hpp"
#include "../generator/pcg6432.hpp"
#include "../generator/philox2x32_10.hpp"
#include "../generator/ran2.hpp"
#include "../generator/tinymt32.hpp"
#include "../generator/tinymt64.hpp"
#include "../generator/tyche.hpp"
#include "../generator/tyche_i.hpp"
#include "../generator/well512.hpp"
#include "../generator/xorshift1024.hpp"
#include "../generator/xorshift6432star.hpp"

#ifndef __CLPRNG_HPP
    #define __CLPRNG_HPP
    #define CLPRNG_VERSION_MAJOR 0
    #define CLPRNG_VERSION_MINOR 0
    #define CLPRNG_VERSION_REV   1
#endif

#if defined( __WIN32 )
    #if defined( CLPRNG_STATIC )
        #define CLPRNG_DLL
    #elif  defined( CLPRNG_EXPORT )
        #define CLPRNG_DLL __declspec(dllexport)
    #else
        #define CLPRNG_DLL __declspec(dllimport)
    #endif
#else
    #define CLPRNG_DLL
#endif

// Prototype class
CLPRNG_DLL class ClPRNG {
    private:
        cl::Device        device;
        cl::Context       context;
        cl::CommandQueue  com_queue;

        cl::Program       rng_program;
        cl::Kernel        seed_rng;
        cl::Kernel        generate_bitstream;

        cl::Buffer        stateBuffer;
        cl::Buffer        tmpOutputBuffer;
        size_t            state_size;
        size_t            total_count;
        size_t            valid_count;
        size_t            offset;

        cl_uint           wkgrp_size;
        cl_uint           wkgrp_count;

        const char*       rng_name;
        const char*       rng_precision;
        std::string       rng_source;
        ulong             seedVal;

        bool              source_ready;
        bool              init_flag;

        int LookupPRNG(std::string name);
        void generateBufferKernel(std::string name, std::string type, std::string src);
        cl_int fillBuffer();
        void SetStateSize();
        cl_int PrivateGenerateStream(); // To implement

    public:
        ClPRNG();
        ~ClPRNG();

        void Init(cl_device_id dev_id, const char * name);

        void BuildSource();
        std::string GetSource() { return rng_source; }

        cl_int BuildKernelProgram();
        cl_int ReadyGenerator(); // To complete
        cl_int SeedGenerator(); // To implement
        cl_int GenerateStream(); // To implement

        size_t GetNumBufferEntries() { return total_count; }
        void SetNumBufferEntries(size_t num) { total_count = num; }

        size_t GetNumValidEntries() { return valid_count; }
        void SetNumValidEntries(size_t num) { valid_count = num; }

        void SetBufferOffset(size_t ptr);
        size_t GetBufferOffset();

        std::string GetPrecision() { return std::string(rng_precision); }
        int SetPrecision(const char * precision);

        std::string GetName() { return std::string(rng_name); }
        void SetName(const char * name) { rng_name = name; }

        void SetSeed(ulong seed) { seedVal = seed; }

        bool IsSourceReady() { return source_ready; }
        bool IsInitialized() { return init_flag; }

	cl_int CopyBufferEntries(cl_mem dst, size_t dst_offset, size_t count);
};

// Internal functions
cl_int buildPRNGKernelProgram(ClPRNG* p);

// External functions
#ifdef __cplusplus
extern "C" {
#endif
CLPRNG_DLL void initialize_prng(ClPRNG* p, cl_device_id dev_id, const char *name);

CLPRNG_DLL ClPRNG* clPRNG_create_stream();

CLPRNG_DLL cl_int clPRNG_generate_stream(ClPRNG* p, int count, cl_mem dst);

CLPRNG_DLL const char * get_precision(ClPRNG* p) {
    return (*p).GetPrecision().c_str();
}

CLPRNG_DLL int set_precision(ClPRNG* p, const char* precision) {
    return (*p).SetPrecision(precision);
}

CLPRNG_DLL const char * get_name(ClPRNG* p) {
    return (*p).GetName().c_str();
}

CLPRNG_DLL cl_int set_name(ClPRNG* p, const char* name) {
    (*p).SetName(name);
    return buildPRNGKernelProgram(p);
}

#ifdef __cplusplus
}
#endif
