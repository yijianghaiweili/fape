#include "CDirectedGraph.h"

void CDirectedGraph::Init()
{
	setNode_.clear();
	mapShrink_.clear();
	vOutDegreeDistribution_.clear();
	vOutDegreeDistribution_.clear();
	iVexNum_ = 0;
	iArcNum_ = 0;
	iMaxInNbrNum_ = 0;
	iMaxOutNbrNum_ = 0;
	iMaxVerId_ = 0;
	vGraph_.clear();
	vGraphRev_.clear();
	free(arrOutOffset_);
	arrOutOffset_ = nullptr;
	free(arrOutAdj_);
	arrOutAdj_ = nullptr;
	free(arrOutDegree_);
	arrOutDegree_ = nullptr;
	free(arrInOffset_);
	arrInOffset_ = nullptr;
	free(arrInAdj_);
	arrInAdj_ = nullptr;
	free(arrInDegree_);
	arrInDegree_ = nullptr;
}

void CDirectedGraph::PrintMeta()
{
#ifdef USE_LOG
	log_info("Graph Node Size: %u", iVexNum_);
	log_info("Graph Edge Size: %u", iArcNum_);
	log_info("Graph Max Indegree Size: %u", iMaxInNbrNum_);
	log_info("Graph Max Outdegree Size: %u", iMaxOutNbrNum_);
	log_info("Graph Max Node ID: %u", iMaxVerId_);
#else
	std::cout << "Graph Node Size: " << iVexNum_ << std::endl;
	std::cout << "Graph Edge Size: " << iArcNum_ << std::endl;
	std::cout << "Graph Max Indegree Size: " << iMaxInNbrNum_ << std::endl;
	std::cout << "Graph Max Outdegree Size: " << iMaxOutNbrNum_ << std::endl;
	std::cout << "Graph Max Node ID: " << iMaxVerId_ << std::endl
			  << std::endl;
#endif
}

void CDirectedGraph::InsertEdgeAdjList(uint iHead, uint iTail)
{
	setNode_.insert(iHead);
	setNode_.insert(iTail);
	uint iNewSize = std::max(iHead, iTail);
	if (iNewSize > iMaxVerId_)
	{
		// vGraph_.resize(iNewSize + 1);
		// vGraphRev_.resize(iNewSize + 1);
		iMaxVerId_ = iNewSize;
	}
	vGraph_[iHead].emplace_back(iTail);
	vGraphRev_[iTail].emplace_back(iHead);
	iVexNum_ = setNode_.size();
	iArcNum_++;
}

void CDirectedGraph::InsertVertexAdjList(uint iNode)
{
	setNode_.insert(iNode);
	if (iNode > iMaxVerId_)
	{
		// vGraph_.resize(iNode + 1);
		// vGraphRev_.resize(iNode + 1);
		iMaxVerId_ = iNode;
	}
	iVexNum_ = setNode_.size();
}

void CDirectedGraph::DeleteEdgeAdjList(uint iHead, uint iTail)
{
	// TODO
}

void CDirectedGraph::DeleteVertexAdjList(uint iNode)
{
	// TODO
}

void CDirectedGraph::CreateGraphAdjList(const std::string &strFilePath, const char cSkip)
{
	auto start = std::chrono::high_resolution_clock::now();

	std::ifstream steFile(strFilePath);
	if (!steFile.is_open())
	{
		log_error("failed to open %s", strFilePath.c_str());
		// exit(-1);
		return;
	}
	uint max_vertex_id = 0;
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
				log_error("Cannot convert line %u to edge.", line_count);
				// exit(-1);
				return;
			}
			if (src > max_vertex_id)
			{
				max_vertex_id = src;
			}
			if (dst > max_vertex_id)
			{
				max_vertex_id = dst;
			}
			setNode_.insert(src); // 计算点数
			setNode_.insert(dst); // 计算点数
			// vGraph_.resize(max_vertex_id + 1);
			// vGraphRev_.resize(max_vertex_id + 1);
			vGraph_[src].emplace_back(dst);
			vGraphRev_[dst].emplace_back(src);
		}
	}
	iVexNum_ = setNode_.size(); // 计算点数
	iMaxVerId_ = max_vertex_id;
	iArcNum_ = line_count;

	uint dsize = 0;
	uint dsizetemp = 0;
	for (auto &a : vGraph_)
	{
		dsizetemp = a.size();
		dsize = (dsize < dsizetemp) ? dsizetemp : dsize;
	}
	iMaxOutNbrNum_ = dsize;

	dsize = 0;
	dsizetemp = 0;
	for (auto &a : vGraphRev_)
	{
		dsizetemp = a.size();
		dsize = (dsize < dsizetemp) ? dsizetemp : dsize;
	}
	iMaxInNbrNum_ = dsize;

	auto end = std::chrono::high_resolution_clock::now();
	log_info("CreateGraphAdjList time: %.6lf seconds", std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1000000000.0);
}

