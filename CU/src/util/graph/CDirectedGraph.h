#pragma once
#ifndef CGRAPH_H
#define CGRAPH_H

#include "../common.h"

class CDirectedGraph
{
private:
	/**
	 * 辅助
	 */
	spp::sparse_hash_set<uint> setNode_;		 // 点集
	spp::sparse_hash_map<uint, uint> mapShrink_; //
	MyVec vOutDegreeDistribution_;
	MyVec cInDegreeDistribution_;

	/**
	 * 元数据
	 */
	uint iVexNum_;		 // 实际点的数量
	uint iArcNum_;		 // 边的数量
	uint iMaxVerId_;	 // 最大的节点id	shrink变
	uint iMaxInNbrNum_;	 // 最大的入度
	uint iMaxOutNbrNum_; // 最大的出度

	/**
	 * AdjList
	 */
	std::vector<MyVec> vGraph_;	   // 原图 		shrink变
	std::vector<MyVec> vGraphRev_; // 反向图 	shrink变

	/**
	 * CSR 模式
	 */
	uint *arrOutOffset_; // vPos_
	uint *arrOutAdj_;	 // vEdg_
	uint *arrOutDegree_; // vDeg_
	uint *arrInOffset_;	 // vPosR_
	uint *arrInAdj_;	 // vEdgR_
	uint *arrInDegree_;	 // vDegR_

public:
	inline uint iVexNum() { return iVexNum_; }
	inline uint iArcNum() { return iArcNum_; }
	inline uint iMaxInNbrNum() { return iMaxInNbrNum_; }
	inline uint iMaxOutNbrNum() { return iMaxOutNbrNum_; }
	inline uint iMaxVerId() { return iMaxVerId_; }
	// inline spp::sparse_hash_map<uint, uint> mapShrink() { return mapShrink_; }
	// inline spp::sparse_hash_set<uint> setNode() { return setNode_; }

	/**
	 * AdjList
	 */
	inline void CreateGraphRev_()
	{
		auto iSize = vGraph_.size();
		vGraphRev_.resize(iSize);
		for (uint i = 0; i < iSize; i++)
		{
			for (auto j : vGraph_[i])
			{
				vGraphRev_[j].emplace_back(i);
			}
		}
	}
	inline std::vector<MyVec> vGraph()
	{
		if (vGraph_.size() == 0)
		{
			log_warn("Graph's size is 0");
		}
		return vGraph_;
	}
	inline std::vector<MyVec> vGraphRev()
	{
		if (vGraphRev_.size() == 0)
		{
			// CreateGraphRev_();
			log_warn("Graph's size is 0");
		}
		return vGraphRev_;
	}
	inline MyVec GetOutNbr(uint v)
	{
		if (vGraph_.size() == 0)
		{
			log_warn("Graph's size is 0");
		}
		return vGraph_[v];
	}
	inline MyVec GetInNbr(uint v)
	{
		if (vGraphRev_.size() == 0)
		{
			// CreateGraphRev_();
			log_warn("Graph's size is 0");
		}
		return vGraphRev_[v];
	}

	/**
	 * CSR 模式
	 */
	inline uint *arrOutDegree() { return arrOutDegree_; }
	inline uint *arrOutOffset() { return arrOutOffset_; }
	inline uint *arrOutAdj() { return arrOutAdj_; }
	inline uint *arrInDegree() { return arrInDegree_; }
	inline uint *arrInOffset() { return arrInOffset_; }
	inline uint *arrInAdj() { return arrInAdj_; }
	// inline std::pair<uint *, uint> GetOutNbrCSR(uint u)
	// {
	// 	if (u <= iMaxVerId_ && iVexNum_ != 0)
	// 	{
	// 		return std::make_pair(arrOutAdj_ + arrOutOffset_[u], arrOutDegree_[u]);
	// 	}
	// 	else
	// 	{
	// 		return std::make_pair(arrOutAdj_, 0);
	// 	}
	// }
	// inline std::pair<uint *, uint> GetInNbrCSR(uint u)
	// {
	// 	if (u <= iMaxVerId_ && iVexNum_ != 0)
	// 	{
	// 		return std::make_pair(arrInAdj_ + arrInOffset_[u], arrInDegree_[u]);
	// 	}
	// 	else
	// 	{
	// 		return std::make_pair(arrInAdj_, 0);
	// 	}
	// }
	// inline uint GetOutNbrNumCSR(uint u)
	// {
	// 	if (u <= iMaxVerId_ && iVexNum_ != 0)
	// 	{
	// 		return arrOutDegree_[u];
	// 	}
	// 	else
	// 	{
	// 		return 0;
	// 	}
	// }
	// inline uint GetInNbrNumCSR(uint u)
	// {
	// 	if (u <= iMaxVerId_ && iVexNum_ != 0)
	// 	{
	// 		return arrInDegree_[u];
	// 	}
	// 	else
	// 	{
	// 		return 0;
	// 	}
	// }
	// inline uint GetNbrNumCSR(uint u)
	// {
	// 	if (u <= iMaxVerId_ && iVexNum_ != 0)
	// 	{
	// 		return arrInDegree_[u] + arrOutDegree_[u];
	// 	}
	// 	else
	// 	{
	// 		return 0;
	// 	}
	// }
	inline std::pair<uint *, uint> GetOutNbrCSR(uint u)
	{
		return std::make_pair(arrOutAdj_ + arrOutOffset_[u], arrOutDegree_[u]);
	}
	inline std::pair<uint *, uint> GetInNbrCSR(uint u)
	{
		return std::make_pair(arrInAdj_ + arrInOffset_[u], arrInDegree_[u]);
	}
	inline uint GetOutNbrNumCSR(uint u)
	{
		return arrOutDegree_[u];
	}
	inline uint GetInNbrNumCSR(uint u)
	{
		return arrInDegree_[u];
	}
	inline uint GetNbrNumCSR(uint u)
	{
		return arrInDegree_[u] + arrOutDegree_[u];
	}

