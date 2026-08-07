#include "CHost.h"
#ifdef USE_FPGA
#define MAX_K_Kernel 8
#define PADDING (16)
// *PEFP_RE
void CHost::PEFPOnFPGA(CDirectedGraph *graph, const uint &s, const uint &t, const ushort &k, ushort *&arrSdut, ulonglong &time)
{
    if (graph == nullptr)
        return;
    if (is_init_)
    {
        time = 0;
        deviceResults_[0] = 0;
        deviceResults_[1] = 0;
        size_t iSize_16 = sizeof(ushort);
        size_t iSize_32 = sizeof(uint);
        size_t iSize_64 = sizeof(ulonglong);
        size_t iMaxNodeSize = 0;
        size_t iMaxEdgeSize = 0;
        size_t iMaxDDRSize = 1024 * 1024 * 32 * 9;
        size_t iResSize = 1;
        size_t iDebugSize = 50;
        size_t iMaxTmpCnt = 360 * 1000 * 1000;

        size_t iNodeSize = graph->iMaxVerId() + 1;
        size_t iEdgeSize = graph->iArcNum();
        if (iEdgeSize == 0 || iNodeSize == 1)
            return;
        if (iNodeSize + 1 > 10 * 1024 || iEdgeSize > 100 * 1024)
        {
            log_info("warning bigdata: %u , %u", iNodeSize, iEdgeSize);
        }

        cl_int err;
        if (!is_buffer_init_)
        {
            iMaxNodeSize = iNodeSize + 1 > iMaxNodeSize ? iNodeSize + 1 : iMaxNodeSize;
            iMaxEdgeSize = iEdgeSize > iMaxEdgeSize ? iEdgeSize : iMaxEdgeSize;
            OCL_CHECK(err, cl::Kernel krnl = cl::Kernel(program_, "task_parallel", &err));
            kernels_.push_back(krnl);
            vSDu_t_.resize(iMaxNodeSize);
            vPos_.resize(iMaxNodeSize + 1);
            vEdg_.resize(iMaxEdgeSize);
            bufferDeviceDDRPaths_.resize(iMaxDDRSize, 0);
            bufferDeviceResultsLong_.resize(iResSize, 0);
            bufferDeviceDebugLong_.resize(iDebugSize, 0);
            is_buffer_init_ = true;
        }
        else
        {
            if ((iNodeSize + 1) > iMaxNodeSize)
            {
                iMaxNodeSize = iNodeSize + 1;
                vSDu_t_.resize(iMaxNodeSize);
                vPos_.resize(iMaxNodeSize + 1);
            }
            if (iEdgeSize > iMaxEdgeSize)
            {
                iMaxEdgeSize = iEdgeSize;
                vEdg_.resize(iMaxEdgeSize);
            }
        }

        events_.clear();
        events_.resize(MAX_EVENTS_NUM);
        uint events_temp = 0, events_size = 0;

        vPos_[0] = 0;
        vSDu_t_[0] = MAX_K;
        for (uint i = 0; i < iNodeSize; i++)
        {
            vPos_[i + 1] = graph->arrOutOffset()[i]; // 点的编号加1
            vSDu_t_[i + 1] = arrSdut[i];
        }
        vPos_[iNodeSize + 1] = graph->arrOutOffset()[iNodeSize];
        for (uint i = 0; i < iEdgeSize; i++)
        {
            vEdg_[i] = graph->arrOutAdj()[i] + 1; // 点的编号加1
        }
        bufferDeviceDDRPaths_[0] = s + 1; // 点的编号加1
        for (uint i = 1; i < 9; i++)
        {
            bufferDeviceDDRPaths_[i] = 0;
        }
        size_t iDDRTempSize = 1;
        bufferDeviceResultsLong_[0] = 0;
        bufferDeviceDebugLong_[0] = 0;
        bufferDeviceDebugLong_[1] = 0;
        bufferDeviceDebugLong_[2] = 0;

        OCL_CHECK(err, cl::Buffer bPos(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, iSize_32 * (iMaxNodeSize + 1), vPos_.data(), &err));
        OCL_CHECK(err, cl::Buffer bEdg(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, iSize_32 * iMaxEdgeSize, vEdg_.data(), &err));
        OCL_CHECK(err, cl::Buffer bSdut(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, iSize_16 * iMaxNodeSize, vSDu_t_.data(), &err));
        OCL_CHECK(err, cl::Buffer bPath(context_, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, iSize_32 * iMaxDDRSize, bufferDeviceDDRPaths_.data(), &err));
        OCL_CHECK(err, cl::Buffer bRes(context_, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, iSize_64 * iResSize, bufferDeviceResultsLong_.data(), &err));
        OCL_CHECK(err, cl::Buffer bDebug(context_, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, iSize_64 * iDebugSize, bufferDeviceDebugLong_.data(), &err));
        uint narg = 0;
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, bPos));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, bEdg));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, bSdut));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, bPath));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, bRes));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, bDebug));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, (uint)(iNodeSize + 1)));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, (uint)iEdgeSize));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, (uint)iDDRTempSize));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, t + 1));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, k));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, (ulonglong)iMaxTmpCnt));
        OCL_CHECK(err, err = q_.enqueueMigrateMemObjects({bPos, bEdg, bSdut, bPath}, 0, NULL, &events_[events_size++]));
        events_temp = events_size;
        OCL_CHECK(err, err = q_.enqueueTask(kernels_[0], NULL, &events_[events_size++]));
        events_[events_size - 1].wait();
        OCL_CHECK(err, err = q_.enqueueMigrateMemObjects({bPath, bRes, bDebug}, CL_MIGRATE_MEM_OBJECT_HOST));
        OCL_CHECK(err, err = q_.finish());
        for (uint eventIndex = events_temp; eventIndex < events_size; eventIndex++)
        {
            time += get_duration_ns(events_[eventIndex]);
        }

        // for (uint i = 0; i < 10; i++) // iDebugSize
        //     log_info(",,debug %u : %lu \n", i, bufferDeviceDebugLong_[i]);

        deviceResults_[0] = bufferDeviceResultsLong_[0];
        deviceResults_[1] = bufferDeviceDebugLong_[0]; // err
        deviceResults_[2] = bufferDeviceDebugLong_[1]; // d2b
        deviceResults_[3] = bufferDeviceDebugLong_[2]; // b2d
        deviceResults_[4] = bufferDeviceDebugLong_[3]; // loop
        deviceResults_[5] = bufferDeviceDebugLong_[4]; // checkpath
        deviceResults_[6] = bufferDeviceDebugLong_[5]; // checknode
    }
}