void CDirectedGraph::InsertEdgeAdjListShrink(uint iHead, uint iTail)
{
	if (mapShrink_.find(iHead) == mapShrink_.end())
	{
		uint newid = mapShrink_.size() + 1;
		mapShrink_[iHead] = newid;
		iHead = newid;
		setNode_.insert(iHead);
	}
	else
	{
		iHead = mapShrink_[iHead];
	}
	if (mapShrink_.find(iTail) == mapShrink_.end())
	{
		uint newid = mapShrink_.size() + 1;
		mapShrink_[iTail] = newid;
		iTail = newid;
		setNode_.insert(iTail);
	}
	else
	{
		iHead = mapShrink_[iTail];
	}
	iMaxVerId_ = std::max(iHead, iTail);
	vGraph_[iHead].emplace_back(iTail);
	vGraphRev_[iTail].emplace_back(iHead);
	iVexNum_ = setNode_.size();
	iArcNum_++;
}

void CDirectedGraph::InsertVertexAdjListShrink(uint iNode)
{
	// TODO
}

void CDirectedGraph::DeleteEdgeAdjListShrink(uint iHead, uint iTail)
{
	// TODO
}

void CDirectedGraph::DeleteVertexAdjListShrink(uint iNode)
{
	// TODO
}

void CDirectedGraph::CreateGraphAdjListShrink(const std::string &strFilePath, const char cSkip)
{
	auto start = std::chrono::high_resolution_clock::now();

	std::ifstream steFile(strFilePath);
	if (!steFile.is_open())
	{
		log_error("failed to open %s", strFilePath.c_str());
		// exit(-1);
		return;
	}
	mapShrink_.clear();
	uint max_vertex_id = 0;
	uint line_count = 0;
	uint src, dst;
	std::string tmp_str;
	uint iNewid = 0;
	while (std::getline(steFile, tmp_str))
	{
		line_count += 1;
		if (tmp_str[0] != cSkip)
		{
			std::stringstream ss(tmp_str);
			if (!(ss >> src >> dst))
			{
				log_error("Cannot convert line %u to edge.", line_count);
				// exit(-1);
				return;
			}
			if (src > max_vertex_id)
			{
				max_vertex_id = src;
			}
			if (dst > max_vertex_id)
			{
				max_vertex_id = dst;
			}
			if (mapShrink_.find(src) == mapShrink_.end())
			{
				mapShrink_[src] = ++iNewid;
			}
			if (mapShrink_.find(dst) == mapShrink_.end())
			{
				mapShrink_[dst] = ++iNewid;
			}
			// vGraph_.resize(iNewid + 1);
			// vGraphRev_.resize(iNewid + 1);
			vGraph_[mapShrink_[src]].emplace_back(mapShrink_[dst]);
			vGraphRev_[mapShrink_[dst]].emplace_back(mapShrink_[src]);
		}
	}
	iVexNum_ = iNewid; // 计算点数
	iMaxVerId_ = iNewid;
	iArcNum_ = line_count;

	uint dsize = 0;
	uint dsizetemp = 0;
	for (auto &a : vGraph_)
	{
		dsizetemp = a.size();
		dsize = (dsize < dsizetemp) ? dsizetemp : dsize;
	}
	iMaxOutNbrNum_ = dsize;

	dsize = 0;
	dsizetemp = 0;
	for (auto &a : vGraphRev_)
	{
		dsizetemp = a.size();
		dsize = (dsize < dsizetemp) ? dsizetemp : dsize;
	}
	iMaxInNbrNum_ = dsize;

	auto end = std::chrono::high_resolution_clock::now();
	log_info("CreateGraphAdjListShrink time: %.6lf seconds", std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1000000000.0);
}