	/**
	 * Shrink
	 */
	inline bool GetShrinkID(uint &id)
	{
		if (mapShrink_.find(id) != mapShrink_.end())
		{
			id = mapShrink_[id];
			return true;
		}
		else
			return false;
	}

public:
	explicit CDirectedGraph() : iVexNum_(0),
								iArcNum_(0),
								iMaxVerId_(0),
								iMaxInNbrNum_(0),
								iMaxOutNbrNum_(0),
								arrOutOffset_(nullptr),
								arrOutAdj_(nullptr),
								arrOutDegree_(nullptr),
								arrInOffset_(nullptr),
								arrInAdj_(nullptr),
								arrInDegree_(nullptr) {}

	~CDirectedGraph()
	{
		free(arrOutAdj_);
		free(arrOutDegree_);
		free(arrOutOffset_);
		free(arrInAdj_);
		free(arrInDegree_);
		free(arrInOffset_);
	}

	void Init();

public:
	void PrintMeta();

	void InsertEdgeAdjList(uint iHead, uint iTail);
	void InsertVertexAdjList(uint iNode);
	void DeleteEdgeAdjList(uint iHead, uint iTail);
	void DeleteVertexAdjList(uint iNode);
	void CreateGraphAdjList(const std::string &strFilePath, const char cSkip);

	void InsertEdgeAdjListShrink(uint iHead, uint iTail);
	void InsertVertexAdjListShrink(uint iNode);
	void DeleteEdgeAdjListShrink(uint iHead, uint iTail);
	void DeleteVertexAdjListShrink(uint iNode);
	void CreateGraphAdjListShrink(const std::string &strFilePath, const char cSkip);
	void ShrinkAdjList();

	void InsertEdgeCSR(uint iHead, uint iTail);
	void InsertVertexCSR(uint iNode);
	void DeleteEdgeCSR(uint iHead, uint iTail);
	void DeleteVertexCSR(uint iNode);
	void CreateGraphCSRSub(std::vector<std::pair<uint, uint>> &edge_list, uint *&offset, uint *&degree, uint *&adj, const uint key_pos);
	void CreateGraphCSR(const std::string &strFilePath, const char cSkip);
	void CreateGraphCSR(std::vector<std::pair<uint, uint>> &edge_list);
	void CreateGraphCSRByAdjList();
	void ShrinkCSR(ulonglong &itime);
	void ShrinkCSR2(ulonglong &itime);

	void CreateSubGraphCSRSub(std::vector<std::pair<uint, uint>> &edge_list, uint *&offset, uint *&degree, uint *&adj, const uint key_pos);
	void CreateSubGraphCSR(std::vector<std::pair<uint, uint>> &edge_list, ushort *sdus, ushort *sdut, bool needRev, ulonglong &time);
	void CreateSubGraphCSR2(std::vector<std::pair<uint, uint>> &edge_list, ushort *sdus, ushort *sdut, bool needRev, ulonglong &time);
	void CreateSubGraphCSRSub2(std::vector<std::pair<uint, uint>> &edge_list, uint *&offset, uint *&degree, uint *&adj, const uint key_pos);
};

class hot_degree
{
public:
	uint node;
	uint degree;
	hot_degree(uint a, uint b) : node(a), degree(b) {}
	bool operator<(const hot_degree &m) const
	{
		return degree > m.degree;
	}
	bool operator==(const hot_degree &m) const
	{
		return (node == m.node && degree == m.degree);
	}
};

class paths
{
public:
	std::vector<std::vector<uint>> path;
	paths(){};
	paths(std::vector<std::vector<uint>> a) : path(a) {}

	inline void drop_repeat_paths_with_sort()
	{
		sort(path.begin(), path.end());
		auto it = unique(path.begin(), path.end());
		path.erase(it, path.end());
	}

	void sort_by_string_order()
	{
		stable_sort(path.begin(), path.end(), std::less<std::vector<uint>>());
	}

	void drop_path_with_stop_nodes(std::set<uint> &stop_nodes)
	{
		for (auto iter = path.begin(); iter != path.end();)
		{
			if (intersect_vector_set(*iter, stop_nodes))
			{
				iter = path.erase(iter);
			}
			else
			{
				iter++;
			}
		}
	}

	void drop_path_with_stop_nodes_with_distance_range(std::set<uint> &stop_nodes, ushort startDis, ushort endDis)
	{
		for (auto iter = path.begin(); iter != path.end();)
		{
			if (intersect_vector_set_start_end_dis(*iter, stop_nodes, startDis, endDis))
			{
				iter = path.erase(iter);
			}
			else
			{
				iter++;
			}
		}
	}