// *MYDFS_RE
void CHost::MYDFSOnFPGA2(CDirectedGraph *graph, const uint &s, const uint &t, const ushort &k, ushort *&arrSdut, ulonglong &time)
{
    if (graph == nullptr)
        return;
    if (is_init_)
    {
        time = 0;
        deviceResults_[0] = 0;
        deviceResults_[1] = 0;
        size_t iSize_16 = sizeof(ushort);
        size_t iSize_32 = sizeof(uint);
        size_t iSize_64 = sizeof(ulonglong);
        size_t iMaxNodeSize = 0;
        size_t iMaxEdgeSize = 0;
        size_t iMaxDDRSize = 1024 * 1024 * 32 * 9;
        size_t iResSize = 1;
        size_t iDebugSize = 50;
        size_t iMaxTmpCnt = 360 * 1000 * 1000;

        size_t iNodeSize = graph->iMaxVerId() + 1;
        size_t iEdgeSize = graph->iArcNum();
        if (iEdgeSize == 0 || iNodeSize == 1)
            return;
        if (iNodeSize > 10 * 1024 || iEdgeSize > 100 * 1024)
        {
            log_info("warning bigdata: %u , %u", iNodeSize, iEdgeSize);
        }

        cl_int err;
        if (!is_buffer_init_)
        {
            iMaxNodeSize = iNodeSize > iMaxNodeSize ? iNodeSize : iMaxNodeSize;
            iMaxEdgeSize = iEdgeSize > iMaxEdgeSize ? iEdgeSize : iMaxEdgeSize;
            OCL_CHECK(err, cl::Kernel krnl = cl::Kernel(program_, "MYDFS", &err));
            kernels_.push_back(krnl);
            vSDu_t_.resize(iMaxNodeSize);
            vPos_.resize(iMaxNodeSize + 1);
            vEdg_.resize(iMaxEdgeSize);
            bufferDeviceDDRPaths_.resize(iMaxDDRSize, 0);
            bufferDeviceResultsLong_.resize(iResSize, 0);
            bufferDeviceDebugLong_.resize(iDebugSize, 0);
            is_buffer_init_ = true;
        }
        else
        {
            if (iNodeSize > iMaxNodeSize)
            {
                iMaxNodeSize = iNodeSize;
                vSDu_t_.resize(iMaxNodeSize);
                vPos_.resize(iMaxNodeSize + 1);
            }
            if (iEdgeSize > iMaxEdgeSize)
            {
                iMaxEdgeSize = iEdgeSize;
                vEdg_.resize(iMaxEdgeSize);
            }
        }

        events_.clear();
        events_.resize(MAX_EVENTS_NUM);
        uint events_temp = 0, events_size = 0;

        vPos_[0] = 0;
        vSDu_t_[0] = MAX_K;
        for (uint i = 0; i < iNodeSize; i++)
        {
            vPos_[i] = graph->arrOutOffset()[i];
            vSDu_t_[i] = arrSdut[i];
        }
        vPos_[iNodeSize] = graph->arrOutOffset()[iNodeSize];
        for (uint i = 0; i < iEdgeSize; i++)
        {
            vEdg_[i] = graph->arrOutAdj()[i];
        }
        bufferDeviceDDRPaths_[0] = s;
        for (uint i = 1; i < 9; i++)
        {
            bufferDeviceDDRPaths_[i] = 0;
        }
        size_t iDDRTempSize = 1;
        bufferDeviceResultsLong_[0] = 0;
        bufferDeviceDebugLong_[0] = 0;
        bufferDeviceDebugLong_[1] = 0;
        bufferDeviceDebugLong_[2] = 0;
        bufferDeviceDebugLong_[3] = 0;

        OCL_CHECK(err, cl::Buffer bPos(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, iSize_32 * (iMaxNodeSize + 1), vPos_.data(), &err));
        OCL_CHECK(err, cl::Buffer bEdg(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, iSize_32 * iMaxEdgeSize, vEdg_.data(), &err));
        OCL_CHECK(err, cl::Buffer bSdut(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, iSize_16 * iMaxNodeSize, vSDu_t_.data(), &err));
        OCL_CHECK(err, cl::Buffer bPath(context_, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, iSize_32 * iMaxDDRSize, bufferDeviceDDRPaths_.data(), &err));
        OCL_CHECK(err, cl::Buffer bRes(context_, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, iSize_64 * iResSize, bufferDeviceResultsLong_.data(), &err));
        OCL_CHECK(err, cl::Buffer bDebug(context_, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, iSize_64 * iDebugSize, bufferDeviceDebugLong_.data(), &err));
        uint narg = 0;
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, bPos));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, bEdg));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, bSdut));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, bPath));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, bRes));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, bDebug));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, (uint)iNodeSize));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, (uint)iEdgeSize));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, (uint)iDDRTempSize));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, t));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, k));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, (ulonglong)iMaxTmpCnt));
        OCL_CHECK(err, err = q_.enqueueMigrateMemObjects({bPos, bEdg, bSdut, bPath}, 0, NULL, &events_[events_size++]));
        events_temp = events_size;
        OCL_CHECK(err, err = q_.enqueueTask(kernels_[0], NULL, &events_[events_size++]));
        events_[events_size - 1].wait();
        OCL_CHECK(err, err = q_.enqueueMigrateMemObjects({bPath, bRes, bDebug}, CL_MIGRATE_MEM_OBJECT_HOST));
        OCL_CHECK(err, err = q_.finish());
        for (uint eventIndex = events_temp; eventIndex < events_size; eventIndex++)
        {
            time += get_duration_ns(events_[eventIndex]);
        }

        // for (uint i = 0; i < 10; i++) // iDebugSize
        //     log_info(",,debug %u : %lu \n", i, bufferDeviceDebugLong_[i]);

        deviceResults_[0] = bufferDeviceResultsLong_[0];
        deviceResults_[1] = bufferDeviceDebugLong_[0]; // err
        deviceResults_[2] = bufferDeviceDebugLong_[1]; // d2b
        deviceResults_[3] = bufferDeviceDebugLong_[2]; // b2d
        deviceResults_[4] = bufferDeviceDebugLong_[3]; // loop
        deviceResults_[5] = bufferDeviceDebugLong_[4]; // checkpath
        deviceResults_[6] = bufferDeviceDebugLong_[5]; // checknode
    }
}