void CDirectedGraph::ShrinkAdjList()
{
	if (iVexNum_ == 0)
		return;
	uint id = 0, iSize = iVexNum_ + 1;
	mapShrink_.clear();
	std::vector<MyVec> vGraphNew, vGraphRevNew;
	vGraphNew.resize(iSize, MyVec());
	vGraphRevNew.resize(iSize, MyVec());
	uint s = 0, t = 0;
	for (uint v = 0; v <= iMaxVerId_; v++)
	{
		if (!vGraph_[v].empty() || !vGraphRev_[v].empty())
			if (mapShrink_.find(v) == mapShrink_.end())
			{
				mapShrink_[v] = ++id;
				s = id;
			}
			else
			{
				s = mapShrink_[v];
			}

		for (auto &vv : vGraph_[v])
		{
			if (mapShrink_.find(vv) == mapShrink_.end())
			{
				mapShrink_[vv] = ++id;
				t = id;
			}
			else
			{
				t = mapShrink_[vv];
			}
			vGraphNew[s].emplace_back(t);
		}

		for (auto &vv : vGraphRev_[v])
		{
			if (mapShrink_.find(vv) == mapShrink_.end())
			{
				mapShrink_[vv] = ++id;
				t = id;
			}
			else
			{
				t = mapShrink_[vv];
			}
			vGraphRevNew[s].emplace_back(t);
		}
	}
	iMaxVerId_ = id;
	vGraph_.clear();
	vGraphRev_.clear();
	vGraph_.resize(iSize, MyVec());
	vGraphRev_.resize(iSize, MyVec());
	for (auto v = 0; v < iSize; v++)
	{
		for (auto &vv : vGraphNew[v])
		{
			vGraph_[v].emplace_back(vv);
		}

		for (auto &vv : vGraphRevNew[v])
		{
			vGraphRev_[v].emplace_back(vv);
		}
	}
}

void CDirectedGraph::CreateGraphCSRSub(std::vector<std::pair<uint, uint>> &edge_list, uint *&offset, uint *&degree, uint *&adj, const uint key_pos)
{
	if (key_pos == 0)
	{
		std::sort(edge_list.begin(), edge_list.end(),
				  [](const std::pair<int, int> &left, const std::pair<int, int> &right)
				  {
					  //   if (left.first == right.first)
					  //   {
					  // 	  return left.second < right.second;
					  //   }
					  return left.first < right.first;
				  });
	}
	else
	{
		std::sort(edge_list.begin(), edge_list.end(),
				  [](const std::pair<int, int> &left, const std::pair<int, int> &right)
				  {
					  //   if (left.second == right.second)
					  //   {
					  // 	  return left.first < right.first;
					  //   }
					  return left.second < right.second;
				  });
	}

	uint iV = iMaxVerId_ + 1;
	if (iVexNum_ == 0)
		iV = 0;
	iArcNum_ = 0;
	std::vector<uint> degree_arr(iV, 0);
	std::vector<std::vector<uint>> adj_arr(iV);

	std::pair<uint, uint> prev_edge = std::make_pair(iV, iV);
	for (const auto &edge : edge_list)
	{
		// Remove parallel edges.
		if (prev_edge != edge)
		{
			prev_edge = edge;
			uint src, dst;
			std::tie(src, dst) = edge;
			// Remove self loops.
			if (src != dst)
			{
				if (key_pos == 0)
				{
					degree_arr[src] += 1;
					adj_arr[src].emplace_back(dst);
				}
				else
				{
					degree_arr[dst] += 1;
					adj_arr[dst].emplace_back(src);
				}
				iArcNum_ += 1;
			}
		}
	}

	ulonglong size = sizeof(uint) * iV;
	degree = (uint *)malloc(size);
	std::copy(degree_arr.begin(), degree_arr.end(), degree);

	size = sizeof(uint) * (iV + 1);
	offset = (uint *)malloc(size);
	offset[0] = 0;
	for (uint i = 0; i < iV; ++i)
	{
		offset[i + 1] = offset[i] + degree[i];
	}

	size = sizeof(uint) * (iArcNum_ + 16);
	adj = (uint *)malloc(size);

	for (uint i = 0; i < iV; ++i)
	{
		std::copy(adj_arr[i].begin(), adj_arr[i].end(), adj + offset[i]);
	}
}