	void drop_repeat_path()
	{
		if (path.size() <= 1)
		{
			return;
		}
		auto iter1 = path.begin();
		auto iter2 = path.begin();
		iter2++;
		while (iter2 != path.end())
		{
			if (*iter1 == *iter2)
			{
				iter2 = path.erase(iter2);
			}
			else
			{
				iter1++;
				iter2++;
			}
		}
	}
	void clear()
	{
		path.clear();
	}
	void push_back(std::vector<uint> a)
	{
		path.push_back(a);
	}
	void output()
	{
		for (auto iter = path.begin(); iter != path.end(); iter++)
		{
			for (auto iter1 = iter->begin(); iter1 != iter->end(); iter1++)
			{
				std::cout << *iter1 << " ";
			}
			std::cout << " end " << std::endl;
		}
	}

	std::vector<std::vector<uint>> &get_path()
	{
		return path;
	}

	void add_paths(paths a)
	{
		for (auto iter = a.get_path().begin(); iter != a.get_path().end(); iter++)
		{
			path.push_back(*iter);
		}
	}

	void write_to_file_append(std::string filename)
	{
		std::ofstream out;
		out.open(filename, std::ios::app);
		for (auto iter = path.begin(); iter != path.end(); iter++)
		{
			for (auto iter1 = iter->begin(); iter1 != iter->end(); iter1++)
			{
				out << *iter1 << " ";
			}
			out << std::endl;
		}
		out.close();
	}

	void write_to_file(std::string filename)
	{
		std::ofstream out;
		out.open(filename);
		for (auto iter = path.begin(); iter != path.end(); iter++)
		{
			for (auto iter1 = iter->begin(); iter1 != iter->end(); iter1++)
			{
				out << *iter1 << " ";
			}
			out << std::endl;
		}
		out.close();
	}
	void clear_file(std::string filename)
	{
		std::ofstream out;
		out.open(filename, std::fstream::out | std::ios_base::trunc);
		out.close();
	}
	void write_to_file_append_edgeID_and_result_size(std::string filename, uint edgeID, uint query_node1, uint query_node2)
	{
		std::ofstream out;
		out.open(filename, std::ofstream::out | std::ofstream::app);
		out << edgeID << " : " << query_node1 << " : " << query_node2 << " : " << path.size() << std::endl;
		out.close();
	}
	void write_to_file_append_edgeID(std::string filename, uint edgeID, uint query_node1, uint query_node2)
	{
		std::ofstream out;
		out.open(filename, std::ofstream::out | std::ofstream::app);
		out << edgeID << " : " << query_node1 << " : " << query_node2 << " : " << path.size() << std::endl;
		for (auto iter = path.begin(); iter != path.end(); iter++)
		{
			for (auto iter1 = iter->begin(); iter1 != iter->end(); iter1++)
			{
				out << *iter1 << " ";
			}
			out << std::endl;
		}
		out.close();
	}
	void drop_path_length_less_than_k(uint k)
	{
		for (auto iter = path.begin(); iter != path.end();)
		{
			if (iter->size() < k + 1)
			{
				iter = path.erase(iter);
			}
			else
			{
				iter++;
			}
		}
	}
	void drop_path_not_start_from_nodeS(uint s)
	{
		for (auto iter = path.begin(); iter != path.end();)
		{
			if (*(iter->begin()) != s)
			{
				iter = path.erase(iter);
			}
			else
			{
				iter++;
			}
		}
	}

	bool intersect_vector_set(std::vector<uint> &v, std::set<uint> &s)
	{
		for (auto iter = v.begin(); iter != v.end(); iter++)
		{
			if (s.find(*iter) != s.end()) // has common nodes
			{
				return true;
			}
		}
		return false;
	}

	bool intersect_vector_set_start_end_dis(std::vector<uint> &v, std::set<uint> &s, ushort startDis, ushort endDis)
	{

		for (auto i = startDis; i < v.size() - 1 - endDis; i++)
		{
			if (s.find(v[i]) != s.end()) // has common nodes
			{
				return true;
			}
		}
		return false;
	}

	bool containsDuplicate(const std::vector<uint> &v)
	{
		for (int i = 0; i < v.size() - 1; i++)
		{
			for (int j = i + 1; j < v.size(); j++)
			{
				if (v[i] == v[j])
					return true;
			}
		}
		return false;
	}

	bool containsDuplicate_middle(std::vector<uint> v)
	{
		sort(v.begin(), v.end());
		auto it = unique(v.begin(), v.end());
		if (it != v.end())
		{
			return true;
		}
		return false;
	}

	bool containsDuplicate_slow(const std::vector<uint> &v)
	{
		std::unordered_set<uint> s(v.size() * 2);
		for (auto x : v)
		{
			if (!s.insert(x).second)
				return true;
		}
		return false;
	}

	void drop_path_with_repeat_node()
	{
		for (auto iter = path.begin(); iter != path.end();)
		{
			if (containsDuplicate(*iter))
			{
				iter = path.erase(iter);
			}
			else
			{
				iter++;
			}
		}
	}
	void drop_path_length_more_than_k_drop_repeated_node(uint k)
	{
		for (auto iter = path.begin(); iter != path.end();)
		{
			if (iter->size() > k + 1 || containsDuplicate(*iter))
			{
				iter = path.erase(iter);
			}
			else
			{
				iter++;
			}
		}
	}

