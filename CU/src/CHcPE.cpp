#include "CHcPE.h"
#include <chrono>


bool g_exit = false;



#define BUCKET_ID(i, j, l) ((i) * (l) + (j))

CHcPE::CHcPE()
{
	is_first_query = true;

	iK_ = 0;

	arrSDu_t_ = nullptr;
	arrSDu_s_ = nullptr;
	arrStack_ = nullptr;
	arrVisited_ = nullptr;

	arrBarrier_ = nullptr;

	arrVisitedCnt_ = nullptr;

	distance_ = nullptr;
	updated_values_ = nullptr;
	bucket_degree_sum_ = nullptr;
	buckets_offset_ = nullptr;
	buckets_adj_ = nullptr;
	single_bigraph_node_ = nullptr;
	single_bigraph_offset_ = nullptr;
	single_bigraph_adj_ = nullptr;
	single_reverse_bigraph_node_ = nullptr;
	single_reverse_bigraph_offset_ = nullptr;
	single_reverse_bigraph_adj_ = nullptr;
}

CHcPE::~CHcPE()
{
	iK_ = 0;

	if (arrSDu_s_ != nullptr)
	{
		free(arrSDu_s_);
		arrSDu_s_ = nullptr;
	}
	if (arrSDu_t_ != nullptr)
	{
		free(arrSDu_t_);
		arrSDu_t_ = nullptr;
	}
	if (arrStack_ != nullptr)
	{
		free(arrStack_);
		arrStack_ = nullptr;
	}
	if (arrVisited_ != nullptr)
	{
		free(arrVisited_);
		arrVisited_ = nullptr;
	}

	if (arrBarrier_ != nullptr)
	{
		free(arrBarrier_);
		arrBarrier_ = nullptr;
	}

	if (arrVisitedCnt_ != nullptr)
	{
		free(arrVisitedCnt_);
		arrVisitedCnt_ = nullptr;
	}

	if (distance_ != nullptr)
	{
		free(distance_);
		distance_ = nullptr;
	}
	if (updated_values_ != nullptr)
	{
		free(updated_values_);
		updated_values_ = nullptr;
	}
	if (bucket_degree_sum_ != nullptr)
	{
		free(bucket_degree_sum_);
		bucket_degree_sum_ = nullptr;
	}
	if (buckets_offset_ != nullptr)
	{
		free(buckets_offset_);
		buckets_offset_ = nullptr;
	}
	if (buckets_adj_ != nullptr)
	{
		free(buckets_adj_);
		buckets_adj_ = nullptr;
	}
	if (single_bigraph_node_ != nullptr)
	{
		free(single_bigraph_node_);
		single_bigraph_node_ = nullptr;
	}
	if (single_bigraph_offset_ != nullptr)
	{
		free(single_bigraph_offset_);
		single_bigraph_offset_ = nullptr;
	}
	if (single_bigraph_adj_ != nullptr)
	{
		free(single_bigraph_adj_);
		single_bigraph_adj_ = nullptr;
	}
	if (single_reverse_bigraph_node_ != nullptr)
	{
		free(single_reverse_bigraph_node_);
		single_reverse_bigraph_node_ = nullptr;
	}
	if (single_reverse_bigraph_offset_ != nullptr)
	{
		free(single_reverse_bigraph_offset_);
		single_reverse_bigraph_offset_ = nullptr;
	}
	if (single_reverse_bigraph_adj_ != nullptr)
	{
		free(single_reverse_bigraph_adj_);
		single_reverse_bigraph_adj_ = nullptr;
	}

	clear_performance_counter();

#ifdef USE_FPGA
	delete oHost_;
#endif
}

void CHcPE::Init(CDirectedGraph *oG, CDirectedGraph *oSG, const ushort length_constraint, const std::string &binaryFile)
{
	assert(length_constraint > 0 && oG != nullptr && oSG != nullptr);
	oGraph_ = oG;
	oSubGraph_ = oSG;

	iSrc_ = 0;
	iDst_ = 0;
	iK_ = length_constraint;
	bCountOnly_ = false;
	is_first_query = true;

	ulonglong iSize = sizeof(ushort) * oG->iMaxVerId() + 1;
	arrSDu_t_ = (ushort *)malloc(iSize);
	memset(arrSDu_t_, MAX_K, iSize);
	arrSDu_s_ = (ushort *)malloc(iSize);
	memset(arrSDu_s_, MAX_K, iSize);
	iSize = sizeof(uint) * (iK_ + 1);
	arrStack_ = (uint *)malloc(iSize);
	memset(arrStack_, 0, iSize);
	iSize = sizeof(bool) * oG->iMaxVerId() + 1;
	arrVisited_ = (bool *)malloc(iSize);
	memset(arrVisited_, 0, iSize);

	iSize = sizeof(ushort) * oG->iMaxVerId() + 1;
	arrBarrier_ = (ushort *)malloc(iSize);
	memset(arrBarrier_, MAX_K, iSize);

	iSize = sizeof(uint) * oG->iMaxVerId() + 1;
	arrVisitedCnt_ = (uint *)malloc(iSize);
	memset(arrVisitedCnt_, 0, iSize);

	iSize = sizeof(std::pair<ushort, ushort>) * oG->iMaxVerId() + 1;
	distance_ = (std::pair<ushort, ushort> *)malloc(iSize);
	memset((ushort *)distance_, static_cast<ushort>(iK_) + 1, iSize);
	iSize = sizeof(uint) * oG->iMaxVerId() + 1;
	updated_values_ = (uint *)malloc(iSize);
	memset(updated_values_, 0, iSize);
	iSize = (iK_ + 1) * (iK_ + 1) * (iK_ + 1);
	bucket_degree_sum_ = (uint *)malloc(sizeof(uint) * iSize);
	memset(bucket_degree_sum_, 0, sizeof(uint) * iSize);
	iSize = (iK_ + 1) * (iK_ + 1) + 1;
	buckets_offset_ = (uint *)malloc(sizeof(uint) * iSize);
	memset(buckets_offset_, 0, sizeof(uint) * iSize);

	initialize_performance_counter();

	oTest_ = new CKernelTest();
#ifdef USE_FPGA
	oHost_ = new CHost(binaryFile, true);
	// oHost_ = new CHost(binaryFile, false);
#endif
}

void CHcPE::Init(CDirectedGraph *oG, CDirectedGraph *oSG, const ushort length_constraint)
{
	assert(length_constraint > 0 && oG != nullptr && oSG != nullptr);
	oGraph_ = oG;
	oSubGraph_ = oSG;

	iSrc_ = 0;
	iDst_ = 0;
	iK_ = length_constraint;
	bCountOnly_ = false;
	is_first_query = true;

	ulonglong iSize = sizeof(ushort) * oG->iMaxVerId() + 1;
	arrSDu_t_ = (ushort *)malloc(iSize);
	memset(arrSDu_t_, MAX_K, iSize);
	arrSDu_s_ = (ushort *)malloc(iSize);
	memset(arrSDu_s_, MAX_K, iSize);
	iSize = sizeof(uint) * (iK_ + 1);
	arrStack_ = (uint *)malloc(iSize);
	memset(arrStack_, 0, iSize);
	iSize = sizeof(bool) * oG->iMaxVerId() + 1;
	arrVisited_ = (bool *)malloc(iSize);
	memset(arrVisited_, 0, iSize);

	iSize = sizeof(ushort) * oG->iMaxVerId() + 1;
	arrBarrier_ = (ushort *)malloc(iSize);
	memset(arrBarrier_, MAX_K, iSize);

	iSize = sizeof(uint) * oG->iMaxVerId() + 1;
	arrVisitedCnt_ = (uint *)malloc(iSize);
	memset(arrVisitedCnt_, 0, iSize);

	iSize = sizeof(std::pair<ushort, ushort>) * oG->iMaxVerId() + 1;
	distance_ = (std::pair<ushort, ushort> *)malloc(iSize);
	memset((ushort *)distance_, static_cast<ushort>(iK_) + 1, iSize);
	iSize = sizeof(uint) * oG->iMaxVerId() + 1;
	updated_values_ = (uint *)malloc(iSize);
	memset(updated_values_, 0, iSize);
	iSize = (iK_ + 1) * (iK_ + 1) * (iK_ + 1);
	bucket_degree_sum_ = (uint *)malloc(sizeof(uint) * iSize);
	memset(bucket_degree_sum_, 0, sizeof(uint) * iSize);
	iSize = (iK_ + 1) * (iK_ + 1) + 1;
	buckets_offset_ = (uint *)malloc(sizeof(uint) * iSize);
	memset(buckets_offset_, 0, sizeof(uint) * iSize);

	initialize_performance_counter();

	oTest_ = new CKernelTest();
}

void CHcPE::reset_performance_counter()
{
	Paths_.clear();
	iPathCount_ = 0;
	iQueryTime_ = 0;
	iPreprocessTime_ = 0;
	iPartialResultCount_ = 0;
	iInvalidPartialResultCount_ = 0;
	iNeighborsAccessCount_ = 0;
	iConflictCount_ = 0;
	iPreliminaryEstimatedResultCount_ = 0;
	iFullFledgedEstimatedResultCount_ = 0;
	dEstimateAccuracy_ = 0.0;
	iSubGraphNodeCount_ = 0;
	iSubGraphEdgeCount_ = 0;
	iTempPathsCount_ = 0;

	iForwardBfsTime_ = 0;
	iBackwardBfsTime_ = 0;
	iConstructBigraphTime_ = 0;
	iIndexEdgeCount_ = 0;
	iIndexVertexCount_ = 0;

	iFledgedEstimationTime_ = 0;
	iLeftDfsTime_ = 0;
	iRightDfsTime_ = 0;
	iJoinTime_ = 0;

	estimated_dfs_cost_ = 0;
	estimated_join_cost_ = 0;
	estimated_left_relation_size_ = 0;
	estimated_right_relation_size_ = 0;
	min_cut_position_ = 0;
	preliminary_selection_ = 0;
	full_fledged_selection_ = 0;
	estimated_result_count_ = 0;

	iBRAM2DDRTimes_ = 0;
	iBRAM2DDRDatas_ = 0;
	iDDR2BRAMTimes_ = 0;
	iDDR2BRAMDatas_ = 0;
}

void CHcPE::initialize_performance_counter()
{
	if (!profile_enabled_) return;

	reset_performance_counter();

	vResultCount_.reserve(cDefaultQuerySize_);
	vQueryTime_.reserve(cDefaultQuerySize_);
	vPreprocessTime_.reserve(cDefaultQuerySize_);
	vPartialResultCount_.reserve(cDefaultQuerySize_);
	vInvalidPartialResultCount_.reserve(cDefaultQuerySize_);
	vNeighborsAccessCount_.reserve(cDefaultQuerySize_);
	vConflictCount_.reserve(cDefaultQuerySize_);
	vPreliminaryEstimatedResultCount_.reserve(cDefaultQuerySize_);
	vFullFledgedEstimatedResultCount_.reserve(cDefaultQuerySize_);
	vEstimateAccuracy_.reserve(cDefaultQuerySize_);
	vSubGraphNodeCount_.reserve(cDefaultQuerySize_);
	vSubGraphEdgeCount_.reserve(cDefaultQuerySize_);
	vTempPathsCount_.reserve(cDefaultQuerySize_);

	vForwardBfsTime_.reserve(cDefaultQuerySize_);
	vBackwardBfsTime_.reserve(cDefaultQuerySize_);
	vConstructBigraphTime_.reserve(cDefaultQuerySize_);
	vIndexEdgeCount_.reserve(cDefaultQuerySize_);
	vIndexVertexCount_.reserve(cDefaultQuerySize_);

	vFledgedEstimationTime_.reserve(cDefaultQuerySize_);
	vLeftDfsTime_.reserve(cDefaultQuerySize_);
	vRightDfsTime_.reserve(cDefaultQuerySize_);
	vJoinTime_.reserve(cDefaultQuerySize_);

	estimated_dfs_cost_arr_.reserve(cDefaultQuerySize_);
	estimated_join_cost_arr_.reserve(cDefaultQuerySize_);
	estimated_left_relation_size_arr_.reserve(cDefaultQuerySize_);
	estimated_right_relation_size_arr_.reserve(cDefaultQuerySize_);
	cut_position_arr_.reserve(cDefaultQuerySize_);
	preliminary_selection_arr_.reserve(cDefaultQuerySize_);
	full_fledged_selection_arr_.reserve(cDefaultQuerySize_);

	vBRAM2DDRTimes_.reserve(cDefaultQuerySize_);
	vBRAM2DDRDatas_.reserve(cDefaultQuerySize_);
	vDDR2BRAMTimes_.reserve(cDefaultQuerySize_);
	vDDR2BRAMDatas_.reserve(cDefaultQuerySize_);
}

void CHcPE::clear_performance_counter()
{
	if (!profile_enabled_) return;

	reset_performance_counter();

	vResultCount_.clear();
	vQueryTime_.clear();
	vPreprocessTime_.clear();
	vPartialResultCount_.clear();
	vInvalidPartialResultCount_.clear();
	vNeighborsAccessCount_.clear();
	vConflictCount_.clear();
	vPreliminaryEstimatedResultCount_.clear();
	vFullFledgedEstimatedResultCount_.clear();
	vEstimateAccuracy_.clear();
	vSubGraphNodeCount_.clear();
	vSubGraphEdgeCount_.clear();
	vTempPathsCount_.clear();

	vForwardBfsTime_.clear();
	vBackwardBfsTime_.clear();
	vConstructBigraphTime_.clear();
	vIndexEdgeCount_.clear();
	vIndexVertexCount_.clear();

	vFledgedEstimationTime_.clear();
	vLeftDfsTime_.clear();
	vRightDfsTime_.clear();
	vJoinTime_.clear();

	estimated_dfs_cost_arr_.clear();
	estimated_join_cost_arr_.clear();
	estimated_left_relation_size_arr_.clear();
	estimated_right_relation_size_arr_.clear();
	cut_position_arr_.clear();
	preliminary_selection_arr_.clear();
	full_fledged_selection_arr_.clear();

	vBRAM2DDRTimes_.clear();
	vBRAM2DDRDatas_.clear();
	vDDR2BRAMTimes_.clear();
	vDDR2BRAMDatas_.clear();
}