void CHost::MYDFSOnFPGA3(CDirectedGraph *graph, const uint &s, const uint &t, const ushort &k, ushort *&arrSdut, ulonglong &time)
{
    if (graph == nullptr)
        return;
    if (is_init_)
    {
        time = 0;
        deviceResults_[0] = 0;
        deviceResults_[1] = 0;
        size_t iSize_16 = sizeof(ushort);
        size_t iSize_32 = sizeof(uint);
        size_t iSize_64 = sizeof(ulonglong);
        size_t iMaxNodeSize = 0;
        size_t iMaxEdgeSize = 0;
        size_t iMaxDDRSize = 1024 * 1024 * 32 * 9;
        size_t iResSize = 1;
        size_t iDebugSize = 50;
        size_t iMaxTmpCnt = 360 * 1000 * 1000;

        size_t iNodeSize = graph->iMaxVerId() + 1;
        size_t iEdgeSize = graph->iArcNum();
        if (iEdgeSize == 0 || iNodeSize == 1)
            return;
        if (iNodeSize > 10 * 1024 || iEdgeSize > 100 * 1024)
        {
            log_info("warning bigdata: %u , %u", iNodeSize, iEdgeSize);
        }

        cl_int err;
        if (!is_buffer_init_)
        {
            iMaxNodeSize = iNodeSize > iMaxNodeSize ? iNodeSize : iMaxNodeSize;
            iMaxEdgeSize = iEdgeSize > iMaxEdgeSize ? iEdgeSize : iMaxEdgeSize;
            OCL_CHECK(err, cl::Kernel krnl = cl::Kernel(program_, "MYDFS", &err));
            kernels_.push_back(krnl);
            vSDu_t_.resize(iMaxNodeSize + 1);
            vPos_.resize(iMaxNodeSize + 2);
            vEdg_.resize(iMaxEdgeSize);
            bufferDeviceDDRPaths_.resize(iMaxDDRSize, 0);
            bufferDeviceResultsLong_.resize(iResSize, 0);
            bufferDeviceDebugLong_.resize(iDebugSize, 0);
            is_buffer_init_ = true;
        }
        else
        {
            if (iNodeSize > iMaxNodeSize)
            {
                iMaxNodeSize = iNodeSize;
                vSDu_t_.resize(iMaxNodeSize + 1);
                vPos_.resize(iMaxNodeSize + 2);
            }
            if (iEdgeSize > iMaxEdgeSize)
            {
                iMaxEdgeSize = iEdgeSize;
                vEdg_.resize(iMaxEdgeSize);
            }
        }

        events_.clear();
        events_.resize(MAX_EVENTS_NUM);
        uint events_temp = 0, events_size = 0;

        vPos_[0] = 0;
        vSDu_t_[0] = MAX_K;
        for (uint i = 0; i < iNodeSize; i++)
        {
            vPos_[i + 1] = graph->arrOutOffset()[i];
            vSDu_t_[i + 1] = arrSdut[i];
        }
        vPos_[iNodeSize + 1] = graph->arrOutOffset()[iNodeSize];
        for (uint i = 0; i < iEdgeSize; i++)
        {
            vEdg_[i] = graph->arrOutAdj()[i] + 1;
        }
        bufferDeviceDDRPaths_[0] = s + 1;
        for (uint i = 1; i < 9; i++)
        {
            bufferDeviceDDRPaths_[i] = 0;
        }
        size_t iDDRTempSize = 1;
        bufferDeviceResultsLong_[0] = 0;
        bufferDeviceDebugLong_[0] = 0;
        bufferDeviceDebugLong_[1] = 0;
        bufferDeviceDebugLong_[2] = 0;
        bufferDeviceDebugLong_[3] = 0;

        OCL_CHECK(err, cl::Buffer bPos(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, iSize_32 * (iMaxNodeSize + 2), vPos_.data(), &err));
        OCL_CHECK(err, cl::Buffer bEdg(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, iSize_32 * iMaxEdgeSize, vEdg_.data(), &err));
        OCL_CHECK(err, cl::Buffer bSdut(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, iSize_16 * (iMaxNodeSize + 1), vSDu_t_.data(), &err));
        OCL_CHECK(err, cl::Buffer bPath(context_, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, iSize_32 * iMaxDDRSize, bufferDeviceDDRPaths_.data(), &err));
        OCL_CHECK(err, cl::Buffer bRes(context_, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, iSize_64 * iResSize, bufferDeviceResultsLong_.data(), &err));
        OCL_CHECK(err, cl::Buffer bDebug(context_, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, iSize_64 * iDebugSize, bufferDeviceDebugLong_.data(), &err));
        uint narg = 0;
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, bPos));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, bEdg));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, bSdut));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, bPath));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, bRes));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, bDebug));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, (uint)(iNodeSize + 1)));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, (uint)iEdgeSize));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, (uint)iDDRTempSize));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, t + 1));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, k));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, (ulonglong)iMaxTmpCnt));
        OCL_CHECK(err, err = q_.enqueueMigrateMemObjects({bPos, bEdg, bSdut, bPath}, 0, NULL, &events_[events_size++]));
        events_temp = events_size;
        OCL_CHECK(err, err = q_.enqueueTask(kernels_[0], NULL, &events_[events_size++]));
        events_[events_size - 1].wait();
        OCL_CHECK(err, err = q_.enqueueMigrateMemObjects({bPath, bRes, bDebug}, CL_MIGRATE_MEM_OBJECT_HOST));
        OCL_CHECK(err, err = q_.finish());
        for (uint eventIndex = events_temp; eventIndex < events_size; eventIndex++)
        {
            time += get_duration_ns(events_[eventIndex]);
        }

        // for (uint i = 0; i < 10; i++) // iDebugSize
        //     log_info(",,debug %u : %lu \n", i, bufferDeviceDebugLong_[i]);

        deviceResults_[0] = bufferDeviceResultsLong_[0];
        deviceResults_[1] = bufferDeviceDebugLong_[0]; // err
        deviceResults_[2] = bufferDeviceDebugLong_[1]; // d2b
        deviceResults_[3] = bufferDeviceDebugLong_[2]; // b2d
        deviceResults_[4] = bufferDeviceDebugLong_[3]; // loop
        deviceResults_[5] = bufferDeviceDebugLong_[4]; // checkpath
        deviceResults_[6] = bufferDeviceDebugLong_[5]; // checknode
    }
}
// *MYDFS_RE
void CHost::MYDFSOnFPGA_Multi(CDirectedGraph *graph, const uint &s, const uint &t, const ushort &k, ushort *&arrSdut, ulonglong &time, MyVec &b_path, MyVec &b_hop, uint &tmp_path_count)
{
    if (graph == nullptr)
        return;
    uint core_num = 6;
    if (is_init_)
    {
        time = 0;
        deviceResults_[0] = 0;
        deviceResults_[1] = 0;
        deviceResults_[2] = 0;
        deviceResults_[3] = 0;
        size_t iSize_16 = sizeof(ushort);
        size_t iSize_32 = sizeof(uint);
        size_t iSize_64 = sizeof(ulonglong);
        size_t iMaxNodeSize = 0;
        size_t iMaxEdgeSize = 0;
        size_t iMaxDDRSize = 1024 * 1024 * 32 * 9;
        size_t iResSize = 1;
        size_t iDebugSize = 50;
        size_t iMaxTmpCnt = 1000 * 1000 * 1000;

        size_t iNodeSize = graph->iMaxVerId() + 1;
        size_t iEdgeSize = graph->iArcNum();
        if (iNodeSize == 1 || iEdgeSize == 0)
            return;
//        if (iNodeSize > 10 * 1024 || iEdgeSize > 100 * 1024)
//            log_info("warning overlarge: %u , %u", iNodeSize, iEdgeSize);

        cl_int err;
        if (!is_buffer_init_)
        {
            iMaxNodeSize = iNodeSize > iMaxNodeSize ? iNodeSize : iMaxNodeSize;
            iMaxEdgeSize = iEdgeSize > iMaxEdgeSize ? iEdgeSize : iMaxEdgeSize;
            for (uint c = 0; c < core_num; c++)
            {
                OCL_CHECK(err, cl::Kernel krnl = cl::Kernel(program_, "MYDFS", &err));
                kernels_.push_back(krnl);
            }
            vSDu_t_.resize(iMaxNodeSize * core_num);
            vPos_.resize((iMaxNodeSize + 1) * core_num);
            vEdg_.resize(iMaxEdgeSize * core_num);
            bufferDeviceDDRPaths_.resize(iMaxDDRSize * core_num, 0);
            bufferDeviceResultsLong_.resize(iResSize * core_num, 0);
            bufferDeviceDebugLong_.resize(iDebugSize * core_num, 0);
            is_buffer_init_ = true;
        }
        else
        {
            if (iNodeSize > iMaxNodeSize)
            {
                iMaxNodeSize = iNodeSize;
                vSDu_t_.resize(iMaxNodeSize * core_num);
                vPos_.resize((iMaxNodeSize + 1) * core_num);
            }
            if (iEdgeSize > iMaxEdgeSize)
            {
                iMaxEdgeSize = iEdgeSize;
                vEdg_.resize(iMaxEdgeSize * core_num);
            }
        }

        events_.clear();
        events_.resize(MAX_EVENTS_NUM);
        uint events_size = 0, task_event_num = 0;

        MyVec p_count(core_num, 0);
        for (uint c = 0; c < core_num; c++)
        {
            uint offset = c * iMaxNodeSize;
            for (uint i = 0; i < iNodeSize; i++)
            {
                vSDu_t_[offset + i] = arrSdut[i];
            }
            offset = c * (iMaxNodeSize + 1);
            for (uint i = 0; i < iNodeSize + 1; i++)
            {
                vPos_[offset + i] = graph->arrOutOffset()[i];
            }
            offset = c * iMaxEdgeSize;
            for (uint i = 0; i < iEdgeSize; i++)
            {
                vEdg_[offset + i] = graph->arrOutAdj()[i];
            }

            offset = c * iMaxDDRSize;
            uint ddrindex = 0;
            for (uint i = c; i < tmp_path_count; i += core_num)
            {
                for (auto j = 0; j < MAX_K_Kernel; j++)
                {
                    bufferDeviceDDRPaths_[offset + ddrindex * (MAX_K_Kernel + 1) + j] = b_path[i * MAX_K_Kernel + j];
                }
                bufferDeviceDDRPaths_[offset + ddrindex * (MAX_K_Kernel + 1) + MAX_K_Kernel] = b_hop[i];
                ddrindex++;
            }
            p_count[c] = ddrindex;
        }
        //打印每個CU分配多少path
//                for (uint c = 0; c < core_num; ++c) {
//                    if (p_count[c] > 0) {
//                        log_info("[FPGA_MYDFS_Multi] CU%u: get %u paths for query (s=%u, t=%u)",
//                                 c, p_count[c], s, t);
//                    } else {
//                        log_info("[FPGA_MYDFS_Multi] CU%u: get 0 paths for query (s=%u, t=%u)",
//                                 c, s, t);
//                    }
//                }
        bufferDeviceResultsLong_.resize(iResSize * core_num, 0);
        bufferDeviceDebugLong_.resize(iDebugSize * core_num, 0);

        auto bank_of_cu = [&](uint c){ return (c < 3) ? XCL_MEM_DDR_BANK1 : XCL_MEM_DDR_BANK0; };

        std::vector<cl::Buffer> bPos(core_num), bEdg(core_num), bSdut(core_num),
                                bPaths(core_num), bCount(core_num), bDebug(core_num);

        for (uint c = 0; c < core_num; ++c) {
            cl_int err;
            unsigned int bankFlag = bank_of_cu(c);

            cl_mem_ext_ptr_t pos_ext;   pos_ext.obj   = nullptr;pos_ext.param = 0;pos_ext.flags = bankFlag;

            cl_mem_ext_ptr_t edg_ext;   edg_ext.obj   = nullptr;edg_ext.param = 0;edg_ext.flags = bankFlag;

            cl_mem_ext_ptr_t sdut_ext;  sdut_ext.obj  = nullptr;sdut_ext.param = 0;sdut_ext.flags = bankFlag;

            cl_mem_ext_ptr_t path_ext;  path_ext.obj  = nullptr;path_ext.param = 0;path_ext.flags = bankFlag;

            cl_mem_ext_ptr_t cnt_ext;   cnt_ext.obj   = nullptr;cnt_ext.param = 0;cnt_ext.flags = bankFlag;

            cl_mem_ext_ptr_t dbg_ext;   dbg_ext.obj   = nullptr;dbg_ext.param = 0;dbg_ext.flags = bankFlag;

            OCL_CHECK(err, bPos[c]   = cl::Buffer(context_, CL_MEM_READ_ONLY  | CL_MEM_EXT_PTR_XILINX,
                                                  iSize_32 * (iMaxNodeSize + 1), &pos_ext,  &err));
            OCL_CHECK(err, bEdg[c]   = cl::Buffer(context_, CL_MEM_READ_ONLY  | CL_MEM_EXT_PTR_XILINX,
                                                  iSize_32 * iMaxEdgeSize,     &edg_ext,  &err));
            OCL_CHECK(err, bSdut[c]  = cl::Buffer(context_, CL_MEM_READ_ONLY  | CL_MEM_EXT_PTR_XILINX,
                                                  iSize_16 * iMaxNodeSize,     &sdut_ext, &err));
            OCL_CHECK(err, bPaths[c]  = cl::Buffer(context_, CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX,
                                                  iSize_32 * iMaxDDRSize,      &path_ext, &err));
            OCL_CHECK(err, bCount[c]  = cl::Buffer(context_, CL_MEM_WRITE_ONLY | CL_MEM_EXT_PTR_XILINX,
                                                  iSize_64 * iResSize,         &cnt_ext,  &err));
            OCL_CHECK(err, bDebug[c]  = cl::Buffer(context_, CL_MEM_WRITE_ONLY | CL_MEM_EXT_PTR_XILINX,
                                                  iSize_64 * iDebugSize,       &dbg_ext,  &err));
        }

        for (uint c = 0; c < core_num; c++)
        {
            uint narg = 0;
            OCL_CHECK(err, err = kernels_[c].setArg(narg++, bPos[c]));
            OCL_CHECK(err, err = kernels_[c].setArg(narg++, bEdg[c]));
            OCL_CHECK(err, err = kernels_[c].setArg(narg++, bSdut[c]));
            OCL_CHECK(err, err = kernels_[c].setArg(narg++, bPaths[c]));
            OCL_CHECK(err, err = kernels_[c].setArg(narg++, bCount[c]));
            OCL_CHECK(err, err = kernels_[c].setArg(narg++, bDebug[c]));
            OCL_CHECK(err, err = kernels_[c].setArg(narg++, (uint)(iNodeSize)));
            OCL_CHECK(err, err = kernels_[c].setArg(narg++, (uint)iEdgeSize));
            OCL_CHECK(err, err = kernels_[c].setArg(narg++, (uint)p_count[c]));
            OCL_CHECK(err, err = kernels_[c].setArg(narg++, t));
            OCL_CHECK(err, err = kernels_[c].setArg(narg++, k));
            OCL_CHECK(err, err = kernels_[c].setArg(narg++, (ulonglong)iMaxTmpCnt));

            // 写入数据到设备缓冲区，添加检查避免写入0大小数据
            OCL_CHECK(err, err = qo_.enqueueWriteBuffer(bPos[c], CL_FALSE, 0, iSize_32 * (iNodeSize + 1), vPos_.data() + c * (iMaxNodeSize + 1), NULL, &events_[events_size++]));
            OCL_CHECK(err, err = qo_.enqueueWriteBuffer(bEdg[c], CL_FALSE, 0, iSize_32 * iEdgeSize, vEdg_.data() + c * iMaxEdgeSize, NULL, &events_[events_size++]));
            OCL_CHECK(err, err = qo_.enqueueWriteBuffer(bSdut[c], CL_FALSE, 0, iSize_16 * iNodeSize, vSDu_t_.data() + c * iMaxNodeSize, NULL, &events_[events_size++]));

            // 只有当p_count[c]大于0时才写入路径数据
            if (p_count[c] > 0) {
                OCL_CHECK(err, err = qo_.enqueueWriteBuffer(bPaths[c], CL_FALSE, 0, iSize_32 * p_count[c] * (MAX_K_Kernel + 1), bufferDeviceDDRPaths_.data() + c * iMaxDDRSize, NULL, &events_[events_size++]));
            }
        }
        // for (uint i = task_event_num; i < events_size; i++)
        //     events_[i].wait();
        task_event_num = events_size;
        static cl::Event evKernel[6];
        for (uint c = 0; c < core_num; c++)
            OCL_CHECK(err, err = qo_.enqueueTask(kernels_[c], NULL, &events_[events_size++]));
        for (uint i = task_event_num; i < events_size; i++)
            events_[i].wait();
        for (uint c = 0; c < core_num; c++) {
            // 从设备读取结果数据
        	if (p_count[c] > 0) {
        	        OCL_CHECK(err, err = qo_.enqueueReadBuffer(
        	            bPaths[c], CL_FALSE, 0,
        	            iSize_32 * p_count[c] * (MAX_K_Kernel + 1),
        	            bufferDeviceDDRPaths_.data() + c * iMaxDDRSize,
        	            NULL, &events_[events_size++]));
        	    }

        	    OCL_CHECK(err, err = qo_.enqueueReadBuffer(
        	        bCount[c], CL_FALSE, 0,
        	        iSize_64 * iResSize,
        	        bufferDeviceResultsLong_.data() + c * iResSize,
        	        NULL, &events_[events_size++]));

        	    OCL_CHECK(err, err = qo_.enqueueReadBuffer(
        	        bDebug[c], CL_FALSE, 0,
        	        iSize_64 * iDebugSize,
        	        bufferDeviceDebugLong_.data() + c * iDebugSize,
        	        NULL, &events_[events_size++]));
        	}
        OCL_CHECK(err, err = qo_.finish());

//
//                // 打印每个 CU 的负载和执行时间，用于分析负载均衡
//                log_info("================= CU Load Balance Report =================");
//                ulonglong max_cu_time = 0;
//                ulonglong total_tasks = 0;
//
//                for (uint c = 0; c < core_num; c++)
//                {
//                    // 1. 获取该 CU 的内核执行事件索引
//                    // 这里的逻辑基于：代码中 enqueueTask 是紧接着 task_event_num 顺序 push 到 events_ 里的
//                    uint kernel_evt_idx = task_event_num + c;
//
//                    // 2. 获取时间 (纳秒) - 假设 get_duration_ns 是你已有的辅助函数
//                    auto cu_time_ns = get_duration_ns(events_[kernel_evt_idx]);
//                    double cu_time_ms = cu_time_ns / 1000000.0;
//
//                    // 3. 记录最大时间 (这就是决定性的瓶颈时间)
//                    if (cu_time_ns > max_cu_time) max_cu_time = cu_time_ns;
//                    total_tasks += p_count[c];
//
//                    // 4. 打印格式化信息
//                    // p_count[c]: 该CU分配到的任务数
//                    // cu_time_ms: 该CU实际运行时间
//                    log_info("CU[%u]: Load = %8u paths | Time = %10.3f ms", c, p_count[c], cu_time_ms);
//                }
//
//                // 计算负载不平衡度 (最大时间 / 平均时间 - 1)
//                // 或者是简单的 (最大负载 - 最小负载)
//                log_info("Total Tasks: %llu, Bottleneck Time: %.3f ms", total_tasks, max_cu_time / 1000000.0);
//                log_info("========================================================");


        for (uint eventIndex = task_event_num; eventIndex < events_size; eventIndex++)
        {
            auto tt = get_duration_ns(events_[eventIndex]);
            time = time > tt ? time : tt;
        }

        ulonglong cerror = 0;
        for (uint c = 0; c < core_num; c++)
        {
            uint offset = c * iResSize;
            deviceResults_[0] += bufferDeviceResultsLong_[offset + 0];

            offset = c * iDebugSize;
            cerror = bufferDeviceDebugLong_[offset + 0];
            if (cerror != 0)
                log_info("kernel[%u]err: %lu", c, deviceResults_[1]);
            deviceResults_[1] += cerror;
            deviceResults_[2] += bufferDeviceDebugLong_[offset + 1];
            deviceResults_[3] += bufferDeviceDebugLong_[offset + 2];
            deviceResults_[4] += bufferDeviceDebugLong_[offset + 3];
            deviceResults_[5] += bufferDeviceDebugLong_[offset + 4];
            deviceResults_[6] += bufferDeviceDebugLong_[offset + 5];
        }
    }
}