	void drop_path_length_more_than_k(uint k)
	{
		for (auto iter = path.begin(); iter != path.end();)
		{
			if (iter->size() > k + 1)
			{
				iter = path.erase(iter);
			}
			else
			{
				iter++;
			}
		}
	}

	void reverse()
	{
		for (auto iter = path.begin(); iter != path.end(); iter++)
		{
			std::reverse(iter->begin(), iter->end());
		}
	}
	std::map<ushort, paths> construct_distance_paths(std::vector<std::vector<uint>> &p)
	{
		std::map<ushort, paths> result;
		for (auto iter = p.begin(); iter != p.end(); iter++)
		{
			auto iter2 = result.find(iter->size() - 1);
			if (iter2 == result.end())
			{
				paths temp;
				temp.push_back(*iter);
				result.insert(std::make_pair(iter->size() - 1, temp));
			}
			else
			{
				iter2->second.push_back(*iter);
			}
		}
		return result;
	}
	std::map<ushort, paths> construct_distance_paths(paths &p)
	{
		std::map<ushort, paths> result;
		for (auto iter = p.path.begin(); iter != p.path.end(); iter++)
		{
			auto iter2 = result.find(iter->size() - 1);
			if (iter2 == result.end())
			{
				paths temp;
				temp.push_back(*iter);
				result.insert(std::make_pair(iter->size() - 1, temp));
			}
			else
			{
				iter2->second.push_back(*iter);
			}
		}
		return result;
	}
	paths join_remove_repeat_nodes_only_join_right_sizeor_minus_one(paths &a, ushort k) // result path's length must be no more than distance
	{
		paths result;
		if (a.get_path().size() == 0)
		{
			result.add_paths(path);
		}
		else if (path.size() == 0)
		{
			result.add_paths(a);
		}
		else
		{
			// construct map<size, paths>
			std::map<ushort, paths> left_distance_paths = construct_distance_paths(path);
			std::map<ushort, paths> right_distance_paths = construct_distance_paths(a);
			for (int i = 1; i <= k - (k / 2); i++)
			{
				auto iter_left = left_distance_paths.find(i);
				auto iter_right = right_distance_paths.find(i);
				auto iter_right_minus_one = right_distance_paths.find(i - 1);
				if (iter_left == left_distance_paths.end() || (iter_right == right_distance_paths.end() && iter_right_minus_one == right_distance_paths.end()))
				{
					continue;
				}
				paths left_paths = iter_left->second;
				paths right_paths;
				if (iter_right == right_distance_paths.end())
				{
					right_paths = iter_right_minus_one->second;
				}
				else if (iter_right_minus_one == right_distance_paths.end())
				{
					right_paths = iter_right->second;
				}
				else
				{
					right_paths = iter_right->second;
					right_paths.add_paths(iter_right_minus_one->second);
				}
				for (auto iter1 = left_paths.path.begin(); iter1 != left_paths.path.end(); iter1++)
				{
					for (auto iter2 = right_paths.path.begin(); iter2 != right_paths.path.end(); iter2++)
					{
						if (iter1->back() != iter2->front())
						{
							continue;
						}
						else
						{
							std::unordered_set<uint> temp_set(iter1->begin() - 1, iter1->end() - 1);
							bool skip = false;
							for (int i = 1; i < iter2->size() - 1; i++)
							{
								if (!temp_set.insert(iter2->at(i)).second)
								{
									skip = true;
									break;
								}
							}
							if (skip)
							{
								continue;
							}
							std::vector<uint> temp_path(*iter1);
							temp_path.insert(temp_path.end(), iter2->begin() + 1, iter2->end());
							result.push_back(temp_path);
						}
					}
				}
			}
		}
		return result;
	}

	paths join_remove_repeat_nodes(paths &a) // result path's length must be no more than distance
	{
		paths result;
		if (a.get_path().size() == 0)
		{
			result.add_paths(path);
		}
		else if (path.size() == 0)
		{
			result.add_paths(a);
		}
		else
		{
			for (auto iter1 = path.begin(); iter1 != path.end(); iter1++)
			{
				for (auto iter2 = a.path.begin(); iter2 != a.path.end(); iter2++)
				{
					if (iter1->back() != iter2->front())
					{
						continue;
					}
					else
					{
						std::unordered_set<uint> temp_set(iter1->begin() - 1, iter1->end() - 1);
						bool skip = false;
						for (int i = 1; i < iter2->size() - 1; i++)
						{
							if (!temp_set.insert(iter2->at(i)).second)
							{
								skip = true;
								break;
							}
						}
						if (skip)
						{
							continue;
						}
						std::vector<uint> temp_path(*iter1);
						temp_path.insert(temp_path.end(), iter2->begin() + 1, iter2->end());
						result.push_back(temp_path);
					}
				}
			}
		}
		return result;
	}

