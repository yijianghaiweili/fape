#pragma once
#ifndef CHCPE_H
#define CHCPE_H

#include "util/common.h"
#include "util/io/io.h"
#include "util/graph/CDirectedGraph.h"
#include "CKernelTest.h"
#include "CHost.h"

extern bool g_exit;

class CHcPE
{
public:
	// 每条 query 的起始时间
	    std::chrono::steady_clock::time_point query_start_time_;

	    // 超时阈值（秒）
	    static constexpr double QUERY_TIME_LIMIT_SEC = 300.0;

	    // 判断是否超时
	    inline bool query_time_exceeded() const {
	        auto now = std::chrono::steady_clock::now();
	        return std::chrono::duration<double>(now - query_start_time_).count()
	               >= QUERY_TIME_LIMIT_SEC;
	    }
	enum class query_method
	{
		DFS,

		C_DFS,
		T_DFS,
		T_DFS2,
		HP_Index,
		BC_DFS,
		BC_DFS_Induced,
		BC_DFS_Induced_BlockInitedBySDt,
		BC_JOIN,
		IDX_DFS,
		IDX_JOIN,
		IDX_ENUM,

		D_DFS,
		DBC_DFS,
		SCB,
		SCB_plus,

		MY_DFS,
		PEFP_DFS,
		MY_JOIN,
		MY_DFS2,
		MY_DFS3,

		FPGA_PEFP,
		FPGA_MYDFS,
		FPGA_MYDFS_R,
		FPGA_MYDFS_Multi,
		FPGA_MYJOIN,
		FPGA_IDX_DFS,
		FPGA_IDX_DFS_Multi,
		FPGA_D_DFS
	};

public:
	const uint cDefaultQuerySize_ = 1000; // 默认一般查询最大大小

	/**
	 * Performance counter.
	 */
	// std::vector<std::vector<MyVec>> vPaths_;	   // 结果路径
	std::vector<ulonglong> vResultCount_;					  // 结果数量
	std::vector<ulonglong> vQueryTime_;						  // 查询时间
	std::vector<ulonglong> vPreprocessTime_;				  // 预处理时间
	std::vector<ulonglong> vPartialResultCount_;			  // Exclude result.
	std::vector<ulonglong> vInvalidPartialResultCount_;		  //
	std::vector<ulonglong> vNeighborsAccessCount_;			  // 访问点数
	std::vector<ulonglong> vConflictCount_;					  //
	std::vector<ulonglong> vPreliminaryEstimatedResultCount_; //
	std::vector<ulonglong> vFullFledgedEstimatedResultCount_; //
	std::vector<double> vEstimateAccuracy_;					  //
	std::vector<ulonglong> vSubGraphNodeCount_;				  // 子图大小
	std::vector<ulonglong> vSubGraphEdgeCount_;				  // 子图大小
	std::vector<MyVec> Paths_;								  //
	std::vector<ulonglong> vTempPathsCount_;				  // 中间路径数量
	ulonglong iPathCount_;									  //
	ulonglong iQueryTime_;									  //
	ulonglong iPreprocessTime_;								  //
	ulonglong iPartialResultCount_;							  //
	ulonglong iInvalidPartialResultCount_;					  //
	ulonglong iNeighborsAccessCount_;						  //
	ulonglong iConflictCount_;								  //
	ulonglong iPreliminaryEstimatedResultCount_;			  //
	ulonglong iFullFledgedEstimatedResultCount_;			  //
	double dEstimateAccuracy_;								  //
	ulonglong iSubGraphNodeCount_;							  //
	ulonglong iSubGraphEdgeCount_;							  //
	ulonglong iTempPathsCount_;								  //

	/**
	 * Detailed performance counter for IDX.
	 */
	std::vector<ulonglong> vForwardBfsTime_;
	std::vector<ulonglong> vBackwardBfsTime_;
	std::vector<ulonglong> vConstructBigraphTime_;
	std::vector<ulonglong> vIndexEdgeCount_;
	std::vector<ulonglong> vIndexVertexCount_;
	ulonglong iForwardBfsTime_;
	ulonglong iBackwardBfsTime_;
	ulonglong iConstructBigraphTime_;
	ulonglong iIndexEdgeCount_;
	ulonglong iIndexVertexCount_;