void CDirectedGraph::CreateGraphCSR(const std::string &strFilePath, const char cSkip)
{
	auto start = std::chrono::high_resolution_clock::now();

	std::ifstream steFile(strFilePath);
	if (!steFile.is_open())
	{
		log_error("failed to open %s", strFilePath.c_str());
		// exit(-1);
		return;
	}
	uint max_vertex_id = 0;
	uint line_count = 0;
	uint src, dst;
	std::string tmp_str;
	std::vector<std::pair<uint, uint>> edge_list;
	while (std::getline(steFile, tmp_str))
	{
		line_count += 1;
		if (tmp_str[0] != '#' && tmp_str[0] != '%' && tmp_str[0] != '/') // Todo if (tmp_str[0] != cSkip)
		{
			std::stringstream ss(tmp_str);
			if (!(ss >> src >> dst))
			{
				log_error("Cannot convert line %u to edge.", line_count);
				// exit(-1);
				return;
			}
			if (src > max_vertex_id)
			{
				max_vertex_id = src;
			}
			if (dst > max_vertex_id)
			{
				max_vertex_id = dst;
			}
			setNode_.insert(src); // 计算点数
			setNode_.insert(dst); // 计算点数
			edge_list.emplace_back(src, dst);
		}
	}
	iMaxVerId_ = max_vertex_id;
	iVexNum_ = setNode_.size();
	// iArcNum_ = line_count;

	std::sort(edge_list.begin(), edge_list.end());
	auto it = std::unique(edge_list.begin(), edge_list.end());
	edge_list.erase(it, edge_list.end());
	CreateGraphCSRSub(edge_list, arrOutOffset_, arrOutDegree_, arrOutAdj_, 0);
	CreateGraphCSRSub(edge_list, arrInOffset_, arrInDegree_, arrInAdj_, 1);

	uint dsize = 0;
	uint dsizetemp = 0;
	for (uint i = 0; i < iVexNum_; ++i)
	{
		dsizetemp = arrOutDegree_[i];
		dsize = (dsize < dsizetemp) ? dsizetemp : dsize;
	}
	iMaxOutNbrNum_ = dsize;

	dsize = 0;
	dsizetemp = 0;
	for (uint i = 0; i < iVexNum_; ++i)
	{
		dsizetemp = arrInDegree_[i];
		dsize = (dsize < dsizetemp) ? dsizetemp : dsize;
	}
	iMaxInNbrNum_ = dsize;

	auto end = std::chrono::high_resolution_clock::now();
	log_info("CreateGraphCSR time: %.6lf seconds", std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1000000000.0);
}

void CDirectedGraph::CreateGraphCSR(std::vector<std::pair<uint, uint>> &edge_list)
{
	auto start = std::chrono::high_resolution_clock::now();

	uint max_vertex_id = 0;
	uint line_count = 0;
	for (const auto &edge : edge_list)
	{
		uint src, dst;
		std::tie(src, dst) = edge;
		if (src > max_vertex_id)
		{
			max_vertex_id = src;
		}
		if (dst > max_vertex_id)
		{
			max_vertex_id = dst;
		}
		setNode_.insert(src); // 计算点数
		setNode_.insert(dst); // 计算点数
	}
	iMaxVerId_ = max_vertex_id;
	iVexNum_ = setNode_.size();
	iArcNum_ = edge_list.size();

	CreateGraphCSRSub(edge_list, arrOutOffset_, arrOutDegree_, arrOutAdj_, 0);
	CreateGraphCSRSub(edge_list, arrInOffset_, arrInDegree_, arrInAdj_, 1);

	uint dsize = 0;
	uint dsizetemp = 0;
	for (uint i = 0; i < iVexNum_; ++i)
	{
		dsizetemp = arrOutDegree_[i];
		dsize = (dsize < dsizetemp) ? dsizetemp : dsize;
	}
	iMaxOutNbrNum_ = dsize;

	dsize = 0;
	dsizetemp = 0;
	for (uint i = 0; i < iVexNum_; ++i)
	{
		dsizetemp = arrInDegree_[i];
		dsize = (dsize < dsizetemp) ? dsizetemp : dsize;
	}
	iMaxInNbrNum_ = dsize;

	auto end = std::chrono::high_resolution_clock::now();
	log_info("CreateGraphCSR time: %.6lf seconds", std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1000000000.0);
}