// *IDX_DFS_RE
bool CHost::IDXDFSOnFPGA2(const uint &s, const uint &t, const ushort &k, uint *&arrNode, uint *&arrOffset, uint *&arrAdj, uint &node_count, uint &adj_count, ulonglong &time)
{
    if (is_init_)
    {
        time = 0;
        deviceResults_[0] = 0;
        deviceResults_[1] = 0;
        size_t iSize_16 = sizeof(ushort);
        size_t iSize_32 = sizeof(uint);
        size_t iSize_64 = sizeof(ulonglong);
        size_t iMaxNodeSize = 0;
        size_t iMaxEdgeSize = 0;
        size_t iMaxDDRSize = 1024 * 1024 * 16 * 9;
        size_t iResSize = 1;
        size_t iDebugSize = 50;
        cl_int err;

        if (node_count == 0 || adj_count == 0)
            return true;
        if (node_count + 1 > 64 * 1024 || adj_count > 640 * 1024)
            log_info("warning bigdata: %u , %u", node_count + 1, adj_count);

        if (!is_buffer_init_)
        {
            iMaxNodeSize = node_count + 1 > iMaxNodeSize ? node_count + 1 : iMaxNodeSize;
            iMaxEdgeSize = adj_count > iMaxEdgeSize ? adj_count : iMaxEdgeSize;
            OCL_CHECK(err, cl::Kernel krnl = cl::Kernel(program_, "IDX_DFS_FPGA", &err));
            kernels_.push_back(krnl);
            vVer_.resize(iMaxNodeSize, 0);
            vPos_.resize(iMaxNodeSize * k, 0);
            vEdg_.resize(iMaxEdgeSize, 0);
            bufferDeviceDDRPaths_.resize(iMaxDDRSize, 0);
            bufferDeviceResultsLong_.resize(iResSize, 0);
            bufferDeviceDebugLong_.resize(iDebugSize, 0);
            is_buffer_init_ = true;
        }
        else
        {
            if ((node_count + 1) > iMaxNodeSize)
            {
                iMaxNodeSize = node_count + 1;
                vVer_.resize(iMaxNodeSize);
                vPos_.resize(iMaxNodeSize * k);
            }
            if (adj_count > iMaxEdgeSize)
            {
                iMaxEdgeSize = adj_count;
                vEdg_.resize(iMaxEdgeSize);
            }
        }

        events_.clear();
        events_.resize(MAX_EVENTS_NUM);
        uint events_size = 0, events_temp = 0;

        for (uint i = 0; i < node_count + 1; i++)
        {
            vVer_[i] = arrNode[i];
            for (auto j = 0; j < k; j++)
                vPos_[i * k + j] = arrOffset[i * k + j];
        }
        for (uint i = 0; i < adj_count; i++)
        {
            vEdg_[i] = arrAdj[i];
        }
        bufferDeviceDDRPaths_[0] = s;
        for (uint i = 1; i < 9; i++)
        {
            bufferDeviceDDRPaths_[i] = 0;
        }
        bufferDeviceResultsLong_[0] = 0;
        bufferDeviceDebugLong_[0] = 0; // err
        bufferDeviceDebugLong_[1] = 0; //
        bufferDeviceDebugLong_[2] = 0; //

        OCL_CHECK(err, cl::Buffer bNode = cl::Buffer(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, iSize_32 * iMaxNodeSize, vVer_.data(), &err));
        OCL_CHECK(err, cl::Buffer bOffset = cl::Buffer(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, iSize_32 * iMaxNodeSize * k, vPos_.data(), &err));
        OCL_CHECK(err, cl::Buffer bAdj = cl::Buffer(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, iSize_32 * iMaxEdgeSize, vEdg_.data(), &err));
        OCL_CHECK(err, cl::Buffer bPaths = cl::Buffer(context_, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, iSize_32 * iMaxDDRSize, bufferDeviceDDRPaths_.data(), &err));
        OCL_CHECK(err, cl::Buffer bCount = cl::Buffer(context_, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, iSize_64 * iResSize, bufferDeviceResultsLong_.data(), &err));
        OCL_CHECK(err, cl::Buffer bDebug = cl::Buffer(context_, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, iSize_64 * iDebugSize, bufferDeviceDebugLong_.data(), &err));
        uint narg = 0;
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, bNode));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, bOffset));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, bAdj));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, bPaths));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, bCount));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, bDebug));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, t));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, k));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, (uint)(node_count + 1)));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, (uint)adj_count));
        OCL_CHECK(err, err = kernels_[0].setArg(narg++, (uint)1));
        OCL_CHECK(err, err = q_.enqueueMigrateMemObjects({bNode, bOffset, bAdj, bPaths}, 0, NULL, &events_[events_size++]));
        events_temp = events_size;
        OCL_CHECK(err, err = q_.enqueueTask(kernels_[0], NULL, &events_[events_size++]));
        events_[events_size - 1].wait();
        OCL_CHECK(err, err = q_.enqueueMigrateMemObjects({bPaths, bCount, bDebug}, CL_MIGRATE_MEM_OBJECT_HOST));
        OCL_CHECK(err, err = q_.finish());

        for (uint eventIndex = events_temp; eventIndex < events_size; eventIndex++)
        {
            time += get_duration_ns(events_[eventIndex]);
        }

        deviceResults_[0] = bufferDeviceResultsLong_[0];
        deviceResults_[1] = bufferDeviceDebugLong_[0];
        deviceResults_[2] = bufferDeviceDebugLong_[1];
        deviceResults_[3] = bufferDeviceDebugLong_[2];

        bool large = deviceResults_[1] != 0;
        if (large)
            log_info("err: %d", deviceResults_[1]);

        // for (uint i = 0; i < 10; i++)
        //     log_info(",,,debug %u : %lu \n", i, bufferDeviceDebugLong_[i]);

        return !large;
    }
    return true;
}

