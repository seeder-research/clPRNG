#include "clPRNG.hpp"

// Main C interface functions

CLPRNG_DLL ClPRNG* clPRNG_create_stream() {
    return new ClPRNG;
}

CLPRNG_DLL cl_int clPRNG_generate_stream(ClPRNG* p, int count, cl_mem dst) {
    if (count < 0) {
        std::cout << "ERROR: count must be a positive integer!" << std::endl;
        return -1;
    }
    for (; count > 0;) {
        if ((count == 0) || (dst == NULL)) {
            break;
        }
        if (p->GetNumValidEntries() <= 0) {
            cl_int err = p->GenerateStream();
            if (err != 0) {
                std::cout << "ERROR: unable to generate random bit stream!" << std::endl;
                return err;
            }
            p->SetNumValidEntries(p->GetNumBufferEntries());
            p->SetBufferOffset(0);
        }
        size_t dst_offset = 0;
        if (count <= p->GetNumValidEntries()) {
	    cl_int err = p->CopyBufferEntries(dst, dst_offset, (size_t)(count));
            if (err != 0) {
                std::cout << "ERROR: unable to copy random bit stream from buffer to dst!" << std::endl;
                return err;
            }
            p->SetNumValidEntries(p->GetNumValidEntries() - count);
            p->SetBufferOffset(count + p->GetBufferOffset());
            break;
        } else {
	    cl_int err = p->CopyBufferEntries(dst, dst_offset, p->GetNumValidEntries());
            count -= p->GetNumValidEntries();
            dst_offset += p->GetNumValidEntries();
            p->SetNumValidEntries(0);
        }
    }
    return 0;
}

CLPRNG_DLL void initialize_prng(ClPRNG* p, cl_device_id dev_id, const char *name) {
    (*p).Init(dev_id, name);
}

CLPRNG_DLL cl_int buildPRNGKernelProgram(ClPRNG* p) {
    (*p).BuildSource();
    return (*p).BuildKernelProgram();
}

// Default constructor and destructor
ClPRNG::ClPRNG() {
    device = 0;
    context = 0;
    com_queue = 0;
    total_count = 0;
    valid_count = 0;
    seedVal = 0;
    offset = 0;
    wkgrp_size = 0;
    wkgrp_count = 0;
    init_flag = false;
    source_ready = false;
    rng_name = "mt19937";
    rng_precision = "ulong";
    rng_source = "";
}

ClPRNG::~ClPRNG() {
}

void ClPRNG::Init(cl_device_id dev_id, const char *name) {
    device = cl::Device(dev_id);
    cl_int err;
    context = cl::Context(device, NULL, NULL, NULL, &err);
    if (err) {
        std::cout << "ERROR: Unable to create context!" << std::endl;
        return;
    }
    com_queue = cl::CommandQueue(context, device, 0, &err);
    if (err) {
        std::cout << "ERROR: Unable to create command queue!" << std::endl;
        return;
    }
    rng_name = name;
    std::string prng_name = std::string(rng_name);
    rng_precision = "uint";
    init_flag = true;
}

int ClPRNG::SetPrecision(const char * precision) {
    std::string str = std::string(precision);
    if ((str == "uint") || (str == "ulong") || (str == "float") || (str == "double")) {
        rng_precision = precision;
    } else {
        fprintf(stderr, "Can only generate numbers of types: uint, ulong, float, double!");
        return -1;
    }
    return 0;
}

int ClPRNG::LookupPRNG(std::string name) {
    if (name == "isaac") {
        return 1;
    } else if (name == "kiss09") {
        return 2;
    } else if (name == "kiss99") {
        return 3;
    } else if (name == "lcg6432") {
        return 4;
    } else if (name == "lcg12864") {
        return 5;
    } else if (name == "lfib") {
        return 6;
    } else if (name == "mrg31k3p") {
        return 7;
    } else if (name == "mrg63k3a") {
        return 8;
    } else if (name == "msws") {
        return 9;
    } else if (name == "mt19937") {
        return 10;
    } else if (name == "mwc64x") {
        return 11;
    } else if (name == "pcg6432") {
        return 12;
    } else if (name == "philox2x32_10") {
        return 13;
    } else if (name == "ran2") {
        return 14;
    } else if (name == "tinymt32") {
        return 15;
    } else if (name == "tinymt64") {
        return 16;
    } else if (name == "tyche") {
        return 17;
    } else if (name == "tyche_i") {
        return 18;
    } else if (name == "well512") {
        return 19;
    } else if (name == "xorshift1024") {
        return 20;
    } else if (name == "xorshift6432star") {
        return 21;
    } else {
        return -1;
    }
    return -1;
}

