#pragma once
#ifndef CKERNELTEST_H
#define CKERNELTEST_H

#include "util/common.h"

#define MAX_K_Kernel (8)
#define PADDING (9)
#define MAX_NODE_SIZE (1)
#define MAX_EDGE_SIZE (1)
#define MAX_IN_SIZE (16 * 1024)
#define MAX_OUT_SIZE (16 * 1024)
#define MAX_BUF_SIZE (16 * 1024)
#define MAX_BUF_SIZE_UR (16 * 1024)
#define DDR_BATCH_SIZE (16 * 1024)
#define MAX_DDR_PATH_SIZE (1024 * 64)

class CKernelTest
{
public:
	explicit CKernelTest() {}
	~CKernelTest() {}

private:
	uint buffer_pos_list[MAX_NODE_SIZE + 1];			//
	uint buffer_adj_list[MAX_EDGE_SIZE];				//
	ushort buffer_sdt_list[MAX_NODE_SIZE];				//
	uint buffer_path_ur[MAX_BUF_SIZE_UR][MAX_K_Kernel]; // path
	ushort buffer_path_layer_ur[MAX_BUF_SIZE_UR];		// layer
	uint buffer_path[MAX_BUF_SIZE][MAX_K_Kernel];		// path
	ushort buffer_path_layer[MAX_BUF_SIZE];				// layer
	uint batched_path[MAX_IN_SIZE][MAX_K_Kernel];		// path
	ushort batched_path_layer[MAX_IN_SIZE];				// layer
	uint batched_path_node[MAX_IN_SIZE];				// node
	uint batched_nbr_idx[MAX_OUT_SIZE];					// idx
	uint batched_nbr[MAX_OUT_SIZE];						// nbr
	bool target_valid[MAX_OUT_SIZE];					//
	bool visited_valid[MAX_OUT_SIZE];					//

	uint dram_path_size = 0;
	uint bram_path_size = 0;
	uint uram_path_size = 0;
	uint check_path_size = 0;
	uint check_node_size = 0;
	uint target_node = 0;
	ushort buffer_k = 8;
	ulonglong ans_paths_num = 0;
	ulonglong bufferDebug[50];

	uint iii = 4;

	void feed_stream_data(std::vector<uint> &pos_list,
						  std::vector<uint> &adj_list,
						  std::vector<ushort> &sdt_list,
						  std::queue<uint> &nbr_chkT_stream,
						  std::queue<ushort> &layer_chkT_stream,
						  std::queue<uint> &nbr_chkV_stream,
						  std::queue<uint> &idx_chkV_stream,
						  const ushort kk, const uint iSize)
	{
		uint node_count = 0;
		uint temp_psize = 0;
		bool bg, bbatch = true;
		for (uint i = 0; i < iSize && bbatch; i++)
		{
			ushort layer = batched_path_layer[i];
			uint node = batched_path_node[i];
			uint start_idx = node >= MAX_NODE_SIZE ? pos_list[node] : buffer_pos_list[node];
			uint end_idx = node >= MAX_NODE_SIZE ? pos_list[node + 1] : buffer_pos_list[node + 1];
			if (node_count + end_idx - start_idx < MAX_OUT_SIZE)
			{
				bg = true;
				for (uint j = start_idx; j < end_idx && bg; j++)
				{
					uint nbr = j >= MAX_EDGE_SIZE ? adj_list[j] : buffer_adj_list[j];
					ushort nbr_sdt = nbr >= MAX_NODE_SIZE ? sdt_list[nbr] : buffer_sdt_list[nbr];
					if (layer + nbr_sdt < kk)
					{
						nbr_chkT_stream.push(nbr);
						layer_chkT_stream.push(layer);
						nbr_chkV_stream.push(nbr);
						idx_chkV_stream.push(i);
						batched_nbr_idx[node_count] = i;
						batched_nbr[node_count] = nbr;
						node_count++;
					}
					else
						bg = false;
				}
				++temp_psize;
			}
			else
				bbatch = false;
		}
		nbr_chkT_stream.push(0);
		nbr_chkV_stream.push(0);
		check_node_size = node_count;
		check_path_size = temp_psize;
		bram_path_size -= temp_psize;
	}

	void check_target(std::queue<uint> &nbr_chkT_stream,
					  std::queue<ushort> &layer_chkT_stream,
					  const ushort kk, const uint target)
	{
		uint i = 0;
		while (true)
		{
			uint nbr = nbr_chkT_stream.front();
			nbr_chkT_stream.pop();
			if (nbr == 0)
				break;
			ushort layer = layer_chkT_stream.front();
			layer_chkT_stream.pop();
			target_valid[i] = (nbr == target) || (layer == (kk - 2));
			i++;
		}
	}