// *IDX_DFS_RE
bool CHost::IDXDFSOnFPGA_Multi2(const uint &s, const uint &t, const short &k,
                                uint *&arrNode, uint *&arrOffset, uint *&arrAdj,
                                uint &node_count, uint &adj_count, ulonglong &time,
                                MyVec &b_path, MyVec &b_hop, uint &tmp_path_count)
{
    uint core_num = 4;
    if (is_init_)
    {
        time = 0;
        deviceResults_[0] = 0;
        deviceResults_[1] = 0;
        deviceResults_[2] = 0;
        deviceResults_[3] = 0;
        size_t iSize_16 = sizeof(ushort);
        size_t iSize_32 = sizeof(uint);
        size_t iSize_64 = sizeof(ulonglong);
        size_t iMaxNodeSize = 16 * 256; // !
        size_t iMaxEdgeSize = 160 * 256;
        size_t iMaxDDRSize = 1024 * 1024 * 16;
        size_t iResSize = 1;
        size_t iDebugSize = 50;
        cl_int err;

        if (node_count == 0 || adj_count == 0)
            return true;
        if (node_count + 1 > iMaxNodeSize || adj_count > iMaxEdgeSize)
            log_info("warning bigdata: %u , %u", node_count + 1, adj_count);

        if (!is_buffer_init_)
        {
            iMaxNodeSize = node_count + 1 > iMaxNodeSize ? node_count + 1 : iMaxNodeSize;
            iMaxEdgeSize = adj_count > iMaxEdgeSize ? adj_count : iMaxEdgeSize;
            for (uint c = 0; c < core_num; c++)
            {
                OCL_CHECK(err, cl::Kernel krnl = cl::Kernel(program_, "IDX_DFS_FPGA", &err));
                kernels_.push_back(krnl);
            }
            vVer_.resize(iMaxNodeSize * core_num, 0);
            vPos_.resize(iMaxNodeSize * k * core_num, 0);
            vEdg_.resize(iMaxEdgeSize * core_num, 0);
            bufferDeviceDDRPaths_.resize(iMaxDDRSize * (MAX_K_Kernel + 1) * core_num, 0);
            bufferDeviceResultsLong_.resize(iResSize * core_num, 0);
            bufferDeviceDebugLong_.resize(iDebugSize * core_num, 0);
            is_buffer_init_ = true;
        }
        else
        {
            if ((node_count + 1) > iMaxNodeSize)
            {
                iMaxNodeSize = node_count + 1;
                vVer_.resize(iMaxNodeSize);
                vPos_.resize(iMaxNodeSize * k);
            }
            if (adj_count > iMaxEdgeSize)
            {
                iMaxEdgeSize = adj_count;
                vEdg_.resize(iMaxEdgeSize);
            }
        }

        events_.clear();
        events_.resize(MAX_EVENTS_NUM);
        uint events_size = 0, task_event_num = 0;

        MyVec p_count(core_num, 0);
        for (uint c = 0; c < core_num; c++)
        {
            uint offset = c * iMaxNodeSize;
            for (uint i = 0; i < node_count + 1; i++)
            {
                vVer_[offset + i] = arrNode[i];
                for (auto j = 0; j < k; j++)
                    vPos_[offset * k + i * k + j] = arrOffset[i * k + j];
            }

            offset = c * iMaxEdgeSize;
            for (uint i = 0; i < adj_count; i++)
            {
                vEdg_[offset + i] = arrAdj[i];
            }

            offset = c * iMaxDDRSize * (MAX_K_Kernel + 1);
            uint ddrindex = 0;
            for (uint i = c; i < tmp_path_count; i += core_num)
            {
                for (auto j = 0; j < MAX_K_Kernel; j++)
                {
                    bufferDeviceDDRPaths_[offset + ddrindex * (MAX_K_Kernel + 1) + j] = b_path[i * MAX_K_Kernel + j];
                }
                bufferDeviceDDRPaths_[offset + ddrindex * (MAX_K_Kernel + 1) + MAX_K_Kernel] = b_hop[i];
                ddrindex++;
            }
            p_count[c] = ddrindex;
        }
        bufferDeviceResultsLong_.resize(iResSize * core_num, 0);
        bufferDeviceDebugLong_.resize(iDebugSize * core_num, 0);

        std::vector<cl::Buffer> bNode(core_num);
        std::vector<cl::Buffer> bOffset(core_num);
        std::vector<cl::Buffer> bAdj(core_num);
        std::vector<cl::Buffer> bPaths(core_num);
        std::vector<cl::Buffer> bCount(core_num);
        std::vector<cl::Buffer> bDebug(core_num);
        task_event_num = events_size;
        for (uint c = 0; c < core_num; c++)
        {
            OCL_CHECK(err, bNode[c] = cl::Buffer(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, iSize_32 * iMaxNodeSize, vVer_.data() + c * iMaxNodeSize, &err));
            OCL_CHECK(err, bOffset[c] = cl::Buffer(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, iSize_32 * iMaxNodeSize * k, vPos_.data() + c * iMaxNodeSize * k, &err));
            OCL_CHECK(err, bAdj[c] = cl::Buffer(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, iSize_32 * iMaxEdgeSize, vEdg_.data() + c * iMaxEdgeSize, &err));
            OCL_CHECK(err, bPaths[c] = cl::Buffer(context_, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, iSize_32 * iMaxDDRSize * (MAX_K_Kernel + 1), bufferDeviceDDRPaths_.data() + c * iMaxDDRSize * (MAX_K_Kernel + 1), &err));
            OCL_CHECK(err, bCount[c] = cl::Buffer(context_, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, iSize_64 * iResSize, bufferDeviceResultsLong_.data() + c * iResSize, &err));
            OCL_CHECK(err, bDebug[c] = cl::Buffer(context_, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, iSize_64 * iDebugSize, bufferDeviceDebugLong_.data() + c * iDebugSize, &err));
        }
        for (uint c = 0; c < core_num; c++)
        {
            uint narg = 0;
            OCL_CHECK(err, err = kernels_[c].setArg(narg++, bNode[c]));
            OCL_CHECK(err, err = kernels_[c].setArg(narg++, bOffset[c]));
            OCL_CHECK(err, err = kernels_[c].setArg(narg++, bAdj[c]));
            OCL_CHECK(err, err = kernels_[c].setArg(narg++, bPaths[c]));
            OCL_CHECK(err, err = kernels_[c].setArg(narg++, bCount[c]));
            OCL_CHECK(err, err = kernels_[c].setArg(narg++, bDebug[c]));
            OCL_CHECK(err, err = kernels_[c].setArg(narg++, t));
            OCL_CHECK(err, err = kernels_[c].setArg(narg++, k));
            OCL_CHECK(err, err = kernels_[c].setArg(narg++, (uint)(node_count + 1)));
            OCL_CHECK(err, err = kernels_[c].setArg(narg++, (uint)adj_count));
            OCL_CHECK(err, err = kernels_[c].setArg(narg++, (uint)p_count[c]));
            OCL_CHECK(err, err = qo_.enqueueMigrateMemObjects({bNode[c], bOffset[c], bAdj[c], bPaths[c]}, 0, NULL, &events_[events_size++]));
        }

        for (uint i = task_event_num; i < events_size; i++)
            events_[i].wait();

        task_event_num = events_size;
        for (uint c = 0; c < core_num; c++)
        {
            OCL_CHECK(err, err = qo_.enqueueTask(kernels_[c], NULL, &events_[events_size++]));
        }
        for (uint i = task_event_num; i < events_size; i++)
            events_[i].wait();

        for (uint c = 0; c < core_num; c++)
            OCL_CHECK(err, err = qo_.enqueueMigrateMemObjects({bPaths[c], bCount[c], bDebug[c]}, CL_MIGRATE_MEM_OBJECT_HOST));

        OCL_CHECK(err, err = qo_.finish());

        // cl_ulong start[MAX_EVENTS_NUM];
        // cl_ulong end[MAX_EVENTS_NUM];
        // cl_ulong start_time = 0xFFFFFFFFFFFFFFFUL, end_time = 0;
        // for (uint i = task_event_num; i < events_size; i++)
        // {
        //     start[i] = events_[i].getProfilingInfo<CL_PROFILING_COMMAND_START>();
        //     end[i] = events_[i].getProfilingInfo<CL_PROFILING_COMMAND_END>();
        //     start_time = std::min(start_time, start[i]);
        //     end_time = std::max(end_time, end[i]);
        // }
        // time = end_time - start_time;

        for (uint eventIndex = task_event_num; eventIndex < events_size; eventIndex++)
        {
            auto tt = get_duration_ns(events_[eventIndex]);
            time = time > tt ? time : tt;
        }

        // for (uint c = 0; c < core_num; c++)
        //     for (uint i = 0; i < vsDeviceDebugLong_[c].size(); i++)
        //         log_info("kernel[%u] debug %u : %lu \n", c, i, vsDeviceDebugLong_[c][i]);

        bool cerror = false;
        for (uint c = 0; c < core_num; c++)
        {
            uint offset = c * iResSize;
            deviceResults_[0] += bufferDeviceResultsLong_[offset + 0];
            offset = c * iDebugSize;
            deviceResults_[1] = bufferDeviceDebugLong_[offset + 0];
            cerror |= deviceResults_[1] != 0;
            if (cerror)
                log_info("kernel[%u]err: %d", c, deviceResults_[1]);
            deviceResults_[2] += bufferDeviceDebugLong_[offset + 1];
            deviceResults_[3] += bufferDeviceDebugLong_[offset + 2];
        }

        return !cerror;
    }
    return true;
}

#endif
