#include <hls_stream.h>
#include <ap_int.h>

typedef ap_uint<4> uInt4;
typedef ap_uint<24> uInt20;
typedef ap_uint<32> uInt32;
typedef ap_uint<64> uInt64;

#define MAX_K_Kernel (8)
#define PADDING (9)
#define MAX_NODE_SIZE (8 * 1024)
#define MAX_EDGE_SIZE (32 * 1024)
#define MAX_IN_SIZE (4 * 1024)
#define MAX_OUT_SIZE (4 * 1024)
#define MAX_BUF_SIZE (8 * 1024)//bram
#define DDR_BATCH_SIZE (4 * 1024)
#define MAX_DDR_PATH_SIZE (1024 * 1024 * 32)

const unsigned int c_max_depth = MAX_OUT_SIZE;

//
uInt20 buffer_pos_list[MAX_NODE_SIZE + 1];      //
uInt20 buffer_adj_list[MAX_EDGE_SIZE];          //
uInt4 buffer_sdt_list[MAX_NODE_SIZE];           //
uInt20 buffer_path[MAX_BUF_SIZE][MAX_K_Kernel]; // path
uInt4 buffer_path_layer[MAX_BUF_SIZE];          // layer
uInt20 batched_path[MAX_IN_SIZE][MAX_K_Kernel]; // path
uInt4 batched_path_layer[MAX_IN_SIZE];          // layer
uInt20 batched_path_node[MAX_IN_SIZE];          // node
uInt20 batched_nbr_idx[MAX_OUT_SIZE];           // idx
uInt20 batched_nbr[MAX_OUT_SIZE];               // nbr
bool target_valid[MAX_OUT_SIZE];                //
bool visited_valid[MAX_OUT_SIZE];               //

uInt32 dram_path_size = 0;
uInt32 bram_path_size = 0;
uInt32 check_path_size = 0;
uInt32 check_node_size = 0;
uInt32 target_node = 0;
uInt4 buffer_k = 8;
uInt64 ans_paths_num = 0;
uInt64 bufferDebug[50];

uInt32 iii = 4;

static void feed_stream_data(unsigned int *pos_list,
                             unsigned int *adj_list,
                             unsigned short int *sdt_list,
                             hls::stream<uInt20> &nbr_chkT_stream,
                             hls::stream<uInt4> &layer_chkT_stream,
                             hls::stream<uInt20> &nbr_chkV_stream,
                             hls::stream<uInt20> &idx_chkV_stream,
                             const uInt4 kk, const uInt32 iSize)
{
    uInt32 node_count = 0;
    uInt32 temp_psize = 0;
    //bool bg, bbatch = true;
bool bbatch = true;
feed_stream_data:
    for (uInt32 i = 0; i < iSize && bbatch; i++)
    {
#pragma HLS loop_flatten off
        uInt4 layer = batched_path_layer[i];
        uInt32 node = batched_path_node[i];
        uInt32 start_idx = node >= MAX_NODE_SIZE ? (uInt32)pos_list[node] : (uInt32)buffer_pos_list[node];
        uInt32 end_idx = node >= MAX_NODE_SIZE ? (uInt32)pos_list[node + 1] : (uInt32)buffer_pos_list[node + 1];
        if (node_count + end_idx - start_idx < MAX_OUT_SIZE)
        {
        	// 固定边界，去掉 bg 带来的循环依赖
        	uInt32 deg   = end_idx - start_idx;
        	uInt32 room  = (node_count < (uInt32)MAX_OUT_SIZE) ? (uInt32)(MAX_OUT_SIZE - node_count) : (uInt32)0;
        	uInt32 take  = (deg < room) ? deg : room;
        	uInt32 limit = start_idx + take;
        	uInt32 add_cnt = 0;

        	for (uInt32 j = start_idx; j < limit; ++j) {
        	#pragma HLS PIPELINE II=1
        	#pragma HLS DEPENDENCE variable=batched_nbr inter false
        	#pragma HLS DEPENDENCE variable=batched_nbr_idx inter false

        	  uInt32 nbr = (j >= (uInt32)MAX_EDGE_SIZE) ? (uInt32)adj_list[j] : (uInt32)buffer_adj_list[j];
        	  uInt4 nbr_sdt = (nbr >= (uInt32)MAX_NODE_SIZE) ? (uInt4)sdt_list[nbr] : (uInt4)buffer_sdt_list[nbr];
        	  bool pass = ((uInt4)(layer + nbr_sdt) < kk);

        	  if (pass) {
        	    // 使用 node_count + add_cnt，避免循环中修改 node_count
        	    uInt32 idx = node_count + add_cnt;

        	    nbr_chkT_stream.write(nbr);
        	    layer_chkT_stream.write(layer);
        	    nbr_chkV_stream.write(nbr);
        	    idx_chkV_stream.write(i);

        	    batched_nbr_idx[idx] = i;
        	    batched_nbr[idx] = nbr;
        	    add_cnt++;
        	    //++temp_psize;
        	  }
        	}
        	++temp_psize;
        	node_count += add_cnt; // 循环结束后统一更新 node_count
        }else
        	bbatch = false;
        }
    				nbr_chkT_stream.write(0);
            	    nbr_chkV_stream.write(0);
            	    //layer_chkT_stream.write(0);
            	    //idx_chkV_stream.write(0);
            	    check_node_size = node_count;
            	    check_path_size = temp_psize;
            	    bram_path_size -= temp_psize;
}