	paths join(paths &a) // result path's length must be no more than distance
	{
		paths result;
		if (a.get_path().size() == 0)
		{
			result.add_paths(path);
		}
		else if (path.size() == 0)
		{
			result.add_paths(a);
		}
		else
		{
			for (auto iter1 = path.begin(); iter1 != path.end(); iter1++)
			{
				for (auto iter2 = a.path.begin(); iter2 != a.path.end(); iter2++)
				{
					if (iter1->back() != iter2->front())
					{
						continue;
					}
					else
					{
						std::vector<uint> temp_path(*iter1);
						temp_path.insert(temp_path.end(), iter2->begin() + 1, iter2->end());
						result.push_back(temp_path);
					}
				}
			}
		}
		return result;
	}
	uint get_min_distance()
	{
		bool first_updated = true;
		uint min_distance = 0;
		for (auto iter = path.begin(); iter != path.end(); iter++)
		{
			if (iter->size() == 0)
			{
				continue;
			}
			if (first_updated)
			{
				min_distance = iter->size() - 1;
				first_updated = false;
			}
			else
			{
				if (iter->size() - 1 < min_distance)
				{
					min_distance = iter->size() - 1;
				}
			}
		}
		return min_distance;
	}

	void join_drop_longpaths_and_repeat_nodes(paths &a, uint distance, paths &result) // result path's length must be no more than distance
	{
		if (a.get_path().size() == 0)
		{
			result.add_paths(path);
		}
		else if (path.size() == 0)
		{
			result.add_paths(a);
		}
		else
		{
			for (auto iter1 = path.begin(); iter1 != path.end(); iter1++)
			{
				std::unordered_map<uint, bool> duplicated;
				for (int i = 0; i < iter1->size(); i++)
				{
					duplicated.insert(std::make_pair((*iter1)[i], true));
				}
				for (auto iter2 = a.path.begin(); iter2 != a.path.end(); iter2++)
				{
					if (iter1->back() != iter2->front())
					{
						continue;
					}
					else if (iter1->size() + iter2->size() > distance + 2)
					{
						continue;
					}
					bool force_con = false;
					for (int j = 1; j < iter2->size(); j++)
					{
						if (duplicated.find((*iter2)[j]) != duplicated.end())
						{
							force_con = true;
							break;
						}
					}
					if (force_con)
						continue;
					std::vector<uint> temp_path(*iter1);
					temp_path.insert(temp_path.end(), iter2->begin() + 1, iter2->end());
					result.push_back(temp_path);
				}
			}
		}
		// return result;
	}

	inline void join_drop_longpaths_and_repeat_nodes_and_short_paths(paths &a, uint distance, paths &result, ushort short_dis) // result path's length must be no more than distance
	{
		if (a.get_path().size() == 0)
		{
			result.add_paths(path);
		}
		else if (path.size() == 0)
		{
			result.add_paths(a);
		}
		else
		{
			for (auto iter1 = path.begin(); iter1 != path.end(); iter1++)
			{
				std::unordered_map<uint, bool> duplicated;
				for (int i = 0; i < iter1->size(); i++)
				{
					duplicated.insert(std::make_pair((*iter1)[i], true));
				}
				for (auto iter2 = a.path.begin(); iter2 != a.path.end(); iter2++)
				{
					if (iter1->back() != iter2->front())
					{
						continue;
					}
					else if (iter1->size() + iter2->size() > distance + 2 || iter1->size() + iter2->size() <= short_dis + 2)
					{
						continue;
					}
					bool force_con = false;
					for (int j = 1; j < iter2->size(); j++)
					{
						if (duplicated.find((*iter2)[j]) != duplicated.end())
						{
							force_con = true;
							break;
						}
					}
					if (force_con)
						continue;
					std::vector<uint> temp_path(*iter1);
					temp_path.insert(temp_path.end(), iter2->begin() + 1, iter2->end());
					result.push_back(temp_path);
				}
			}
		}
	}

	void join_drop_longpaths_and_repeat_nodes_and_short_paths_stop_ndoeswithdis(paths &a, uint distance, paths &result, ushort short_dis, std::set<uint> &stop_nodes, ushort start_dis, ushort end_pos_dis) // result path's length must be no more than distance
	{
		if (a.get_path().size() == 0)
		{
			// result.add_paths(path);//should be remain?
			return;
		}
		else if (path.size() == 0)
		{
			// result.add_paths(a);
			return;
		}
		else
		{
			std::unordered_map<uint, bool> stop_map;
			for (auto iter_stop_map = stop_nodes.begin(); iter_stop_map != stop_nodes.end(); iter_stop_map++)
			{
				stop_map.insert(std::make_pair(*iter_stop_map, true));
			}
			for (auto iter1 = path.begin(); iter1 != path.end(); iter1++)
			{

				std::unordered_map<uint, bool> duplicated;
				for (int i = 0; i < iter1->size(); i++)
				{
					duplicated.insert(std::make_pair((*iter1)[i], true));
				}
				for (auto iter2 = a.path.begin(); iter2 != a.path.end(); iter2++)
				{

					if (iter1->back() != iter2->front())
					{
						continue;
					}
					else if (iter1->size() + iter2->size() > distance + 2 || iter1->size() + iter2->size() <= short_dis + 2)
					{
						continue;
					}
					bool force_con = false;
					for (int j = 1; j < iter2->size(); j++)
					{
						if (duplicated.find((*iter2)[j]) != duplicated.end())
						{
							force_con = true;
							break;
						}
					}
					if (force_con)
						continue;

					std::vector<uint> temp_path(*iter1);
					if (stop_nodes.empty())
					{
						temp_path.insert(temp_path.end(), iter2->begin() + 1, iter2->end());
						result.push_back(temp_path);
					}
					else
					{
						bool force_continue = false;
						temp_path.insert(temp_path.end(), iter2->begin() + 1, iter2->end());
						for (auto iter_temp_path = temp_path.begin() + start_dis; iter_temp_path != temp_path.end() - end_pos_dis; iter_temp_path++)
						{
							if (stop_map.find(*iter_temp_path) != stop_map.end()) // in stop map
							{
								force_continue = true;
								break;
							}
						}
						if (force_continue)
						{
							continue;
						}
						result.push_back(temp_path);
					}
				}
			}
		}
	}