	/**
	 * Detailed performance counter for JOIN.
	 */
	std::vector<ulonglong> vFledgedEstimationTime_;
	std::vector<ulonglong> vLeftDfsTime_;
	std::vector<ulonglong> vRightDfsTime_;
	std::vector<ulonglong> vJoinTime_;
	ulonglong iFledgedEstimationTime_;
	ulonglong iLeftDfsTime_;
	ulonglong iRightDfsTime_;
	ulonglong iJoinTime_;

	/**
	 * Tune query optimizer
	 */
	std::vector<ulonglong> estimated_dfs_cost_arr_;
	std::vector<ulonglong> estimated_join_cost_arr_;
	std::vector<ulonglong> estimated_left_relation_size_arr_;
	std::vector<ulonglong> estimated_right_relation_size_arr_;
	std::vector<ulonglong> cut_position_arr_;
	std::vector<ulonglong> preliminary_selection_arr_;	// 0: DFS, 1: JOIN
	std::vector<ulonglong> full_fledged_selection_arr_; // 0: DFS, 1: JOIN
	ulonglong estimated_dfs_cost_;
	ulonglong estimated_join_cost_;
	ulonglong estimated_left_relation_size_;
	ulonglong estimated_right_relation_size_;
	ulonglong min_cut_position_;
	ulonglong preliminary_selection_;
	ulonglong full_fledged_selection_;
	ulonglong estimated_result_count_;

	/**
	 * Estimate the memory consumption of the index or relation.
	 */
	// std::vector<uint64_t> calculated_memory_cost_arr_;
	// uint64_t calculated_memory_cost_;

	/**
	 * FPGA Performance counter.
	 */
	std::vector<ulonglong> vBRAM2DDRTimes_;
	std::vector<ulonglong> vBRAM2DDRDatas_;
	std::vector<ulonglong> vDDR2BRAMTimes_;
	std::vector<ulonglong> vDDR2BRAMDatas_;
	ulonglong iBRAM2DDRTimes_;
	ulonglong iBRAM2DDRDatas_;
	ulonglong iDDR2BRAMTimes_;
	ulonglong iDDR2BRAMDatas_;

private:
	bool is_first_query = false;

	/**
	 * Input parameter.
	 */
	CDirectedGraph *oGraph_; // 输入图
	ushort iK_;				 // K-constraint
	uint iSrc_;				 // 起点
	uint iDst_;				 // 终点
	bool bCountOnly_;		 //

	/**
	 * Helper data structures.
	 */
	CDirectedGraph *oSubGraph_; // 处理图（剪枝或者非剪枝）
	ushort *arrSDu_t_;
	ushort *arrSDu_s_;
	uint *arrStack_;
	bool *arrVisited_;

	/**
	 * HP-Index data structures.
	 */
	std::unordered_map<uint, std::set<uint>> adjacency_map_;
	std::unordered_map<uint, std::set<uint>> adjacency_map_reverse_;
	std::set<uint> hot_points_;
	path_index index_;

	/**
	 * BC-DFS data structures.
	 */
	ushort *arrBarrier_;
	std::map<uint, std::set<uint>> B; // peng

	/**
	 * BC-JOIN data structures.
	 */
	std::set<uint> meet_nodes;
	std::unordered_map<uint, ushort> src_distance;
	std::unordered_map<uint, ushort> dst_distance;
	std::unordered_map<uint, std::set<uint>> reverse_adjacency_in_subgraph_left;
	std::unordered_map<uint, std::set<uint>> reverse_adjacency_in_subgraph_right;
	std::unordered_map<uint, std::set<uint>> dag_min_induced_subgraph;

	/**
	 * MY-Join data structures.
	 */
	uint *arrVisitedCnt_;
	std::vector<std::vector<std::vector<MyVec>>> Lpaths_, Rpaths_;
	std::vector<std::pair<spp::sparse_hash_set<uint>, spp::sparse_hash_set<uint>>> JoinVerts_;

	/**
	 * IDX data structures
	 */
	uint active_vertices_count_;							  // 双向bfs子图大小（不含t）
	std::pair<ushort, ushort> *distance_;					  // 距离s与t的最短距离
	uint *updated_values_;									  // 大小 iMaxVerID + 1 前向k-1跳BFS的拓扑的点
	uint *bucket_degree_sum_;								  // 大小 (iK_ + 1) * (iK_ + 1) * (iK_ + 1)
	uint *buckets_offset_;									  // 大小 (iK_ + 1) * (iK_ + 1) + 1 ，起始offset是1
	uint *buckets_adj_;										  // 大小 active_vertices_count_ ，第一个是s
	spp::sparse_hash_map<uint, uint> single_bigraph_;		  // IDX 大小 active_vertices_count_
	uint *single_bigraph_node_;								  // shrink 版 single_bigraph_
	uint *single_bigraph_offset_;							  // 大小 iK_ * (active_vertices_count_ + 1)
	uint *single_bigraph_adj_;								  // 大小
	uint single_bigraph_adj_size_;							  // single_bigraph_adj_大小
	spp::sparse_hash_map<uint, uint> single_reverse_bigraph_; //
	uint *single_reverse_bigraph_node_;						  // shrink 版 single_reverse_bigraph
	uint *single_reverse_bigraph_offset_;					  //
	uint *single_reverse_bigraph_adj_;						  //
	uint single_reverse_bigraph_adj_size_;					  // single_reverse_bigraph_adj_大小