void CDirectedGraph::CreateSubGraphCSRSub(std::vector<std::pair<uint, uint>> &edge_list, uint *&offset, uint *&degree, uint *&adj, const uint key_pos)
{
	uint iV = iMaxVerId_ + 1;
	if (iVexNum_ == 0)
		iV = 0;

	std::vector<ulonglong> degree_arr(iV, 0);
	std::vector<std::vector<uint>> adj_arr(iV);

	std::pair<uint, uint> prev_edge = std::make_pair(iV, iV);
	for (const auto &edge : edge_list)
	{
		uint src, dst;
		std::tie(src, dst) = edge;
		if (key_pos == 0)
		{
			degree_arr[src] += 1;
			adj_arr[src].emplace_back(dst);
		}
		else
		{
			degree_arr[dst] += 1;
			adj_arr[dst].emplace_back(src);
		}
	}

	// log_info("max_degree: %lu", *std::max_element(degree_arr.begin(), degree_arr.end()));

	ulonglong size = sizeof(uint) * iV;
	degree = (uint *)malloc(size);
	std::copy(degree_arr.begin(), degree_arr.end(), degree);

	size = sizeof(uint) * (iV + 1);
	offset = (uint *)malloc(size);
	offset[0] = 0;
	// for (uint i = 0; i < iV; ++i)
	// {
	// 	offset[i + 1] = offset[i] + degree[i];
	// }
	uint maxd = 0;
	for (uint i = 0; i < iV; ++i)
	{
		offset[i + 1] = offset[i] + degree[i];
		maxd = maxd < degree[i] ? degree[i] : maxd;
	}
	if (key_pos == 0)
	{
		iMaxOutNbrNum_ = maxd;
	}
	else
	{
		iMaxInNbrNum_ = maxd;
	}

	size = sizeof(uint) * (iArcNum_ + 16);
	adj = (uint *)malloc(size);
	for (uint i = 0; i < iV; ++i)
	{
		std::copy(adj_arr[i].begin(), adj_arr[i].end(), adj + offset[i]);
	}
}