void ClPRNG::generateBufferKernel(std::string name, std::string type, std::string src) {
    static std::string tmp = std::string((type=="double") ? " #pragma OPENCL EXTENSION cl_khr_fp64 : enable \n" : "");
    switch(ClPRNG::LookupPRNG(name)) {
        case 1 :
            tmp += isaac_prng_kernel;
            break;
        case 2 :
            tmp += kiss09_prng_kernel;
            break;
        case 3 :
            tmp += kiss99_prng_kernel;
            break;
        case 4 :
            tmp += lcg6432_prng_kernel;
            break;
        case 5 :
            tmp += lcg12864_prng_kernel;
            break;
        case 6 :
            tmp += lfib_prng_kernel;
            break;
        case 7 :
            tmp += mrg31k3p_prng_kernel;
            break;
        case 8 :
            tmp += mrg63k3a_prng_kernel;
            break;
        case 9 :
            tmp += msws_prng_kernel;
            break;
        case 10 :
            tmp += mt19937_prng_kernel;
            break;
        case 11 :
            tmp += mwc64x_prng_kernel;
            break;
        case 12 :
            tmp += pcg6432_prng_kernel;
            break;
        case 13 :
            tmp += philox2x32_10_prng_kernel;
            break;
        case 14 :
            tmp += ran2_prng_kernel;
            break;
        case 15 :
            tmp += tinymt32_prng_kernel;
            break;
        case 16 :
            tmp += tinymt64_prng_kernel;
            break;
        case 17 :
            tmp += tyche_prng_kernel;
            break;
        case 18 :
            tmp += tyche_i_prng_kernel;
            break;
        case 19 :
            tmp += well512_prng_kernel;
            break;
        case 20 :
            tmp += xorshift1024_prng_kernel;
            break;
        case 21 :
            tmp += xorshift6432star_prng_kernel;
            break;
        default :
            std::cout << "Unknown PRNG. No implementation found!" << std::endl;
            break;
    }
    switch(ClPRNG::LookupPRNG(name)) {
        case 20 :
            tmp += "\n"
                   "kernel void seed_prng(uint num, global ulong* seed, global " + name + "_state* state){\n"
                   "    uint gid=get_global_id(0);\n"
                   "    uint gsize=get_global_size(0);\n"
                   "\n"
                   "    " + name + "_seed(state,seed[gid]);\n"
                   "}";
                   "\n"
                   "kernel void generate(uint num, global ulong* seed, global " + type + "* res, global " + name + "_state* stateBuf, local " + name + "_state* state){\n"
                   "    uint gid=get_global_id(0);\n"
                   "    uint gsize=get_global_size(0);\n"
                   "\n"
                   "    state = stateBuf;\n"
                   "    uint num_gsize = ((num - 1) / gsize + 1)*gsize; //next multiple of gsize, larger or equal to N\n"
                   "    for (int i = gid; i<num_gsize; i += gsize) {\n"
                   "        " + type + " val = " + name + "_" + type + "(state); //all threads within workgroup must call generator, even if result is not needed!\n"
                   "        if (i<num) {\n"
                   "            res[i] = val;\n"
                   "        }\n"
                   "    }\n"
                   "}";
                   break;
        default :
            tmp += "\n"
                   "kernel void seed_prng(global ulong* seed, global " + name + "_state* stateBuf){\n"
                   "    uint gid=get_global_id(0);\n"
                   "    uint gsize=get_global_size(0);\n"
                   "    " + name + "_state state;\n"
                   "    " + name + "_seed(&state,seed[gid]);\n"
                   "    stateBuf[gid] = state;\n"
                   "}"
                   "\n"
                   "kernel void generate(uint num, global " + name + "_state* stateBuf, global " + type + "* res){\n"
                   "    uint gid=get_global_id(0);\n"
                   "    uint gsize=get_global_size(0);\n"
                   "    " + name + "_state state;\n"
                   "    state = stateBuf[gid];\n"
                   "    for(uint i=gid;i<num;i+=gsize){\n"
                   "        res[i]=" + name + "_" + type + "(state);\n"
                   "    }\n"
                   "}";
                   break;
    }
    src = tmp;
}

void ClPRNG::BuildSource() {
    source_ready = false;
    std::string &kernel_src = rng_source;
    generateBufferKernel(std::string(rng_name), std::string(rng_precision), kernel_src);
    source_ready = true;
}

cl_int ClPRNG::BuildKernelProgram() {
    cl_int err;
    if (init_flag && source_ready) {
        cl::Program::Sources sources(1, std::make_pair(rng_source.c_str(), rng_source.length()));
        rng_program = cl::Program(context, sources);
        std::string build_args = "-cl-std=CL1.2 -cl-kernel-arg-info";
        err = rng_program.build(std::vector<cl::Device>({device}), build_args.c_str());
        if (err) {
            std::cout << "ERROR: Unable to build PRNG program!" << std::endl;
            return err;
        }
        seed_rng = cl::Kernel(rng_program, "seed_prng");
        generate_bitstream = cl::Kernel(rng_program, "generate");
        return err;
    }
    return -1;
}