	void check_visited(std::queue<uint> &nbr_chkV_stream,
					   std::queue<uint> &idx_chkV_stream)
	{
		uint i = 0;
		while (true)
		{
			uint nbr = nbr_chkV_stream.front();
			nbr_chkV_stream.pop();
			if (nbr == 0)
				break;
			uint input_idx = idx_chkV_stream.front();
			idx_chkV_stream.pop();
			bool found = false;
			for (uint j = 0; j < MAX_K_Kernel; j++)
			{
				found |= (batched_path[input_idx][j] == nbr);
			}
			visited_valid[i] = !found;
			i++;
		}
	}

	void run(std::vector<uint> &pos_list,
			 std::vector<uint> &adj_list,
			 std::vector<ushort> &sdt_list,
			 const uint iSize)
	{
		ushort kk = buffer_k;
		uint tt = target_node;
		std::queue<uint> nbr_chkT_stream;	  // 20KB
		std::queue<ushort> layer_chkT_stream; // 4KB
		std::queue<uint> nbr_chkV_stream;	  // 20KB
		std::queue<uint> idx_chkV_stream;	  // 32KB
		feed_stream_data(pos_list, adj_list, sdt_list, nbr_chkT_stream, layer_chkT_stream, nbr_chkV_stream, idx_chkV_stream, kk, iSize);
		check_target(nbr_chkT_stream, layer_chkT_stream, kk, tt);
		check_visited(nbr_chkV_stream, idx_chkV_stream);
	}