void CDirectedGraph::CreateSubGraphCSR(std::vector<std::pair<uint, uint>> &edge_list, ushort *sdus, ushort *sdut, bool needRev, ulonglong &time)
{
	time = 0;
	uint max_vertex_id = 0;
	uint line_count = 0;
	for (const auto &edge : edge_list)
	{
		uint src, dst;
		std::tie(src, dst) = edge;
		if (src > max_vertex_id)
		{
			max_vertex_id = src;
		}
		if (dst > max_vertex_id)
		{
			max_vertex_id = dst;
		}
		setNode_.insert(src); // 计算点数
		setNode_.insert(dst); // 计算点数
	}
	iMaxVerId_ = max_vertex_id;
	iVexNum_ = setNode_.size();
	iArcNum_ = edge_list.size();

	auto start = std::chrono::high_resolution_clock::now();
	if (sdut != nullptr)
		std::sort(edge_list.begin(), edge_list.end(),
				  [&](const std::pair<int, int> &left, const std::pair<int, int> &right)
				  {
					  if (left.first == right.first)
					  {
						  return sdut[left.second] < sdut[right.second];
					  }
					  return left.first < right.first;
				  });
	else
		std::sort(edge_list.begin(), edge_list.end(),
				  [&](const std::pair<int, int> &left, const std::pair<int, int> &right)
				  {
					  return left.first < right.first;
				  });
	CreateSubGraphCSRSub(edge_list, arrOutOffset_, arrOutDegree_, arrOutAdj_, 0);
	if (needRev)
	{
		if (sdus != nullptr)
			std::sort(edge_list.begin(), edge_list.end(),
					  [&](const std::pair<int, int> &left, const std::pair<int, int> &right)
					  {
						  if (left.second == right.second)
						  {
							  return sdus[left.first] < sdus[right.first];
						  }
						  return left.second < right.second;
					  });
		else
			std::sort(edge_list.begin(), edge_list.end(),
					  [&](const std::pair<int, int> &left, const std::pair<int, int> &right)
					  {
						  return left.second < right.second;
					  });
		CreateSubGraphCSRSub(edge_list, arrInOffset_, arrInDegree_, arrInAdj_, 1);
	}

	auto end = std::chrono::high_resolution_clock::now();
	time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void CDirectedGraph::CreateSubGraphCSR2(std::vector<std::pair<uint, uint>> &edge_list, ushort *sdus, ushort *sdut, bool needRev, ulonglong &time)
{
	time = 0;
	uint max_vertex_id = 0;
	uint line_count = 0;
	std::vector<uint> buckets[MAX_K];

	for (const auto &edge : edge_list)
	{
		uint src, dst;
		std::tie(src, dst) = edge;
		if (src > max_vertex_id)
		{
			max_vertex_id = src;
		}
		if (dst > max_vertex_id)
		{
			max_vertex_id = dst;
		}
		setNode_.insert(src); // 计算点数
		setNode_.insert(dst); // 计算点数
	}
	iMaxVerId_ = max_vertex_id;
	iVexNum_ = setNode_.size();
	iArcNum_ = edge_list.size();

	auto start = std::chrono::high_resolution_clock::now();
	if (sdut != nullptr)
		std::sort(edge_list.begin(), edge_list.end(),
				  [&](const std::pair<int, int> &left, const std::pair<int, int> &right)
				  {
					  if (left.first == right.first)
					  {
						  return sdut[left.second] < sdut[right.second];
					  }
					  return left.first < right.first;
				  });
	else
		std::sort(edge_list.begin(), edge_list.end(),
				  [&](const std::pair<int, int> &left, const std::pair<int, int> &right)
				  {
					  return left.first < right.first;
				  });
	CreateSubGraphCSRSub2(edge_list, arrOutOffset_, arrOutDegree_, arrOutAdj_, 0);
	if (needRev)
	{
		if (sdus != nullptr)
			std::sort(edge_list.begin(), edge_list.end(),
					  [&](const std::pair<int, int> &left, const std::pair<int, int> &right)
					  {
						  if (left.second == right.second)
						  {
							  return sdus[left.first] < sdus[right.first];
						  }
						  return left.second < right.second;
					  });
		else
			std::sort(edge_list.begin(), edge_list.end(),
					  [&](const std::pair<int, int> &left, const std::pair<int, int> &right)
					  {
						  return left.second < right.second;
					  });
		CreateSubGraphCSRSub2(edge_list, arrInOffset_, arrInDegree_, arrInAdj_, 1);
	}
	auto end = std::chrono::high_resolution_clock::now();
	time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void CDirectedGraph::CreateSubGraphCSRSub2(std::vector<std::pair<uint, uint>> &edge_list, uint *&offset, uint *&degree, uint *&adj, const uint key_pos)
{
	uint iV = iMaxVerId_ + 1;
	if (iVexNum_ == 0)
		iV = 0;

	std::vector<uint> degree_arr(iV, 0);
	std::vector<std::vector<uint>> adj_arr(iV);

	std::pair<uint, uint> prev_edge = std::make_pair(iV, iV);
	for (const auto &edge : edge_list)
	{
		uint src, dst;
		std::tie(src, dst) = edge;
		if (key_pos == 0)
		{
			degree_arr[src] += 1;
			adj_arr[src].emplace_back(dst);
		}
		else
		{
			degree_arr[dst] += 1;
			adj_arr[dst].emplace_back(src);
		}
	}

	ulonglong size = sizeof(uint) * iV;
	degree = (uint *)malloc(size);
	std::copy(degree_arr.begin(), degree_arr.end(), degree);

	size = sizeof(uint) * (iV + 1);
	offset = (uint *)malloc(size);
	offset[0] = 0;
	// for (uint i = 0; i < iV; ++i)
	// {
	// 	offset[i + 1] = offset[i] + degree[i];
	// }
	uint maxd = 0;
	for (uint i = 0; i < iV; ++i)
	{
		offset[i + 1] = offset[i] + degree[i];
		maxd = maxd < degree[i] ? degree[i] : maxd;
	}
	if (key_pos == 0)
	{
		iMaxOutNbrNum_ = maxd;
	}
	else
	{
		iMaxInNbrNum_ = maxd;
	}

	size = sizeof(uint) * (iArcNum_ + 16);
	adj = (uint *)malloc(size);
	for (uint i = 0; i < iV; ++i)
	{
		std::copy(adj_arr[i].begin(), adj_arr[i].end(), adj + offset[i]);
	}
}

void CDirectedGraph::ShrinkCSR(ulonglong &itime)
{
	if (iVexNum_ == 0 || iMaxVerId_ + 1 == iVexNum_)
		return;
	uint id = 0;
	mapShrink_.clear();
	auto start = std::chrono::high_resolution_clock::now();
	if (arrInDegree_ == nullptr)
	{
		spp::sparse_hash_set<uint> use_set;
		for (uint i = 0; i < iMaxVerId_ + 1; ++i)
		{
			if (arrOutDegree_[i] != 0)
			{
				use_set.insert(i);
			}
		}
		for (uint i = 0; i < iArcNum_; ++i)
		{
			use_set.insert(arrOutAdj_[i]);
		}

		for (uint i = 0; i < iMaxVerId_ + 1; ++i)
		{

			if (use_set.find(i) != use_set.end())
			{
				mapShrink_[i] = id;
				if (i != id)
				{
					arrOutDegree_[id] = arrOutDegree_[i];
					arrOutOffset_[id] = arrOutOffset_[i];
				}
				id += 1;
			}
		}
		arrOutOffset_[id] = arrOutOffset_[iMaxVerId_ + 1];
		iMaxVerId_ = id - 1;
		for (uint i = 0; i < iArcNum_; ++i)
		{
			auto e = arrOutAdj_[i];
			arrOutAdj_[i] = mapShrink_[e];
		}
	}
	else
	{
		for (uint i = 0; i < iMaxVerId_ + 1; ++i)
		{
			auto d = arrOutDegree_[i] + arrInDegree_[i];
			if (d != 0)
			{
				mapShrink_[i] = id;
				if (i != id)
				{
					arrOutDegree_[id] = arrOutDegree_[i];
					arrInDegree_[id] = arrInDegree_[i];
					arrOutOffset_[id] = arrOutOffset_[i];
					arrInOffset_[id] = arrInOffset_[i];
				}
				id += 1;
			}
		}
		arrOutOffset_[id] = arrOutOffset_[iMaxVerId_ + 1];
		arrInOffset_[id] = arrInOffset_[iMaxVerId_ + 1];
		iMaxVerId_ = id - 1;
		for (uint i = 0; i < iArcNum_; ++i)
		{
			auto e = arrOutAdj_[i];
			arrOutAdj_[i] = mapShrink_[e];
			e = arrInAdj_[i];
			arrInAdj_[i] = mapShrink_[e];
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	itime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	// log_info("ShrinkCSR time: %.6lf seconds", std::chrono::duration_cast<std::chrono::nanoseconds> itime / 1000000000.0);
}

// todo id从1开始
void CDirectedGraph::ShrinkCSR2(ulonglong &itime)
{
	if (iVexNum_ == 0 || iMaxVerId_ + 1 == iVexNum_)
		return;
	size_t isize = sizeof(uint) * (iVexNum_ + 1);
	uint *arrOutDegree_new;
	uint *arrInDegree_new;
	uint *arrOutOffset_new;
	uint *arrInOffset_new;
	uint *arrOutAdj_new;
	uint *arrInAdj_new;

	uint id = 1;
	mapShrink_.clear();
	auto start = std::chrono::high_resolution_clock::now();
	if (arrInDegree_ == nullptr)
	{
		arrOutDegree_new = (uint *)malloc(isize);
		isize = sizeof(uint) * (iVexNum_ + 2);
		arrOutOffset_new = (uint *)malloc(isize);
		isize = sizeof(uint) * (iArcNum_);
		arrOutAdj_new = (uint *)malloc(isize);
		arrOutDegree_new[0] = 0;
		arrOutOffset_new[0] = 0;

		spp::sparse_hash_set<uint> use_set;
		for (uint i = 0; i < iMaxVerId_ + 1; ++i)
		{
			if (arrOutDegree_[i] != 0)
			{
				use_set.insert(i);
			}
		}
		for (uint i = 0; i < iArcNum_; ++i)
		{
			use_set.insert(arrOutAdj_[i]);
		}

		for (uint i = 0; i < iMaxVerId_ + 1; ++i)
		{

			if (use_set.find(i) != use_set.end())
			{
				mapShrink_[i] = id;
				arrOutDegree_new[id] = arrOutDegree_[i];
				arrOutOffset_new[id] = arrOutOffset_[i];
				id += 1;
			}
		}
		arrOutOffset_new[id] = arrOutOffset_[iMaxVerId_ + 1];
		iMaxVerId_ = id - 1;
		for (uint i = 0; i < iArcNum_; ++i)
		{
			auto e = arrOutAdj_[i];
			arrOutAdj_new[i] = mapShrink_[e];
		}

		delete arrOutDegree_;
		arrOutDegree_ = arrOutDegree_new;
		delete arrOutOffset_;
		arrOutOffset_ = arrOutOffset_new;
		delete arrOutAdj_;
		arrOutAdj_ = arrOutAdj_new;
	}
	else
	{
		arrOutDegree_new = (uint *)malloc(isize);
		arrInDegree_new = (uint *)malloc(isize);
		isize = sizeof(uint) * (iVexNum_ + 2);
		arrOutOffset_new = (uint *)malloc(isize);
		arrInOffset_new = (uint *)malloc(isize);
		isize = sizeof(uint) * (iArcNum_);
		arrOutAdj_new = (uint *)malloc(isize);
		arrInAdj_new = (uint *)malloc(isize);
		arrOutDegree_new[0] = 0;
		arrInDegree_new[0] = 0;
		arrOutOffset_new[0] = 0;
		arrInOffset_new[0] = 0;

		for (uint i = 0; i < iMaxVerId_ + 1; ++i)
		{
			auto d = arrOutDegree_[i] + arrInDegree_[i];
			if (d != 0)
			{
				mapShrink_[i] = id;
				arrOutDegree_new[id] = arrOutDegree_[i];
				arrInDegree_new[id] = arrInDegree_[i];
				arrOutOffset_new[id] = arrOutOffset_[i];
				arrInOffset_new[id] = arrInOffset_[i];
				id += 1;
			}
		}
		arrOutOffset_new[id] = arrOutOffset_[iMaxVerId_ + 1];
		arrInOffset_new[id] = arrInOffset_[iMaxVerId_ + 1];
		iMaxVerId_ = id - 1;
		for (uint i = 0; i < iArcNum_; ++i)
		{
			auto e = arrOutAdj_[i];
			arrOutAdj_new[i] = mapShrink_[e];
			e = arrInAdj_[i];
			arrInAdj_new[i] = mapShrink_[e];
		}
		delete arrOutDegree_;
		arrOutDegree_ = arrOutDegree_new;
		delete arrOutOffset_;
		arrOutOffset_ = arrOutOffset_new;
		delete arrOutAdj_;
		arrOutAdj_ = arrOutAdj_new;
		delete arrInDegree_;
		arrInDegree_ = arrInDegree_new;
		delete arrInOffset_;
		arrInOffset_ = arrInOffset_new;
		delete arrInAdj_;
		arrInAdj_ = arrInAdj_new;
	}

	iVexNum_++; // ! 加上点0，数量将比原图大1个，使得iVexNum_ == iMaxVerID_
	auto end = std::chrono::high_resolution_clock::now();
	itime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	// log_info("ShrinkCSR time: %.6lf seconds", std::chrono::duration_cast<std::chrono::nanoseconds> itime / 1000000000.0);
}

void CDirectedGraph::CreateGraphCSRByAdjList()
{
	auto start = std::chrono::high_resolution_clock::now();

	if (iArcNum_ == 0)
	{
		log_warn("graph is empty!");
		return;
	}

	ulonglong iSize = sizeof(uint) * iVexNum_;
	arrOutDegree_ = (uint *)malloc(iSize);
	arrInDegree_ = (uint *)malloc(iSize);
	iSize = sizeof(uint) * (iVexNum_ + 1);
	arrOutOffset_ = (uint *)malloc(iSize);
	arrInOffset_ = (uint *)malloc(iSize);
	iSize = sizeof(uint) * (iArcNum_ + 16);
	arrOutAdj_ = (uint *)malloc(iSize);
	arrInAdj_ = (uint *)malloc(iSize);
	uint iPos = 0, iPosR = 0;
	for (uint i = 0; i < iVexNum_; i++)
	{
		auto iNbrNo = vGraph_[i].size();
		arrOutDegree_[i] = iNbrNo;
		arrOutOffset_[i] = iPos;
		std::copy(vGraph_[i].begin(), vGraph_[i].end(), arrOutAdj_ + iPos);
		iPos += iNbrNo;

		iNbrNo = vGraphRev_[i].size();
		arrInDegree_[i] = iNbrNo;
		arrInOffset_[i] = iPosR;
		std::copy(vGraphRev_[i].begin(), vGraphRev_[i].end(), arrInAdj_ + iPosR);
		iPosR += iNbrNo;
	}
	arrOutOffset_[iVexNum_] = iPos;
	arrInOffset_[iVexNum_] = iPosR;

	auto end = std::chrono::high_resolution_clock::now();
	log_info("CreateGraphCSRByAdjList time: %.6lf seconds", std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1000000000.0);
}