	void join_drop_longpaths_and_repeat_nodes_and_short_paths_stop_ndoeswithdis_with_length_fast_pruning(paths &a, uint distance, paths &result, ushort short_dis, std::set<uint> &stop_nodes, ushort start_dis, ushort end_pos_dis) // result path's length must be no more than distance
	{
		if (a.get_path().size() == 0)
		{
			// result.add_paths(path);//should be remain?
		}
		else if (path.size() == 0)
		{
			// result.add_paths(a);
		}
		else
		{
			std::unordered_map<uint, bool> stop_map;
			for (auto iter_stop_map = stop_nodes.begin(); iter_stop_map != stop_nodes.end(); iter_stop_map++)
			{
				stop_map.insert(std::make_pair(*iter_stop_map, true));
			}

			std::map<ushort, paths> left_distance_paths = construct_distance_paths(path);
			std::map<ushort, paths> right_distance_paths = construct_distance_paths(a);
			for (auto left_len = left_distance_paths.begin(); left_len != left_distance_paths.end(); left_len++)
			{
				for (auto right_len = right_distance_paths.begin(); right_len != right_distance_paths.end(); right_len++)
				{
					ushort total_dis = left_len->first + right_len->first;
					if (total_dis > distance || total_dis <= short_dis) // skip paths whose length do not meet our requirments
					{
						continue;
					}
					else
					{
						for (auto iter1 = left_len->second.path.begin(); iter1 != left_len->second.path.end(); iter1++)
						{

							std::unordered_map<uint, bool> duplicated;
							for (int i = 0; i < iter1->size(); i++)
							{
								duplicated.insert(std::make_pair((*iter1)[i], true));
							}
							for (auto iter2 = right_len->second.path.begin(); iter2 != right_len->second.path.end(); iter2++)
							{

								if (iter1->back() != iter2->front())
								{
									continue;
								}

								bool force_con = false;
								for (int j = 1; j < iter2->size(); j++)
								{
									if (duplicated.find((*iter2)[j]) != duplicated.end())
									{
										force_con = true;
										break;
									}
								}
								if (force_con)
									continue;

								std::vector<uint> temp_path(*iter1);
								if (stop_nodes.empty())
								{
									temp_path.insert(temp_path.end(), iter2->begin() + 1, iter2->end());
									result.push_back(temp_path);
								}
								else
								{
									bool force_continue = false;
									temp_path.insert(temp_path.end(), iter2->begin() + 1, iter2->end());
									for (auto iter_temp_path = temp_path.begin() + start_dis; iter_temp_path != temp_path.end() - end_pos_dis; iter_temp_path++)
									{
										if (stop_map.find(*iter_temp_path) != stop_map.end()) // in stop map
										{
											force_continue = true;
											break;
										}
									}
									if (force_continue)
									{
										continue;
									}
									result.push_back(temp_path);
								}
							}
						}
					}
				}
			}
		}
	}

	void join_drop_longpaths_and_repeat_nodes_and_short_paths_stop_ndoeswithdis_with_length_fast_pruning_include_meetnode(paths &a, uint distance, paths &result, ushort short_dis, std::set<uint> &stop_nodes, ushort start_dis, ushort end_pos_dis, uint meetnode) // result path's length must be no more than distance
	{
		if (a.get_path().size() == 0)
		{
			// result.add_paths(path);//should be remain?
		}
		else if (path.size() == 0)
		{
			// result.add_paths(a);
		}
		else
		{
			std::unordered_map<uint, bool> stop_map;
			for (auto iter_stop_map = stop_nodes.begin(); iter_stop_map != stop_nodes.end(); iter_stop_map++)
			{
				stop_map.insert(std::make_pair(*iter_stop_map, true));
			}

			std::map<ushort, paths> left_distance_paths = construct_distance_paths(path);
			std::map<ushort, paths> right_distance_paths = construct_distance_paths(a);
			for (auto left_len = left_distance_paths.begin(); left_len != left_distance_paths.end(); left_len++)
			{
				for (auto right_len = right_distance_paths.begin(); right_len != right_distance_paths.end(); right_len++)
				{
					ushort total_dis = left_len->first + right_len->first;
					if (total_dis > distance || total_dis <= short_dis) // skip paths whose length do not meet our requirments
					{
						continue;
					}
					else
					{
						for (auto iter1 = left_len->second.path.begin(); iter1 != left_len->second.path.end(); iter1++)
						{

							std::unordered_map<uint, bool> duplicated;
							for (int i = 0; i < iter1->size(); i++)
							{
								duplicated.insert(std::make_pair((*iter1)[i], true));
							}
							for (auto iter2 = right_len->second.path.begin(); iter2 != right_len->second.path.end(); iter2++)
							{

								if (iter1->back() != iter2->front())
								{
									continue;
								}

								bool force_con = false;
								for (int j = 1; j < iter2->size(); j++)
								{
									if (duplicated.find((*iter2)[j]) != duplicated.end())
									{
										force_con = true;
										break;
									}
								}
								if (force_con)
									continue;

								std::vector<uint> temp_path(*iter1);
								temp_path.insert(temp_path.end(), iter2->begin() + 1, iter2->end());
								bool not_include_meetnode = true;
								for (auto iter_temp_path = temp_path.begin() + start_dis; iter_temp_path != temp_path.end() - end_pos_dis; iter_temp_path++)
								{
									if (*iter_temp_path == meetnode) // in stop map
									{
										not_include_meetnode = false;
										break;
									}
								}
								if (not_include_meetnode)
								{
									continue;
								}

								if (stop_nodes.empty())
								{

									result.push_back(temp_path);
								}
								else
								{
									bool force_continue = false;
									for (auto iter_temp_path = temp_path.begin() + start_dis; iter_temp_path != temp_path.end() - end_pos_dis; iter_temp_path++)
									{
										if (stop_map.find(*iter_temp_path) != stop_map.end()) // in stop map
										{
											force_continue = true;
											break;
										}
									}
									if (force_continue)
									{
										continue;
									}
									result.push_back(temp_path);
								}
							}
						}
					}
				}
			}
		}
	}