	bool next_task(std::vector<uint> &ddr_paths)
	{
		uint temp_psize = bram_path_size;
		for (uint i = check_node_size; i > 0; i--)
		{
			if (visited_valid[i - 1])
			{
				if (target_valid[i - 1])
					++ans_paths_num;
				else
				{
					if (temp_psize == MAX_BUF_SIZE)
					{
						uint temp_psize_ur = uram_path_size;
						if (temp_psize_ur >= MAX_BUF_SIZE_UR)
						{
							uint temp_psize_ddr = dram_path_size;
							if (temp_psize_ddr >= MAX_DDR_PATH_SIZE)
							{
								bufferDebug[0] = 2;
								return false;
							}
							else
							{
								bufferDebug[2]++;
								uint tr_size = (MAX_DDR_PATH_SIZE - temp_psize_ddr > MAX_BUF_SIZE_UR) ? (uint)MAX_BUF_SIZE_UR : (uint)(MAX_DDR_PATH_SIZE - temp_psize_ddr);
								uint start_idx = temp_psize_ur - tr_size;
								for (uint j = 0; j < tr_size; j++, temp_psize_ddr++)
								{
									for (uint k = 0; k < PADDING; k++)
									{
										if (k == MAX_K_Kernel)
											ddr_paths[temp_psize_ddr * PADDING + k] = (uint)buffer_path_layer_ur[start_idx + j];
										else
											ddr_paths[temp_psize_ddr * PADDING + k] = (uint)buffer_path_ur[start_idx + j][k];
									}
								}
								dram_path_size = temp_psize_ddr;
								temp_psize_ur = start_idx;

								tr_size = (MAX_BUF_SIZE_UR - temp_psize_ur > MAX_BUF_SIZE) ? (uint)MAX_BUF_SIZE : (uint)(MAX_BUF_SIZE_UR - temp_psize_ur);
								start_idx = temp_psize - tr_size;
								for (uint j = 0; j < tr_size; j++, temp_psize_ur++)
								{
									for (uint k = 0; k < MAX_K_Kernel; k++)
									{
										buffer_path_ur[temp_psize_ur][k] = buffer_path[start_idx + j][k];
									}
									buffer_path_layer_ur[temp_psize_ur] = buffer_path_layer[start_idx + j];
								}
								uram_path_size = temp_psize_ur;
								temp_psize = start_idx;
							}
						}
						else
						{
							uint tr_size = (MAX_BUF_SIZE_UR - temp_psize_ur > MAX_BUF_SIZE) ? (uint)MAX_BUF_SIZE : (uint)(MAX_BUF_SIZE_UR - temp_psize_ur);
							uint start_idx = temp_psize - tr_size;
							for (uint j = 0; j < tr_size; j++, temp_psize_ur++)
							{
								for (uint k = 0; k < MAX_K_Kernel; k++)
								{
									buffer_path_ur[temp_psize_ur][k] = buffer_path[start_idx + j][k];
								}
								buffer_path_layer_ur[temp_psize_ur] = buffer_path_layer[start_idx + j];
							}
							uram_path_size = temp_psize_ur;
							temp_psize = start_idx;
						}
					}

					uint path_idx = batched_nbr_idx[i - 1];
					uint iLayer = batched_path_layer[path_idx] + 1;
					for (uint j = 0; j < PADDING; j++)
					{
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
			if (uram_path_size == 0)
			{
				if (dram_path_size == 0)
				{
					bufferDebug[0] = 0;
					return false;
				}
				else
				{
					bufferDebug[1]++;
					uint temp_psize_ddr = dram_path_size;
					uint tr_size = temp_psize_ddr > DDR_BATCH_SIZE ? (uint)DDR_BATCH_SIZE : temp_psize_ddr;
					temp_psize_ddr -= tr_size;
					for (uint i = 0; i < tr_size; i++, temp_psize_ddr++)
					{
						for (uint j = 0; j < PADDING; j++)
						{
							if (j == MAX_K_Kernel)
								buffer_path_layer[i] = (ushort)ddr_paths[temp_psize_ddr * PADDING + j];
							else
								buffer_path[i][j] = ddr_paths[temp_psize_ddr * PADDING + j];
						}
					}
					bram_path_size = tr_size;
					dram_path_size -= tr_size;
				}
			}
			else
			{
				uint temp_psize_uram = uram_path_size;
				uint tr_size = temp_psize_uram > DDR_BATCH_SIZE ? (uint)DDR_BATCH_SIZE : temp_psize_uram;
				temp_psize_uram -= tr_size;
				for (uint i = 0; i < tr_size; i++, temp_psize_uram++)
				{
					for (uint k = 0; k < MAX_K_Kernel; k++)
					{
						buffer_path[i][k] = buffer_path_ur[temp_psize_uram][k];
					}
					buffer_path_layer[i] = buffer_path_layer_ur[temp_psize_uram];
				}
				bram_path_size = tr_size;
				uram_path_size -= tr_size;
			}
		}

		check_path_size = bram_path_size > MAX_IN_SIZE ? (uint)MAX_IN_SIZE : bram_path_size;
		temp_psize = bram_path_size;
		for (uint i = 0; i < check_path_size; i++, temp_psize--)
		{
			ushort ll = buffer_path_layer[temp_psize - 1];
			uint final_node = buffer_path[temp_psize - 1][ll];
			for (uint j = 0; j < PADDING; j++)
			{
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
		// bram_path_size = temp_psize;

		return bram_path_size != 0;
	}

public:
	void MYDFS(std::vector<uint> &_pos_list,
			   std::vector<uint> &_adj_list,
			   std::vector<ushort> &_sdt_list,
			   std::vector<uint> &_path_list,
			   std::vector<ulonglong> &_count_list,
			   std::vector<ulonglong> &_debug_list,
			   uint _node_size,
			   uint _edge_size,
			   uint _task_size,
			   uint _targe_node,
			   ushort _k,
			   ulonglong _max_tpc)
	{
		uint vertex_list_size = _node_size;
		uint edge_list_size = _edge_size;
		target_node = _targe_node;
		buffer_k = _k;
		ans_paths_num = 0;

		iii = 6;
		for (uint i = 0; i < iii; i++)
		{
#pragma HLS PIPELINE II = 1
			bufferDebug[i] = 0;
		}

		check_node_size = 0;
		check_path_size = 0;
		bram_path_size = _task_size;
		uram_path_size = 0;
		dram_path_size = 0;

		uint isize = vertex_list_size > MAX_NODE_SIZE ? (uint)MAX_NODE_SIZE : vertex_list_size;
		for (uint i = 0; i < isize + 1; i++)
		{
			buffer_pos_list[i] = _pos_list[i];
		}
		for (uint i = 0; i < isize; i++)
		{
			buffer_sdt_list[i] = _sdt_list[i];
		}
		isize = edge_list_size > MAX_EDGE_SIZE ? (uint)MAX_EDGE_SIZE : edge_list_size;
		for (uint i = 0; i < isize; i++)
		{
			buffer_adj_list[i] = _adj_list[i];
		}
		isize = bram_path_size > MAX_BUF_SIZE ? (uint)MAX_BUF_SIZE : bram_path_size;
		for (uint i = 0; i < isize; i++)
		{
			for (uint j = 0; j < PADDING; j++)
			{
				if (j == MAX_K_Kernel)
					buffer_path_layer[i] = _path_list[i * PADDING + j];
				else
					buffer_path[i][j] = _path_list[i * PADDING + j];
			}
		}

		ulonglong mtpc = _max_tpc;
		while (next_task(_path_list))
		{
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
		for (ushort i = 0; i < iii; i++)
		{
			_debug_list[i] = bufferDebug[i];
		}
	}
};
#endif