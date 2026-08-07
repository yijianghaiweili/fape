#pragma once
#ifdef USE_FPGA

#include "util/common.h"
#include "util/log/log.h"
#include "util/graph/CDirectedGraph.h"
#include "util/xcl2/xcl2.hpp"
// #include <CL/cl2.hpp>

#define MAX_EVENTS_NUM 64

typedef std::vector<ushort, aligned_allocator<ushort>> FPGAVecShort;
typedef std::vector<uint, aligned_allocator<uint>> FPGAVec;
typedef std::vector<ulonglong, aligned_allocator<ulonglong>> FPGAVecLong;

class CHost
{
private:
	bool is_init_;
	bool is_buffer_init_;

	std::string binaryFile_;
	cl::Device device_;
	cl::Context context_;
	cl::CommandQueue q_;
	cl::CommandQueue qo_;
	cl::Program program_;
	std::vector<cl::Kernel> kernels_;
	std::vector<cl::Event> events_;

	FPGAVec vVer_;
	FPGAVec vPos_;
	FPGAVec vEdg_;
	FPGAVecShort vSDu_t_;
	FPGAVec bufferDeviceDDRPaths_;
	FPGAVec bufferDeviceResults_;
	FPGAVec bufferDeviceError_;
	FPGAVecLong bufferDeviceResultsLong_;
	FPGAVecLong bufferDeviceDebugLong_;

	FPGAVec vVer_1_;
	FPGAVec vPos_1_;
	FPGAVec vEdg_1_;
	FPGAVecShort vSDu_t_1_;
	FPGAVec vDeviceDDRPaths_1_;
	FPGAVec vDeviceResults_1_;
	FPGAVec vDeviceError_;
	FPGAVecLong vDeviceResultsLong_1_;
	FPGAVecLong vDeviceDebugLong_1_;

	FPGAVec vVer_2_;
	FPGAVec vPos_2_;
	FPGAVec vEdg_2_;
	FPGAVecShort vSDu_t_2_;
	FPGAVec vDeviceDDRPaths_2_;
	FPGAVec vDeviceResults_2_;
	FPGAVec vDeviceError_2_;
	FPGAVecLong vDeviceResultsLong_2_;
	FPGAVecLong vDeviceDebugLong_2_;

	FPGAVec vVer_3_;
	FPGAVec vPos_3_;
	FPGAVec vEdg_3_;
	FPGAVecShort vSDu_t_3_;
	FPGAVec vDeviceDDRPaths_3_;
	FPGAVec vDeviceResults_3_;
	FPGAVec vDeviceError_3_;
	FPGAVecLong vDeviceResultsLong_3_;
	FPGAVecLong vDeviceDebugLong_3_;

	FPGAVec vVer_4_;
	FPGAVec vPos_4_;
	FPGAVec vEdg_4_;
	FPGAVecShort vSDu_t_4_;
	FPGAVec vDeviceDDRPaths_4_;
	FPGAVec vDeviceResults_4_;
	FPGAVec vDeviceError_4_;
	FPGAVecLong vDeviceResultsLong_4_;
	FPGAVecLong vDeviceDebugLong_4_;

	MyVecLong deviceResults_; // [0]-res, [1]-err, [2]-d2b, [3]-b2d

	ulonglong get_duration_ns(const cl::Event &event)
	{
		cl_int err;
		ulonglong nstimestart, nstimeend;
		OCL_CHECK(err, err = event.getProfilingInfo<ulonglong>(CL_PROFILING_COMMAND_START, &nstimestart));
		OCL_CHECK(err, err = event.getProfilingInfo<ulonglong>(CL_PROFILING_COMMAND_END, &nstimeend));
		return (nstimeend - nstimestart);
	}

public:
	explicit CHost(std::string binaryFile, bool bInit)
	{
		binaryFile_ = binaryFile;
		is_init_ = bInit;
		is_buffer_init_ = false;
		if (bInit)
		{
			cl_int err;

			auto devices = xcl::get_xil_devices();
			device_ = devices[0];

			OCL_CHECK(err, context_ = cl::Context(device_, NULL, NULL, NULL, &err));
			OCL_CHECK(err, q_ = cl::CommandQueue(context_, device_, CL_QUEUE_PROFILING_ENABLE, &err));
			OCL_CHECK(err, qo_ = cl::CommandQueue(context_, device_, CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err));
			std::string device_name = device_.getInfo<CL_DEVICE_NAME>(&err);
			// OCL_CHECK(err, std::string device_name = device_.getInfo<CL_DEVICE_NAME>(&err));
			auto fileBuf = xcl::read_binary_file(binaryFile_);
			cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
			devices.resize(1);
			OCL_CHECK(err, program_ = cl::Program(context_, devices, bins, NULL, &err));

			events_.resize(MAX_EVENTS_NUM);
		}
		deviceResults_.resize(7, 0);
	}

	~CHost()
	{
		events_.clear();
		deviceResults_.clear();
	}

	inline MyVecLong deviceResults() { return deviceResults_; }

	void PEFPOnFPGA(CDirectedGraph *graph, const uint &s, const uint &t, const ushort &k, ushort *&arrSdut, ulonglong &time);
	void MYDFSOnFPGA2(CDirectedGraph *graph, const uint &s, const uint &t, const ushort &k, ushort *&arrSdut, ulonglong &time);
	void MYDFSOnFPGA3(CDirectedGraph *graph, const uint &s, const uint &t, const ushort &k, ushort *&arrSdut, ulonglong &time);
	void MYDFSOnFPGA_Multi(CDirectedGraph *graph, const uint &s, const uint &t, const ushort &k, ushort *&arrSdut, ulonglong &time, MyVec &b_path, MyVec &b_hop, uint &tmp_path_count);
	bool IDXDFSOnFPGA2(const uint &s, const uint &t, const ushort &k, uint *&arrNode, uint *&arrOffset, uint *&arrAdj, uint &node_count, uint &adj_count, ulonglong &time);
	bool IDXDFSOnFPGA_Multi2(const uint &s, const uint &t, const short &k, uint *&arrNode, uint *&arrOffset, uint *&arrAdj, uint &node_count, uint &adj_count, ulonglong &time, MyVec &b_path, MyVec &b_hop, uint &tmp_path_count);
};

#endif