	uint *left_relation_; // had Free
	ulonglong left_relation_size_;
	uint *left_cursor_;		   // nothing to be free
	uint *left_partial_begin_; // nothing to be free
	uint *left_partial_end_;   // nothing to be free
	uint left_part_length_;
	uint *right_relation_; // nothing to be free
	ulonglong right_relation_size_;
	uint *right_cursor_; // nothing to be free
	uint right_part_length_;
	uint *right_partial_begin_; // nothing to be free
	uint *right_partial_end_;	// nothing to be free
	spp::sparse_hash_map<uint, std::pair<uint *, ulonglong>> index_table_;

	/**
	 * FPGA Class
	 */
	CKernelTest *oTest_;
#ifdef USE_FPGA
	CHost *oHost_;
#endif

public:
	inline uint iSrc() { return iSrc_; }
	inline uint iDst() { return iDst_; }
	inline ushort iK() { return iK_; }

	inline ushort *arrSDu_t() { return arrSDu_t_; }
	inline ushort *arrSDu_s() { return arrSDu_s_; }

	explicit CHcPE();
	~CHcPE();

	/**
	 * Load Query
	 */
	bool profile_enabled_ = true;
	inline void SetProfileEnabled(bool en) { profile_enabled_ = en; }

	static std::unordered_set<std::pair<uint, uint>, pair_hash> GenerateQueries(const int &cnt, const int &upper_bound);
	static std::vector<std::pair<uint, uint>> LoadQuery(const std::string strFilePath, const char cSkip);
	static std::vector<std::pair<uint, uint>> LoadQueryFromBin(const std::string strFilePath);
	static std::vector<std::pair<uint, uint>> LoadRadomQuery(const std::string strFilePath, const char cSkip, const uint iQueyNum);

	void Init(CDirectedGraph *oG, CDirectedGraph *oSG, const ushort length_constraint, const std::string &binaryFile);
	void Init(CDirectedGraph *oG, CDirectedGraph *oSG, const ushort length_constraint);
	ulonglong Excute(const query_method method_type, const uint iSouce, const uint iTarget, const bool count_only, uint &overlarge);

private:
	/**
	 * meta operate
	 */
	inline void khopForwardBFS(uint iStartVer, ushort k);
	inline void khopBackwardBFS(uint iStartVer, ushort k);
	inline spp::sparse_hash_map<uint, ushort> khopBackwardBFSWithBlock(uint iStartVer, ushort k);
	inline spp::sparse_hash_map<uint, ushort> khopBackwardBFSWithBlock(uint iStartVer, ushort k, spp::sparse_hash_set<uint> blocked);

	/**
	 * processing
	 */
	void Pre_DouKHopBFS(const uint &hop, bool sds_order, bool sdt_order, ulonglong &time);																		   // Sdut
	void Pre_DouHalfHopBFS(bool sds_order, bool sdt_order, ulonglong &time);																					   // Sdut
	void Pre_DouHalfHopBFS_1(bool sds_order, bool sdt_order, ulonglong &time);																					   // Sdut
	void Pre_DouHalfHopAndSigHalfHopBFS(bool sds_order, bool sdt_order, ulonglong &time);																		   // Sdut
	void Pre_DouHalfHopAndSigHalfHopBFS_1(bool sds_order, bool sdt_order, ulonglong &time);																		   // Sdut
	void Pre_DouHalfHopAndSigHalfHopBFS_2(bool sds_order, bool sdt_order, ulonglong &time);																		   //
	void Pre_DouHalfHopAndDouHalfHopBFS(bool sds_order, bool sdt_order, ulonglong &time);																		   // Sdut
	void Pre_DouHalfHopAndDouHalfHopBFS(bool sds_order, bool sdt_order, std::unordered_map<uint, uint> &nodemap, ulonglong &time);								   // Sdut reordering
	std::set<uint> find_hot_points(CDirectedGraph *&oGraph, double threshold);																					   //
	std::set<uint> find_hot_points_top_t(CDirectedGraph *&oGraph, int t);																						   //
	path_index construct_hot_point_index_dfs_using_new_algorithm(std::unordered_map<uint, std::set<uint>> &induced_subgraph, ushort k, std::set<uint> hot_points); //
	void Pre_HPIndex(CDirectedGraph *&oGraph, ulonglong &index_time);																							   // HP-Index
	void construct_pruned_dag_min_subgraph(uint k, uint query_node1, uint query_node2, ulonglong &time);														   // BC-JOIN
	void Pre_BiGraphIDX(ulonglong &time);																														   // IDX
	void Pre_BiGraphIDX_Shrink(spp::sparse_hash_map<uint, uint> &nodemap, ulonglong &itime);																	   // IDX
	inline void clear_bigraph();																																   // IDX
	bool preliminary_cardinality_estimator();																													   //