static void check_target(hls::stream<uInt20> &nbr_chkT_stream,
                         hls::stream<uInt4> &layer_chkT_stream,
                         const uInt4 kk, const uInt32 target)
{
    uInt32 i = 0;
check_target:
    while (true)
    {
#pragma HLS PIPELINE II = 1
        uInt20 nbr = nbr_chkT_stream.read();
        if (nbr == 0)
            break;
        uInt4 layer = layer_chkT_stream.read();
        target_valid[i] = (nbr == target) || (layer == (kk - 2));
        i++;
    }
}

static void check_visited(hls::stream<uInt20> &nbr_chkV_stream,
                          hls::stream<uInt20> &idx_chkV_stream)
{
    uInt32 i = 0;
check_visited:
    while (true)
    {
#pragma HLS PIPELINE II = 1
        uInt20 nbr = nbr_chkV_stream.read();
        if (nbr == 0)
            break;
        uInt32 input_idx = idx_chkV_stream.read();
        bool found = false;
    check_visited_inner:
        for (uInt32 j = 0; j < MAX_K_Kernel; j++)
        {
#pragma HLS UNROLL
            found |= (batched_path[input_idx][j] == nbr);
        }
        visited_valid[i] = !found;
        i++;
    }
}

extern "C"
{
    void run(unsigned int *pos_list,
             unsigned int *adj_list,
             unsigned short int *sdt_list,
             const uInt32 iSize)
    {
        uInt4 kk = buffer_k;
        uInt20 tt = target_node;
        hls::stream<uInt20> nbr_chkT_stream;  // 20KB
        hls::stream<uInt4> layer_chkT_stream; // 4KB
        hls::stream<uInt20> nbr_chkV_stream;  // 20KB
        hls::stream<uInt20> idx_chkV_stream;  // 32KB
#pragma HLS STREAM variable = nbr_chkT_stream depth = c_max_depth
#pragma HLS STREAM variable = layer_chkT_stream depth = c_max_depth
#pragma HLS STREAM variable = nbr_chkV_stream depth = c_max_depth
#pragma HLS STREAM variable = idx_chkV_stream depth = c_max_depth
#pragma HLS dataflow
        feed_stream_data(pos_list, adj_list, sdt_list, nbr_chkT_stream, layer_chkT_stream, nbr_chkV_stream, idx_chkV_stream, kk, iSize);
        check_target(nbr_chkT_stream, layer_chkT_stream, kk, tt);
        check_visited(nbr_chkV_stream, idx_chkV_stream);
    }

    bool next_task(unsigned int *ddr_paths)
        {
            uInt32 temp_psize = bram_path_size;

        // [优化] 移除了这里的 #pragma HLS PIPELINE II = 1
        // 因为内部有不定长的 flush 循环，外层流水线无法实现，移除后消除警告
        merge_last_task:
            for (uInt32 i = check_node_size; i > 0; i--)
            {
    // #pragma HLS PIPELINE II = 1
    #pragma HLS loop_flatten off
                if (visited_valid[i - 1])
                {
                    if (target_valid[i - 1])
                        ++ans_paths_num;
                    else
                    {
                        if (temp_psize == MAX_BUF_SIZE)
                        {
                            uInt32 temp_psize_ddr = dram_path_size;
                            if (temp_psize_ddr >= MAX_DDR_PATH_SIZE)
                            {
                                bufferDebug[0] = 2;
                                return false;
                            }
                            else
                            {
                                bufferDebug[2]++;
                                uInt32 tr_size = (MAX_DDR_PATH_SIZE - temp_psize_ddr > MAX_BUF_SIZE) ? (uInt32)MAX_BUF_SIZE : (uInt32)(MAX_DDR_PATH_SIZE - temp_psize_ddr);
                                uInt32 start_idx = temp_psize - tr_size;

                            // [优化] 保持这里的 Pipeline，让写入 DDR 的操作流水化
                            flush_to_dram:
                                for (uInt32 j = 0; j < tr_size; j++, temp_psize_ddr++)
                                {
                                    for (uInt32 k = 0; k < PADDING; k++)
                                    {
                                        // 确保 Pipeline 作用于最内层的数据搬运
    #pragma HLS PIPELINE II = 1
                                        if (k == MAX_K_Kernel)
                                            ddr_paths[temp_psize_ddr * PADDING + k] = (uInt32)buffer_path_layer[start_idx + j];
                                        else
                                            ddr_paths[temp_psize_ddr * PADDING + k] = (uInt32)buffer_path[start_idx + j][k];
                                    }
                                }
                                dram_path_size = temp_psize_ddr;
                                temp_psize = start_idx;
                            }
                        }

                        uInt32 path_idx = batched_nbr_idx[i - 1];
                        uInt32 iLayer = batched_path_layer[path_idx] + 1;

                        // 这里的小循环是定长的，可以展开，为了加速处理逻辑
                        for (uInt32 j = 0; j < PADDING; j++)
                        {
    #pragma HLS UNROLL
                            if (j == MAX_K_Kernel)
                                buffer_path_layer[temp_psize] = iLayer;
                            else if (j == iLayer)
                                buffer_path[temp_psize][j] = batched_nbr[i - 1];
                            else
                                buffer_path[temp_psize][j] = batched_path[path_idx][j];
                        }
                        ++temp_psize;
                    }
                }
            }

            bram_path_size = temp_psize;
            check_node_size = 0;
            check_path_size = 0;

            // if buffer is empty, flush to bram
            if (bram_path_size == 0)
            {
                if (dram_path_size == 0)
                {
                    bufferDebug[0] = 0;
                    return false;
                }
                else
                {
                    bufferDebug[1]++;
                    uInt32 temp_psize_ddr = dram_path_size;
                    uInt32 tr_size = temp_psize_ddr > DDR_BATCH_SIZE ? (uInt32)DDR_BATCH_SIZE : temp_psize_ddr;
                    temp_psize_ddr -= tr_size;

                // [优化] 保持这里的 Pipeline，加速从 DDR 读取
                flush_to_bram:
                    for (uInt32 i = 0; i < tr_size; i++, temp_psize_ddr++)
                    {
                        for (uInt32 j = 0; j < PADDING; j++)
                        {
    #pragma HLS PIPELINE II = 1
                            if (j == MAX_K_Kernel)
                                buffer_path_layer[i] = (uInt4)ddr_paths[temp_psize_ddr * PADDING + j];
                            else
                                buffer_path[i][j] = ddr_paths[temp_psize_ddr * PADDING + j];
                        }
                    }
                    bram_path_size = tr_size;
                    dram_path_size -= tr_size;
                }
            }

            check_path_size = bram_path_size > MAX_IN_SIZE ? (uInt32)MAX_IN_SIZE : bram_path_size;
            temp_psize = bram_path_size;

        // [优化] 读取到 batched_paths 的部分通常很快，保持现状即可
        read_to_batched_paths:
            for (uInt32 i = 0; i < check_path_size; i++, temp_psize--)
            {
    #pragma HLS PIPELINE II = 1
                uInt4 ll = buffer_path_layer[temp_psize - 1];
                uInt32 final_node = buffer_path[temp_psize - 1][ll];
            read_to_batched_paths_inner:
                for (uInt32 j = 0; j < PADDING; j++)
                {
    #pragma HLS UNROLL
                    if (j == MAX_K_Kernel)
                        batched_path_layer[i] = ll;
                    else if (j == ll)
                    {
                        batched_path[i][j] = final_node;
                    }
                    else
                        batched_path[i][j] = buffer_path[temp_psize - 1][j];
                }
                batched_path_node[i] = final_node;
            }

            return bram_path_size != 0;
        }

    void MYDFS(unsigned int *_pos_list,
               unsigned int *_adj_list,
               unsigned short int *_sdt_list,
               unsigned int *_path_list,
               unsigned long long int *_count_list,
               unsigned long long int *_debug_list,
               const unsigned int _node_size,
               const unsigned int _edge_size,
               const unsigned int _task_size,
               const unsigned int _targe_node,
               const unsigned short int _k,
               unsigned long long int _max_tpc)
    {
#pragma HLS INTERFACE m_axi port = _pos_list offset = slave bundle = gmem0
#pragma HLS INTERFACE m_axi port = _adj_list offset = slave bundle = gmem1
#pragma HLS INTERFACE m_axi port = _sdt_list offset = slave bundle = gmem2
#pragma HLS INTERFACE m_axi port = _path_list offset = slave bundle = gmem1
#pragma HLS INTERFACE m_axi port = _count_list offset = slave bundle = gmem3
#pragma HLS INTERFACE m_axi port = _debug_list offset = slave bundle = gmem4


// #pragma HLS INTERFACE s_axilite port = _deg_list bundle = control
#pragma HLS INTERFACE s_axilite port = _pos_list bundle = control
#pragma HLS INTERFACE s_axilite port = _adj_list bundle = control
#pragma HLS INTERFACE s_axilite port = _sdt_list bundle = control
#pragma HLS INTERFACE s_axilite port = _path_list bundle = control
#pragma HLS INTERFACE s_axilite port = _count_list bundle = control
#pragma HLS INTERFACE s_axilite port = _debug_list bundle = control
#pragma HLS INTERFACE s_axilite port = _node_size bundle = control
#pragma HLS INTERFACE s_axilite port = _edge_size bundle = control
#pragma HLS INTERFACE s_axilite port = _task_size bundle = control
#pragma HLS INTERFACE s_axilite port = _targe_node bundle = control
#pragma HLS INTERFACE s_axilite port = _k bundle = control
#pragma HLS INTERFACE s_axilite port = _max_tpc bundle = control
#pragma HLS INTERFACE s_axilite port = return bundle = control

#pragma HLS CLOCK_DETECTION enable
         #pragma HLS RESOURCE variable = buffer_path core = RAM_1P_URAM
         #pragma HLS RESOURCE variable = buffer_path_layer core = RAM_1P_URAM

#pragma HLS ARRAY_PARTITION variable = buffer_path dim = 2 complete
#pragma HLS ARRAY_PARTITION variable = batched_path dim = 2 complete

        uInt32 vertex_list_size = _node_size;
        uInt32 edge_list_size = _edge_size;
        target_node = _targe_node;
        buffer_k = _k;
        ans_paths_num = 0;

        iii = 6;
        for (uInt32 i = 0; i < iii; i++)
        {
#pragma HLS PIPELINE II = 1
            bufferDebug[i] = 0;
        }

        check_node_size = 0;
        check_path_size = 0;
        bram_path_size = _task_size;
        dram_path_size = 0;

        uInt32 isize = vertex_list_size > MAX_NODE_SIZE ? (uInt32)MAX_NODE_SIZE : vertex_list_size;
        for (uInt32 i = 0; i < isize + 1; i++)
        {
#pragma HLS PIPELINE II = 1
            buffer_pos_list[i] = _pos_list[i];
        }
        for (uInt32 i = 0; i < isize; i++)
        {
#pragma HLS PIPELINE II = 1
            buffer_sdt_list[i] = _sdt_list[i];
        }
        isize = edge_list_size > MAX_EDGE_SIZE ? (uInt32)MAX_EDGE_SIZE : edge_list_size;
        for (uInt32 i = 0; i < isize; i++)
        {
#pragma HLS PIPELINE II = 1
            buffer_adj_list[i] = _adj_list[i];
        }
        isize = bram_path_size > MAX_BUF_SIZE ? (uInt32)MAX_BUF_SIZE : bram_path_size;
        for (uInt32 i = 0; i < isize; i++)
        {
            for (uInt32 j = 0; j < PADDING; j++)
            {
#pragma HLS PIPELINE II = 1
                if (j == MAX_K_Kernel)
                    buffer_path_layer[i] = _path_list[i * PADDING + j];
                else
                    buffer_path[i][j] = _path_list[i * PADDING + j];
            }
        }

        uInt64 mtpc = _max_tpc;
        while (next_task(_path_list))
        {
#pragma HLS loop_flatten off
            run(_pos_list, _adj_list, _sdt_list, check_path_size);
            ++bufferDebug[3];
            bufferDebug[4] += check_path_size;
            bufferDebug[5] += check_node_size;
            if (bufferDebug[5] >= mtpc)
            {
                bufferDebug[0] = 3;
                break;
            }
            if (check_path_size == 0)
            {
                bufferDebug[0] = 1;
                break;
            }
        }

        _count_list[0] = ans_paths_num;
        for (uInt4 i = 0; i < iii; i++)
        {
#pragma HLS PIPELINE II = 1
            _debug_list[i] = bufferDebug[i];
        }
    }
}