	void join(paths &a, uint distance, paths &result) // result path's length must be no more than distance
	{
		if (a.get_path().size() == 0)
		{
			result.add_paths(path);
		}
		else if (path.size() == 0)
		{
			result.add_paths(a);
		}
		else
		{
			for (auto iter1 = path.begin(); iter1 != path.end(); iter1++)
			{
				for (auto iter2 = a.path.begin(); iter2 != a.path.end(); iter2++)
				{
					if (iter1->back() != iter2->front())
					{
						continue;
					}
					else if (iter1->size() + iter2->size() > distance + 2)
					{
						continue;
					}
					else
					{
						std::vector<uint> temp_path(*iter1);
						temp_path.insert(temp_path.end(), iter2->begin() + 1, iter2->end());
						result.push_back(temp_path);
					}
				}
			}
		}
	}

	// result path's length must be no more than distance
	paths join(paths &a, uint distance)
	{
		paths result;
		if (a.get_path().size() == 0)
		{
			result.add_paths(path);
		}
		else if (path.size() == 0)
		{
			result.add_paths(a);
		}
		else
		{
			for (auto iter1 = path.begin(); iter1 != path.end(); iter1++)
			{
				for (auto iter2 = a.path.begin(); iter2 != a.path.end(); iter2++)
				{
					if (iter1->back() != iter2->front())
					{
						continue;
					}
					else if (iter1->size() + iter2->size() > distance + 2)
					{
						continue;
					}
					else
					{
						std::vector<uint> temp_path(*iter1);
						temp_path.insert(temp_path.end(), iter2->begin() + 1, iter2->end());
						result.push_back(temp_path);
					}
				}
			}
		}
		return result;
	}
};

class cur_path
{
public:
	std::vector<std::vector<uint>> path;

	cur_path(){};

	cur_path(std::vector<std::vector<uint>> a) : path(a){};

	void push_back(std::vector<uint> temp)
	{
		path.push_back((temp));
	}

	uint get_length()
	{
		uint length = 0;
		for (auto iter = path.begin(); iter != path.end(); iter++)
		{
			length += iter->size();
		}
		return length;
	}

	paths get_path()
	{
		paths result;
		for (auto iter = path.begin(); iter != path.end(); iter++)
		{
			paths temp;
			temp.push_back(*iter);
			result = result.join(temp);
		}
		return result;
	}
};

class two_nodes_path_index
{
public:
	uint nodeStart;
	uint nodeEnd;
	std::vector<std::vector<uint>> paths;

	two_nodes_path_index(uint start, uint end) : nodeStart(start), nodeEnd(end) {} // should define start and end node first

	void push_back(std::vector<uint> a)
	{
		if (nodeStart != a.front() || nodeEnd != a.back())
		{
			return;
		}
		else
		{
			paths.push_back(a);
		}
	}

	uint get_start() { return nodeStart; }
	uint get_end() { return nodeEnd; }
	std::vector<std::vector<uint>> &get_paths() { return paths; }

	void output()
	{
		if (paths.size() == 0)
		{
			return;
		}
		for (auto iter = paths.begin(); iter != paths.end(); iter++)
		{
			for (auto iter2 = iter->begin(); iter2 != iter->end(); iter2++)
			{
			}
		}
	}

	std::vector<std::vector<uint>> get_paths_with_distance(int distance)
	{
		if (distance < 0)
		{
			std::vector<std::vector<uint>> a;
			return a;
		}
		else
		{
			std::vector<std::vector<uint>> result;
			for (auto iter = paths.begin(); iter != paths.end(); iter++)
			{
				if (iter->size() <= distance + 1)
				{
					result.push_back(*iter);
				}
			}
			return result;
		}
	}

	void add_path(two_nodes_path_index t)
	{
		if (nodeStart != t.get_start() || nodeEnd != t.get_end())
		{
			// cout << "two path share not the same start node and end node " << endl;
		}
		else
		{
			for (auto iter = t.get_paths().begin(); iter != t.get_paths().end(); iter++)
			{
				paths.push_back(*iter);
			}
		}
	}
};