	/**
	 * Algorithm
	 */
	void CDFSRecWithCSR(CDirectedGraph *&oGraph, const uint &u, MyVec &vTempPath);																							//
	void CDFSPathEnumCSR(CDirectedGraph *&oGraph, ulonglong &time);																											// C-DFS
	void TDFSRecWithCSR(CDirectedGraph *&oGraph, uint u, MyVec &vTempPath);																									//
	void TDFSPathEnumCSR(CDirectedGraph *&oGraph, ulonglong &time);																											// T-DFS
	void TDFS2RecWithCSR(CDirectedGraph *&oGraph, uint u, MyVec vTempPath, spp::sparse_hash_set<uint> visited);																//
	void TDFS2PathEnumCSR(CDirectedGraph *&oGraph, ulonglong &time);																										// T-DFS2
	paths HPIndexPathEnumCSR(CDirectedGraph *&oGraph, ulonglong &query_time);																								// HP-Index
	void BCDFSUpdateBarrier(CDirectedGraph *&oGraph, const uint &u, const uint &distance_u_t);																				//
	uint BCDFSRecWithCSR(CDirectedGraph *&oGraph, const uint &u, MyVec &vTempPath);																							//
	void BCDFSPathEnumCSR(CDirectedGraph *&oGraph, ulonglong &time);																										// BC-DFS
	void unblock(uint u, ushort unreach_dis);																																//
	int dfs_find_k_paths_with_block(CDirectedGraph *&oGraph, uint cur_node, std::set<uint> c_path_set, ushort cur_distance, std::vector<uint> c_path);						//
	void BCDFSPathEnumCSR2(CDirectedGraph *&oGraph, ulonglong &time);																										// BC-DFS peng
	void BCDFSPathEnumCSR2_BlockInitedBySDt(CDirectedGraph *&oGraph, ulonglong &time);																						// BC-DFS peng
	void join_left_right_index_into_left();																																	//
	void find_all_meet_nodes_in_induced_subgraph(uint k, uint query_node1, uint query_node2);																				//
	std::map<uint, paths> dfs_without_recursion_all_meetpoints(std::unordered_map<uint, std::set<uint>> &reverse_adjacency_in_subgraph, uint src, uint dst, ushort k);		//
	std::map<uint, paths> dfs_without_recursion_all_meetpoints_reverse(std::unordered_map<uint, std::set<uint>> &reverse_adjacency_in_subgraph, uint src, uint dst, int k); //
	void find_all_k_pahts_dfs_write_number_left_right_path(uint k, uint query_node1, uint query_node2);																		//
	void BCJOINPathEnumCSR(ulonglong &time);																																// BC-JOIN peng
	void IDXDFSRecWithCSR(uint u, uint k);																																	//
	void IDXDFSPathEnumCSR(ulonglong &time);																																// IDX-DFS
	void generate_single_join_plan();																																		//
	void left_dfs(uint32_t u, uint32_t k);																																	//
	void right_dfs(uint32_t u, uint32_t k);																																	//
	void single_join();																																						//
	void single_join_on_bigraph();																																			//
	void IDXJOINPathEnumCSR(ulonglong &time);																																// IDX-JOIN
	void IDXMIXPathEnumCSR(ulonglong &time);																																// PATH-ENUM
	void MYDFSPathEnumCSR(CDirectedGraph *&oGraph, ulonglong &time);																										// MY-DFS
	void MYDFSPathEnumCSR(CDirectedGraph *&oGraph, std::vector<uint> pp, ushort hop, uint dst, ulonglong &time);															//
	void MYDFSPathEnumCSR(CDirectedGraph *&oGraph, uint src, uint dst, ulonglong &time);																					// MY-DFS test
	void MYDFSRecWithCSR(CDirectedGraph *&oGraph, const uint u, const ushort iLayer);																						//
	void MYDFSPathEnumCSR_1(CDirectedGraph *&oGraph, ulonglong &time);																										// MY-DFS
	void MYDFSPathEnumCSR_2(CDirectedGraph *&oGraph, ulonglong &time);																										// MY-DFS
	void PEFPPathEnumCSR(CDirectedGraph *&oGraph, ulonglong &time);																											// PEFP-DFS
	void JoinCounter(uint l, uint r);																																		//
	void NativeJoin(CDirectedGraph *&oGraph);																																//
	void MYJOINPathEnumCSR(CDirectedGraph *&oGraph, ulonglong &time);																										// MY-JOIN
	void DynamicJoin(CDirectedGraph *&oGraph, const uint iSrc, const uint iDst, ulonglong &time, ulonglong &iAnsNum);														// D-JOIN