void CHcPE::update_performance_counter()
{
	if (!profile_enabled_) return;

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
	vTempPathsCount_.emplace_back(iTempPathsCount_);

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

std::unordered_set<std::pair<uint, uint>, pair_hash> CHcPE::GenerateQueries(const int &cnt, const int &upper_bound)
{
	std::unordered_set<std::pair<uint, uint>, pair_hash> queries;
	srand(time(NULL));
	while (queries.size() < cnt)
	{
		uint start = rand() % upper_bound;
		uint target = rand() % upper_bound;
		if (start != target)
		{
			auto q = std::make_pair(start, target);
			if (queries.find(q) == queries.end())
			{
				queries.insert(q);
			}
		}
	}
	return queries;
}

std::vector<std::pair<uint, uint>> CHcPE::LoadQuery(const std::string strFilePath, const char cSkip)
{
	std::vector<std::pair<uint, uint>> queries;
	std::ifstream steFile(strFilePath);
	if (!steFile.is_open())
	{
		log_error("failed to open %s", strFilePath.c_str());
		return queries;
	}
	uint line_count = 0;
	uint src, dst;
	std::string tmp_str;
	while (std::getline(steFile, tmp_str))
	{
		line_count += 1;
		if (tmp_str[0] != cSkip)
		{
			std::stringstream ss(tmp_str);
			if (!(ss >> src >> dst))
			{
				log_error("Cannot convert line %u to query.", line_count);
				exit(-1);
			}
			queries.emplace_back(src, dst);
		}
	}
	return queries;
}

std::vector<std::pair<uint, uint>> CHcPE::LoadQueryFromBin(const std::string strFilePath)
{
	std::vector<std::pair<uint, uint>> vQuery;
	IO::read(strFilePath, vQuery);
	return vQuery;
}

std::vector<std::pair<uint, uint>> CHcPE::LoadRadomQuery(const std::string strFilePath, const char cSkip, const uint iQueyNum)
{
	// TODO
	std::vector<std::pair<uint, uint>> queries;
	return queries;
}

void CHcPE::khopForwardBFS(uint iStartVer, ushort k)
{
	for (uint i = 0; i <= oGraph_->iMaxVerId(); i++)
	{
		arrSDu_s_[i] = MAX_K;
	}
	arrSDu_s_[iStartVer] = 0;

	std::queue<uint> q;
	q.push(iStartVer);
	while (q.size() != 0)
	{
		uint v = q.front();
		q.pop();
		auto out_neighbors = oGraph_->GetOutNbrCSR(v);
		for (uint i = 0; i < out_neighbors.second; ++i)
		{
			uint vv = out_neighbors.first[i];
			if (arrSDu_s_[vv] == MAX_K) // 未访问
			{
				arrSDu_s_[vv] = arrSDu_s_[v] + 1;
				if (arrSDu_s_[vv] < k)
					q.push(vv);
			}
		}
	}
}

void CHcPE::khopBackwardBFS(uint iStartVer, ushort k)
{
	for (uint i = 0; i <= oGraph_->iMaxVerId(); i++)
	{
		arrSDu_t_[i] = MAX_K;
	}
	arrSDu_t_[iStartVer] = 0;

	std::queue<uint> q;
	q.push(iStartVer);
	while (q.size() != 0)
	{
		uint v = q.front();
		q.pop();
		auto in_neighbors = oGraph_->GetInNbrCSR(v);
		for (uint i = 0; i < in_neighbors.second; ++i)
		{
			uint vv = in_neighbors.first[i];
			if (arrSDu_t_[vv] == MAX_K) // 未访问
			{
				arrSDu_t_[vv] = arrSDu_t_[v] + 1;
				if (arrSDu_t_[vv] < k)
					q.push(vv);
			}
		}
	}
}

spp::sparse_hash_map<uint, ushort> CHcPE::khopBackwardBFSWithBlock(uint iStartVer, ushort k)
{
	spp::sparse_hash_map<uint, ushort> sdt_map;
	sdt_map[iStartVer] = 0;
	std::queue<uint> q;
	q.push(iStartVer);
	while (q.size() != 0)
	{
		uint v = q.front();
		q.pop();
		auto in_neighbors = oGraph_->GetInNbrCSR(v);
		for (uint i = 0; i < in_neighbors.second; ++i)
		{
			uint vv = in_neighbors.first[i];
			if (sdt_map.find(vv) == sdt_map.end() && !arrVisited_[vv])
			{
				sdt_map[vv] = sdt_map[v] + 1;
				if (sdt_map[vv] < k)
					q.push(vv);
			}
		}
	}
	return sdt_map;
}

spp::sparse_hash_map<uint, ushort> CHcPE::khopBackwardBFSWithBlock(uint iStartVer, ushort k, spp::sparse_hash_set<uint> arrBarrier_)
{
	spp::sparse_hash_map<uint, ushort> sdt_map;
	sdt_map[iStartVer] = 0;
	std::queue<uint> q;
	q.push(iStartVer);
	while (q.size() != 0)
	{
		uint v = q.front();
		q.pop();
		auto in_neighbors = oGraph_->GetInNbrCSR(v);
		for (uint i = 0; i < in_neighbors.second; ++i)
		{
			uint vv = in_neighbors.first[i];
			if (sdt_map.find(vv) == sdt_map.end() && arrBarrier_.find(vv) == arrBarrier_.end())
			{
				sdt_map[vv] = sdt_map[v] + 1;
				if (sdt_map[vv] < k)
					q.push(vv);
			}
		}
	}
	return sdt_map;
}

/**
 * @brief 先正向k跳建立sdus，然后反向k跳建立sdut，求交集得induced graph，sdu排序
 *
 * @param time 耗时
 */
void CHcPE::Pre_DouKHopBFS(const uint &hop, bool sds_order, bool sdt_order, ulonglong &time)
{
	time = 0;
	for (uint i = 0; i <= oGraph_->iMaxVerId(); i++)
	{
		arrSDu_t_[i] = MAX_K;
		arrSDu_s_[i] = MAX_K;
	}
	arrSDu_t_[iDst_] = 0;
	arrSDu_s_[iSrc_] = 0;

	spp::sparse_hash_set<uint> setInduce;
	std::queue<uint> q;
	oSubGraph_->Init();

	uint test_c = 0;
	auto start = std::chrono::high_resolution_clock::now();

	q.push(iSrc_);
	while (!q.empty())
	{
		uint v = q.front();
		q.pop();
		auto out_neighbors = oGraph_->GetOutNbrCSR(v);
		for (uint i = 0; i < out_neighbors.second; ++i)
		{
			test_c++;
			uint vv = out_neighbors.first[i];
			if (arrSDu_s_[vv] == MAX_K)
			{
				arrSDu_s_[vv] = arrSDu_s_[v] + 1;
				if (arrSDu_s_[vv] < hop)
					q.push(vv);
			}
		}
	}
	q.push(iDst_);
	while (!q.empty())
	{
		uint v = q.front();
		q.pop();
		auto in_neighbors = oGraph_->GetInNbrCSR(v);
		for (uint i = 0; i < in_neighbors.second; ++i)
		{
			test_c++;
			uint vv = in_neighbors.first[i];
			if (arrSDu_t_[vv] == MAX_K && arrSDu_s_[vv] != MAX_K)
			{
				arrSDu_t_[vv] = arrSDu_t_[v] + 1;
				setInduce.insert(vv);
				if (arrSDu_t_[vv] < hop)
					q.push(vv);
			}
		}
	}
	setInduce.insert(iSrc_);
	setInduce.insert(iDst_);
	auto end = std::chrono::high_resolution_clock::now();
	time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	iBRAM2DDRTimes_ = test_c;

	ulonglong gtime = 0;
	std::vector<std::pair<uint, uint>> edge_list;
	for (uint s : setInduce)
	{
		if (s != iDst_)
		{
			auto out_neighbors = oGraph_->GetOutNbrCSR(s);
			for (uint i = 0; i < out_neighbors.second; ++i)
			{
				uint t = out_neighbors.first[i];
				if (setInduce.find(t) != setInduce.end())
				{
					edge_list.emplace_back(s, t);
				}
			}
		}
	}
	if (sds_order)
	{
		if (sdt_order)
			oSubGraph_->CreateSubGraphCSR(edge_list, arrSDu_s_, arrSDu_t_, true, gtime);
		else
			oSubGraph_->CreateSubGraphCSR(edge_list, arrSDu_s_, nullptr, true, gtime);
	}
	else
	{
		if (sdt_order)
			oSubGraph_->CreateSubGraphCSR(edge_list, nullptr, arrSDu_t_, true, gtime);
		else
			oSubGraph_->CreateSubGraphCSR(edge_list, nullptr, nullptr, true, gtime);
	}

	// time += gtime;
}

/**
 * @brief 先正反向k/2跳求交得搜索范围，再用反向k-1跳建立sdut，直接得induced graph，sdut排序
 *
 * @param time
 */
void CHcPE::Pre_DouHalfHopBFS(bool sds_order, bool sdt_order, ulonglong &time)
{
	time = 0;
	for (uint i = 0; i <= oGraph_->iMaxVerId(); i++)
	{
		arrSDu_t_[i] = MAX_K;
		arrSDu_s_[i] = MAX_K;
		arrVisited_[i] = false;
	}
	arrSDu_t_[iDst_] = 0;
	arrSDu_s_[iSrc_] = 0;

	spp::sparse_hash_set<uint> setInduce;
	std::queue<uint> q;
	oSubGraph_->Init();

	auto start = std::chrono::high_resolution_clock::now();

	q.push(iSrc_);
	while (!q.empty())
	{
		uint v = q.front();
		q.pop();
		auto out_neighbors = oGraph_->GetOutNbrCSR(v);
		for (uint i = 0; i < out_neighbors.second; ++i)
		{
			uint vv = out_neighbors.first[i];
			if (arrSDu_s_[vv] == MAX_K)
			{
				arrSDu_s_[vv] = arrSDu_s_[v] + 1;
				if (arrSDu_s_[vv] < (iK_ + 1) / 2)
					q.push(vv);
			}
		}
	}
	q.push(iDst_);
	while (!q.empty())
	{
		uint v = q.front();
		q.pop();
		auto in_neighbors = oGraph_->GetInNbrCSR(v);
		for (uint i = 0; i < in_neighbors.second; ++i)
		{
			uint vv = in_neighbors.first[i];
			if (arrSDu_t_[vv] == MAX_K)
			{
				arrSDu_t_[vv] = arrSDu_t_[v] + 1;
				if (arrSDu_t_[vv] < iK_ / 2)
					q.push(vv);
			}
		}
	}
	arrVisited_[iDst_] = true;
	q.push(iDst_);
	while (!q.empty())
	{
		uint v = q.front();
		q.pop();
		auto in_neighbors = oGraph_->GetInNbrCSR(v);
		for (uint i = 0; i < in_neighbors.second; ++i)
		{
			uint vv = in_neighbors.first[i];
			if ((arrSDu_s_[vv] != MAX_K || arrSDu_t_[vv] != MAX_K) && !arrVisited_[vv])
			{
				arrVisited_[vv] = true;
				arrSDu_t_[vv] = arrSDu_t_[v] + 1;
				setInduce.insert(vv);
				if (arrSDu_t_[vv] < iK_ - 1)
					q.push(vv);
			}
		}
	}
	setInduce.insert(iSrc_);
	setInduce.insert(iDst_);

	auto end = std::chrono::high_resolution_clock::now();
	std::vector<std::pair<uint, uint>> edge_list;
	for (uint s : setInduce)
	{
		if (s != iDst_)
		{
			auto out_neighbors = oGraph_->GetOutNbrCSR(s);
			for (uint i = 0; i < out_neighbors.second; ++i)
			{
				uint t = out_neighbors.first[i];
				if (setInduce.find(t) != setInduce.end())
				{
					edge_list.emplace_back(s, t);
				}
			}
		}
	}
	if (sds_order)
	{
		if (sdt_order)
			oSubGraph_->CreateSubGraphCSR(edge_list, arrSDu_s_, arrSDu_t_, true, time);
		else
			oSubGraph_->CreateSubGraphCSR(edge_list, arrSDu_s_, nullptr, true, time);
	}
	else
	{
		if (sdt_order)
			oSubGraph_->CreateSubGraphCSR(edge_list, nullptr, arrSDu_t_, true, time);
		else
			oSubGraph_->CreateSubGraphCSR(edge_list, nullptr, nullptr, true, time);
	}

	time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void CHcPE::Pre_DouHalfHopBFS_1(bool sds_order, bool sdt_order, ulonglong &time)
{
	time = 0;
	for (uint i = 0; i <= oGraph_->iMaxVerId(); i++)
	{
		arrSDu_t_[i] = MAX_K;
		arrSDu_s_[i] = MAX_K;
	}
	arrSDu_t_[iDst_] = 0;
	arrSDu_s_[iSrc_] = 0;

	ulonglong iSize = sizeof(bool) * oGraph_->iMaxVerId() + 1;
	bool *label = (bool *)malloc(iSize);
	memset(label, 0, iSize);
	bool *rlabel = (bool *)malloc(iSize);
	memset(rlabel, 0, iSize);

	spp::sparse_hash_set<uint> setInduce;
	std::queue<uint> hfnodes, hbnodes, bnodes;
	oSubGraph_->Init();

	auto start = std::chrono::high_resolution_clock::now();

	label[iSrc_] = true;
	hfnodes.push(iSrc_);
	int layer = 0;
	while (!hfnodes.empty())
	{
		int size = hfnodes.size();
		layer++;
		for (int i = 0; i < size; i++)
		{
			uint node = hfnodes.front();
			hfnodes.pop();
			auto out_neighbors = oGraph_->GetOutNbrCSR(node);
			for (uint i = 0; i < out_neighbors.second; ++i)
			{
				uint nbr = out_neighbors.first[i];
				if (!label[nbr])
				{
					label[nbr] = true;
					// arrSDu_s_[nbr] = layer;
					hfnodes.push(nbr);
				}
			}
		}
		if (layer >= ((iK_ + 1) / 2))
			break;
	}

	rlabel[iDst_] = true;
	hbnodes.push(iDst_);
	layer = 0;
	while (!hbnodes.empty())
	{
		int size = hbnodes.size();
		layer++;
		for (int i = 0; i < size; i++)
		{
			uint node = hbnodes.front();
			hbnodes.pop();
			auto in_neighbors = oGraph_->GetInNbrCSR(node);
			for (uint i = 0; i < in_neighbors.second; ++i)
			{
				uint nbr = in_neighbors.first[i];
				if (!rlabel[nbr])
				{
					rlabel[nbr] = true;
					// arrSDu_t_[nbr] = layer;
					hbnodes.push(nbr);
					// setInduce.insert(nbr);
				}
			}
		}
		if (layer >= (iK_ / 2))
			break;
	}

	bnodes.push(iDst_);
	layer = 0;
	while (!bnodes.empty())
	{
		int size = bnodes.size();
		layer++;
		for (int i = 0; i < size; i++)
		{
			uint node = bnodes.front();
			bnodes.pop();
			auto in_neighbors = oGraph_->GetInNbrCSR(node);
			for (uint i = 0; i < in_neighbors.second; ++i)
			{
				uint nbr = in_neighbors.first[i];
				if ((rlabel[nbr] || label[nbr]) && arrSDu_t_[nbr] == MAX_K)
				{
					arrSDu_t_[nbr] = layer;
					bnodes.push(nbr);
					setInduce.insert(nbr);
				}
			}
		}
		if (layer >= iK_ - 1)
			break;
	}
	setInduce.insert(iSrc_);
	setInduce.insert(iDst_);
	auto end = std::chrono::high_resolution_clock::now();

	std::vector<std::pair<uint, uint>> edge_list;
	for (uint s : setInduce)
	{
		if (s != iDst_)
		{
			auto out_neighbors = oGraph_->GetOutNbrCSR(s);
			for (uint i = 0; i < out_neighbors.second; ++i)
			{
				uint t = out_neighbors.first[i];
				if (setInduce.find(t) != setInduce.end())
				{
					edge_list.emplace_back(s, t);
				}
			}
		}
	}
	if (sds_order)
	{
		if (sdt_order)
			oSubGraph_->CreateSubGraphCSR(edge_list, arrSDu_s_, arrSDu_t_, true, time);
		else
			oSubGraph_->CreateSubGraphCSR(edge_list, arrSDu_s_, nullptr, true, time);
	}
	else
	{
		if (sdt_order)
			oSubGraph_->CreateSubGraphCSR(edge_list, nullptr, arrSDu_t_, true, time);
		else
			oSubGraph_->CreateSubGraphCSR(edge_list, nullptr, nullptr, true, time);
	}

	time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	delete label, rlabel;
}

/**
 * @brief 先正反向k/2跳求交得搜索范围，再用反向继续k/2 - 1跳建立sdut，直接得induced graph，sdut排序
 *
 * @param time
 */
void CHcPE::Pre_DouHalfHopAndSigHalfHopBFS(bool sds_order, bool sdt_order, ulonglong &time)
{
	time = 0;
	for (uint i = 0; i <= oGraph_->iMaxVerId(); i++)
	{
		arrSDu_t_[i] = MAX_K;
		arrSDu_s_[i] = MAX_K;
		arrVisited_[i] = false;
	}
	arrSDu_t_[iDst_] = 0;
	arrSDu_s_[iSrc_] = 0;

	spp::sparse_hash_set<uint> setInduce;
	std::queue<uint> q, q1;
	oSubGraph_->Init();

	auto start = std::chrono::high_resolution_clock::now();

	q.push(iSrc_);
	while (!q.empty())
	{
		uint v = q.front();
		q.pop();
		auto out_neighbors = oGraph_->GetOutNbrCSR(v);
		for (uint i = 0; i < out_neighbors.second; ++i)
		{
			uint vv = out_neighbors.first[i];
			if (arrSDu_s_[vv] == MAX_K)
			{
				arrSDu_s_[vv] = arrSDu_s_[v] + 1;
				if (arrSDu_s_[vv] < (iK_ + 1) / 2)
					q.push(vv);
			}
		}
	}
	q.push(iDst_);
	while (!q.empty())
	{
		uint v = q.front();
		q.pop();
		auto in_neighbors = oGraph_->GetInNbrCSR(v);
		for (uint i = 0; i < in_neighbors.second; ++i)
		{
			uint vv = in_neighbors.first[i];
			if (arrSDu_t_[vv] == MAX_K)
			{
				arrSDu_t_[vv] = arrSDu_t_[v] + 1;
				if (arrSDu_t_[vv] < iK_ / 2)
					q.push(vv);
				else
					q1.push(vv);
				setInduce.insert(vv);
			}
		}
	}
	while (!q1.empty())
	{
		uint v = q1.front();
		q1.pop();
		auto in_neighbors = oGraph_->GetInNbrCSR(v);
		for (uint i = 0; i < in_neighbors.second; ++i)
		{
			uint vv = in_neighbors.first[i];
			if (arrSDu_s_[vv] != MAX_K && arrSDu_t_[vv] == MAX_K)
			{
				arrSDu_t_[vv] = arrSDu_t_[v] + 1;
				setInduce.insert(vv);
				if (arrSDu_t_[vv] < iK_ - 1)
					q.push(vv);
			}
		}
	}
	setInduce.insert(iSrc_);
	setInduce.insert(iDst_);

	auto end = std::chrono::high_resolution_clock::now();

	std::vector<std::pair<uint, uint>> edge_list;
	for (uint s : setInduce)
	{
		if (s != iDst_)
		{
			auto out_neighbors = oGraph_->GetOutNbrCSR(s);
			for (uint i = 0; i < out_neighbors.second; ++i)
			{
				uint t = out_neighbors.first[i];
				if (setInduce.find(t) != setInduce.end())
				{
					edge_list.emplace_back(s, t);
				}
			}
		}
	}
	if (sds_order)
	{
		if (sdt_order)
			oSubGraph_->CreateSubGraphCSR(edge_list, arrSDu_s_, arrSDu_t_, true, time);
		else
			oSubGraph_->CreateSubGraphCSR(edge_list, arrSDu_s_, nullptr, true, time);
	}
	else
	{
		if (sdt_order)
			oSubGraph_->CreateSubGraphCSR(edge_list, nullptr, arrSDu_t_, true, time);
		else
			oSubGraph_->CreateSubGraphCSR(edge_list, nullptr, nullptr, true, time);
	}

	time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void CHcPE::Pre_DouHalfHopAndSigHalfHopBFS_1(bool sds_order, bool sdt_order, ulonglong &time)
{
	time = 0;
	for (uint i = 0; i <= oGraph_->iMaxVerId(); i++)
	{
		arrSDu_t_[i] = MAX_K;
		arrSDu_s_[i] = MAX_K;
	}
	arrSDu_t_[iDst_] = 0;
	arrSDu_s_[iSrc_] = 0;

	ulonglong iSize = sizeof(bool) * oGraph_->iMaxVerId() + 1;
	bool *label = (bool *)malloc(iSize);
	memset(label, 0, iSize);
	bool *rlabel = (bool *)malloc(iSize);
	memset(rlabel, 0, iSize);

	spp::sparse_hash_set<uint> setInduce;
	std::queue<uint> hfnodes, hbnodes, bnodes;
	oSubGraph_->Init();

	auto start = std::chrono::high_resolution_clock::now();

	label[iSrc_] = true;
	hfnodes.push(iSrc_);
	int layer = 0;
	while (!hfnodes.empty())
	{
		int size = hfnodes.size();
		layer++;
		for (int i = 0; i < size; i++)
		{
			uint node = hfnodes.front();
			hfnodes.pop();
			auto out_neighbors = oGraph_->GetOutNbrCSR(node);
			for (uint i = 0; i < out_neighbors.second; ++i)
			{
				uint nbr = out_neighbors.first[i];
				if (!label[nbr])
				{
					label[nbr] = true;
					// arrSDu_s_[nbr] = layer;
					hfnodes.push(nbr);
				}
			}
		}
		if (layer >= ((iK_ + 1) / 2))
			break;
	}

	rlabel[iDst_] = true;
	hbnodes.push(iDst_);
	layer = 0;
	while (!hbnodes.empty())
	{
		int size = hbnodes.size();
		layer++;
		for (int i = 0; i < size; i++)
		{
			uint node = hbnodes.front();
			hbnodes.pop();
			auto in_neighbors = oGraph_->GetInNbrCSR(node);
			for (uint i = 0; i < in_neighbors.second; ++i)
			{
				uint nbr = in_neighbors.first[i];
				if (!rlabel[nbr])
				{
					rlabel[nbr] = true;
					arrSDu_t_[nbr] = layer;
					hbnodes.push(nbr);
					setInduce.insert(nbr);
				}
			}
		}
		if (layer >= (iK_ / 2))
			break;
	}

	while (!hbnodes.empty())
	{
		int size = hbnodes.size();
		layer++;
		for (int i = 0; i < size; i++)
		{
			uint node = hbnodes.front();
			hbnodes.pop();
			auto in_neighbors = oGraph_->GetInNbrCSR(node);
			for (uint i = 0; i < in_neighbors.second; ++i)
			{
				uint nbr = in_neighbors.first[i];
				if (label[nbr] && arrSDu_t_[nbr] == MAX_K)
				{
					arrSDu_t_[nbr] = layer;
					hbnodes.push(nbr);
					setInduce.insert(nbr);
				}
			}
		}
		if (layer >= iK_ - 1)
			break;
	}
	setInduce.insert(iSrc_);
	setInduce.insert(iDst_);
	auto end = std::chrono::high_resolution_clock::now();

	std::vector<std::pair<uint, uint>> edge_list;
	for (uint s : setInduce)
	{
		if (s != iDst_)
		{
			auto out_neighbors = oGraph_->GetOutNbrCSR(s);
			for (uint i = 0; i < out_neighbors.second; ++i)
			{
				uint t = out_neighbors.first[i];
				if (setInduce.find(t) != setInduce.end())
				{
					edge_list.emplace_back(s, t);
				}
			}
		}
	}
	if (sds_order)
	{
		if (sdt_order)
			oSubGraph_->CreateSubGraphCSR(edge_list, arrSDu_s_, arrSDu_t_, true, time);
		else
			oSubGraph_->CreateSubGraphCSR(edge_list, arrSDu_s_, nullptr, true, time);
	}
	else
	{
		if (sdt_order)
			oSubGraph_->CreateSubGraphCSR(edge_list, nullptr, arrSDu_t_, true, time);
		else
			oSubGraph_->CreateSubGraphCSR(edge_list, nullptr, nullptr, true, time);
	}

	time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	delete label, rlabel;
}

// todo 减少后一半bfs，看看时间快多少，虽然子图变大了
void CHcPE::Pre_DouHalfHopAndSigHalfHopBFS_2(bool sds_order, bool sdt_order, ulonglong &time)
{
	time = 0;
	for (uint i = 0; i <= oGraph_->iMaxVerId(); i++)
	{
		arrSDu_t_[i] = MAX_K;
		arrSDu_s_[i] = MAX_K;
	}
	arrSDu_t_[iDst_] = 0;
	arrSDu_s_[iSrc_] = 0;

	spp::sparse_hash_set<uint> setInduce;
	std::queue<uint> hfnodes, hbnodes;
	oSubGraph_->Init();

	auto start = std::chrono::high_resolution_clock::now();

	hfnodes.push(iSrc_);
	int layer = 0;
	while (!hfnodes.empty())
	{
		int size = hfnodes.size();
		layer++;
		for (int i = 0; i < size; i++)
		{
			uint node = hfnodes.front();
			hfnodes.pop();
			auto out_neighbors = oGraph_->GetOutNbrCSR(node);
			for (uint i = 0; i < out_neighbors.second; ++i)
			{
				uint nbr = out_neighbors.first[i];
				if (arrSDu_s_[nbr] == MAX_K)
				{
					arrSDu_s_[nbr] = layer;
					hfnodes.push(nbr);
				}
			}
		}
		if (layer >= ((iK_ + 1) / 2))
			break;
	}

	bool has_com = false;
	hbnodes.push(iDst_);
	layer = 0;
	while (!hbnodes.empty())
	{
		int size = hbnodes.size();
		layer++;
		for (int i = 0; i < size; i++)
		{
			uint node = hbnodes.front();
			hbnodes.pop();
			auto in_neighbors = oGraph_->GetInNbrCSR(node);
			for (uint i = 0; i < in_neighbors.second; ++i)
			{
				uint nbr = in_neighbors.first[i];
				if (arrSDu_t_[nbr] == MAX_K)
				{
					arrSDu_t_[nbr] = layer;
					hbnodes.push(nbr);
					setInduce.insert(nbr);
					if (arrSDu_s_[nbr] != MAX_K) // ! 创新点1
					{
						has_com = true;
					}
				}
			}
		}
		if (layer >= (iK_ / 2))
			break;
	}

	auto end = std::chrono::high_resolution_clock::now();
	if (!has_com) // ! 创新点3
	{
		time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		return;
	}

	layer = (iK_ + 1) / 2;
	while (!hfnodes.empty())
	{
		int size = hfnodes.size();
		layer++;
		for (int i = 0; i < size; i++)
		{
			uint node = hfnodes.front();
			hfnodes.pop();
			auto out_neighbors = oGraph_->GetOutNbrCSR(node);
			for (uint i = 0; i < out_neighbors.second; ++i)
			{
				uint nbr = out_neighbors.first[i];
				if (arrSDu_t_[nbr] + layer <= iK_ && arrSDu_s_[nbr] == MAX_K) // arrSDu_t_[nbr] != MAX_K && arrSDu_s_[nbr] == MAX_K // ! 创新点2
				{
					arrSDu_s_[nbr] = layer;
					hfnodes.push(nbr);
					setInduce.insert(nbr);
				}
			}
		}
		if (layer >= (iK_ - 1))
			break;
	}

	layer = iK_ / 2;
	while (!hbnodes.empty())
	{
		int size = hbnodes.size();
		layer++;
		for (int i = 0; i < size; i++)
		{
			uint node = hbnodes.front();
			hbnodes.pop();
			auto in_neighbors = oGraph_->GetInNbrCSR(node);
			for (uint i = 0; i < in_neighbors.second; ++i)
			{
				uint nbr = in_neighbors.first[i];
				if (arrSDu_s_[nbr] + layer <= iK_ && arrSDu_t_[nbr] == MAX_K) // arrSDu_s_[nbr] != MAX_K && arrSDu_t_[nbr] == MAX_K // ! 创新点2
				{
					arrSDu_t_[nbr] = layer;
					hbnodes.push(nbr);
					setInduce.insert(nbr);
				}
			}
		}
		if (layer >= iK_ - 1)
			break;
	}
	setInduce.insert(iSrc_);
	setInduce.insert(iDst_);

	end = std::chrono::high_resolution_clock::now();

	std::vector<std::pair<uint, uint>> edge_list;
	for (uint s : setInduce)
	{
		if (s != iDst_)
		{
			auto out_neighbors = oGraph_->GetOutNbrCSR(s);
			for (uint i = 0; i < out_neighbors.second; ++i)
			{
				uint t = out_neighbors.first[i];
				if (setInduce.find(t) != setInduce.end())
				{
					edge_list.emplace_back(s, t);
				}
			}
		}
	}
	if (sds_order)
	{
		if (sdt_order)
			oSubGraph_->CreateSubGraphCSR(edge_list, arrSDu_s_, arrSDu_t_, true, time);
		else
			oSubGraph_->CreateSubGraphCSR(edge_list, arrSDu_s_, nullptr, true, time);
	}
	else
	{
		if (sdt_order)
			oSubGraph_->CreateSubGraphCSR(edge_list, nullptr, arrSDu_t_, true, time);
		else
			oSubGraph_->CreateSubGraphCSR(edge_list, nullptr, nullptr, true, time);
	}

	time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

/**
 * @brief 先正反向k/2跳求交得搜索范围，再从中间继续向正反向搜索到k-1跳建立sdus、sdut，交集得induced graph，sdut排序，无反向图
 *
 * @param time
 */
void CHcPE::Pre_DouHalfHopAndDouHalfHopBFS(bool sds_order, bool sdt_order, ulonglong &time)
{
	time = 0;
	for (uint i = 0; i <= oGraph_->iMaxVerId(); i++)
	{
		arrSDu_t_[i] = MAX_K;
		arrSDu_s_[i] = MAX_K;
	}
	arrSDu_t_[iDst_] = 0;
	arrSDu_s_[iSrc_] = 0;

	spp::sparse_hash_set<uint> setInduce;
	std::queue<uint> hfnodes, hbnodes;
	oSubGraph_->Init();

	auto start = std::chrono::high_resolution_clock::now();

	hfnodes.push(iSrc_);
	int layer = 0;
	while (!hfnodes.empty())
	{
		int size = hfnodes.size();
		layer++;
		for (int i = 0; i < size; i++)
		{
			uint node = hfnodes.front();
			hfnodes.pop();
			auto out_neighbors = oGraph_->GetOutNbrCSR(node);
			for (uint i = 0; i < out_neighbors.second; ++i)
			{
				uint nbr = out_neighbors.first[i];
				if (arrSDu_s_[nbr] == MAX_K)
				{
					arrSDu_s_[nbr] = layer;
					hfnodes.push(nbr);
				}
			}
		}
		if (layer >= ((iK_ + 1) / 2))
			break;
	}

	hbnodes.push(iDst_);
	layer = 0;
	while (!hbnodes.empty())
	{
		int size = hbnodes.size();
		layer++;
		for (int i = 0; i < size; i++)
		{
			uint node = hbnodes.front();
			hbnodes.pop();
			auto in_neighbors = oGraph_->GetInNbrCSR(node);
			for (uint i = 0; i < in_neighbors.second; ++i)
			{
				uint nbr = in_neighbors.first[i];
				if (arrSDu_t_[nbr] == MAX_K)
				{
					arrSDu_t_[nbr] = layer;
					hbnodes.push(nbr);
					if (arrSDu_s_[nbr] != MAX_K) // ! 创新点1
					{
						setInduce.insert(nbr);
					}
				}
			}
		}
		if (layer >= (iK_ / 2))
			break;
	}

	auto end = std::chrono::high_resolution_clock::now();
	if (setInduce.empty()) // ! 创新点3
	{
		time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		return;
	}

	layer = (iK_ + 1) / 2;
	while (!hfnodes.empty())
	{
		int size = hfnodes.size();
		layer++;
		for (int i = 0; i < size; i++)
		{
			uint node = hfnodes.front();
			hfnodes.pop();
			auto out_neighbors = oGraph_->GetOutNbrCSR(node);
			for (uint i = 0; i < out_neighbors.second; ++i)
			{
				uint nbr = out_neighbors.first[i];
				if (arrSDu_t_[nbr] + layer <= iK_ && arrSDu_s_[nbr] == MAX_K) // arrSDu_t_[nbr] != MAX_K && arrSDu_s_[nbr] == MAX_K // ! 创新点2
				{
					arrSDu_s_[nbr] = layer;
					hfnodes.push(nbr);
					setInduce.insert(nbr);
				}
			}
		}
		if (layer >= (iK_ - 1))
			break;
	}

	layer = iK_ / 2;
	while (!hbnodes.empty())
	{
		int size = hbnodes.size();
		layer++;
		for (int i = 0; i < size; i++)
		{
			uint node = hbnodes.front();
			hbnodes.pop();
			auto in_neighbors = oGraph_->GetInNbrCSR(node);
			for (uint i = 0; i < in_neighbors.second; ++i)
			{
				uint nbr = in_neighbors.first[i];
				if (arrSDu_s_[nbr] + layer <= iK_ && arrSDu_t_[nbr] == MAX_K) // arrSDu_s_[nbr] != MAX_K && arrSDu_t_[nbr] == MAX_K // ! 创新点2
				{
					arrSDu_t_[nbr] = layer;
					hbnodes.push(nbr);
					setInduce.insert(nbr);
				}
			}
		}
		if (layer >= iK_ - 1)
			break;
	}
	setInduce.insert(iSrc_);
	setInduce.insert(iDst_);

	end = std::chrono::high_resolution_clock::now();
	time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

	ulonglong gtime = 0;
	std::vector<std::pair<uint, uint>> edge_list;
	for (uint s : setInduce)
	{
		if (s != iDst_)
		{
			auto out_neighbors = oGraph_->GetOutNbrCSR(s);
			for (uint i = 0; i < out_neighbors.second; ++i)
			{
				uint t = out_neighbors.first[i];
				if (setInduce.find(t) != setInduce.end())
				{
					edge_list.emplace_back(s, t);
				}
			}
		}
	}
	if (sds_order)
	{
		if (sdt_order)
			oSubGraph_->CreateSubGraphCSR(edge_list, arrSDu_s_, arrSDu_t_, true, gtime);
		else
			oSubGraph_->CreateSubGraphCSR(edge_list, arrSDu_s_, nullptr, true, gtime);
	}
	else
	{
		if (sdt_order)
			oSubGraph_->CreateSubGraphCSR(edge_list, nullptr, arrSDu_t_, true, gtime);
		else
			oSubGraph_->CreateSubGraphCSR(edge_list, nullptr, nullptr, true, gtime);
	}
	// time += gtime;
}

/**
 * @brief 添加图排序
 *
 * @param sds_order
 * @param sdt_order
 * @param nodemap 排序后的id
 * @param time
 */
/**
 * @brief 先正反向k/2跳求交得搜索范围，再从中间继续向正反向搜索到k-1跳建立sdus、sdut，交集得induced graph，sdut排序，无反向图

 * @brief 4参数版本：包含剪枝 + 重编号 + 排序
 */
void CHcPE::Pre_DouHalfHopAndDouHalfHopBFS(bool sds_order, bool sdt_order, std::unordered_map<uint, uint> &nodemap, ulonglong &time)
{
	time = 0;
	// 1. 初始化距离数组
	for (uint i = 0; i <= oGraph_->iMaxVerId(); i++)
	{
		arrSDu_t_[i] = MAX_K;
		arrSDu_s_[i] = MAX_K;
	}
	arrSDu_t_[iDst_] = 0;
	arrSDu_s_[iSrc_] = 0;

	spp::sparse_hash_set<uint> setInduce;
	std::queue<uint> hfnodes, hbnodes;
	oSubGraph_->Init();

	uint test_c = 0;
	auto start = std::chrono::high_resolution_clock::now();

	// 2. 正向 BFS (From Source)
	hfnodes.push(iSrc_);
	int layer = 0;
	while (!hfnodes.empty())
	{
		int size = hfnodes.size();
		layer++;
		for (int i = 0; i < size; i++)
		{
			uint node = hfnodes.front();
			hfnodes.pop();
			auto out_neighbors = oGraph_->GetOutNbrCSR(node);
			for (uint i = 0; i < out_neighbors.second; ++i)
			{
				test_c++;
				uint nbr = out_neighbors.first[i];
				if (arrSDu_s_[nbr] == MAX_K)
				{
					arrSDu_s_[nbr] = layer;
					hfnodes.push(nbr);
				}
			}
		}
		if (layer >= ((iK_ + 1) / 2))
			break;
	}

	// 3. 反向 BFS (From Target)
	hbnodes.push(iDst_);
	layer = 0;
	while (!hbnodes.empty())
	{
		int size = hbnodes.size();
		layer++;
		for (int i = 0; i < size; i++)
		{
			uint node = hbnodes.front();
			hbnodes.pop();
			auto in_neighbors = oGraph_->GetInNbrCSR(node);
			for (uint i = 0; i < in_neighbors.second; ++i)
			{
				test_c++;
				uint nbr = in_neighbors.first[i];
				if (arrSDu_t_[nbr] == MAX_K)
				{
					arrSDu_t_[nbr] = layer;
					hbnodes.push(nbr);
					if (arrSDu_s_[nbr] != MAX_K)
					{
						setInduce.insert(nbr);
					}
				}
			}
		}
		if (layer >= (iK_ / 2))
			break;
	}

	auto end = std::chrono::high_resolution_clock::now();
	if (setInduce.empty())
	{
		time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		return;
	}

	// 4. 中间向外扩展 BFS
	layer = (iK_ + 1) / 2;
	while (!hfnodes.empty())
	{
		int size = hfnodes.size();
		layer++;
		for (int i = 0; i < size; i++)
		{
			uint node = hfnodes.front();
			hfnodes.pop();
			auto out_neighbors = oGraph_->GetOutNbrCSR(node);
			for (uint i = 0; i < out_neighbors.second; ++i)
			{
				uint nbr = out_neighbors.first[i];
				if (arrSDu_t_[nbr] + layer <= iK_ && arrSDu_s_[nbr] == MAX_K)
				{
					arrSDu_s_[nbr] = layer;
					hfnodes.push(nbr);
					setInduce.insert(nbr);
				}
			}
		}
		if (layer >= (iK_ - 1))
			break;
	}

	layer = iK_ / 2;
	while (!hbnodes.empty())
	{
		int size = hbnodes.size();
		layer++;
		for (int i = 0; i < size; i++)
		{
			uint node = hbnodes.front();
			hbnodes.pop();
			auto in_neighbors = oGraph_->GetInNbrCSR(node);
			for (uint i = 0; i < in_neighbors.second; ++i)
			{
				uint nbr = in_neighbors.first[i];
				if (arrSDu_s_[nbr] + layer <= iK_ && arrSDu_t_[nbr] == MAX_K)
				{
					arrSDu_t_[nbr] = layer;
					hbnodes.push(nbr);
					setInduce.insert(nbr);
				}
			}
		}
		if (layer >= iK_ - 1)
			break;
	}
	setInduce.insert(iSrc_);
	setInduce.insert(iDst_);

	end = std::chrono::high_resolution_clock::now();
	time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	iBRAM2DDRTimes_ = test_c;

	ulonglong gtime = 0;
	std::vector<uint> bucketsort[iK_ + 1];
	std::vector<uint> indegree(oGraph_->iMaxVerId() + 1, 0);
	std::vector<ushort> sdut(setInduce.size() + 1, MAX_K);
	std::vector<ushort> sdus(setInduce.size() + 1, MAX_K);

	// 5. 生成边表 + 强力剪枝
		std::vector<std::pair<uint, uint>> edge_list;
		for (uint s : setInduce)
		{
			// [修复]：增加 (s != iSrc_ && s != iDst_) 判断
			// 原理：BFS 只搜到了 K-1 层，起点的 SDt 和终点的 SDs 很可能依然是 MAX_K。
			// 如果不跳过检查，起点和终点会被误认为是"距离无穷大"的点而被剪掉。
			if (s != iSrc_ && s != iDst_)
			{
				if (arrSDu_s_[s] + arrSDu_t_[s] > iK_) continue;
			}

			if (s != iDst_)
			{
				auto out_neighbors = oGraph_->GetOutNbrCSR(s);
				for (uint i = 0; i < out_neighbors.second; ++i)
				{
					uint t = out_neighbors.first[i];
					if (setInduce.find(t) != setInduce.end())
					{
						// 边的剪枝逻辑通常是安全的，因为 s->t 意味着 t 离终点更近一步，
						// 只要 Path <= K，t 的 SDt 通常在 K-1 范围内，是有效值。
						if (arrSDu_s_[s] + 1 + arrSDu_t_[t] <= iK_) {
							edge_list.emplace_back(s, t);
							indegree[t]++;
						}
					}
				}
			}

	        // 下面的 Bucket 加入逻辑同理，最好也保护一下或者放入上面的逻辑块中
			if (arrSDu_t_[s] <= iK_)
				bucketsort[arrSDu_t_[s]].emplace_back(s);
			else
	            // 如果起点的 SDt 是 MAX_K，把它放到最后一层桶里，或者特殊处理
	            // 这里放 K 桶通常没问题，只要前面没 continue 掉
				bucketsort[iK_].emplace_back(s);
		}

	start = std::chrono::high_resolution_clock::now();
	uint ind = 1;
	for (uint i = 0; i < iK_; i++)
	{
		std::sort(bucketsort[i].begin(), bucketsort[i].end(),
				  [&](const uint &left, const uint &right)
				  {
					  return indegree[left] > indegree[right];
				  });
	}
	end = std::chrono::high_resolution_clock::now();
	if (setInduce.size() > 10240)
		time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

	// 6. 生成重编号映射 nodemap
	for (uint i = 0; i < iK_; i++)
	{
		for (auto a : bucketsort[i])
		{
			nodemap.insert(std::pair<uint, uint>(a, ind));
			sdut[ind] = arrSDu_t_[a];
			sdus[ind] = arrSDu_s_[a];
			ind++;
		}
	}

	// 7. 生成重编号后的边表
	std::vector<std::pair<uint, uint>> n_edge_list;
	for (auto &e : edge_list)
	{
		// 注意：如果剪枝逻辑正确，edge_list 里的点一定都在 nodemap 里
		n_edge_list.emplace_back(std::pair<uint, uint>(nodemap[e.first], nodemap[e.second]));
	}
	// 8. 更新 arrSDu 为新 ID 的距离 (关键!)
	for (uint i = 0; i < sdut.size(); i++)
	{
		arrSDu_t_[i] = sdut[i];
		arrSDu_s_[i] = sdus[i];
	}



	// 10. 创建 CSR
	if (sds_order)
	{
		if (sdt_order)
			oSubGraph_->CreateSubGraphCSR2(n_edge_list, arrSDu_s_, arrSDu_t_, true, gtime);
		else
			oSubGraph_->CreateSubGraphCSR2(n_edge_list, arrSDu_s_, nullptr, true, gtime);
	}
	else
	{
		if (sdt_order)
			oSubGraph_->CreateSubGraphCSR2(n_edge_list, nullptr, arrSDu_t_, true, gtime);
		else
			oSubGraph_->CreateSubGraphCSR2(n_edge_list, nullptr, nullptr, true, gtime);
	}

	// time += gtime;
}

std::set<uint> CHcPE::find_hot_points(CDirectedGraph *&oGraph, double threshold)
{
	std::vector<hot_degree> hot_points;
	// double threhold = 0.05;
	// this means we find top 5% nodes in degree rank as hot-points(this part can change to k-core value, truess value or centrality)
	for (uint i = 0; i < oGraph->iMaxVerId() + 1; i++)
	{
		uint temp_degree = oGraph->GetInNbrNumCSR(i) + oGraph->GetInNbrNumCSR(i); // sum in and out degree
		if (temp_degree != 0)
		{
			hot_degree hd1(i, temp_degree);
			hot_points.push_back(hd1);
		}
	}
	stable_sort(hot_points.begin(), hot_points.end(), std::less<hot_degree>());
	uint end = (uint)(hot_points.size() * threshold);
	std::set<uint> result;
	for (uint i = 0; i < end; i++)
	{
		result.insert(hot_points[i].node);
	}
	return result;
}

std::set<uint> CHcPE::find_hot_points_top_t(CDirectedGraph *&oGraph, int t)
{
	std::vector<hot_degree> hot_points;
	for (uint i = 0; i < oGraph->iMaxVerId() + 1; i++)
	{
		uint temp_degree = oGraph->GetInNbrNumCSR(i) + oGraph->GetInNbrNumCSR(i); // sum in and out degree
		if (temp_degree != 0)
		{
			hot_degree hd1(i, temp_degree);
			hot_points.push_back(hd1);
		}
	}
	stable_sort(hot_points.begin(), hot_points.end(), std::less<hot_degree>());
	std::set<uint> result;
	for (uint i = 0; i < t; i++)
	{
		result.insert(hot_points[i].node);
	}
	return result;
}

path_index CHcPE::construct_hot_point_index_dfs_using_new_algorithm(std::unordered_map<uint, std::set<uint>> &induced_subgraph, ushort k, std::set<uint> hot_points)
{
	path_index index(hot_points);
	// use bfs to find some paths between node A and B
	for (auto start = hot_points.begin(); start != hot_points.end(); start++)
	{
		paths temp;
		std::map<uint, paths> result;
		std::vector<uint> c_path;
		std::set<uint> c_path_set;
		ushort min_stop_distance = k;
		std::set<uint> temp_stop_points(hot_points);
		for (auto iter = temp_stop_points.begin(); iter != temp_stop_points.end();)
		{
			if (*iter == *start)
			{
				iter = temp_stop_points.erase(iter);
			}
			else
			{
				iter++;
			}
		}
		meet_nodes = temp_stop_points;
		result = dfs_without_recursion_all_meetpoints(induced_subgraph, *start, 0, k); // !
		for (auto iter = result.begin(); iter != result.end(); iter++)
		{
			index.push_back(iter->second);
		}
	}
	return index;
}

void CHcPE::Pre_HPIndex(CDirectedGraph *&oGraph, ulonglong &index_time)
{
	index_time = 0;
	uint node_num = oGraph->iMaxVerId() + 1;
	adjacency_map_.clear();
	adjacency_map_reverse_.clear();
	hot_points_.clear();

	for (int i = 0; i < node_num; i++)
	{
		bool first_time = true;
		auto out_neighbors = oGraph->GetOutNbrCSR(i);
		for (uint j = 0; j < out_neighbors.second; ++j)
		{
			auto nbr = out_neighbors.first[j];
			if (first_time)
			{
				std::set<uint> temp_s;
				temp_s.insert(nbr);
				adjacency_map_.insert(std::make_pair(i, temp_s));
				first_time = false;
			}
			else
			{
				adjacency_map_.find(i)->second.insert(nbr);
			}
		}
	}
	for (int i = 0; i < node_num; i++)
	{
		bool first_time = true;
		auto in_neighbors = oGraph->GetInNbrCSR(i);
		for (uint j = 0; j < in_neighbors.second; ++j)
		{
			auto nbr = in_neighbors.first[j];
			if (first_time)
			{
				std::set<uint> temp_s;
				temp_s.insert(nbr);
				adjacency_map_reverse_.insert(std::make_pair(i, temp_s));
			}
			else
			{
				adjacency_map_reverse_.find(i)->second.insert(nbr);
			}
		}
	}

	auto index_start = std::chrono::high_resolution_clock::now();
	//  hot_points = find_hot_points(oGraph,  0.05);
	hot_points_ = find_hot_points_top_t(oGraph, iK_);
	index_ = construct_hot_point_index_dfs_using_new_algorithm(adjacency_map_, iK_, hot_points_);
	auto index_end = std::chrono::high_resolution_clock::now();
	index_time = std::chrono::duration_cast<std::chrono::nanoseconds>(index_end - index_start).count();
}

void CHcPE::construct_pruned_dag_min_subgraph(uint k, uint query_node1, uint query_node2, ulonglong &time)
{
	time = 0;

	meet_nodes.clear();
	src_distance.clear();
	dst_distance.clear();
	reverse_adjacency_in_subgraph_left.clear();
	reverse_adjacency_in_subgraph_right.clear();
	dag_min_induced_subgraph.clear();

	auto start = std::chrono::high_resolution_clock::now();
	// store the final induced subgraph into reverse_adjacency_in_subgraph_left
	src_distance.insert(std::make_pair(query_node1, 0));
	dst_distance.insert(std::make_pair(query_node2, 0));
	ushort cur_distance = 0;
	std::set<uint> left_proprogate;
	std::set<uint> right_proprogate;
	std::set<uint> node1_set;
	std::set<uint> node2_set;
	std::set<uint> left_visited_nodes;
	std::set<uint> right_visited_nodes;
	left_visited_nodes.insert(query_node1);
	right_visited_nodes.insert(query_node2);
	left_proprogate.insert(query_node1);
	right_proprogate.insert(query_node2);
	bool left_skip = false;
	bool right_skip = false;
	int left_max_distance = k;
	int right_max_distance = k;
	cur_distance = 0;
	while (true)
	{
		if (g_exit)
			return;
		cur_distance++;
		if (cur_distance <= left_max_distance && !left_proprogate.empty())
		{
			std::set<uint> temp_proprogate;
			uint cur_node;
			for (auto iter = left_proprogate.begin(); iter != left_proprogate.end(); iter++)
			{
				if (g_exit)
					return;
				cur_node = *iter;
				std::set<uint> temp_set;
				temp_set.insert(cur_node);
				auto out_neighbors = oGraph_->GetOutNbrCSR(cur_node);
				for (uint i = 0; i < out_neighbors.second; ++i)
				{
					if (g_exit)
						return;
					uint nbr = out_neighbors.first[i];
					auto iter_src_dis = src_distance.find(nbr);
					if (iter_src_dis == src_distance.end())
					{
						src_distance.insert(std::make_pair(nbr, cur_distance));
					}
					std::set<uint> temp_set2;
					temp_set2.insert(nbr);
					if (left_visited_nodes.find(nbr) != left_visited_nodes.end()) // already visited
					{
						if (dag_min_induced_subgraph.find(nbr) == dag_min_induced_subgraph.end()) // not in index subgraph
						{
							dag_min_induced_subgraph.insert(std::make_pair(nbr, temp_set));
						}
						else
						{
							dag_min_induced_subgraph.find(nbr)->second.insert(cur_node);
						}
						continue;
					}
					dag_min_induced_subgraph.insert(std::make_pair(nbr, temp_set));
					left_visited_nodes.insert(nbr);
					temp_proprogate.insert(nbr);
				}
			}
			left_proprogate = temp_proprogate;
		}
		else
		{
			break; // left_skip = true;
		}
	}
	cur_distance = 0;
	while (true)
	{
		if (g_exit)
			return;
		cur_distance++;
		if (cur_distance <= right_max_distance && !right_proprogate.empty()) // right propogate
		{
			std::set<uint> temp_proprogate;
			uint cur_node;
			for (auto iter = right_proprogate.begin(); iter != right_proprogate.end(); iter++)
			{
				if (g_exit)
					return;
				cur_node = *iter;
				std::set<uint> temp_set;
				temp_set.insert(cur_node);
				if (dag_min_induced_subgraph.find(cur_node) == dag_min_induced_subgraph.end())
				{
					continue;
				}
				auto iter_temp = dag_min_induced_subgraph.find(cur_node);
				for (auto iter2 = iter_temp->second.begin(); iter2 != iter_temp->second.end(); iter2++)
				{
					if (g_exit)
						return;
					if (right_visited_nodes.find(*iter2) != right_visited_nodes.end()) // already visited
					{
						if (src_distance.find(*iter2)->second + cur_distance <= k)
						{ // ready to insert this edge
							auto iter_dst_dis = dst_distance.find(*iter2);
							if (iter_dst_dis == dst_distance.end()) // no distance information for node *iter2
							{
								dst_distance.insert(std::make_pair(*iter2, cur_distance));
							}
							if (reverse_adjacency_in_subgraph_right.find(*iter2) == reverse_adjacency_in_subgraph_right.end()) // not in index subgraph
							{
								reverse_adjacency_in_subgraph_right.insert(std::make_pair(*iter2, temp_set));
							}
							else
							{
								reverse_adjacency_in_subgraph_right.find(*iter2)->second.insert(cur_node);
							}
							iSubGraphEdgeCount_++;
						}
						continue;
					}
					if (src_distance.find(*iter2)->second + cur_distance <= k)
					{ // ready to insert this edge
						auto iter_dst_dis = dst_distance.find(*iter2);
						if (iter_dst_dis == dst_distance.end()) // no distance information for node *iter2
						{
							dst_distance.insert(std::make_pair(*iter2, cur_distance));
						}
						reverse_adjacency_in_subgraph_right.insert(std::make_pair(*iter2, temp_set));
						iSubGraphEdgeCount_++;
					}
					right_visited_nodes.insert(*iter2);
					temp_proprogate.insert(*iter2);
				}
			}
			right_proprogate = temp_proprogate;
		}
		else
		{
			break; // right_skip = true;
		}
	}
	dag_min_induced_subgraph = reverse_adjacency_in_subgraph_right;
	auto end = std::chrono::high_resolution_clock::now();
	time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void CHcPE::Pre_BiGraphIDX(ulonglong &time)
{
	for (uint i = 0; i <= oGraph_->iMaxVerId(); i++)
	{
		arrVisited_[i] = false;
	}

	ulonglong num_edges = 0;
	uint num_vertices = oGraph_->iMaxVerId() + 1;
	auto k = static_cast<ushort>(iK_);

	std::queue<uint> q; // bfs队列

	/* Forward breadth-first search. k-1跳 */
	uint updated_values_count = 0; // Forward breadth-first search 激活的点数 对应updated_values_
	auto forward_bfs_start = std::chrono::high_resolution_clock::now();
	arrVisited_[iSrc_] = true;
	arrVisited_[iDst_] = true;
	updated_values_[updated_values_count++] = iSrc_;
	updated_values_[updated_values_count++] = iDst_;
	q.push(iSrc_);
	distance_[iSrc_].first = 0;
	while (!q.empty())
	{
		if (g_exit)
			return;
		uint v = q.front();
		q.pop();
		if (distance_[v].first < k - 1)
		{
			ushort next_distance = distance_[v].first + 1;
			auto out_neighbors = oGraph_->GetOutNbrCSR(v);
			for (uint i = 0; i < out_neighbors.second; ++i)
			{
				if (g_exit)
					return;
				uint vv = out_neighbors.first[i];
				if (!arrVisited_[vv])
				{
					arrVisited_[vv] = true;
					distance_[vv].first = next_distance;
					updated_values_[updated_values_count++] = vv;
					q.push(vv);
				}
			}
		}
	}
	auto forward_bfs_end = std::chrono::high_resolution_clock::now();
	// forward_bfs_time_ = std::chrono::duration_cast<std::chrono::nanoseconds>(forward_bfs_end - forward_bfs_start).count();

	// One cache line contains 64 boolean values and each AVX256 instruction can manipulate 32 boolean values.
	if (num_vertices / updated_values_count > 64 * 32)
	{
		for (uint i = 0; i < updated_values_count; ++i)
		{
			arrVisited_[updated_values_[i]] = false;
		}
	}
	else
	{
		memset(arrVisited_, 0, sizeof(bool) * num_vertices);
	}

	/* Backward breadth-first search. k-1跳 arrVisited_标记，active_vertices_count计数（除了t） */
	active_vertices_count_ = 0; // Backward breadth-first search 激活的点数
	std::vector<std::vector<uint>> temp_buckets((iK_ + 1) * (iK_ + 1));
	auto backward_bfs_start = std::chrono::high_resolution_clock::now();
	arrVisited_[iDst_] = true;
	arrVisited_[iSrc_] = true;
	q.push(iDst_);
	distance_[iDst_].second = 0;
	while (!q.empty())
	{
		if (g_exit)
			return;
		uint v = q.front();
		q.pop();

		if (distance_[v].second < k - 1)
		{
			uint next_distance = distance_[v].second + 1;
			auto in_neighbors = oGraph_->GetInNbrCSR(v);
			for (uint i = 0; i < in_neighbors.second; ++i)
			{
				if (g_exit)
					return;
				uint vv = in_neighbors.first[i];
				if (!arrVisited_[vv] && distance_[vv].first + next_distance <= k)
				{
					arrVisited_[vv] = true;
					distance_[vv].second = next_distance;
					q.push(vv);
					active_vertices_count_ += 1;

					// Put vertices into temp buckets based on the distance_ to src and dst respectively.
					uint bucket_id = BUCKET_ID(distance_[vv].first, distance_[vv].second, k + 1);
					temp_buckets[bucket_id].push_back(vv);
				}
			}
		}
	}
	auto backward_bfs_end = std::chrono::high_resolution_clock::now();
	// backward_bfs_time_ = std::chrono::duration_cast<std::chrono::nanoseconds>(backward_bfs_end - forward_bfs_end).count();

	/* Put vertices into different buckets. Include the src and dst vertex. */
	active_vertices_count_ += 1; // 加上s，没有t
	buckets_adj_ = (uint *)malloc(sizeof(uint) * active_vertices_count_);
	buckets_adj_[0] = iSrc_;
	uint offset = 1; // 0 是 source node
	for (uint i = 0; i < iK_ + 1; ++i)
	{
		for (uint j = 0; j < iK_ + 1; ++j)
		{
			uint bucket_id = BUCKET_ID(i, j, iK_ + 1);
			buckets_offset_[bucket_id] = offset; // 起始 offset
			memcpy(buckets_adj_ + offset, temp_buckets[bucket_id].data(), sizeof(uint) * temp_buckets[bucket_id].size());
			offset += temp_buckets[bucket_id].size();
			temp_buckets[bucket_id].clear();
		}
	}
	buckets_offset_[(iK_ + 1) * (iK_ + 1)] = offset;

	/* Construct the forward bipartite graph. */
	std::vector<uint> temp_bigraph_adj;
	temp_bigraph_adj.reserve(1024);
	std::vector<std::vector<uint>> temp_adj(iK_); // 临时的IDX
	single_bigraph_offset_ = (uint *)malloc(sizeof(uint) * iK_ * (active_vertices_count_ + 1));
	memset(single_bigraph_offset_, 0, sizeof(uint) * iK_ * (active_vertices_count_ + 1));

	uint cur_bucket_id = 0;										   // 实际是从1开始，0是source node
	uint cur_bucket_offset = buckets_offset_[cur_bucket_id + 1];   // 起始offset
	uint cur_bucket_max_degree_offset = cur_bucket_id * (iK_ + 1); // ? 什么来的

	for (uint i = 0; i < active_vertices_count_; ++i)
	{
		// Find the bucket id.
		if (i != 0)
		{
			while (i >= cur_bucket_offset)
			{
				cur_bucket_id += 1;
				cur_bucket_offset = buckets_offset_[cur_bucket_id + 1];
				cur_bucket_max_degree_offset = cur_bucket_id * (iK_ + 1);
			}
		}

		uint v = buckets_adj_[i];
		auto out_neighbors = oGraph_->GetOutNbrCSR(v);
		for (uint j = 0; j < out_neighbors.second; ++j)
		{
			uint vv = out_neighbors.first[j];
			if (vv == iDst_)
			{
				temp_adj[0].push_back(vv);
			}
			else if (arrVisited_[vv] && distance_[vv].second < k)
			{
				temp_adj[distance_[vv].second].push_back(vv);
			}
		}

		uint temp_offset = i * iK_;
		uint local_degree = 0;
		for (uint j = 0; j < iK_; ++j)
		{
			single_bigraph_offset_[temp_offset + j] = temp_bigraph_adj.size();
			temp_bigraph_adj.insert(temp_bigraph_adj.end(), temp_adj[j].begin(), temp_adj[j].end());
			local_degree += temp_adj[j].size();
			temp_adj[j].clear();

			// Collect statistics for the preliminary estimator.
			if (i != 0)
				bucket_degree_sum_[cur_bucket_max_degree_offset + j] += local_degree;
			else
				bucket_degree_sum_[j] = local_degree;
		}
		single_bigraph_offset_[temp_offset + iK_] = temp_bigraph_adj.size();
		single_bigraph_[v] = temp_offset;
	}
	single_bigraph_adj_size_ = temp_bigraph_adj.size();
	num_edges += single_bigraph_adj_size_;
	single_bigraph_adj_ = (uint *)malloc(sizeof(uint) * single_bigraph_adj_size_);
	memcpy(single_bigraph_adj_, temp_bigraph_adj.data(), sizeof(uint) * single_bigraph_adj_size_);
	temp_bigraph_adj.clear();

	/* Construct the backward bipartite graph. */
	single_reverse_bigraph_offset_ = (uint *)malloc(sizeof(uint) * iK_ * (active_vertices_count_ + 1));
	memset(single_reverse_bigraph_offset_, 0, sizeof(uint) * iK_ * (active_vertices_count_ + 1));
	buckets_adj_[0] = iDst_;
	for (uint i = 0; i < active_vertices_count_; ++i)
	{
		uint v = buckets_adj_[i];
		auto in_neighbors = oGraph_->GetInNbrCSR(v);
		for (uint j = 0; j < in_neighbors.second; ++j)
		{
			uint vv = in_neighbors.first[j];
			if (vv == iSrc_)
			{
				temp_adj[0].push_back(vv);
			}
			else if (arrVisited_[vv] && distance_[vv].first < iK_)
			{
				temp_adj[distance_[vv].first].push_back(vv);
			}
		}

		uint temp_offset = i * iK_;
		for (uint j = 0; j < iK_; ++j)
		{
			single_reverse_bigraph_offset_[temp_offset + j] = temp_bigraph_adj.size();
			temp_bigraph_adj.insert(temp_bigraph_adj.end(), temp_adj[j].begin(), temp_adj[j].end());
			temp_adj[j].clear();
		}
		single_reverse_bigraph_offset_[temp_offset + iK_] = temp_bigraph_adj.size();
		single_reverse_bigraph_[v] = temp_offset;
	}
	single_reverse_bigraph_adj_size_ = temp_bigraph_adj.size();
	num_edges += single_reverse_bigraph_adj_size_;
	single_reverse_bigraph_adj_ = (uint *)malloc(sizeof(uint) * single_reverse_bigraph_adj_size_);
	memcpy(single_reverse_bigraph_adj_, temp_bigraph_adj.data(), sizeof(uint) * single_reverse_bigraph_adj_size_);
	// temp_bigraph_adj.clear();

	// AVX512 清空arrVisited_与distance_
	if (num_vertices / updated_values_count > 16 * 8)
	{
		for (uint i = 0; i < updated_values_count; ++i)
		{
			arrVisited_[updated_values_[i]] = false;
			distance_[updated_values_[i]] = {k + 1, k + 1};
		}
	}
	else
	{
		memset(arrVisited_, 0, sizeof(bool) * (num_vertices + 1));
		memset((ushort *)distance_, k + 1, sizeof(std::pair<ushort, ushort>) * (num_vertices + 1));
	}

	auto construct_bigraph_end = std::chrono::high_resolution_clock::now();
	// construct_bigraph_time_ = std::chrono::duration_cast<std::chrono::nanoseconds>(construct_bigraph_end - backward_bfs_end).count();

	time = std::chrono::duration_cast<std::chrono::nanoseconds>(construct_bigraph_end - forward_bfs_start).count();

	/* Compute the memory cost of the index. */
	// calculated_memory_cost_ = num_edges;							  // Edges
	// calculated_memory_cost_ += (iK_ + 1) * active_vertices_count_ * 2; // Offset
	// calculated_memory_cost_ += active_vertices_count_ * 2 * 2;		  // Hash table
	// calculated_memory_cost_ *= sizeof(uint);

	// index_edge_count_ = num_edges;
	// index_vertex_count_ = num_vertices; // TODO
}

/**
 * @brief 对点重新编号，0是空号
 *
 * @param nodemap
 * @param itime
 */
void CHcPE::Pre_BiGraphIDX_Shrink(spp::sparse_hash_map<uint, uint> &nodemap, ulonglong &itime)
{
	if (single_bigraph_.empty())
		return;

	uint id = 1;
	nodemap.clear();

	auto iSize = single_bigraph_.size() + 1;
	single_bigraph_node_ = (uint *)malloc(sizeof(uint) * iSize);
	memset(single_bigraph_node_, 0, sizeof(uint) * iSize);
	single_reverse_bigraph_node_ = (uint *)malloc(sizeof(uint) * iSize);
	memset(single_reverse_bigraph_node_, 0, sizeof(uint) * iSize);

	auto start = std::chrono::high_resolution_clock::now();
	for (auto iter : single_bigraph_)
	{
		nodemap.insert(std::make_pair(iter.first, id));
		single_bigraph_node_[id] = iter.second;
		single_reverse_bigraph_node_[id] = single_reverse_bigraph_[iter.first];
		id++;
	}
	nodemap.insert(std::make_pair(iDst_, id++));

	single_bigraph_.clear();
	single_reverse_bigraph_.clear();
	for (auto i = 1; i <= active_vertices_count_; i++)
	{
		single_bigraph_[i] = single_bigraph_node_[i];
		single_reverse_bigraph_[i] = single_reverse_bigraph_node_[i];
	}

	// auto adj_size = _msize(single_bigraph_adj_) / sizeof(uint);
	for (auto i = 0; i < single_bigraph_adj_size_; i++)
	{
		single_bigraph_adj_[i] = nodemap[single_bigraph_adj_[i]];
	}

	for (auto i = 0; i < single_reverse_bigraph_adj_size_; i++)
	{
		single_reverse_bigraph_adj_[i] = nodemap[single_reverse_bigraph_adj_[i]];
	}

	auto end = std::chrono::high_resolution_clock::now();
	itime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void CHcPE::clear_bigraph()
{
	memset(bucket_degree_sum_, 0, sizeof(uint) * (iK_ + 1) * (iK_ + 1) * (iK_ + 1));
	memset(buckets_offset_, 0, sizeof(uint) * ((iK_ + 1) * (iK_ + 1) + 1));
	free(buckets_adj_);
	buckets_adj_ = nullptr;

	single_bigraph_.clear();
	free(single_bigraph_node_);
	single_bigraph_node_ = nullptr;
	free(single_bigraph_adj_);
	single_bigraph_adj_ = nullptr;
	free(single_bigraph_offset_);
	single_bigraph_offset_ = nullptr;
	single_bigraph_adj_size_ = 0;

	single_reverse_bigraph_.clear();
	free(single_reverse_bigraph_node_);
	single_reverse_bigraph_node_ = nullptr;
	free(single_reverse_bigraph_adj_);
	single_reverse_bigraph_adj_ = nullptr;
	free(single_reverse_bigraph_offset_);
	single_reverse_bigraph_offset_ = nullptr;
	single_reverse_bigraph_adj_size_ = 0;
}

bool CHcPE::preliminary_cardinality_estimator()
{
	auto estimation_start = std::chrono::high_resolution_clock::now();
	// the first bucket stores src.
	ulonglong iPreliminaryEstimatedResultCount_ = bucket_degree_sum_[iK_ - 1];
	for (uint i = 1; i < iK_; ++i)
	{
		uint degree_sum = 1;
		uint vertex_sum = 1;
		for (uint j = 1; j <= i; ++j)
		{
			for (uint k = 1; k <= iK_ - i; ++k)
			{
				uint budget = iK_ - i - 1;
				uint cur_degree = bucket_degree_sum_[BUCKET_ID(j, k, iK_ + 1) * (iK_ + 1) + budget];
				uint cur_vertex = buckets_offset_[BUCKET_ID(j, k, iK_ + 1) + 1] - buckets_offset_[BUCKET_ID(j, k, iK_ + 1)];
				degree_sum += cur_degree;
				vertex_sum += cur_vertex;
			}
		}

		iPreliminaryEstimatedResultCount_ *= (degree_sum / vertex_sum) > 1 ? (degree_sum / vertex_sum) : 1;
	}

	auto estimation_end = std::chrono::high_resolution_clock::now();
	ulonglong preliminary_estimation_time_ = std::chrono::duration_cast<std::chrono::nanoseconds>(estimation_end - estimation_start).count();

	// If the number of estimated results > 100000, then invoke the fine grained optimizer. Our experiment results
	// show that this estimation generally underestimates the number of results.
	return iPreliminaryEstimatedResultCount_ > 100000;
}

//*****************************************************************************************************************
//*****************************************************************************************************************

void CHcPE::CDFSRecWithCSR(CDirectedGraph *&oGraph, const uint &u, MyVec &vTempPath)
{
	auto iLayer = vTempPath.size();
	vTempPath.push_back(u);
	arrVisited_[u] = true;
	if (u == iDst_)
	{
		iPathCount_++;
		if (!bCountOnly_)
			Paths_.push_back(vTempPath);
	}
	else if (iLayer < iK_)
	{
		auto out_neighbors = oGraph->GetOutNbrCSR(u);
		for (uint i = 0; i < out_neighbors.second; ++i)
		{
			if (g_exit)
				return;
			auto nbr = out_neighbors.first[i];
			if (!arrVisited_[nbr])
			{
				CDFSRecWithCSR(oGraph, nbr, vTempPath);
			}
		}
	}
	vTempPath.pop_back();
	arrVisited_[u] = false;
}

void CHcPE::CDFSPathEnumCSR(CDirectedGraph *&oGraph, ulonglong &time)
{
	for (uint i = 0; i <= oGraph->iMaxVerId(); i++)
	{
		arrVisited_[i] = false;
	}

	iPathCount_ = 0;
	MyVec tempath;
	auto start = std::chrono::high_resolution_clock::now();
	CDFSRecWithCSR(oGraph, iSrc_, tempath);
	auto end = std::chrono::high_resolution_clock::now();
	time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void CHcPE::TDFSRecWithCSR(CDirectedGraph *&oGraph, uint u, MyVec &vTempPath)
{
	auto iLayer = vTempPath.size();
	vTempPath.push_back(u);
	arrVisited_[u] = true;
	if (u == iDst_)
	{
		iPathCount_++;
		if (!bCountOnly_)
			Paths_.push_back(vTempPath);
	}
	else
	{
		auto sdt_map = khopBackwardBFSWithBlock(iDst_, iK_ - iLayer); // ! 创新点
		// auto sdt_map = khopBackwardBFSWithBlock(iDst_, iK_);
		auto out_neighbors = oGraph->GetOutNbrCSR(u);
		for (uint i = 0; i < out_neighbors.second; ++i)
		{
			if (g_exit)
				return;
			auto nbr = out_neighbors.first[i];
			if (!arrVisited_[nbr] && sdt_map.find(nbr) != sdt_map.end())
			{
				if (iLayer + sdt_map[nbr] + 1 <= iK_)
					TDFSRecWithCSR(oGraph, nbr, vTempPath);
			}
		}
	}
	vTempPath.pop_back();
	arrVisited_[u] = false;
}

void CHcPE::TDFSPathEnumCSR(CDirectedGraph *&oGraph, ulonglong &time)
{
	for (uint i = 0; i <= oGraph->iMaxVerId(); i++)
	{
		arrVisited_[i] = false;
	}

	iPathCount_ = 0;
	MyVec tempath;
	auto start = std::chrono::high_resolution_clock::now();
	TDFSRecWithCSR(oGraph, iSrc_, tempath);
	auto end = std::chrono::high_resolution_clock::now();
	time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void CHcPE::TDFS2RecWithCSR(CDirectedGraph *&oGraph, uint u, MyVec vTempPath, spp::sparse_hash_set<uint> visited)
{
	auto iLayer = vTempPath.size();
	vTempPath.push_back(u);
	visited.insert(u);
	if (u == iDst_)
	{
		iPathCount_++;
		if (!bCountOnly_)
			Paths_.push_back(vTempPath);
	}
	else
	{
		auto sdt_map = khopBackwardBFSWithBlock(iDst_, iK_ - iLayer, visited); // ! 创新点
		// auto sdt_map = khopBackwardBFSWithBlock(iDst_, iK_, visited);
		std::set<uint> good_nbr;
		uint temp_node;
		while (true)
		{
			if (iLayer >= iK_)
				break;
			good_nbr.clear();
			auto out_neighbors = oGraph->GetOutNbrCSR(u);
			for (uint i = 0; i < out_neighbors.second; ++i)
			{
				if (g_exit)
					return;
				auto nbr = out_neighbors.first[i];
				if (visited.find(nbr) == visited.end() && sdt_map.find(nbr) != sdt_map.end())
				{
					if (iLayer + sdt_map[nbr] + 1 <= iK_)
					{
						if (nbr == iDst_)
							TDFS2RecWithCSR(oGraph, nbr, vTempPath, visited);
						else
						{
							good_nbr.insert(nbr);
							temp_node = nbr;
						}
					}
				}
			}
			if (good_nbr.size() > 1)
			{
				for (auto iter_neighbor = good_nbr.begin(); iter_neighbor != good_nbr.end(); iter_neighbor++)
				{
					TDFS2RecWithCSR(oGraph, *iter_neighbor, vTempPath, visited);
				}
				break;
			}
			else if (good_nbr.empty())
			{
				break;
			}
			else
			{
				u = temp_node;
				vTempPath.push_back(u);
				visited.insert(u);
				iLayer++;
			}
		}
	}
	// vTempPath.pop_back();
	// visited.erase(u);
}

void CHcPE::TDFS2PathEnumCSR(CDirectedGraph *&oGraph, ulonglong &time)
{
	iPathCount_ = 0;
	MyVec tempath;
	spp::sparse_hash_set<uint> visited;
	auto start = std::chrono::high_resolution_clock::now();
	TDFS2RecWithCSR(oGraph, iSrc_, tempath, visited);
	auto end = std::chrono::high_resolution_clock::now();
	time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

paths CHcPE::HPIndexPathEnumCSR(CDirectedGraph *&oGraph, ulonglong &query_time)
{
	query_time = 0;
	ulonglong update_time = 0;

	auto query_start = std::chrono::high_resolution_clock::now();
	ushort min_stop_distance = iK_;
	ushort right_stop_distance = iK_;
	std::map<uint, paths> left_dfs;
	std::map<uint, paths> right_dfs;
	std::set<uint> stop_points(hot_points_);
	stop_points.insert(iDst_);
	meet_nodes = stop_points;
	left_dfs = dfs_without_recursion_all_meetpoints(adjacency_map_, iSrc_, 0, iK_); // ! 0这个情况要改改
	paths paths_without_hot;
	auto iter_left = left_dfs.find(iDst_);
	if (iter_left != left_dfs.end())
	{
		if (iter_left->second.path.size() - 1 < min_stop_distance)
		{
			min_stop_distance = iter_left->second.path.size() - 1;
		}
		paths_without_hot.add_paths(iter_left->second);
	}
	if (paths_without_hot.path.size() == 0)
	{
		return paths_without_hot;
	}
	std::set<uint> right_stop_points(hot_points_);
	right_stop_points.insert(iSrc_);
	meet_nodes = right_stop_points;
	right_dfs = dfs_without_recursion_all_meetpoints(adjacency_map_reverse_, iDst_, 0, iK_ - min_stop_distance); // ! 0这个情况要改改

	// to find hot_paths, judge whether there is a hot path first
	paths hot_paths;
	uint left_node, right_node, temp_left_distance, temp_right_distance;
	paths index_new_paths;
	for (auto iter1 = left_dfs.begin(); iter1 != left_dfs.end(); iter1++)
	{
		// we should remove iDst_ from left_dfs hot_points to find paths_bt_hot_points
		if (iter1->first == iDst_)
		{
			continue;
		}
		for (auto iter2 = right_dfs.begin(); iter2 != right_dfs.end(); iter2++)
		{
			paths temp_result;
			paths paths_bt_hot_points;
			paths temp_left;
			paths temp_right;
			left_node = iter1->first;
			right_node = iter2->first;
			temp_left = iter1->second;
			temp_right = iter2->second;
			temp_right.reverse();
			if (left_node == right_node)
			{
				temp_result = temp_left.join(paths_bt_hot_points, iK_);
				temp_result = temp_result.join(temp_right, iK_);

				hot_paths.add_paths(temp_result);
				continue;
			}
			temp_left_distance = temp_left.get_min_distance();
			temp_right_distance = temp_right.get_min_distance();
			cur_path c_path;
			index_.find_paths_between_two_hot_nodes_index_without_cross_other_hotpoints(paths_bt_hot_points, c_path, left_node, right_node, (iK_ - temp_left_distance - temp_right_distance), 0);
			temp_result = temp_left.join(paths_bt_hot_points, iK_);
			temp_result = temp_result.join(temp_right, iK_);
			hot_paths.add_paths(temp_result);

			auto start = std::chrono::high_resolution_clock::now();
			// update the index use known path, this dynamic update is not correct, it miss some path. So we comment it.s
			paths index_new;
			std::vector<std::vector<uint>> edge;
			std::vector<uint> query_edge;
			query_edge.push_back(iDst_);
			query_edge.push_back(iSrc_);
			edge.push_back(query_edge);
			paths updated_edge(edge);
			index_new = temp_right.join(updated_edge, iK_);
			index_new = index_new.join(temp_left, iK_);
			index_new_paths.add_paths(index_new);
			auto end = std::chrono::high_resolution_clock::now();
			update_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		}
	}
	auto query_end = std::chrono::high_resolution_clock::now();
	query_time = std::chrono::duration_cast<std::chrono::nanoseconds>(query_end - query_start).count() - update_time;

	bool updateIndex = false;
	// if the new edge is already in the adjacency_list, then we do not need to update index
	auto neighbors = oGraph->GetOutNbrCSR(iDst_);
	for (uint j = 0; j < neighbors.second; ++j)
	{
		auto nbr = neighbors.first[j];
		if (nbr == iSrc_)
		{
			updateIndex = false;
			break;
		}
	}
	if (updateIndex)
	{
		index_new_paths.drop_path_length_less_than_k(1);
		index_.push_back(index_new_paths); // already updated in hp index algorithm
	}
	paths result;
	hot_paths.drop_path_length_less_than_k(1); // hot points path may contains path with length equal to 1
	result.add_paths(paths_without_hot);
	result.add_paths(hot_paths);
	return result;
}

void CHcPE::BCDFSUpdateBarrier(CDirectedGraph *&oGraph, const uint &u, const uint &distance_u_t)
{
	// if (distance_u_t > iK_)
	// 	return;
	if (arrBarrier_[u] > distance_u_t)
	{
		arrBarrier_[u] = distance_u_t;
		auto nei = oGraph->GetInNbrCSR(u);
		for (uint i = 0; i < nei.second; ++i)
		{
			if (g_exit)
				return;
			auto v = nei.first[i];
			if (!arrVisited_[v])
			{
				BCDFSUpdateBarrier(oGraph, v, distance_u_t + 1);
			}
		}
	}
}

uint CHcPE::BCDFSRecWithCSR(CDirectedGraph *&oGraph, const uint &u, MyVec &vTempPath)
{
	uint F = iK_ + 1;
	auto iLayer = vTempPath.size();
	vTempPath.push_back(u);
	arrVisited_[u] = true;
	if (u == iDst_)
	{
		iPathCount_++;
		if (!bCountOnly_)
			Paths_.push_back(vTempPath);
		vTempPath.pop_back();
		arrVisited_[u] = false;
		return 0;
	}
	else if (iLayer < iK_)
	{
		auto nei = oGraph->GetOutNbrCSR(u);
		for (uint i = 0; i < nei.second; ++i)
		{
			if (g_exit)
				return F;
			auto nbr = nei.first[i];
			if (!arrVisited_[nbr])
			{
				if (iLayer + arrBarrier_[nbr] + 1 <= iK_)
				{
					uint f = BCDFSRecWithCSR(oGraph, nbr, vTempPath);
					if (f != iK_ + 1)
					{
						F = F < (f + 1) ? F : (f + 1);
					}
				}
			}
		}
	}
	if (F == iK_ + 1)
	{
		arrBarrier_[u] = iK_ - iLayer + 1;
	}
	else
	{
		BCDFSUpdateBarrier(oGraph, u, F);
	}
	vTempPath.pop_back();
	arrVisited_[u] = false;
	return F;
}

void CHcPE::BCDFSPathEnumCSR(CDirectedGraph *&oGraph, ulonglong &time)
{
	time = 0;
	iPathCount_ = 0;
	auto maxidtemp = oGraph->iMaxVerId();
	if (oGraph->iVexNum() == 0 || iSrc_ > maxidtemp || iDst_ > maxidtemp)
		return;
	for (uint i = 0; i <= maxidtemp; i++)
	{
		arrBarrier_[i] = 0;
		arrVisited_[i] = false;
	}
	// arrBarrier_[iDst_] = 0;
	MyVec tempath;
	auto start = std::chrono::high_resolution_clock::now();
	BCDFSRecWithCSR(oGraph, iSrc_, tempath);
	auto end = std::chrono::high_resolution_clock::now();
	time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void CHcPE::unblock(uint u, ushort unreach_dis)
{
	if (arrBarrier_[u] > unreach_dis)
	{
		arrBarrier_[u] = unreach_dis;
	}
	auto iter_B = B.find(u);
	if (iter_B == B.end()) // no nodes in B list
	{
		return;
	}
	for (auto iter = iter_B->second.begin(); iter != iter_B->second.end();)
	{
		if (g_exit)
			return;
		if (arrBarrier_[*iter] > unreach_dis + 1)
		{
			unblock(*iter, unreach_dis + 1);
			iter_B->second.erase(iter++);
		}
		else
		{
			iter++;
		}
	}
}

int CHcPE::dfs_find_k_paths_with_block(CDirectedGraph *&oGraph, uint cur_node, std::set<uint> c_path_set, ushort cur_distance, std::vector<uint> c_path)
{
	if (cur_distance >= iK_)
	{
		return -1;
	}
	arrBarrier_[cur_node] = iK_ - cur_distance;
	c_path.push_back(cur_node);
	std::set<uint> new_c_path_set(c_path_set);
	new_c_path_set.insert(cur_node);
	int f = 0;
	bool unb = false;
	auto nei = oGraph->GetOutNbrCSR(cur_node);
	for (uint i = 0; i < nei.second; ++i)
	{
		if (g_exit)
			break;
		auto nbr = nei.first[i];
		if (nbr == iDst_)
		{
			iPathCount_++;
			if (!bCountOnly_)
			{
				std::vector<uint> temp_result_path(c_path);
				temp_result_path.push_back(iDst_);
				Paths_.push_back(temp_result_path);
			}
			f = 0;
			unb = true;
		}
		else if (c_path_set.find(nbr) != c_path_set.end())
		{
			continue;
		}
		else if (arrBarrier_[nbr] + cur_distance + 1 < iK_)
		{
			f = dfs_find_k_paths_with_block(oGraph, nbr, new_c_path_set, cur_distance + 1, c_path);
			if (f != -1)
			{
				unb = true;
			}
		}
	}
	if (unb)
	{
		unblock(cur_node, f);
	}
	else
	{
		auto nei = oGraph->GetOutNbrCSR(cur_node);
		for (uint i = 0; i < nei.second; ++i)
		{
			if (g_exit)
				break;
			auto nbr = nei.first[i];
			auto iter_B = B.find(nbr);
			if (iter_B == B.end())
			{
				std::set<uint> temp_B;
				temp_B.insert(cur_node);
				B.insert(std::make_pair(nbr, temp_B));
			}
			else
			{
				if (iter_B->second.find(cur_node) == iter_B->second.end())
				{
					iter_B->second.insert(cur_node);
				}
			}
		}
	}
	return f;
}

void CHcPE::BCDFSPathEnumCSR2(CDirectedGraph *&oGraph, ulonglong &time)
{
	time = 0;
	iPathCount_ = 0;
	auto maxidtemp = oGraph->iMaxVerId();
	if (oGraph->iVexNum() == 0 || iSrc_ > maxidtemp || iDst_ > maxidtemp)
		return;
	for (uint i = 0; i <= maxidtemp; i++)
	{
		arrBarrier_[i] = 0;
	}
	B.clear();
	std::set<uint> tempath_set;
	std::vector<uint> tempath;
	auto start = std::chrono::high_resolution_clock::now();
	dfs_find_k_paths_with_block(oGraph, iSrc_, tempath_set, 0, tempath);
	auto end = std::chrono::high_resolution_clock::now();
	time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void CHcPE::BCDFSPathEnumCSR2_BlockInitedBySDt(CDirectedGraph *&oGraph, ulonglong &time)
{
	time = 0;
	iPathCount_ = 0;
	auto maxidtemp = oGraph->iMaxVerId();
	if (oGraph->iVexNum() == 0 || iSrc_ > maxidtemp || iDst_ > maxidtemp)
		return;
	for (uint i = 0; i <= maxidtemp; i++) // ! 不知道为什么，pengyou源代码里arrBarrier最后全部还是设成0，与Lai一样
	{
		if (arrSDu_t_[i] != MAX_K)
			arrBarrier_[i] = arrSDu_t_[i];
		else
			arrBarrier_[i] = 0;
	}
	B.clear();
	std::set<uint> tempath_set;
	std::vector<uint> tempath;
	auto start = std::chrono::high_resolution_clock::now();
	dfs_find_k_paths_with_block(oGraph, iSrc_, tempath_set, 0, tempath);
	auto end = std::chrono::high_resolution_clock::now();
	time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void CHcPE::join_left_right_index_into_left()
{
	for (auto iter = reverse_adjacency_in_subgraph_right.begin(); iter != reverse_adjacency_in_subgraph_right.end(); iter++)
	{
		for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); iter2++)
		{
			if (reverse_adjacency_in_subgraph_left.find(*iter2) == reverse_adjacency_in_subgraph_left.end()) // not in index subgraph
			{
				std::set<uint> temp_set;
				temp_set.insert(iter->first);
				reverse_adjacency_in_subgraph_left.insert(std::make_pair(*iter2, temp_set));
			}
			else
			{
				reverse_adjacency_in_subgraph_left.find(*iter2)->second.insert(iter->first);
			}
		}
	}
}

void CHcPE::find_all_meet_nodes_in_induced_subgraph(uint k, uint query_node1, uint query_node2)
{
	join_left_right_index_into_left(); // we store the left induced subgraph into right one with reverse order
	ushort cur_distance = 0;
	std::set<uint> left_proprogate;
	std::set<uint> right_proprogate;
	std::set<uint> node1_set;
	std::set<uint> node2_set;
	left_proprogate.insert(query_node1);
	right_proprogate.insert(query_node2);
	bool left_skip = false;
	bool right_skip = false;
	int left_max_distance = k - (k / 2);
	int right_max_distance = (k / 2);
	while (true)
	{
		if (query_time_exceeded()) {
		        g_exit = true;
		        break;
		    }
		if (g_exit)
			break;
		cur_distance++;
		if (cur_distance <= left_max_distance && !left_proprogate.empty())
		{
			if (cur_distance == 1)
			{
				std::set<uint> temp_proprogate;
				uint cur_node;
				for (auto iter = left_proprogate.begin(); iter != left_proprogate.end(); iter++)
				{
					cur_node = *iter;
					std::set<uint> temp_set;
					temp_set.insert(cur_node);
					auto iter_map = reverse_adjacency_in_subgraph_right.find(cur_node);
					if (iter_map == reverse_adjacency_in_subgraph_right.end())
					{
						continue;
					}
					for (auto iter2 = iter_map->second.begin(); iter2 != iter_map->second.end(); iter2++)
					{
						if (*iter2 == query_node2)
						{
							continue;
						}
						if (right_proprogate.find(*iter2) != right_proprogate.end())
						{
							meet_nodes.insert(*iter2);
						}
						temp_proprogate.insert(*iter2);
					}
				}
				left_proprogate = temp_proprogate;
			}
			else
			{
				std::set<uint> temp_proprogate;
				uint cur_node;
				for (auto iter = left_proprogate.begin(); iter != left_proprogate.end(); iter++)
				{
					cur_node = *iter;
					std::set<uint> temp_set;
					temp_set.insert(cur_node);
					auto iter_map = reverse_adjacency_in_subgraph_right.find(cur_node);
					if (iter_map == reverse_adjacency_in_subgraph_right.end())
					{
						continue;
					}
					for (auto iter2 = iter_map->second.begin(); iter2 != iter_map->second.end(); iter2++)
					{
						if (right_proprogate.find(*iter2) != right_proprogate.end())
						{
							meet_nodes.insert(*iter2);
						}
						temp_proprogate.insert(*iter2);
					}
				}
				left_proprogate = temp_proprogate;
			}
		}
		else
		{
			left_skip = true;
		}
		if (cur_distance <= right_max_distance && !right_proprogate.empty()) // right propogate
		{
			// cur_distance++;
			std::set<uint> temp_proprogate;
			uint cur_node;
			if (cur_distance == 1)
			{
				for (auto iter = right_proprogate.begin(); iter != right_proprogate.end(); iter++)
				{
					cur_node = *iter;

					std::set<uint> temp_set;
					temp_set.insert(cur_node);
					auto iter_map = reverse_adjacency_in_subgraph_left.find(cur_node);
					if (iter_map == reverse_adjacency_in_subgraph_left.end())
					{
						continue;
					}
					for (auto iter2 = iter_map->second.begin(); iter2 != iter_map->second.end(); iter2++)
					{
						if (*iter2 == query_node1)
						{
							continue;
						}
						if (left_proprogate.find(*iter2) != left_proprogate.end())
						{
							meet_nodes.insert(*iter2);
						}
						temp_proprogate.insert(*iter2);
					}
				}
				right_proprogate = temp_proprogate;
			}
			else
			{
				for (auto iter = right_proprogate.begin(); iter != right_proprogate.end(); iter++)
				{
					cur_node = *iter;
					std::set<uint> temp_set;
					temp_set.insert(cur_node);
					auto iter_map = reverse_adjacency_in_subgraph_left.find(cur_node);
					if (iter_map == reverse_adjacency_in_subgraph_left.end())
					{
						continue;
					}
					for (auto iter2 = iter_map->second.begin(); iter2 != iter_map->second.end(); iter2++)
					{
						if (left_proprogate.find(*iter2) != left_proprogate.end())
						{
							meet_nodes.insert(*iter2);
						}
						temp_proprogate.insert(*iter2);
					}
				}
				right_proprogate = temp_proprogate;
			}
		}
		else
		{
			right_skip = true;
		}
		if (left_skip && right_skip)
		{
			break;
		}
	}
}

std::map<uint, paths> CHcPE::dfs_without_recursion_all_meetpoints(std::unordered_map<uint, std::set<uint>> &reverse_adjacency_in_subgraph, uint src, uint dst, ushort k) // distance means length of path
{
	std::map<uint, paths> result;
	if (meet_nodes.empty())
	{
		return result;
	}
	std::stack<uint> c_path;
	std::vector<uint> c_path_v;
	std::set<uint> c_path_set;
	int cur_distance = 1;
	std::vector<std::set<uint>::iterator> cur_iters;
	c_path.push(src);
	c_path_v.push_back(src);

	auto dst_iter = reverse_adjacency_in_subgraph.find(src);
	if (dst_iter == reverse_adjacency_in_subgraph.end()) // if node1 can not reach node2 in k distance
	{
		return result;
	}

	for (int i = 0; i <= k + 1; i++)
	{
		cur_iters.push_back(dst_iter->second.begin());
	}

	c_path_set.insert(src);
	bool dfs_finish = false;
	uint cur_node;
	while (!dfs_finish)
	{
		if (g_exit)
			break;
		uint cur_node, next_node;
		if (cur_distance <= 0)
			break;

		auto iter = cur_iters[cur_distance];
		if (cur_distance == 1)
		{
			if (cur_iters[cur_distance] == dst_iter->second.end())
			{
				cur_distance--;
				cur_iters[cur_distance]++;
				cur_node = c_path.top();
				c_path.pop();
				c_path_v.pop_back();
				auto iter2 = c_path_set.find(cur_node);
				if (iter2 != c_path_set.end())
				{
					c_path_set.erase(iter2);
				}
				continue;
			}
		}
		else
		{
			auto temp_iter_minus_one = reverse_adjacency_in_subgraph.find(*cur_iters[cur_distance - 1]);
			if (temp_iter_minus_one->second.empty() || cur_iters[cur_distance] == temp_iter_minus_one->second.end() || cur_distance > k)
			{
				// go back
				cur_distance--;
				cur_iters[cur_distance]++;
				cur_node = c_path.top();
				c_path.pop();
				c_path_v.pop_back();
				auto iter2 = c_path_set.find(cur_node);
				if (iter2 != c_path_set.end())
				{
					c_path_set.erase(iter2);
				}
				continue;
			}
		}

		cur_node = *iter;
		// normal logic
		bool force_continue = false;
		while (c_path_set.find(*cur_iters[cur_distance]) != c_path_set.end()) //*cur_iters[cur_distance] < src ||
		{
			cur_iters[cur_distance]++;
			if (cur_distance == 1)
			{
				if (cur_iters[cur_distance] == dst_iter->second.end())
				{
					force_continue = true;
					break;
				}
			}
			else
			{
				auto temp_iter_minus_one = reverse_adjacency_in_subgraph.find(*cur_iters[cur_distance - 1]);
				if (cur_iters[cur_distance] == temp_iter_minus_one->second.end())
				{
					force_continue = true;
					break;
				}
			}
		}
		if (force_continue)
		{
			continue;
		}

		iter = cur_iters[cur_distance];
		if (*iter == dst)
		{
			cur_iters[cur_distance]++;
			continue;
		}
		if (meet_nodes.find(*iter) != meet_nodes.end()) // cur_node is in meet nodes
		{
			// add result path and continue;
			std::vector<uint> temp_result_path(c_path_v);
			temp_result_path.push_back(*iter);
			std::vector<uint> new_temp;
			auto iter_result = result.find(*iter);
			if (iter_result == result.end()) // first time to insert
			{
				paths temp_paths;
				temp_paths.push_back(temp_result_path);
				result.insert(std::make_pair(*iter, temp_paths));
			}
			else
			{
				iter_result->second.push_back(temp_result_path);
			}
		}
		cur_distance++;
		if (reverse_adjacency_in_subgraph.find(*cur_iters[cur_distance - 1]) == reverse_adjacency_in_subgraph.end())
		{
			std::set<uint> temp_set;
			reverse_adjacency_in_subgraph.insert(std::make_pair(*cur_iters[cur_distance - 1], temp_set));
		}
		auto temp_iter_minus_one = reverse_adjacency_in_subgraph.find(*cur_iters[cur_distance - 1]);
		cur_iters[cur_distance] = temp_iter_minus_one->second.begin();
		cur_node = *cur_iters[cur_distance - 1];
		c_path_set.insert(cur_node);
		c_path.push(cur_node);
		c_path_v.push_back(cur_node);
	}
	return result;
}

std::map<uint, paths> CHcPE::dfs_without_recursion_all_meetpoints_reverse(std::unordered_map<uint, std::set<uint>> &reverse_adjacency_in_subgraph, uint src, uint dst, int k) // distance means length of path
{
	std::map<uint, paths> result;
	if (meet_nodes.empty())
	{
		return result;
	}
	std::stack<uint> c_path;
	std::vector<uint> c_path_v;
	std::set<uint> c_path_set;
	int cur_distance = 1;
	std::vector<std::set<uint>::iterator> cur_iters;
	c_path.push(src);
	c_path_v.push_back(src);
	auto dst_iter = reverse_adjacency_in_subgraph.find(src);
	if (dst_iter == reverse_adjacency_in_subgraph.end()) // if node1 can not reach node2 in k distance
	{
		return result;
	}
	for (int i = 0; i <= k + 1; i++)
	{
		cur_iters.push_back(dst_iter->second.begin());
	}
	c_path_set.insert(src);
	bool dfs_finish = false;
	uint cur_node;
	while (!dfs_finish)
	{
		if (g_exit)
			break;
		uint cur_node, next_node;
		if (cur_distance <= 0)
		{
			break;
		}
		auto iter = cur_iters[cur_distance];
		if (cur_distance == 1)
		{
			if (cur_iters[cur_distance] == dst_iter->second.end())
			{
				cur_distance--;
				cur_iters[cur_distance]++;
				cur_node = c_path.top();
				c_path.pop();
				c_path_v.pop_back();
				auto iter2 = c_path_set.find(cur_node);
				if (iter2 != c_path_set.end())
				{
					c_path_set.erase(iter2);
				}
				continue;
			}
		}
		else
		{
			auto temp_iter_minus_one = reverse_adjacency_in_subgraph.find(*cur_iters[cur_distance - 1]);
			if (temp_iter_minus_one->second.empty() || cur_iters[cur_distance] == temp_iter_minus_one->second.end() || cur_distance > k)
			{
				// go back
				cur_distance--;
				cur_iters[cur_distance]++;
				cur_node = c_path.top();
				c_path.pop();
				c_path_v.pop_back();
				auto iter2 = c_path_set.find(cur_node);
				if (iter2 != c_path_set.end())
				{
					c_path_set.erase(iter2);
				}
				continue;
			}
		}
		cur_node = *iter;
		bool force_continue = false;
		while (c_path_set.find(*cur_iters[cur_distance]) != c_path_set.end()) //*cur_iters[cur_distance] < src ||
		{
			cur_iters[cur_distance]++;
			if (cur_distance == 1)
			{
				if (cur_iters[cur_distance] == dst_iter->second.end())
				{
					force_continue = true;
					break;
				}
			}
			else
			{
				auto temp_iter_minus_one = reverse_adjacency_in_subgraph.find(*cur_iters[cur_distance - 1]);
				if (cur_iters[cur_distance] == temp_iter_minus_one->second.end())
				{
					force_continue = true;
					break;
				}
			}
		}
		if (force_continue)
		{
			continue;
		}
		iter = cur_iters[cur_distance];
		if (*iter == dst) // dst
		{
			cur_iters[cur_distance]++;
			continue;
		}
		if (meet_nodes.find(*iter) != meet_nodes.end()) // cur_node is in meet nodes
		{
			std::vector<uint> temp_result_path(c_path_v);
			temp_result_path.push_back(*iter);
			std::vector<uint> new_temp;
			auto iter_result = result.find(*iter);
			paths temp_paths;
			temp_paths.push_back(temp_result_path);
			temp_paths.reverse();
			if (iter_result == result.end()) // first time to insert
			{
				result.insert(std::make_pair(*iter, temp_paths));
			}
			else
			{
				iter_result->second.add_paths(temp_paths);
			}
		}
		cur_distance++;
		if (reverse_adjacency_in_subgraph.find(*cur_iters[cur_distance - 1]) == reverse_adjacency_in_subgraph.end())
		{
			std::set<uint> temp_set;
			reverse_adjacency_in_subgraph.insert(std::make_pair(*cur_iters[cur_distance - 1], temp_set));
		}
		auto temp_iter_minus_one = reverse_adjacency_in_subgraph.find(*cur_iters[cur_distance - 1]);
		cur_iters[cur_distance] = temp_iter_minus_one->second.begin();
		cur_node = *cur_iters[cur_distance - 1];
		c_path_set.insert(cur_node);
		c_path.push(cur_node);
		c_path_v.push_back(cur_node);
	}
	return result;
}

void CHcPE::find_all_k_pahts_dfs_write_number_left_right_path(uint k, uint query_node1, uint query_node2)
{
	for (auto iter = meet_nodes.begin(); iter != meet_nodes.end();)
	{
		if (*iter == query_node1 || *iter == query_node2)
		{
			iter = meet_nodes.erase(iter);
		}
		else
		{
			iter++;
		}
	}
	paths result;
	auto iter_temp = reverse_adjacency_in_subgraph_right.find(query_node1);
	if (iter_temp != reverse_adjacency_in_subgraph_right.end())
	{
		for (auto iter = iter_temp->second.begin(); iter != iter_temp->second.end(); iter++)
		{
			if (*iter == query_node2) // there is one hop path
			{
				std::vector<uint> one_hop_path;
				one_hop_path.push_back(query_node1);
				one_hop_path.push_back(query_node2);
				result.push_back(one_hop_path);
				break;
			}
		}
	}
	std::map<uint, paths> left_map_paths = dfs_without_recursion_all_meetpoints(reverse_adjacency_in_subgraph_right, query_node1, query_node2, k - (k / 2));
	std::map<uint, paths> right_map_paths = dfs_without_recursion_all_meetpoints_reverse(reverse_adjacency_in_subgraph_left, query_node2, query_node1, (k / 2));
	long long temp_num = 0;
	long long temp_num2 = 0;
	for (auto iter = meet_nodes.begin(); iter != meet_nodes.end(); iter++)
	{
		if (query_time_exceeded()) {
		        g_exit = true;
		        goto BCJOIN_OUT;
		    }
		if (g_exit)
			goto BCJOIN_OUT;
		auto left_iter = left_map_paths.find(*iter);
		auto right_iter = right_map_paths.find(*iter);
		if (left_iter == left_map_paths.end())
		{
			continue;
		}
		else if (right_iter == right_map_paths.end())
		{
			continue;
		}
		temp_num = temp_num + left_iter->second.path.size() + right_iter->second.path.size();
		left_iter->second.sort_by_string_order();
		right_iter->second.sort_by_string_order();
		paths total_paths;
		left_iter->second.drop_repeat_path();
		right_iter->second.drop_repeat_path();
		total_paths = left_iter->second.join_remove_repeat_nodes_only_join_right_sizeor_minus_one(right_iter->second, k);
		// total_paths.drop_path_length_more_than_k(k);
		result.add_paths(total_paths);
	}
BCJOIN_OUT:
	iPathCount_ = result.path.size();
}

void CHcPE::BCJOINPathEnumCSR(ulonglong &time)
{
    time = 0;
    iPathCount_ = 0;

    // 每条 query 必须重置
    g_exit = false;
    query_start_time_ = std::chrono::steady_clock::now();

    find_all_meet_nodes_in_induced_subgraph(iK_, iSrc_, iDst_);
    find_all_k_pahts_dfs_write_number_left_right_path(iK_, iSrc_, iDst_);

    // 统计时间（封顶 300s）
    auto end = std::chrono::steady_clock::now();
    double elapsed_sec =
        std::chrono::duration<double>(end - query_start_time_).count();

    if (elapsed_sec > QUERY_TIME_LIMIT_SEC)
        elapsed_sec = QUERY_TIME_LIMIT_SEC;

    time = static_cast<ulonglong>(elapsed_sec * 1e9);
}



void CHcPE::IDXDFSRecWithCSR(uint u, uint k)
{
	arrStack_[k] = u;
	arrVisited_[u] = true;

	ulonglong temp_count = iPathCount_;

	// k is cost; iK_ - k is the remaining budget; minus 1 is the cost of moving to a out neighbor.
	uint budget = iK_ - k - 1;
	uint neighbor_offset = single_bigraph_[u];
	uint start = single_bigraph_offset_[neighbor_offset];
	uint end = single_bigraph_offset_[neighbor_offset + budget + 1];
	for (uint i = start; i < end; ++i)
	{
		if (g_exit) // g_exit || iPathCount_ >= target_number_results_
			break;
		iTempPathsCount_++;
		uint v = single_bigraph_adj_[i];
		if (v == iDst_)
		{
			if (!bCountOnly_)
				arrStack_[k + 1] = iDst_;
			iPathCount_ += 1;
		}
		else if (k + 1 == iK_ - 1 && !arrVisited_[v])
		{
			if (!bCountOnly_)
			{
				arrStack_[k + 1] = v;
				arrStack_[k + 2] = iDst_;
			}
			iPathCount_ += 1;
			// iPartialResultCount_ += 1;
		}
		else if (!arrVisited_[v])
		{
			IDXDFSRecWithCSR(v, k + 1);
		}
		else
		{
			// iConflictCount_ += 1;
		}
	}

	arrVisited_[u] = false;
	// iPartialResultCount_ += 1;
	// if (temp_count == iPathCount_)
	// {
	// 	iInvalidPartialResultCount_ += 1;
	// }
}

void CHcPE::IDXDFSPathEnumCSR(ulonglong &time)
{
	for (uint i = 0; i <= oGraph_->iMaxVerId(); i++)
	{
		arrVisited_[i] = false;
	}
	iPathCount_ = 0;
	time = 0;

	auto start = std::chrono::high_resolution_clock::now();
	IDXDFSRecWithCSR(iSrc_, 0);
	auto end = std::chrono::high_resolution_clock::now();
	time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void CHcPE::generate_single_join_plan()
{
	uint num_vertices = oGraph_->iMaxVerId() + 1;
	std::vector<ulonglong> current_value(num_vertices, 0);
	std::vector<ulonglong> previous_value(num_vertices, 0);

	// Estimate the number of paths to dst.
	std::vector<ulonglong> num_path_to_dst(iK_ + 1, 0);
	for (uint budget = 1; budget < iK_; ++budget)
	{
		// i is the distance to dst and j is the distance from src.
		ulonglong global_sum = 0;
		uint remaining_budget = budget - 1;

		for (uint i = 1; i <= budget; ++i)
		{
			for (uint j = 1; j <= iK_ - budget; ++j)
			{
				uint bucket_id = BUCKET_ID(j, i, iK_ + 1);
				for (uint k = buckets_offset_[bucket_id]; k < buckets_offset_[bucket_id + 1]; ++k)
				{
					uint u = buckets_adj_[k];
					// budget is i - 1.
					uint neighbor_offset = single_bigraph_[u];
					uint start = single_bigraph_offset_[neighbor_offset];
					uint end = single_bigraph_offset_[neighbor_offset + remaining_budget + 1];

					ulonglong local_sum = 0;
					for (uint l = start; l < end; ++l)
					{
						uint v = single_bigraph_adj_[l];
						if (v == iDst_)
						{
							local_sum += 1;
						}
						else
						{
							local_sum += previous_value[v];
						}
					}
					current_value[u] = local_sum;
					global_sum += local_sum;
				}
			}
		}
		num_path_to_dst[budget] = global_sum;
		previous_value.swap(current_value);
	}
	{
		estimated_result_count_ = 0;
		uint neighbor_offset = single_bigraph_[iSrc_];
		uint start = single_bigraph_offset_[neighbor_offset];
		uint end = single_bigraph_offset_[neighbor_offset + iK_];
		for (uint i = start; i < end; ++i)
		{
			uint u = single_bigraph_adj_[i];
			if (u == iDst_)
			{
				estimated_result_count_ += 1;
			}
			else
			{
				estimated_result_count_ += previous_value[u];
			}
		}
	}
	// TODO: After testing, remove the reset.
	std::fill(previous_value.begin(), previous_value.end(), 0);
	std::fill(current_value.begin(), current_value.end(), 0);

	// Estimate the number of paths
	std::vector<ulonglong> num_path_from_src(iK_ + 1, 0);
	for (uint budget = 1; budget < iK_; ++budget)
	{
		// i is the distance from src and j is the distance to dst.
		ulonglong global_sum = 0;
		uint remaining_budget = budget - 1;

		for (uint i = 1; i <= budget; ++i)
		{
			for (uint j = 1; j <= iK_ - budget; ++j)
			{
				uint bucket_id = BUCKET_ID(i, j, iK_ + 1);

				for (uint k = buckets_offset_[bucket_id]; k < buckets_offset_[bucket_id + 1]; ++k)
				{
					uint u = buckets_adj_[k];
					// budget is i - 1.
					uint neighbor_offset = single_reverse_bigraph_[u];
					uint start = single_reverse_bigraph_offset_[neighbor_offset];
					uint end = single_reverse_bigraph_offset_[neighbor_offset + remaining_budget + 1];

					ulonglong local_sum = 0;
					for (uint l = start; l < end; ++l)
					{
						uint v = single_reverse_bigraph_adj_[l];
						if (v == iSrc_)
						{
							local_sum += 1;
						}
						else
						{
							local_sum += previous_value[v];
						}
					}

					current_value[u] = local_sum;
					global_sum += local_sum;
				}
			}
		}
		num_path_from_src[budget] = global_sum;
		previous_value.swap(current_value);
	}

	ulonglong min_sum = std::numeric_limits<ulonglong>::max();
	for (uint i = 1; i < iK_; ++i)
	{
		ulonglong cur_sum = num_path_to_dst[i] + num_path_from_src[iK_ - i];
		if (cur_sum < min_sum)
		{
			min_sum = cur_sum;
			min_cut_position_ = iK_ - i;
		}
	}

	estimated_left_relation_size_ = num_path_from_src[min_cut_position_];
	estimated_right_relation_size_ = num_path_to_dst[iK_ - min_cut_position_];
	iFullFledgedEstimatedResultCount_ = estimated_result_count_;

	// Estimated cost of DFS: the total number of intermediate results.
	estimated_dfs_cost_ = 0;
	for (uint i = 1; i < iK_; ++i)
	{
		estimated_dfs_cost_ += num_path_from_src[i];
	}

	// Materialization cost of the partial results + loop over the results. 1.05 is the penalty of checking the duplicate vertices.
	estimated_join_cost_ = estimated_left_relation_size_ + estimated_right_relation_size_ + (ulonglong)(estimated_result_count_ * 1.05);

	full_fledged_selection_ = estimated_join_cost_ < estimated_dfs_cost_ ? 1 : 0;
}

void CHcPE::left_dfs(uint32_t u, uint32_t k)
{
	arrStack_[k] = u;
	arrVisited_[u] = true;

	// k is cost; iK_ - k is the remaining budget; minus 1 is the cost of moving to a out neighbor.
	uint32_t budget = iK_ - k - 1;
	uint32_t neighbor_offset = single_bigraph_[u];
	uint32_t start = single_bigraph_offset_[neighbor_offset];
	uint32_t end = single_bigraph_offset_[neighbor_offset + budget + 1];

	iNeighborsAccessCount_ += (end - start);

	for (uint32_t i = start; i < end; ++i)
	{
		if (g_exit)
			goto EXIT;

		uint32_t v = single_bigraph_adj_[i];
		if (v == iDst_)
		{
			// Emit the result.
			arrStack_[k + 1] = iDst_;
			iPathCount_ += 1;
		}
		else if (k == min_cut_position_ - 1 && !arrVisited_[v])
		{
			arrStack_[k + 1] = v;
			// Copy the result to the buffer.
			std::copy(left_partial_begin_, left_partial_end_, left_cursor_);
			left_cursor_ += left_part_length_;
			left_relation_size_ += 1;
			iPartialResultCount_ += 1;
		}
		else if (!arrVisited_[v])
		{
			left_dfs(v, k + 1);
		}
		else
		{
			iConflictCount_ += 1;
		}
	}

EXIT:
	iPartialResultCount_ += 1;
	arrVisited_[u] = false;
}

void CHcPE::right_dfs(uint32_t u, uint32_t k)
{
	arrStack_[k] = u;
	arrVisited_[u] = true;

	// k is cost; iK_ - k is the remaining budget; minus 1 is the cost of moving to a out neighbor.
	uint32_t budget = iK_ - k - 1;
	uint32_t neighbor_offset = single_bigraph_[u];
	uint32_t start = single_bigraph_offset_[neighbor_offset];
	uint32_t end = single_bigraph_offset_[neighbor_offset + budget + 1];

	iNeighborsAccessCount_ += (end - start);

	for (uint32_t i = start; i < end; ++i)
	{
		if (g_exit)
			goto EXIT;

		uint32_t v = single_bigraph_adj_[i];
		if (v == iDst_)
		{
			// Emit the result.
			arrStack_[k + 1] = iDst_;
			std::copy(right_partial_begin_, right_partial_end_, right_cursor_);
			right_cursor_ += right_part_length_;
			right_relation_size_ += 1;
		}
		else if (k == iK_ - 2 && !arrVisited_[v])
		{
			arrStack_[k + 1] = v;
			arrStack_[k + 2] = iDst_;
			// Copy the result to the buffer.
			std::copy(right_partial_begin_, right_partial_end_, right_cursor_);
			right_cursor_ += right_part_length_;
			right_relation_size_ += 1;
			iPartialResultCount_ += 1;
		}
		else if (!arrVisited_[v])
		{
			right_dfs(v, k + 1);
		}
		else
		{
			iConflictCount_ += 1;
		}
	}

EXIT:
	iPartialResultCount_ += 1;
	arrVisited_[u] = false;
}

void CHcPE::single_join()
{
	left_cursor_ = left_relation_;
	uint32_t left_key_position = left_part_length_ - 1;
	for (uint64_t i = 0; i < left_relation_size_; ++i)
	{
		// Initialize visited table.
		for (uint32_t j = 0; j < left_part_length_; ++j)
		{
			uint32_t u = left_cursor_[j];
			arrVisited_[u] = true;
		}

		// Join with the partitions.
		uint32_t key = left_cursor_[left_key_position];
		if (index_table_.contains(key))
		{
			auto partitions = index_table_[key];
			right_cursor_ = partitions.first;
			for (uint64_t j = 0; j < partitions.second; ++j)
			{
				if (g_exit)
				{
					return;
				}
				for (uint32_t k = 1; k < right_part_length_; ++k)
				{
					uint32_t u = right_cursor_[k];
					if (u == iDst_)
					{
						iPathCount_ += 1;
						break;
					}
					else if (arrVisited_[u])
					{
						break;
					}
				}
				right_cursor_ += right_part_length_;
			}
		}

		// Clear visited table.
		for (uint32_t j = 0; j < left_part_length_; ++j)
		{
			uint32_t u = left_cursor_[j];
			arrVisited_[u] = false;
		}

		left_cursor_ += left_part_length_;
	}
}

void CHcPE::single_join_on_bigraph()
{
	// Initialize.
	left_part_length_ = min_cut_position_ + 1;
	right_part_length_ = iK_ - min_cut_position_ + 1;
	left_relation_size_ = 0;
	right_relation_size_ = 0;
	left_partial_begin_ = arrStack_;
	left_partial_end_ = left_partial_begin_ + left_part_length_;
	right_partial_begin_ = arrStack_ + min_cut_position_;
	right_partial_end_ = right_partial_begin_ + right_part_length_;

	left_relation_ = (uint32_t *)malloc(sizeof(uint32_t) * left_part_length_ * estimated_left_relation_size_);
	right_relation_ = (uint32_t *)malloc(sizeof(uint32_t) * right_part_length_ * estimated_right_relation_size_);

	auto left_dfs_start = std::chrono::high_resolution_clock::now();
	// Allocate the memory for the materialization.
	left_cursor_ = left_relation_;

	left_dfs(iSrc_, 0);
	auto left_dfs_end = std::chrono::high_resolution_clock::now();
	iLeftDfsTime_ = std::chrono::duration_cast<std::chrono::nanoseconds>(left_dfs_end - left_dfs_start).count();

	// Allocate the memory for the materialization.
	right_cursor_ = right_relation_;

	for (uint32_t i = 1; i <= min_cut_position_; ++i)
	{
		for (uint32_t j = 1; j <= iK_ - min_cut_position_; ++j)
		{
			uint32_t bucket_id = BUCKET_ID(i, j, iK_ + 1);
			for (uint32_t k = buckets_offset_[bucket_id]; k < buckets_offset_[bucket_id + 1]; ++k)
			{
				uint32_t u = buckets_adj_[k];
				uint32_t *cursor = right_cursor_;
				right_dfs(u, min_cut_position_);
				uint64_t temp_count = (right_cursor_ - cursor) / right_part_length_;
				index_table_[u] = std::make_pair(cursor, temp_count);
			}
		}
	}

	auto right_dfs_end = std::chrono::high_resolution_clock::now();
	iRightDfsTime_ = std::chrono::duration_cast<std::chrono::nanoseconds>(right_dfs_end - left_dfs_end).count();

	single_join();

	dEstimateAccuracy_ = estimated_result_count_ == 0 ? 1 : (double)iPathCount_ / estimated_result_count_;

	// Release the memory.

	auto join_end = std::chrono::high_resolution_clock::now();

	iJoinTime_ = std::chrono::duration_cast<std::chrono::nanoseconds>(join_end - right_dfs_end).count();

	free(left_relation_);
	free(right_relation_);
	index_table_.clear();
	//    auto free_end = std::chrono::high_resolution_clock::now();

	//    uint64_t free_time = std::chrono::duration_cast<std::chrono::nanoseconds>(free_end - join_end).count();
	//    uint64_t malloc_time = std::chrono::duration_cast<std::chrono::nanoseconds>(left_dfs_start - estimation_end).count();
	//    printf("Left relation size: %u, Right relation size: %u\n", left_relation_size_, right_relation_size_);
	//    printf("Free time: %.6lf, Malloc time: %.6lf\n", free_time/1000000.0, malloc_time/1000000.0);
}

void CHcPE::IDXJOINPathEnumCSR(ulonglong &time)
{
	for (uint i = 0; i < oGraph_->iMaxVerId() + 1; i++)
	{
		arrVisited_[i] = false;
	}
	for (uint i = 0; i < iK_ + 1; i++)
	{
		arrStack_[i] = 0;
	}
	iPathCount_ = 0;
	time = 0;

	auto start = std::chrono::high_resolution_clock::now();

	auto estimation_start = std::chrono::high_resolution_clock::now();
	generate_single_join_plan();
	auto estimation_end = std::chrono::high_resolution_clock::now();
	iFledgedEstimationTime_ = std::chrono::duration_cast<std::chrono::nanoseconds>(estimation_end - estimation_start).count();

	single_join_on_bigraph();

	auto end = std::chrono::high_resolution_clock::now();
	time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void CHcPE::IDXMIXPathEnumCSR(ulonglong &time)
{
	for (uint i = 0; i < oGraph_->iMaxVerId() + 1; i++)
	{
		arrVisited_[i] = false;
	}
	for (uint i = 0; i < iK_ + 1; i++)
	{
		arrStack_[i] = 0;
	}
	iPathCount_ = 0;
	time = 0;

	auto start = std::chrono::high_resolution_clock::now();

	if (preliminary_cardinality_estimator())
	{
		preliminary_selection_ = 1;
		auto estimation_start = std::chrono::high_resolution_clock::now();
		generate_single_join_plan();
		auto estimation_end = std::chrono::high_resolution_clock::now();
		iFledgedEstimationTime_ = std::chrono::duration_cast<std::chrono::nanoseconds>(estimation_end - estimation_start).count();

		if (full_fledged_selection_ == 0)
		{
			IDXDFSRecWithCSR(iSrc_, 0);
		}
		else
		{
			single_join_on_bigraph();
		}
	}
	else
	{
		preliminary_selection_ = 0;
		IDXDFSRecWithCSR(iSrc_, 0);
	}

	auto end = std::chrono::high_resolution_clock::now();
	time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void CHcPE::PEFPPathEnumCSR(CDirectedGraph *&oGraph, ulonglong &time)
{
	time = 0;
	auto maxidtemp = oGraph->iMaxVerId();
	if (oGraph->iVexNum() == 0 || iSrc_ > maxidtemp || iDst_ > maxidtemp)
		return;
	for (uint i = 0; i <= oGraph->iMaxVerId(); i++)
		arrVisited_[i] = false;

	auto start = std::chrono::high_resolution_clock::now();

	ushort iVisitedLen = 0;	 // 已遍历路径长度 iVisitedLen = |path|+1
	bool bTag = false;		 // 回溯标志
	std::stack<uint> staDFS; // DFS遍历栈
	staDFS.push(iSrc_);
	while (!staDFS.empty())
	{
		if (g_exit)
			break;
		uint v = staDFS.top();
		bTag = false;
		if (!arrVisited_[v])
		{
			arrVisited_[v] = true;
			++iVisitedLen;
			auto out_neighbors = oGraph->GetOutNbrCSR(v);
			for (uint i = 0; i < out_neighbors.second; ++i)
			{
				++iTempPathsCount_;
				auto vt = out_neighbors.first[i];
				if (!arrVisited_[vt])
				{
					if (iVisitedLen + arrSDu_t_[vt] <= iK_)
					{
						if (vt == iDst_)
						{
							++iPathCount_;
							continue;
						}
						else
						{
							bTag = true;
							staDFS.push(vt);
						}
					}
				}
			}
		}
		if (!bTag)
		{
			staDFS.pop();
			arrVisited_[v] = false;
			--iVisitedLen;
		}
	} // while

	auto end = std::chrono::high_resolution_clock::now();
	time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

/**
 * @brief 全优化
 *
 * @param oGraph
 * @param time
 */
void CHcPE::MYDFSPathEnumCSR(CDirectedGraph *&oGraph, ulonglong &time)
{
	time = 0;
	auto maxidtemp = oGraph->iMaxVerId();
	if (oGraph->iVexNum() == 0 || iSrc_ > maxidtemp || iDst_ > maxidtemp)
		return;
	for (uint i = 0; i <= oGraph->iMaxVerId(); i++)
		arrVisited_[i] = false;

	auto start = std::chrono::high_resolution_clock::now();

	ushort iVisitedLen = 0;	 // 已遍历路径长度 iVisitedLen = |path|+1
	bool bTag = false;		 // 回溯标志
	std::stack<uint> staDFS; // DFS遍历栈
	staDFS.push(iSrc_);
	while (!staDFS.empty())
	{
		if (g_exit)
			break;
		uint v = staDFS.top();

		/*type 1*/
		bTag = false;
		if (!arrVisited_[v])
		{
			arrVisited_[v] = true;
			++iVisitedLen;
			auto out_neighbors = oGraph->GetOutNbrCSR(v);
			for (uint i = 0; i < out_neighbors.second; ++i)
			{
				auto vt = out_neighbors.first[i];
				if (!arrVisited_[vt])
				{
					if (iVisitedLen + arrSDu_t_[vt] <= iK_) // ! 创新点1
					{
						++iTempPathsCount_;
						if (vt == iDst_)
						{
							++iPathCount_;
							continue;
						}
						if (iVisitedLen == iK_ - 1)
						{
							++iPathCount_;
							continue;
						}
						else
						{
							bTag = true;
							staDFS.push(vt);
						}
					}
					else
						break;
				}
			}
		}
		if (!bTag)
		{
			staDFS.pop();
			arrVisited_[v] = false;
			--iVisitedLen;
		}

		/*type 2*/
		// if (arrVisited_[v])
		// {
		// 	staDFS.pop();
		// 	arrVisited_[v] = false;
		// 	--iVisitedLen;
		// 	continue;
		// }
		// if (v == iDst_ || iVisitedLen == iK_ - 1)
		// {
		// 	++iPathCount_;
		// 	staDFS.pop();
		// 	continue;
		// }
		// arrVisited_[v] = true;
		// ++iVisitedLen;
		// auto out_neighbors = oGraph->GetOutNbrCSR(v);
		// for (uint i = 0; i < out_neighbors.second; ++i)
		// {
		// 	auto vt = out_neighbors.first[i];
		// 	if (!arrVisited_[vt])
		// 	{
		// 		if (iVisitedLen + arrSDu_t_[vt] <= iK_) // ! 创新点1
		// 			staDFS.push(vt);
		// 		else
		// 			break;
		// 	}
		// }
	} // while

	auto end = std::chrono::high_resolution_clock::now();
	time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void CHcPE::MYDFSPathEnumCSR(CDirectedGraph *&oGraph, std::vector<uint> pp, ushort hop, uint dst, ulonglong &time)
{
	time = 0;
	auto maxidtemp = oGraph->iMaxVerId();
	if (oGraph->iVexNum() == 0)
		return;
	for (uint i = 0; i <= oGraph->iMaxVerId(); i++)
		arrVisited_[i] = false;

	for (uint i = 0; i < hop; i++)
	{
		arrVisited_[pp[i]] = true;
	}

	auto start = std::chrono::high_resolution_clock::now();

	ushort iVisitedLen = hop; // 已遍历路径长度 iVisitedLen = |path|+1
	bool bTag = false;		  // 回溯标志
	std::stack<uint> staDFS;  // DFS遍历栈
	staDFS.push(pp[hop]);
	while (!staDFS.empty())
	{
		if (g_exit)
			break;
		uint v = staDFS.top();

		/*type 1*/
		bTag = false;
		if (!arrVisited_[v])
		{
			arrVisited_[v] = true;
			++iVisitedLen;
			auto out_neighbors = oGraph->GetOutNbrCSR(v);
			for (uint i = 0; i < out_neighbors.second; ++i)
			{
				auto vt = out_neighbors.first[i];
				if (!arrVisited_[vt])
				{
					if (iVisitedLen + arrSDu_t_[vt] <= iK_) // ! 创新点1
					{
						++iTempPathsCount_;
						if (vt == dst)
						{
							++iPathCount_;
							continue;
						}
						if (iVisitedLen == iK_ - 1)
						{
							++iPathCount_;
							continue;
						}
						else
						{
							bTag = true;
							staDFS.push(vt);
						}
					}
					else
						break;
				}
			}
		}
		if (!bTag)
		{
			staDFS.pop();
			arrVisited_[v] = false;
			--iVisitedLen;
		}

	} // while

	auto end = std::chrono::high_resolution_clock::now();
	time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void CHcPE::MYDFSPathEnumCSR(CDirectedGraph *&oGraph, uint src, uint dst, ulonglong &time)
{
	time = 0;
	auto maxidtemp = oGraph->iMaxVerId();
	if (oGraph->iVexNum() == 0 || src > maxidtemp || dst > maxidtemp)
		return;
	for (uint i = 0; i <= oGraph->iMaxVerId(); i++)
		arrVisited_[i] = false;

	auto start = std::chrono::high_resolution_clock::now();

	ushort iVisitedLen = 0;	 // 已遍历路径长度 iVisitedLen = |path|+1
	bool bTag = false;		 // 回溯标志
	std::stack<uint> staDFS; // DFS遍历栈
	staDFS.push(src);
	while (!staDFS.empty())
	{
		if (g_exit)
			break;
		uint v = staDFS.top();

		/*type 1*/
		bTag = false;
		if (!arrVisited_[v])
		{
			arrVisited_[v] = true;
			++iVisitedLen;
			auto out_neighbors = oGraph->GetOutNbrCSR(v);
			for (uint i = 0; i < out_neighbors.second; ++i)
			{
				auto vt = out_neighbors.first[i];
				iBRAM2DDRDatas_++;
				if (vt > 10000)
					iDDR2BRAMDatas_++;
				if (!arrVisited_[vt])
				{
					if (iVisitedLen + arrSDu_t_[vt] <= iK_) // ! 创新点1
					{
						++iTempPathsCount_;
						if (vt == dst)
						{
							++iPathCount_;
							continue;
						}
						if (iVisitedLen == iK_ - 1)
						{
							++iPathCount_;
							continue;
						}
						else
						{
							bTag = true;
							staDFS.push(vt);
						}
					}
					else
						break;
				}
			}
		}
		if (!bTag)
		{
			staDFS.pop();
			arrVisited_[v] = false;
			--iVisitedLen;
		}
	} // while

	auto end = std::chrono::high_resolution_clock::now();
	time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void CHcPE::MYDFSRecWithCSR(CDirectedGraph *&oGraph, const uint u, const ushort iLayer)
{
	arrVisited_[u] = true;
	auto neighbors = oGraph->GetOutNbrCSR(u);
	for (uint i = 0; i < neighbors.second; ++i)
	{
		if (g_exit)
			return;
		uint vt = neighbors.first[i];
		if (!arrVisited_[vt])
			if (iLayer + arrSDu_t_[vt] <= iK_)
			{
				if (vt == iDst_)
				{
					++iPathCount_;
					continue;
				}
				if (iLayer == iK_ - 1)
				{
					++iPathCount_;
					continue;
				}
				else
				{
					MYDFSRecWithCSR(oGraph, vt, iLayer + 1);
				}
			}
			else
				break;
	}
	arrVisited_[u] = false;
}

/**
 * @brief 递归
 *
 * @param oGraph
 * @param time
 */
void CHcPE::MYDFSPathEnumCSR_1(CDirectedGraph *&oGraph, ulonglong &time)
{
	time = 0;
	auto maxidtemp = oGraph->iMaxVerId();
	if (oGraph->iVexNum() == 0 || iSrc_ > maxidtemp || iDst_ > maxidtemp)
		return;
	for (uint i = 0; i <= maxidtemp; i++)
	{
		arrVisited_[i] = false;
	}

	auto start = std::chrono::high_resolution_clock::now();
	MYDFSRecWithCSR(oGraph, iSrc_, 1);
	auto end = std::chrono::high_resolution_clock::now();
	time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

/**
 * @brief 去掉创新点1
 *
 * @param oGraph
 * @param time
 */
void CHcPE::MYDFSPathEnumCSR_2(CDirectedGraph *&oGraph, ulonglong &time)
{
	time = 0;
	auto maxidtemp = oGraph->iMaxVerId();
	if (oGraph->iVexNum() == 0 || iSrc_ > maxidtemp || iDst_ > maxidtemp)
		return;
	for (uint i = 0; i <= oGraph->iMaxVerId(); i++)
		arrVisited_[i] = false;

	auto start = std::chrono::high_resolution_clock::now();

	ushort iVisitedLen = 0;	 // 已遍历路径长度 iVisitedLen = |path|+1
	bool bTag = false;		 // 回溯标志
	std::stack<uint> staDFS; // DFS遍历栈
	staDFS.push(iSrc_);
	while (!staDFS.empty())
	{
		if (g_exit)
			break;
		uint v = staDFS.top();

		bTag = false;
		if (!arrVisited_[v])
		{
			arrVisited_[v] = true;
			++iVisitedLen;
			auto out_neighbors = oGraph->GetOutNbrCSR(v);
			for (uint i = 0; i < out_neighbors.second; ++i)
			{
				auto vt = out_neighbors.first[i];
				if (!arrVisited_[vt])
				{
					if (iVisitedLen + arrSDu_t_[vt] <= iK_)
					{
						if (vt == iDst_)
						{
							++iPathCount_;
							continue;
						}
						if (iVisitedLen == iK_ - 1)
						{
							++iPathCount_;
							continue;
						}
						else
						{
							bTag = true;
							staDFS.push(vt);
						}
					}
				}
			}
		}
		if (!bTag)
		{
			staDFS.pop();
			arrVisited_[v] = false;
			--iVisitedLen;
		}
	} // while
	auto end = std::chrono::high_resolution_clock::now();
	time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void CHcPE::JoinCounter(uint l, uint r)
{
	std::vector<std::vector<MyVec>> &L = Lpaths_[l];
	std::vector<std::vector<MyVec>> &R = Rpaths_[r];
	spp::sparse_hash_set<uint> &join_v = (l == r) ? JoinVerts_[r].first : JoinVerts_[r].second;
	for (auto &v : join_v)
	{
		if (g_exit)
			break;
		if (L[v].empty() || R[v].empty())
		{
			continue;
		}

		for (auto &pl : L[v])
		{
			if (v == iDst_)
			{
				iPathCount_++;
				continue;
			}

			for (uint i = 1; i < pl.size(); i++)
			{
				arrVisited_[pl[i]] = true;
			}
			for (auto &pr : R[v])
			{
				bool good = true;
				for (uint j = 1; j < pr.size() - 1; j++)
				{
					if (arrVisited_[pr[j]])
					{
						good = false;
						break;
					}
				}
				if (good)
					iPathCount_++;
			}

			for (uint i = 1; i < pl.size(); i++)
			{
				arrVisited_[pl[i]] = false;
			}
		}
	}
}

void CHcPE::NativeJoin(CDirectedGraph *&oGraph)
{
	for (int i = 1; i <= (iK_ + 1) / 2; i++)
	{
		if (g_exit)
			break;
		if (i == 1)
		{
			auto out_neighbors = oGraph->GetOutNbrCSR(iSrc_);
			for (uint k = 0; k < out_neighbors.second; ++k)
			{
				if (out_neighbors.first[k] == iDst_)
				{
					iPathCount_++;
					break;
				}
			}
		}
		else
		{
			JoinCounter(i, i - 1);
		}

		if (i == iK_ / 2 + 1)
		{
			break; // iK_ 为 奇数
		}

		JoinCounter(i, i);
	}
}

void CHcPE::MYJOINPathEnumCSR(CDirectedGraph *&oGraph, ulonglong &time)
{
	time = 0;
	iPathCount_ = 0;
	auto maxidtemp = oGraph->iMaxVerId();
	if (oGraph->iVexNum() == 0 || iSrc_ > maxidtemp || iDst_ > maxidtemp)
		return;
	for (uint i = 0; i <= maxidtemp; i++)
	{
		arrVisitedCnt_[i] = 0;
	}
	Lpaths_.resize((iK_ + 1) / 2 + 1, std::vector<std::vector<MyVec>>(maxidtemp + 1, std::vector<MyVec>()));
	Rpaths_.resize(iK_ / 2 + 1, std::vector<std::vector<MyVec>>(maxidtemp + 1, std::vector<MyVec>()));
	JoinVerts_.resize(iK_ / 2 + 1, std::pair<spp::sparse_hash_set<uint>, spp::sparse_hash_set<uint>>()); // 每次join的中间点集合
	ulonglong visited_edges = 0;																		 // 统计访问数
	uint visit_cnt = 0;																					 // 当前访问路径代号，用来标记arrVisitedCnt_每条路径的前缀节点
	MyVec Lm, Rm, ps, pt;
	std::vector<MyVec> Lpath, Rpath;

	auto start = std::chrono::high_resolution_clock::now();

	for (uint i = 0; i <= (iK_ + 1) / 2; i++) // 第i层
	{
		if (g_exit)
			break;
		MyVec tmp_Lm;				  // 记录下一层 节点
		std::vector<MyVec> tmp_Lpath; // 记录下一层 路径

		if (i == 0)
		{
			ps.push_back(iSrc_); // 初始化起始路径 左 起点
			Lpaths_[0][iSrc_].push_back(ps);

			Lm.push_back(iSrc_); // 当前层的待拓扑节点 中间结果
			Lpath.push_back(ps); // 当前层的待拓扑路径 中间结果
		}
		else
		{
			uint Lmsize = Lm.size();		 //
			for (int j = 0; j < Lmsize; j++) // 第j条 子路径
			{
				auto e = Lm[j];
				auto &lpa = Lpath[j]; // 当前路径

				visit_cnt++;		 // 遍历的第几个路径task
				for (auto &vv : lpa) // 标记第几个路径task的已访问节点
				{
					arrVisitedCnt_[vv] = visit_cnt;
				}

				auto out_neighbors = oGraph->GetOutNbrCSR(e);
				for (uint k = 0; k < out_neighbors.second; ++k)
				{
					auto v = out_neighbors.first[k];
					visited_edges++;
					if (arrSDu_t_[v] + i <= iK_)
					{
						if (arrVisitedCnt_[v] != visit_cnt && v != iDst_)
						{
							lpa.push_back(v);
							tmp_Lm.push_back(v);
							tmp_Lpath.push_back(lpa);
							Lpaths_[i][v].push_back(lpa);
							lpa.pop_back();
						}
					}
					else
						break;
				}
			}
			Lm.clear();
			Lm.swap(tmp_Lm);
			Lpath.clear();
			Lpath.swap(tmp_Lpath);
		}
	}
	auto point1 = std::chrono::high_resolution_clock::now();
	iLeftDfsTime_ = std::chrono::duration_cast<std::chrono::nanoseconds>(point1 - start).count();

	for (uint i = 0; i <= iK_ / 2; i++)
	{
		if (g_exit)
			break;
		MyVec tmp_Rm;
		std::vector<MyVec> tmp_Rpath;

		if (i == 0)
		{
			pt.push_back(iDst_); // 初始化起始路径 右 终点
			Rpaths_[0][iDst_].push_back(pt);

			if (i < (iK_ + 1) / 2 && !Lpaths_[i + 1][iDst_].empty())
			{
				JoinVerts_[i].second.insert(iDst_); // L(1)---R(0)
			}

			Rm.push_back(iDst_); // 当前层的待拓扑节点 中间结果
			Rpath.push_back(pt); // 当前层的待拓扑路径 中间结果
		}
		else
		{
			uint Rmsize = Rm.size();
			for (uint j = 0; j < Rmsize; j++)
			{
				auto e = Rm[j];
				auto &rpa = Rpath[j];

				visit_cnt++;
				for (auto &vv : rpa)
				{
					arrVisitedCnt_[vv] = visit_cnt;
				}

				auto in_neighbors = oGraph->GetInNbrCSR(e);
				for (uint k = 0; k < in_neighbors.second; ++k)
				{
					auto v = in_neighbors.first[k];
					visited_edges++;
					if (arrSDu_s_[v] + i <= iK_)
					{
						if (arrVisitedCnt_[v] != visit_cnt && v != iSrc_)
						{
							rpa.push_back(v);
							tmp_Rm.push_back(v);
							tmp_Rpath.push_back(rpa);
							Rpaths_[i][v].push_back(rpa);
							rpa.pop_back();
							// 统计可join的
							if (!Lpaths_[i][v].empty())
							{
								JoinVerts_[i].first.insert(v); // first为平衡join L(i)---R(i)
							}
							if (i < (iK_ + 1) / 2 && !Lpaths_[i + 1][v].empty())
							{
								JoinVerts_[i].second.insert(v); // second为非平衡join L(i+1)---R(i) 即 L(i)---R(i-1)
							}
						}
					}
					else
						break;
				}
			}
			Rm.clear();
			Rm.swap(tmp_Rm);
			Rpath.clear();
			Rpath.swap(tmp_Rpath);
		}
	}
	auto point2 = std::chrono::high_resolution_clock::now();
	iRightDfsTime_ = std::chrono::duration_cast<std::chrono::nanoseconds>(point2 - point1).count();

	NativeJoin(oGraph);
	auto end = std::chrono::high_resolution_clock::now();
	iJoinTime_ = std::chrono::duration_cast<std::chrono::nanoseconds>(end - point2).count();

	time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	Lpaths_.clear();
	Rpaths_.clear();
	JoinVerts_.clear();
}

void CHcPE::generate_subtask_on_CSR(CDirectedGraph *&oGraph, ushort *&sdt, uint u, uint t, uint k, uint generate_max_num, MyVec &bufferPath, MyVec &bufferHop, uint &tmpPathCount, ulonglong &itime, ulonglong &iAnsNum)
{
    uint node_cnt = oGraph->iMaxVerId() + 1;
    if (node_cnt == 1) return;

    for (uint i = 0; i <= node_cnt; i++) {
        arrVisitedCnt_[i] = 0;
    }

    tmpPathCount = 0;
    uint travelCount = 0;
    std::queue<MyVec> qpath;
    std::queue<uint> qhop;

    MyVec tmpPath(MAX_K_Kernel, 0);
    tmpPath[0] = u;
    qpath.push(tmpPath);
    qhop.push(0);

    // 用来控制总任务量（已生成 + 队列中待处理）
    // 防止 k=6 时队列无限膨胀
    uint total_potential_tasks = 1;

    auto start = std::chrono::high_resolution_clock::now();

    if (k >= 3)
    {
        while (!qpath.empty())
        {
        	// 1. 获取当前待处理任务的层级
        	uint current_hop = qhop.empty() ? 0 : qhop.front();

        	// 2. 只有当任务足够多，并且层级足够深（FPGA处理起来不吃力）时，才允许停止分裂
        	bool enough_tasks = (tmpPathCount >= generate_max_num * 50); // 提高任务数门槛
        	bool queue_bloated = (qpath.size() > generate_max_num * 100); // 放宽队列限制

        	// 关键策略：如果当前 hop 还小于 2 (即 0 或 1)，绝对不要 break！
        	// 必须把 Hop 0 和 Hop 1 的大节点在 CPU 端拆散，否则 FPGA 必死。
        	bool safe_to_offload = (current_hop >= 2);

        	if (safe_to_offload && (enough_tasks || queue_bloated)) {
        	    break;
        	}

        	// 只有在内存真要爆了的极端情况下才强制 break (物理上限保护)
        	if (qpath.size() > 500000) break;

            uint hop = qhop.front();
            MyVec p = qpath.front();

            // 预判：如果当前任务不需要分裂，或者必须强制结束
            bool should_split = (hop < k / 2);

            // 如果不分裂，就不需要 pop 出来再展开了，直接 continue 让最后的清理循环去处理？
            // 不行，这里必须 pop 才能推进 BFS
            // 所以我们先看是否满足分裂条件

            if (!should_split) {
                // 不分裂，直接归档
                qpath.pop(); qhop.pop();
                for (auto i = 0; i < MAX_K_Kernel; i++) bufferPath.push_back(p[i]);
                bufferHop.push_back(hop);
                tmpPathCount++;
                continue;
            }

            // --- 开始分裂 ---
            qpath.pop(); qhop.pop();
            travelCount++;

            // 【修复 1】：只标记当前路径上真实存在的点，避免误伤 Node 0
            for (uint i = 0; i <= hop; ++i) {
                arrVisitedCnt_[p[i]] = travelCount;
            }

            uint cur_u = p[hop];
            auto out_nbr = oGraph->GetOutNbrCSR(cur_u);
            uint degree = out_nbr.second;

            for (uint i = 0; i < degree; i++)
            {
                auto v = out_nbr.first[i];

                if (sdt[v] + hop + 1 <= k)
                {
                    if (v == t) {
                        iAnsNum++;
                        continue;
                    }

                    if (arrVisitedCnt_[v] != travelCount)
                    {
                        if (hop == k - 2) {
                            iAnsNum++;
                        }
                        else {
                            p[hop + 1] = v;
                            qpath.push(p);
                            qhop.push(hop + 1);
                        }
                    }
                }
            }
        }
    }

    // 清理队列：把剩下的任务全部转为 buffer
    while(!qpath.empty()) {
        MyVec p = qpath.front(); qpath.pop();
        uint hop = qhop.front(); qhop.pop();
        for (auto i = 0; i < MAX_K_Kernel; i++) bufferPath.push_back(p[i]);
        bufferHop.push_back(hop);
        tmpPathCount++;
    }

    auto end = std::chrono::high_resolution_clock::now();
    itime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}


void CHcPE::generate_subtask_on_bigraph(uint u, uint t, uint k, uint node_cnt, uint generate_max_num, MyVec &bufferPath, MyVec &bufferHop, uint &tmpPathCount, ulonglong &itime, ulonglong &iAnsNum)
{
	for (uint i = 0; i <= node_cnt; i++)
	{
		arrVisitedCnt_[i] = 0;
	}
	tmpPathCount = 0;
	uint travelCount = 0;
	std::queue<MyVec> qpath;
	std::queue<uint> qhop;
	MyVec tmpPath(MAX_K_Kernel, 0);
	tmpPath[0] = u;
	qpath.push(tmpPath);
	qhop.push(0);
	tmpPathCount++;
	auto start = std::chrono::high_resolution_clock::now();
	if (k >= 3) // ! 条件
	{
		while (!qpath.empty())
		{
			MyVec p = qpath.front();
			uint hop = qhop.front();
			if (hop > k / 2) // ! 条件
				break;

			travelCount++;
			for (auto &vv : p) // 标记第几个路径task的已访问节点
			{
				arrVisitedCnt_[vv] = travelCount;
			}

			uint budget = k - hop - 1;
			uint neighbor_offset = single_bigraph_[p[hop]];
			uint start = single_bigraph_offset_[neighbor_offset];
			uint end = single_bigraph_offset_[neighbor_offset + budget + 1];
			if (tmpPathCount + end - start > generate_max_num) // ! 参数64
				break;

			qpath.pop();
			qhop.pop();
			tmpPathCount--;
			for (uint i = start; i < end; ++i)
			{
				uint v = single_bigraph_adj_[i];
				if (v == t)
				{
					iAnsNum++;
					continue;
				}
				if (arrVisitedCnt_[v] != travelCount)
				{
					if (hop == k - 2)
						iAnsNum++;
					else
					{
						p[hop + 1] = v;
						qpath.push(p);
						qhop.push(hop + 1);
						tmpPathCount++;
					}
				}
			}
		}
	}
	auto end = std::chrono::high_resolution_clock::now();
	itime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

	uint indexindex = 0;
	while (!qpath.empty())
	{
		MyVec p = qpath.front();
		qpath.pop();
		uint hop = qhop.front();
		qhop.pop();
		for (auto i = 0; i < MAX_K_Kernel; i++)
			bufferPath.push_back(p[i]);
		bufferHop.push_back(hop);
	}
}

ulonglong CHcPE::Excute(const query_method method_type, const uint iSouce, const uint iTarget, const bool count_only, uint &overlarge)
{
	assert(iK_ > 0);

	ulonglong itime = 0, ires = 0;
	iSrc_ = iSouce;
	iDst_ = iTarget;
	overlarge = 0;
	bCountOnly_ = count_only;

	switch (method_type)
	{
	case query_method::C_DFS: // 11 // 4 0.024657 5 1.812006 o(n^k)
	{
		iPreprocessTime_ = 0;
		CDFSPathEnumCSR(oGraph_, iQueryTime_);
		break;
	}
	case query_method::T_DFS: // 12 // 4 0.049063 5 1.049428 o(kmp)
	{
		iPreprocessTime_ = 0;
		TDFSPathEnumCSR(oGraph_, iQueryTime_);
		break;
	}
	case query_method::T_DFS2: // 13 // 4 0.053805 5 1.140277 o(kmp)
	{
		iPreprocessTime_ = 0;
		TDFS2PathEnumCSR(oGraph_, iQueryTime_);
		break;
	}
	case query_method::HP_Index: // 14 // 4 5 o(n^k)
	{
		if (is_first_query)
			Pre_HPIndex(oGraph_, iPreprocessTime_); // todo 暂时有问题
		HPIndexPathEnumCSR(oGraph_, iQueryTime_);
		break;
	}
	case query_method::BC_DFS: // 15 // 4 0.018675 5 0.865553
	{
		iPreprocessTime_ = 0;
		// BCDFSPathEnumCSR(oGraph_, iQueryTime_); // todo 暂时有问题
		BCDFSPathEnumCSR2(oGraph_, iQueryTime_); // 可以用
		break;
	}
	case query_method::BC_DFS_Induced: // 151 // 4 0.017932 5 0.766316
	{
		bool orderS = false, orderT = false;
		Pre_DouKHopBFS(iK_, orderS, orderT, iPreprocessTime_); // 4 0.013170 5 0.027910
		BCDFSPathEnumCSR2(oSubGraph_, iQueryTime_);			   // 4 0.004762 5 0.738406
		break;
	}
	case query_method::BC_DFS_Induced_BlockInitedBySDt: // 152
	{
		bool orderS = false, orderT = false;
		Pre_DouKHopBFS(iK_, orderS, orderT, iPreprocessTime_);
		BCDFSPathEnumCSR2_BlockInitedBySDt(oSubGraph_, iQueryTime_); // ! 错误的
		break;
	}
	case query_method::BC_JOIN: // 16 // 4 0.019075 5 0.057522
	{
		construct_pruned_dag_min_subgraph(iK_, iSrc_, iDst_, iPreprocessTime_); // 4 0.018801 5 0.051894
		iSubGraphNodeCount_ = dag_min_induced_subgraph.size() == 0 ? 0 : dag_min_induced_subgraph.size() + 1;
		// iSubGraphEdgeCount_ = 0; // 已经算好
		BCJOINPathEnumCSR(iQueryTime_); // 4 0.000274 5 0.005628
		break;
	}
	case query_method::IDX_DFS: // 17 // 4 0.000135 5 0.000735
	{
		Pre_BiGraphIDX(iPreprocessTime_); // 4 0.000133 5 0.000657
		IDXDFSPathEnumCSR(iQueryTime_);	  // 4 0.000002 5 0.000078
		iQueryTime_ = iPreprocessTime_+iQueryTime_;
		iSubGraphNodeCount_ = active_vertices_count_;
		iSubGraphEdgeCount_ = single_bigraph_adj_size_;
		clear_bigraph();
		break;
	}
	case query_method::IDX_JOIN: // 18 // 4 0.003072 5 0.003794
	{
		Pre_BiGraphIDX(iPreprocessTime_); // 4 0.000200 5 0.000830
		IDXJOINPathEnumCSR(iQueryTime_);  // 4 0.002872 5 0.002964
		iSubGraphNodeCount_ = active_vertices_count_;
		iSubGraphEdgeCount_ = single_bigraph_adj_size_;
		clear_bigraph();
		break;
	}
	case query_method::IDX_ENUM: // 19 //  4 0.000200 5 0.000561
	{
		Pre_BiGraphIDX(iPreprocessTime_); // 4 0.000187 5 0.000510
		IDXMIXPathEnumCSR(iQueryTime_);	  // 4 0.000003 5 0.000051
		iSubGraphNodeCount_ = active_vertices_count_;
		iSubGraphEdgeCount_ = single_bigraph_adj_size_;
		clear_bigraph();
		break;
	}
	case query_method::MY_DFS: // 61 // 4 0.000057 5 0.000382
	{
		bool orderS = false, orderT = true;
		// Pre_DouKHopBFS(iK_, orderS, orderT, iPreprocessTime_);				// 4 0.013170 5 0.027910
		// Pre_DouKHopBFS(iK_ - 1, orderS, orderT, iPreprocessTime_);			// 4 0.004094 5 0.012923
		// Pre_DouHalfHopBFS(orderS, orderT, iPreprocessTime_);				// 0.000134
		// Pre_DouHalfHopBFS_1(orderS, orderT, iPreprocessTime_);				// 0.000126
		// Pre_DouHalfHopAndSigHalfHopBFS(orderS, orderT, iPreprocessTime_);	// 0.000118
		// Pre_DouHalfHopAndSigHalfHopBFS_1(orderS, orderT, iPreprocessTime_); // 0.000118
		Pre_DouHalfHopAndDouHalfHopBFS(orderS, orderT, iPreprocessTime_); // 4 0.000055 5 0.000348
		MYDFSPathEnumCSR(oSubGraph_, iQueryTime_);						  // 4 0.000002 5 0.000034
		// MYDFSPathEnumCSR_1(oSubGraph_, iQueryTime_);						// 0.000001 0.000124(6)
		// MYDFSPathEnumCSR_2(oSubGraph_, iQueryTime_);						// 0.000001 0.000448(6)
		iSubGraphNodeCount_ = oSubGraph_->iVexNum();
		iSubGraphEdgeCount_ = oSubGraph_->iArcNum();
		break;
	}
	case query_method::MY_DFS2: // 64 排序统计 // 4 0.000058 5 0.000522
	{
		uint s = iSrc_, t = iDst_, k = iK_;
		bool orderS = false, orderT = true;
		std::unordered_map<uint, uint> nodemap;
		Pre_DouHalfHopAndDouHalfHopBFS(orderS, orderT, nodemap, iPreprocessTime_); // 4 0.000057 5 0.000467
		// MYDFSPathEnumCSR(oSubGraph_, nodemap[s], nodemap[t], iQueryTime_);		   // 4 0.000001 5 0.000055
		iSubGraphNodeCount_ = oSubGraph_->iVexNum();
		iSubGraphEdgeCount_ = oSubGraph_->iArcNum();
		break;
	}
	case query_method::MY_DFS3: // 65 无排序统计 // 4 0.000061 5 0.000461
	{
		uint s = iSrc_, t = iDst_, k = iK_;
		bool orderS = false, orderT = true;
		Pre_DouHalfHopAndDouHalfHopBFS(orderS, orderT, iPreprocessTime_); // 4 0.000061 5 0.000430
		oSubGraph_->ShrinkCSR2(itime);
		for (uint i = 0; i <= oGraph_->iMaxVerId(); i++)
		{
			uint newid = i;
			if (oSubGraph_->GetShrinkID(newid))
			{
				arrSDu_t_[newid] = arrSDu_t_[i];
			}
		}
		oSubGraph_->GetShrinkID(s);
		oSubGraph_->GetShrinkID(t);
		MYDFSPathEnumCSR(oSubGraph_, s, t, iQueryTime_); // 4 0.000003 5 0.000031
		iSubGraphNodeCount_ = oSubGraph_->iVexNum();
		iSubGraphEdgeCount_ = oSubGraph_->iArcNum();
		break;
	}
	case query_method::PEFP_DFS: // 62 // 4 0.004107 5 0.013452
	{
		bool orderS = false, orderT = false;
		Pre_DouKHopBFS(iK_ - 1, orderS, orderT, iPreprocessTime_); // 4 0.004094 5 0.012923
		iSubGraphNodeCount_ = oSubGraph_->iVexNum();
		iSubGraphEdgeCount_ = oSubGraph_->iArcNum();
		// PEFPPathEnumCSR(oSubGraph_, iQueryTime_); // 4 0.000013 5 0.000529
		break;
	}
	case query_method::MY_JOIN: // 63 // 4 5
	{
		bool orderS = false, orderT = false;
		Pre_DouKHopBFS(iK_ - 1, orderS, orderT, iPreprocessTime_); //
		MYJOINPathEnumCSR(oSubGraph_, iQueryTime_);				   //
		break;
	}
	case query_method::FPGA_PEFP: // 71 // 4 5
	{
		bool orderS = false, orderT = false;
		uint s = iSrc_, t = iDst_, k = iK_;

		Pre_DouKHopBFS(iK_ - 1, orderS, orderT, iPreprocessTime_);
		oSubGraph_->ShrinkCSR(itime);
		for (uint i = 0; i <= oGraph_->iMaxVerId(); i++)
		{
			uint newid = i;
			if (oSubGraph_->GetShrinkID(newid))
			{
				arrSDu_t_[newid] = arrSDu_t_[i];
			}
		}
		oSubGraph_->GetShrinkID(s);
		oSubGraph_->GetShrinkID(t);

		uint iSizeV = oSubGraph_->iMaxVerId() + 1;
		uint iSizeE = oSubGraph_->iArcNum();
		iSubGraphNodeCount_ = iSizeV;
		iSubGraphEdgeCount_ = iSizeE;
#ifdef USE_FPGA
		oHost_->PEFPOnFPGA(oSubGraph_, s, t, k, arrSDu_t_, iQueryTime_); // pefp_re
		iPathCount_ = oHost_->deviceResults()[0];
		overlarge = oHost_->deviceResults()[1];		   // err
		iDDR2BRAMTimes_ = oHost_->deviceResults()[2];  // d2b
		iBRAM2DDRTimes_ = oHost_->deviceResults()[3];  // b2d
		iBRAM2DDRDatas_ = oHost_->deviceResults()[4];  // loop
		iDDR2BRAMDatas_ = oHost_->deviceResults()[5];  // checkpath
		iTempPathsCount_ = oHost_->deviceResults()[6]; // checknode
		if (overlarge != 0)
			log_info("err: %lu", overlarge);
#else
		if (iSizeE == 0 || iSizeV == 1)
		{
			iPathCount_ = 0;
			iQueryTime_ = 0;
			break;
		}
		else if (iSizeV + 1 > 10000 || iSizeE > 100000)
		{
			// log_info("warning bigdata: %u , %u", iSizeV + 1, iSizeE);
		}
		ushort iPadding = 9;
		size_t iDDRSize = 1024 * 64 * iPadding;
		size_t iResSize = 1;
		size_t iErrorSize = 50;
		MyVecShort vSDut(iSizeV + 1);
		MyVec vPos(iSizeV + 2);
		MyVec vEdg(iSizeE);
		MyVec bufferDevicePaths(iDDRSize, 0);
		MyVecLong vRes(iResSize, 0);
		MyVecLong vError(iErrorSize, 0);
		for (uint i = 0; i < iSizeV; i++)
		{
			vSDut[i + 1] = arrSDu_t_[i];
			vPos[i + 1] = oSubGraph_->arrOutOffset()[i]; // 点的编号加1
		}
		vPos[iSizeV + 1] = oSubGraph_->arrOutOffset()[iSizeV];
		for (uint i = 0; i < iSizeE; i++)
		{
			vEdg[i] = oSubGraph_->arrOutAdj()[i] + 1; // 点的编号加1
		}
		uint iDDRTempSize = 1;

		bufferDevicePaths[0] = s + 1;
		for (uint i = 1; i < iPadding; i++)
		{
			bufferDevicePaths[i] = 0;
		}
		auto start = std::chrono::high_resolution_clock::now();
		oTest_->task_parallel(vPos, vEdg, vSDut, bufferDevicePaths, vRes, vError, iSizeV + 1, iSizeE, iDDRTempSize, t + 1, iK_, 360 * 1000 * 1000); // 计算pathcount
		auto end = std::chrono::high_resolution_clock::now();
		iQueryTime_ = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		iPathCount_ = vRes[0];
		overlarge = vError[0];		  // err
		iDDR2BRAMTimes_ = vError[1];  // d2b
		iBRAM2DDRTimes_ = vError[2];  // b2d
		iBRAM2DDRDatas_ = vError[3];  // loop
		iDDR2BRAMDatas_ = vError[4];  // checkpath
		iTempPathsCount_ = vError[5]; // checknode
		if (overlarge != 0)
			log_info("err: %lu", vError[0]);
#endif
		break;
	}
	case query_method::FPGA_MYDFS: // 72
	{
		bool orderS = false, orderT = true;
		uint s = iSrc_, t = iDst_, k = iK_;

		std::unordered_map<uint, uint> nodemap;
		Pre_DouHalfHopAndDouHalfHopBFS(orderS, orderT, nodemap, iPreprocessTime_);
		s = nodemap[s];
		t = nodemap[t];

		uint iSizeV = oSubGraph_->iMaxVerId() + 1;
		uint iSizeE = oSubGraph_->iArcNum();
		iSubGraphNodeCount_ = iSizeV;
		iSubGraphEdgeCount_ = iSizeE;
#ifdef USE_FPGA
		oHost_->MYDFSOnFPGA2(oSubGraph_, s, t, k, arrSDu_t_, iQueryTime_);
		iPathCount_ = oHost_->deviceResults()[0];
		overlarge = oHost_->deviceResults()[1];		   // err
		iDDR2BRAMTimes_ = oHost_->deviceResults()[2];  // d2b
		iBRAM2DDRTimes_ = oHost_->deviceResults()[3];  // b2d
		iBRAM2DDRDatas_ = oHost_->deviceResults()[4];  // loop
		iDDR2BRAMDatas_ = oHost_->deviceResults()[5];  // checkpath
		iTempPathsCount_ = oHost_->deviceResults()[6]; // checknode
		if (overlarge != 0)
			log_info("err: %lu", overlarge);
#else
		if (iSizeV == 1 || iSizeE == 0)
		{
			iPathCount_ = 0;
			iQueryTime_ = 0;
			break;
		}
		else if (iSizeV > 10000 || iSizeE > 100000)
		{
			log_info("warning bigdata: %u , %u", iSizeV, iSizeE);
		}

		ushort iPadding = 9;
		size_t iDDRSize = 1024 * 64 * iPadding;
		size_t iResSize = 1;
		size_t iErrorSize = 50;
		MyVecShort vSDut(iSizeV);
		MyVec vPos(iSizeV + 1);
		MyVec vEdg(iSizeE);
		MyVec bufferDevicePaths(iDDRSize, 0);
		MyVecLong vRes(iResSize, 0);
		MyVecLong vError(iErrorSize, 0);
		for (uint i = 0; i < iSizeV; i++)
		{
			vSDut[i] = arrSDu_t_[i];
			vPos[i] = oSubGraph_->arrOutOffset()[i];
		}
		vPos[iSizeV] = oSubGraph_->arrOutOffset()[iSizeV];
		for (uint i = 0; i < iSizeE; i++)
		{
			vEdg[i] = oSubGraph_->arrOutAdj()[i];
		}
		uint iDDRTempSize = 1;

		bufferDevicePaths[0] = s;
		for (uint i = 1; i < iPadding; i++)
		{
			bufferDevicePaths[i] = 0;
		}
		auto start = std::chrono::high_resolution_clock::now();
		oTest_->MYDFS(vPos, vEdg, vSDut, bufferDevicePaths, vRes, vError, iSizeV, iSizeE, iDDRTempSize, t, iK_, 360 * 1000 * 1000); // 点的编号加1
		auto end = std::chrono::high_resolution_clock::now();
		iQueryTime_ = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		iPathCount_ = vRes[0];
		overlarge = vError[0];		  // err
		iDDR2BRAMTimes_ = vError[1];  // d2b
		iBRAM2DDRTimes_ = vError[2];  // b2d
		iBRAM2DDRDatas_ = vError[3];  // loop
		iDDR2BRAMDatas_ = vError[4];  // checkpath
		iTempPathsCount_ = vError[5]; // checknode
		if (overlarge != 0)
			log_info("err: %lu", vError[0]);
#endif
		break;
	}
	case query_method::FPGA_MYDFS_R: // 721
	{
		bool orderS = false, orderT = true;
		uint s = iSrc_, t = iDst_, k = iK_;

		Pre_DouHalfHopAndDouHalfHopBFS(orderS, orderT, iPreprocessTime_);
		oSubGraph_->ShrinkCSR(itime);
		// iPreprocessTime_ += itime;
		for (uint i = 0; i <= oGraph_->iMaxVerId(); i++)
		{
			uint newid = i;
			if (oSubGraph_->GetShrinkID(newid))
			{
				arrSDu_t_[newid] = arrSDu_t_[i];
			}
		}
		oSubGraph_->GetShrinkID(s);
		oSubGraph_->GetShrinkID(t);

		uint iSizeV = oSubGraph_->iMaxVerId() + 1;
		uint iSizeE = oSubGraph_->iArcNum();
		iSubGraphNodeCount_ = iSizeV;
		iSubGraphEdgeCount_ = iSizeE;
#ifdef USE_FPGA
		oHost_->MYDFSOnFPGA3(oSubGraph_, s, t, k, arrSDu_t_, iQueryTime_);
		iPathCount_ = oHost_->deviceResults()[0];
		overlarge = oHost_->deviceResults()[1];		   // err
		iDDR2BRAMTimes_ = oHost_->deviceResults()[2];  // d2b
		iBRAM2DDRTimes_ = oHost_->deviceResults()[3];  // b2d
		iBRAM2DDRDatas_ = oHost_->deviceResults()[4];  // loop
		iDDR2BRAMDatas_ = oHost_->deviceResults()[5];  // checkpath
		iTempPathsCount_ = oHost_->deviceResults()[6]; // checknode
		if (overlarge != 0)
			log_info("err: %lu", overlarge);
#else
		if (iSizeV == 1 || iSizeE == 0)
		{
			iPathCount_ = 0;
			iQueryTime_ = 0;
			break;
		}
		else if (iSizeV > 10000 || iSizeE > 100000)
		{
			log_info("warning bigdata: %u , %u", iSizeV, iSizeE);
		}

		ushort iPadding = 9;
		size_t iDDRSize = 1024 * 64 * iPadding;
		size_t iResSize = 1;
		size_t iErrorSize = 50;
		MyVecShort vSDut(iSizeV + 1);
		MyVec vPos(iSizeV + 2);
		MyVec vEdg(iSizeE);
		MyVec bufferDevicePaths(iDDRSize, 0);
		MyVecLong vRes(iResSize, 0);
		MyVecLong vError(iErrorSize, 0);
		for (uint i = 0; i < iSizeV; i++)
		{
			vSDut[i + 1] = arrSDu_t_[i];
			vPos[i + 1] = oSubGraph_->arrOutOffset()[i];
		}
		vPos[iSizeV + 1] = oSubGraph_->arrOutOffset()[iSizeV];
		for (uint i = 0; i < iSizeE; i++)
		{
			vEdg[i] = oSubGraph_->arrOutAdj()[i] + 1;
		}
		uint iDDRTempSize = 1;

		bufferDevicePaths[0] = s + 1;
		for (uint i = 1; i < iPadding; i++)
		{
			bufferDevicePaths[i] = 0;
		}
		auto start = std::chrono::high_resolution_clock::now();
		// oTest_->MYDFS(vPos, vEdg, vSDut, bufferDevicePaths, vRes, vError, iSizeV + 1, iSizeE, iDDRTempSize, t + 1, iK_, 360 * 1000 * 1000); // 点的编号加1
		auto end = std::chrono::high_resolution_clock::now();
		iQueryTime_ = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		iPathCount_ = vRes[0];
		overlarge = vError[0];		  // err
		iDDR2BRAMTimes_ = vError[1];  // d2b
		iBRAM2DDRTimes_ = vError[2];  // b2d
		iBRAM2DDRDatas_ = vError[3];  // loop
		iDDR2BRAMDatas_ = vError[4];  // checkpath
		iTempPathsCount_ = vError[5]; // checknode
		if (overlarge != 0)
			log_info("err: %lu", vError[0]);
#endif
		break;
	}
	case query_method::FPGA_MYDFS_Multi: // 722
	{
		bool orderS = false, orderT = true;
		uint is = iSrc_, it = iDst_, ik = iK_;
		iPathCount_ = 0;

		std::unordered_map<uint, uint> nodemap;
		Pre_DouHalfHopAndDouHalfHopBFS(orderS, orderT, nodemap, iPreprocessTime_);
		is = nodemap[is];
		it = nodemap[it];

		uint iSizeV = oSubGraph_->iMaxVerId() + 1;
		uint iSizeE = oSubGraph_->iArcNum();
		iSubGraphNodeCount_ = iSizeV;
		iSubGraphEdgeCount_ = iSizeE;

		MyVec ppath;
		MyVec hhop;
		uint tmpPathCount = 0;
		generate_subtask_on_CSR(oSubGraph_, arrSDu_t_, is, it, ik, 64, ppath, hhop, tmpPathCount, itime, iPathCount_);
		// log_info("pc %lu", tmpPathCount);

		double theta = 0.8;
		uint diviPathcount = tmpPathCount * theta;
		iPreprocessTime_ = 0;
		for (uint i = diviPathcount; i < tmpPathCount; i++)
		{
			std::vector<uint> pp(MAX_K_Kernel, 0);
			for (uint j = 0; j < MAX_K_Kernel; j++)
			{
				pp[j] = ppath[i * MAX_K_Kernel + j];
			}
			ulonglong ttttt = 0;
			MYDFSPathEnumCSR(oSubGraph_, pp, hhop[i], it, ttttt);
			iPreprocessTime_ += ttttt;
		}
#ifdef USE_FPGA
		oHost_->MYDFSOnFPGA_Multi(oSubGraph_, is, it, ik, arrSDu_t_, iQueryTime_, ppath, hhop, diviPathcount);
		iPathCount_ += oHost_->deviceResults()[0];
		overlarge = oHost_->deviceResults()[1];		   // err
		iDDR2BRAMTimes_ = oHost_->deviceResults()[2];  // d2b
		iBRAM2DDRTimes_ = oHost_->deviceResults()[3];  // b2d
		iBRAM2DDRDatas_ = oHost_->deviceResults()[4];  // loop
		iDDR2BRAMDatas_ = oHost_->deviceResults()[5];  // checkpath
		iTempPathsCount_ = oHost_->deviceResults()[6]; // checknode

		// if (overlarge != 0)
		// 	log_info("err: %lu", overlarge);
#else
		if (iSizeV == 1 || iSizeE == 0)
		{
			iPathCount_ = 0;
			iQueryTime_ = 0;
			break;
		}
		else if (iSizeV > 10000 || iSizeE > 100000)
		{
			log_info("warning bigdata: %u , %u", iSizeV, iSizeE);
		}

		ushort iPadding = 9;
		size_t iDDRSize = 1024 * 64 * iPadding;
		size_t iResSize = 1;
		size_t iErrorSize = 50;
		MyVecShort vSDut(iSizeV);
		MyVec vPos(iSizeV + 1);
		MyVec vEdg(iSizeE);
		MyVecLong vRes(iResSize, 0);
		MyVecLong vError(iErrorSize, 0);
		for (uint i = 0; i < iSizeV; i++)
		{
			vSDut[i] = arrSDu_t_[i];
			vPos[i] = oSubGraph_->arrOutOffset()[i];
		}
		vPos[iSizeV] = oSubGraph_->arrOutOffset()[iSizeV];
		for (uint i = 0; i < iSizeE; i++)
		{
			vEdg[i] = oSubGraph_->arrOutAdj()[i];
		}

		uint core_num = 6;
		for (uint kore = 0; kore < core_num; kore++)
		{
			MyVec sub_task(iDDRSize, 0);
			uint p_count = 0;
			for (uint t = kore; t < diviPathcount; t += core_num) // t < tmpPathCount * 0.2
			{
				for (uint k = 0; k < MAX_K_Kernel; k++)
					sub_task[p_count * iPadding + k] = ppath[t * MAX_K_Kernel + k];
				sub_task[p_count * iPadding + MAX_K_Kernel] = hhop[t];
				p_count++;
			}
			log_info("cu %u pc: %lu", kore, p_count);
			auto start = std::chrono::high_resolution_clock::now();
			oTest_->MYDFS(vPos, vEdg, vSDut, sub_task, vRes, vError, iSizeV, iSizeE, p_count, it, ik, 360 * 1000 * 1000);
			auto end = std::chrono::high_resolution_clock::now();
			auto iQueryTime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
			iQueryTime_ = iQueryTime > iQueryTime_ ? iQueryTime : iQueryTime_;
			iPathCount_ += vRes[0];
			overlarge = vError[0];		   // err
			iDDR2BRAMTimes_ += vError[1];  // d2b
			iBRAM2DDRTimes_ += vError[2];  // b2d
			iBRAM2DDRDatas_ += vError[3];  // loop
			iDDR2BRAMDatas_ += vError[4];  // checkpath
			iTempPathsCount_ += vError[5]; // checknode
			if (overlarge != 0)
				log_info("cu %u err: %lu", kore, vError[0]);
		}
#endif
		break;
	}



	case query_method::FPGA_MYJOIN:
	{
		break;
	}
	case query_method::FPGA_IDX_DFS:
	{
		Pre_BiGraphIDX(iPreprocessTime_);
		spp::sparse_hash_map<uint, uint> nodemap;
		Pre_BiGraphIDX_Shrink(nodemap, itime);
		// iPreprocessTime_ += itime;
		uint iSizeV = active_vertices_count_ + 1; // shrink后+1，已验证
		uint iSizeE = single_bigraph_adj_size_;
		iSubGraphNodeCount_ = iSizeV;
		iSubGraphEdgeCount_ = iSizeE;
#ifdef USE_FPGA
		overlarge = !oHost_->IDXDFSOnFPGA2(nodemap[iSrc_], nodemap[iDst_], iK_, single_bigraph_node_, single_bigraph_offset_, single_bigraph_adj_, active_vertices_count_, single_bigraph_adj_size_, iQueryTime_);
		iPathCount_ = oHost_->deviceResults()[0];
		overlarge = oHost_->deviceResults()[1];
		if (overlarge != 0)
			log_info("err: %lu", oHost_->deviceResults()[1]);
		iDDR2BRAMTimes_ = oHost_->deviceResults()[2];
		iBRAM2DDRTimes_ = oHost_->deviceResults()[3];
#else
		if (iSizeV == 1 || iSizeE == 0)
		{
			iPathCount_ = 0;
			iQueryTime_ = 0;
			clear_bigraph();
			break;
		}
		if (iSizeV > 80 || iSizeE > 800)
		{
			log_info("warning bigdata: %u , %u", iSizeV, iSizeE);
		}
		ushort iPadding = 9;
		size_t iDDRSize = 1024 * iPadding;
		size_t iResSize = 1;
		size_t iDebugSize = 20;
		MyVec vVer(iSizeV);
		MyVec vPos(iSizeV * iK_);
		MyVec vEdg(iSizeE);
		MyVec bufferDevicePaths(iDDRSize, 0);
		MyVecLong vRes(iResSize, 0);
		MyVecLong vDebug(iDebugSize, 0);
		for (uint i = 0; i < iSizeV; i++)
		{
			vVer[i] = single_bigraph_node_[i];
			for (auto j = 0; j < iK_; j++)
				vPos[i * iK_ + j] = single_bigraph_offset_[i * iK_ + j];
		}
		for (uint i = 0; i < iSizeE; i++)
		{
			vEdg[i] = single_bigraph_adj_[i];
		}
		bufferDevicePaths[0] = nodemap[iSrc_];
		for (uint i = 1; i < iPadding; i++)
		{
			bufferDevicePaths[i] = 0;
		}
		auto start = std::chrono::high_resolution_clock::now();
		// oTest_->IDX_DFS_FPGA(vVer, vPos, vEdg, bufferDevicePaths, vRes, vDebug, nodemap[iDst_], iK_, iSizeV, iSizeE, 1);
		iPathCount_ = vRes[0];
		auto end = std::chrono::high_resolution_clock::now();
		overlarge = vDebug[0];
		if (overlarge != 0)
			log_info("err: %lu", vDebug[0]);
		iDDR2BRAMTimes_ = vDebug[1];
		iBRAM2DDRTimes_ = vDebug[2];
		iQueryTime_ = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif
		clear_bigraph();
		break;
	}
	case query_method::FPGA_IDX_DFS_Multi:
	{
		Pre_BiGraphIDX(itime);
		iPreprocessTime_ = itime;

		spp::sparse_hash_map<uint, uint> nodemap;
		Pre_BiGraphIDX_Shrink(nodemap, itime);
		// iPreprocessTime_ += itime;

		uint iSizeV = active_vertices_count_ + 1; // shrink后+1，已验证
		uint iSizeE = single_bigraph_adj_size_;
		iSubGraphNodeCount_ = iSizeV;
		iSubGraphEdgeCount_ = iSizeE;

		MyVec bufferPath, bufferHop;
		uint tmpPathCount;
		generate_subtask_on_bigraph(nodemap[iSrc_], nodemap[iDst_], iK_, nodemap.size(), 64, bufferPath, bufferHop, tmpPathCount, itime, iPathCount_);
		// iPreprocessTime_ += itime;
#ifdef USE_FPGA
		overlarge = !oHost_->IDXDFSOnFPGA_Multi2(nodemap[iSrc_], nodemap[iDst_], iK_,
												 single_bigraph_node_, single_bigraph_offset_, single_bigraph_adj_,
												 active_vertices_count_, single_bigraph_adj_size_, iQueryTime_,
												 bufferPath, bufferHop, tmpPathCount);
		iPathCount_ += oHost_->deviceResults()[0];
		if (overlarge != 0)
			log_info("err: %lu", oHost_->deviceResults()[1]);
		iDDR2BRAMTimes_ = oHost_->deviceResults()[2];
		iBRAM2DDRTimes_ = oHost_->deviceResults()[3];
#else
		if (iSizeV == 1 || iSizeE == 0)
		{
			iPathCount_ = 0;
			iQueryTime_ = 0;
			clear_bigraph();
			break;
		}
		if (iSizeV > 80 || iSizeE > 800)
		{
			log_info("warning bigdata: %u , %u", iSizeV, iSizeE);
		}

		ushort iPadding = 9;
		size_t iDDRSize = 1024 * iPadding;
		size_t iResSize = 1;
		size_t iDebugSize = 20;
		MyVec vVer(iSizeV);
		MyVec vPos(iSizeV * iK_);
		MyVec vEdg(iSizeE);
		// MyVec bufferDevicePaths(iDDRSize, 0);
		MyVecLong vRes(iResSize, 0);
		MyVecLong vDebug(iDebugSize, 0);
		for (uint i = 0; i < iSizeV; i++)
		{
			vVer[i] = single_bigraph_node_[i];
			for (auto j = 0; j < iK_; j++)
				vPos[i * iK_ + j] = single_bigraph_offset_[i * iK_ + j];
		}
		for (uint i = 0; i < iSizeE; i++)
		{
			vEdg[i] = single_bigraph_adj_[i];
		}
		uint core_num = 8;
		for (uint kore = 0; kore < core_num; kore++)
		{
			MyVec sub_task(iDDRSize, 0);
			uint p_count = 0;
			for (uint t = kore; t < tmpPathCount; t += core_num)
			{
				for (uint k = 0; k < MAX_K_Kernel; k++)
					sub_task[p_count * iPadding + k] = bufferPath[t * MAX_K_Kernel + k];
				sub_task[p_count * iPadding + MAX_K_Kernel] = bufferHop[t];
				p_count++;
			}
			auto start = std::chrono::high_resolution_clock::now();
			// oTest_->IDX_DFS_FPGA(vVer, vPos, vEdg, sub_task, vRes, vDebug, nodemap[iDst_], iK_, iSizeV, iSizeE, p_count);
			auto end = std::chrono::high_resolution_clock::now();
			iPathCount_ += vRes[0];
			overlarge = vDebug[0];
			if (overlarge != 0)
			{
				log_info("err: %lu", vDebug[0]);
			}
			iDDR2BRAMTimes_ += vDebug[1];
			iBRAM2DDRTimes_ += vDebug[2];
			auto iQueryTime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
			iQueryTime_ = iQueryTime > iQueryTime_ ? iQueryTime : iQueryTime_;
		}
#endif
		clear_bigraph();
		break;
	}
	default:
		break;
	}

	ires = iPathCount_;
	update_performance_counter();
	reset_performance_counter();
	if (is_first_query)
		is_first_query = false;
	return ires;
}