class start_paths
{
public:
	std::map<uint, two_nodes_path_index> t_path; // map end to paths
	std::set<uint> reach_nodes;
	uint start_node;

	start_paths(uint s) : start_node(s) {}

	start_paths(two_nodes_path_index &path, uint start)
	{
		start_node = start;
		reach_nodes.insert(path.get_end());
		auto iter = t_path.find(path.get_end());
		if (iter != t_path.end())
		{
			iter->second.add_path(path);
		}
		else
		{
			t_path.insert(std::map<uint, two_nodes_path_index>::value_type(path.get_end(), path));
		}
	}

	paths get_paths()
	{
		paths result;
		for (auto iter = t_path.begin(); iter != t_path.end(); iter++)
		{
			paths temp;
			temp = iter->second.get_paths();
			result.add_paths(temp);
		}
		return result;
	}

	void push_back(two_nodes_path_index path)
	{
		if (path.nodeStart != start_node)
		{
			// cout << " not the same start node " << endl;
			return;
		}
		else
		{
			auto iter = t_path.find(path.get_end());
			if (iter != t_path.end())
			{
				iter->second.add_path(path);
			}
			else
			{
				t_path.insert(std::map<uint, two_nodes_path_index>::value_type(path.get_end(), path));
			}
		}
	}

	std::vector<MyVec> get_paths_from_end_nodes(uint end_node, int distance)
	{
		std::vector<MyVec> result;
		auto iter = t_path.find(end_node);
		if (iter != t_path.end())
		{
			result = iter->second.get_paths_with_distance(distance);
			return result;
		}
		else
		{
			return result;
		}
	}

	void output()
	{
		for (auto iter = t_path.begin(); iter != t_path.end(); iter++)
		{
			iter->second.output();
		}
	}
};

class path_index
{
public:
	std::map<uint, start_paths> t_index; // map start nodes to start_paths, in start_paths, map endnodes to path_two_nodes
	std::set<uint> hot_points;

	path_index() {}

	path_index(std::set<uint> hot) : hot_points(hot){};

	path_index(two_nodes_path_index t)
	{
		uint start_node = t.get_start();
		auto iter = t_index.find(start_node);
		if (iter != t_index.end()) // there is an element
		{
			start_paths temp_s(t, t.get_start());
			iter->second = temp_s;
		}
		else
		{
			iter->second.push_back(t);
		}
	}

	void push_back(two_nodes_path_index t)
	{
		uint start_node = t.get_start();
		auto iter = t_index.find(start_node);
		if (iter != t_index.end()) // there is an element
		{
			iter->second.push_back(t);
		}
		else
		{
			start_paths temp(start_node);
			temp.push_back(t);
			t_index.insert(std::map<uint, start_paths>::value_type(start_node, temp));
		}
	}

	void push_back(paths t)
	{
		for (auto path = t.get_path().begin(); path != t.get_path().end(); path++)
		{
			uint start_node = path->front();
			auto iter = t_index.find(start_node);
			if (iter != t_index.end()) // there is an element
			{
				two_nodes_path_index temp_t(path->front(), path->back());
				temp_t.push_back(*path);
				iter->second.push_back(temp_t);
			}
			else
			{
				two_nodes_path_index temp_t(path->front(), path->back());
				temp_t.push_back(*path);
				start_paths temp(start_node);
				temp.push_back(temp_t);
				t_index.insert(std::map<uint, start_paths>::value_type(start_node, temp));
			}
		}
	}

	paths find_paths_between_two_hot_nodes(uint node1, uint node2, int distance) // distance means length of path
	{
		paths result;
		auto iter1 = t_index.find(node1);
		if (iter1 != t_index.end())
		{
			result = iter1->second.get_paths_from_end_nodes(node2, distance); // find all paths' path less or equal than distance bt node1 and node2

			return result;
		}
		else
		{
			return result;
		}
	}

	void find_paths_between_two_hot_nodes_index_without_cross_other_hotpoints(paths &result, cur_path c_path, uint node1, uint node2, int distance, int cur_distance) // distance means length of path
	{
		// paths result;
		if (node1 == node2)
		{
			return;
		}
		auto iter1 = t_index.find(node1);
		if (iter1 == t_index.end())
		{
			return;
		}
		paths temp_paths = iter1->second.get_paths();
		for (auto iter2 = temp_paths.path.begin(); iter2 != temp_paths.path.end(); iter2++)
		{
			if (iter2->size() == 0)
			{
				continue;
			}
			int temp_distance = cur_distance + iter2->size() - 1;
			if (temp_distance > distance)
			{
				continue;
			}
			else
			{
				cur_path temp_c_path(c_path.path);
				temp_c_path.push_back(*iter2);
				if (iter2->back() == node2)
				{
					paths temp_result = temp_c_path.get_path();
					result.add_paths(temp_result);
				}
				else
				{
					uint start_node = iter2->back();
					find_paths_between_two_hot_nodes_index_without_cross_other_hotpoints(result, temp_c_path, start_node, node2, distance, temp_distance);
				}
			}
		}
	}

	void output()
	{
		for (auto iter = t_index.begin(); iter != t_index.end(); iter++)
		{
			iter->second.output();
		}
	}
};

#endif // CGRAPH_H