	void generate_subtask_on_CSR(CDirectedGraph *&oGraph, ushort *&sdt, uint u, uint t, uint k, uint generate_max_num, MyVec &bufferPath, MyVec &bufferHop, uint &tmpPathCount, ulonglong &itime, ulonglong &iAnsNum);
	void generate_subtask_on_bigraph(uint u, uint t, uint k, uint node_cnt, uint generate_max_num, MyVec &bufferPath, MyVec &bufferHop, uint &tmpPathCount, ulonglong &itime, ulonglong &iAnsNum);

	inline void initialize_performance_counter();
	inline void clear_performance_counter();
	inline void reset_performance_counter();
	inline void update_performance_counter();

public:
	void update_performance_counter1()
	{
		vResultCount_.emplace_back(iPathCount_);
		vQueryTime_.emplace_back(iQueryTime_);
		vPreprocessTime_.emplace_back(iPreprocessTime_);
		vPartialResultCount_.emplace_back(iPartialResultCount_);
		vInvalidPartialResultCount_.emplace_back(iInvalidPartialResultCount_);
		vNeighborsAccessCount_.emplace_back(iNeighborsAccessCount_);
		vConflictCount_.emplace_back(iConflictCount_);
		vPreliminaryEstimatedResultCount_.emplace_back(iPreliminaryEstimatedResultCount_);
		vFullFledgedEstimatedResultCount_.emplace_back(iFullFledgedEstimatedResultCount_);
		vEstimateAccuracy_.emplace_back(dEstimateAccuracy_);
		vSubGraphNodeCount_.emplace_back(iSubGraphNodeCount_);
		vSubGraphEdgeCount_.emplace_back(iSubGraphEdgeCount_);

		vForwardBfsTime_.emplace_back(iForwardBfsTime_);
		vBackwardBfsTime_.emplace_back(iBackwardBfsTime_);
		vConstructBigraphTime_.emplace_back(iConstructBigraphTime_);
		vIndexEdgeCount_.emplace_back(iIndexEdgeCount_);
		vIndexVertexCount_.emplace_back(iIndexVertexCount_);

		vFledgedEstimationTime_.emplace_back(iFledgedEstimationTime_);
		vLeftDfsTime_.emplace_back(iLeftDfsTime_);
		vRightDfsTime_.emplace_back(iRightDfsTime_);
		vJoinTime_.emplace_back(iJoinTime_);

		estimated_join_cost_arr_.emplace_back(estimated_join_cost_);
		estimated_dfs_cost_arr_.emplace_back(estimated_dfs_cost_);
		estimated_left_relation_size_arr_.emplace_back(estimated_left_relation_size_);
		estimated_right_relation_size_arr_.emplace_back(estimated_right_relation_size_);
		cut_position_arr_.emplace_back(min_cut_position_);
		preliminary_selection_arr_.emplace_back(preliminary_selection_);
		full_fledged_selection_arr_.emplace_back(full_fledged_selection_);

		vBRAM2DDRTimes_.emplace_back(iBRAM2DDRTimes_);
		vBRAM2DDRDatas_.emplace_back(iBRAM2DDRDatas_);
		vDDR2BRAMTimes_.emplace_back(iDDR2BRAMTimes_);
		vDDR2BRAMDatas_.emplace_back(iDDR2BRAMDatas_);
	}
};

#endif // CHCPE_H