cl_int ClPRNG::ReadyGenerator() {
    cl_int err;
    err = device.getInfo<cl_uint>(CL_DEVICE_MAX_WORK_GROUP_SIZE, &wkgrp_size);
    if (err) {
        std::cout << "ERROR: failed to get max workgroup size on device!" << std::endl;
        return err;
    }
    err = device.getInfo<cl_uint>(CL_DEVICE_MAX_COMPUTE_UNITS, &wkgrp_count);
    if (err) {
        std::cout << "ERROR: failed to get max number of compute unites on device!" << std::endl;
        return err;
    }
    if (wkgrp_size > 256) {
        wkgrp_size = 256;
    }
    if (std::string(rng_name) == "xorshift1024") {
        wkgrp_size = 32;
    }
    size_t typeSize = 4;
    if (std::string(rng_precision) == "uint") {
        typeSize = 2;
    } else if (std::string(rng_precision) == "double") {
        typeSize = 8;
    } else {
        std::cout << "ERROR: Unknown rng_precision detected!" << std::endl;
        return -1;
    }
    SetStateSize();
    size_t stateBufSize = (size_t)(wkgrp_count * wkgrp_size) * state_size;
    size_t outBufSize = (size_t)(wkgrp_count * wkgrp_size) * typeSize;
    stateBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, stateBufSize, NULL, &err);
    if (err) {
        std::cout << "ERROR: Unable to create state buffer or PRNG!" << std::endl;
        return err;
    }
    tmpOutputBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, outBufSize, NULL, &err);
    if (err) {
        std::cout << "ERROR: Unable to create temporary buffer or PRNG!" << std::endl;
        return err;
    }
    // At this point the buffers are set up...
    // TODO:
    // 1) Call (public) function to set the seed value
    // 2) Call (private) function to initialize the PRNG states and save in stateBuffer
    // 3) Call (private) function to generate a set of random numbers to fill the temporary buffer
    // 4) Initialize the counters that tracks available random numbers
    return err;
}

void ClPRNG::SetStateSize() {
    switch(LookupPRNG(std::string(rng_name)))
    {
        case 1:
            state_size = sizeof(isaac_state);
            break;
        case 2:
            state_size = sizeof(kiss09_state);
            break;
        case 3:
            state_size = sizeof(kiss99_state);
            break;
        case 4:
            state_size = sizeof(lcg6432_state);
            break;
        case 5:
            state_size = sizeof(lcg12864_state);
            break;
        case 6:
            state_size = sizeof(lfib_state);
            break;
        case 7:
            state_size = sizeof(mrg31k3p_state);
            break;
        case 8:
            state_size = sizeof(mrg63k3a_state);
            break;
        case 9:
            state_size = sizeof(msws_state);
            break;
        case 10:
            state_size = sizeof(mt19937_state);
            break;
        case 11:
            state_size = sizeof(mwc64x_state);
            break;
        case 12:
            state_size = sizeof(pcg6432_state);
            break;
        case 13:
            state_size = sizeof(philox2x32_10_state);
            break;
        case 14:
            state_size = sizeof(ran2_state);
            break;
        case 15:
            state_size = sizeof(tinymt32_state);
            break;
        case 16:
            state_size = sizeof(tinymt64_state);
            break;
        case 17:
            state_size = sizeof(tyche_state);
            break;
        case 18:
            state_size = sizeof(tyche_i_state);
            break;
        case 19:
            state_size = sizeof(well512_state);
            break;
        case 20:
            state_size = sizeof(xorshift1024_state);
            break;
        case 21:
            state_size = sizeof(xorshift6432star_state);
            break;
        default :
            state_size = 0;
            break;
    }
}

void ClPRNG::SetBufferOffset(size_t ptr) {
    if ((std::string)(rng_precision) == "double") {
        offset = ptr * 8;
    } else {
        offset = ptr * 4;
    }
}

size_t ClPRNG::GetBufferOffset() {
    if ((std::string)(rng_precision) == "double") {
        return (offset / 8);
    }
    return (offset / 4);
}

// TODO:
cl_int ClPRNG::CopyBufferEntries(cl_mem dst, size_t dst_offset, size_t count) {
    return 0;
}

// TODO:
cl_int ClPRNG::SeedGenerator() {
    return 0;
}

// TODO:
cl_int ClPRNG::GenerateStream() {
    return 0;
}

