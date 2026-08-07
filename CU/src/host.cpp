#include "util/common.h"
#include "util/time/timer.h"
#include "util/io/io.h"
#include "util/graph/CDirectedGraph.h"
#include "CHcPE.h"
#include <sys/types.h>
#include <sys/stat.h>

bool execute_within_time_limit(CHcPE *enumerator, const uint iSouce, const uint iTarget, const bool count_only, CHcPE::query_method method_type, ulonglong time_limit, uint &overlarge)
{
	g_exit = false;
	overlarge = false;
	if (time_limit == 0)
	{
		enumerator->Excute(method_type, iSouce, iTarget, count_only, overlarge);
		return !g_exit;
	}
#ifdef USE_TIME_LIMIT
	std::future<ulonglong> future = std::async(std::launch::async, [enumerator, iSouce, iTarget, count_only, method_type, &overlarge]()
											   { return enumerator->Excute(method_type, iSouce, iTarget, count_only, overlarge); });
	std::future_status status;
	do
	{
		// log_info("wating.... %u", time_limit);
		status = future.wait_for(std::chrono::seconds(time_limit));
		if (status == std::future_status::deferred)
		{
			log_error("Deferred.");
			exit(-1);
		}
		else if (status == std::future_status::timeout)
		{
			// log_info("Timeout.");
			g_exit = true;
		}
	} while (status != std::future_status::ready);
#else
	enumerator->Excute(method_type, iSouce, iTarget, count_only, overlarge);
#endif
	return !g_exit;
}

CHcPE::query_method translate_method_code(std::string input_method)
{
	CHcPE::query_method method_type;
	switch (std::stoul(input_method))
	{
	case 0:
		method_type = CHcPE::query_method::DFS;
		break;
	case 11:
		method_type = CHcPE::query_method::C_DFS;
		break;
	case 12:
		method_type = CHcPE::query_method::T_DFS;
		break;
	case 13:
		method_type = CHcPE::query_method::T_DFS2;
		break;
	case 14:
		method_type = CHcPE::query_method::HP_Index;
		break;
	case 15:
		method_type = CHcPE::query_method::BC_DFS;
		break;
	case 151:
		method_type = CHcPE::query_method::BC_DFS_Induced;
		break;
	case 152:
		method_type = CHcPE::query_method::BC_DFS_Induced_BlockInitedBySDt;
		break;
	case 16:
		method_type = CHcPE::query_method::BC_JOIN;
		break;
	case 17:
		method_type = CHcPE::query_method::IDX_DFS;
		break;
	case 18:
		method_type = CHcPE::query_method::IDX_JOIN;
		break;
	case 19:
		method_type = CHcPE::query_method::IDX_ENUM;
		break;
	case 21:
		method_type = CHcPE::query_method::D_DFS;
		break;
	case 22:
		method_type = CHcPE::query_method::DBC_DFS;
		break;
	case 23:
		method_type = CHcPE::query_method::SCB;
		break;
	case 24:
		method_type = CHcPE::query_method::SCB_plus;
		break;
	case 61:
		method_type = CHcPE::query_method::MY_DFS;
		break;
	case 62:
		method_type = CHcPE::query_method::PEFP_DFS;
		break;
	case 63:
		method_type = CHcPE::query_method::MY_JOIN;
		break;
	case 64:
		method_type = CHcPE::query_method::MY_DFS2;
		break;
	case 65:
		method_type = CHcPE::query_method::MY_DFS3;
		break;
	case 71:
		method_type = CHcPE::query_method::FPGA_PEFP;
		break;
	case 72:
		method_type = CHcPE::query_method::FPGA_MYDFS;
		break;
	case 721:
		method_type = CHcPE::query_method::FPGA_MYDFS_R;
		break;
	case 722:
		method_type = CHcPE::query_method::FPGA_MYDFS_Multi;
		break;
	case 73:
		method_type = CHcPE::query_method::FPGA_MYJOIN;
		break;
	case 74:
		method_type = CHcPE::query_method::FPGA_IDX_DFS;
		break;
	case 741:
		method_type = CHcPE::query_method::FPGA_IDX_DFS_Multi;
		break;
	case 81:
		method_type = CHcPE::query_method::FPGA_D_DFS;
		break;
	default:
		method_type = CHcPE::query_method::DFS;
	}
	return method_type;
}

// HcPE
// ./HcPE.out xcl ../../Data/web-BerkStan.txt 0 ../../Data/for_demo/hot2hot_pairs.bin 5 0 16 0
// /home/hwl/code/build/HcPE/HcPE.out xcl /home/hwl/code/Data/web-BerkStan/web-BerkStan.txt 0 /home/hwl/code/Data/web-BerkStan/hot2hot_pairs.bin 4 0 61 0
// /home/huangwenlei/test/build/HcPE/HcPE.out xcl /home/huangwenlei/test/Data/web-Google/web-Google.txt 0 /home/huangwenlei/test/Data/web-Google/hot2hot_pairs.bin 5 0 16 0
// /home/hwl/code/build/HcPE/HcPE.out xcl /home/hwl/code/Data/reactome/reactome.txt 0 /home/hwl/code/Data/reactome/hot2hot_pairs.bin 5 0 61 0
// /home/huangwenlei/test/build/HcPE/HcPE.out xcl /home/huangwenlei/test/Data/soc-Epinions1/soc-Epinions1.txt 0 /home/huangwenlei/test/Data/soc-Epinions1/hot2hot_pairs.bin 4 0 61 0
int main(int argc, char *argv[])
{
	std::string binaryFile = (argc < 2) ? "MYDFS.xclbin" : argv[1];
	std::string input_graph_folder = (argc < 3) ? "../../Data/reactome/reactome.txt" : argv[2];//图数据文件
	// std::string input_graph_folder = (argc < 3) ? "../../Data/soc-Epinions1/soc-Epinions1.txt" : argv[2];
	// std::string input_graph_folder = (argc < 3) ? "../../Data/soc-LiveJournal1/soc-LiveJournal1.txt" : argv[2];
	// std::string input_graph_folder = (argc < 3) ? "../../Data/web-Google/web-Google.txt" : argv[2];
	// std::string input_graph_folder = (argc < 3) ? "../../Data/web-BerkStan.txt" : argv[2];
	// std::string input_graph_folder = (argc < 3) ? "../../Data/edge.txt" : argv[2];
	std::string input_skip_word = (argc < 4) ? "0" : argv[3];
	std::string input_query_folder = (argc < 5) ? "../../Data/reactome/hot2hot_pairs.bin" : argv[4];// 查询文件
	// std::string input_query_folder = (argc < 5) ? "../../Data/soc-Epinions1/hot2hot_pairs.bin" : argv[4];
	// std::string input_query_folder = (argc < 5) ? "../../Data/soc-LiveJournal1/hot2hot_pairs.bin" : argv[4];
	// std::string input_query_folder = (argc < 5) ? "../../Data/web-Google/hot2hot_pairs.bin" : argv[4];
	// std::string input_query_folder = (argc < 5) ? "../../Data/test_query.txt" : argv[4];
	// std::string input_query_folder = (argc < 5) ? "../../Data/for_demo/hot2hot_pairs.bin" : argv[4];
	// std::string input_query_folder = (argc < 5) ? "../../Data/web-BerkStan/hot2hot_pairs.bin" : argv[4];
	// std::string input_query_folder = (argc < 5) ? "../../Data/web-BerkStan_query.txt" : argv[4];
	// std::string input_query_folder = (argc < 5) ? "../../Data/edge_query.txt" : argv[4];
	std::string input_k = (argc < 6) ? "5" : argv[5];				  // hop constraint
	std::string input_d = (argc < 7) ? "0" : argv[6];				  // diversitify
	std::string input_method = (argc < 8) ? "61" : argv[7];			  // 17
	std::string input_time_limit = (argc < 9) ? "0" : argv[8];		  //
	std::string comparion_method = (argc < 10) ? "0" : argv[9];		  // 16
	std::string comparion_time_limit = (argc < 11) ? "2" : argv[10]; //
	std::string accel_raito_t = (argc < 12) ? "1" : argv[11];		  //
	std::string accel_raito_q = (argc < 13) ? "1" : argv[12];		  //
	std::string accel_select = (argc < 14) ? "100" : argv[13];		  // 选择的query数量

	char skip_character = '#';
	ushort k = std::stoul(input_k);
	ushort d = std::stoul(input_d);
	CHcPE::query_method method_type = translate_method_code(input_method);
	ulonglong per_query_time_limit = std::stoul(input_time_limit);
	CHcPE::query_method compare_method_type = translate_method_code(comparion_method);
	ulonglong compare_time_limit = std::stoul(comparion_time_limit);
	bool if_compare = compare_method_type != CHcPE::query_method::DFS;
	double accel_t = std::stod(accel_raito_t);
	accel_t = accel_t > 0 ? accel_t : 1;
	double accel_q = std::stod(accel_raito_q);
	accel_q = accel_q > 0 ? accel_q : 1;
	uint iAccelSelect = std::stoul(accel_select);
#ifdef USE_LOG
	log_info("graph: %s", input_graph_folder.c_str());
	log_info("query: %s", input_query_folder.c_str());
	log_info("k: %s", input_k.c_str());
	log_info("methed: %s", input_method.c_str());
#endif

	/**
	 * 初始化图、算法
	 */
	CDirectedGraph oGraph, oSubGraph;
	oGraph.CreateGraphCSR(input_graph_folder, skip_character);//// 创建CSR格式图
	oGraph.PrintMeta();
	CHcPE oHcPE;
	oHcPE.Init(&oGraph, &oSubGraph, k, binaryFile);//绑定到图和 FPGA 二进制文件

	/**
	 * 对照组
	 */
	CDirectedGraph oGraph1, oSubGraph1;
	oGraph1.CreateGraphCSR(input_graph_folder, skip_character);
	CHcPE oHcPE1;
	oHcPE1.Init(&oGraph1, &oSubGraph1, k);

	/**
	 * 加载查询
	 */
	std::vector<std::pair<uint, uint>> vQuery;
	if (input_query_folder.substr(input_query_folder.length() - 3, 3) == "bin")
		vQuery = CHcPE::LoadQueryFromBin(input_query_folder);//// 从二进制文件加载
	else
		vQuery = CHcPE::LoadQuery(input_query_folder, skip_character);//// 从文本文件加载

	/**
	 * 查询
	 */
	std::set<uint32_t> good_good; //
	std::set<uint32_t> good_tal;  // 统计加速
	std::set<uint32_t> good_query;
	uint terminated_cnt = 0, run_cnt = 0;
//	int querysize = vQuery.size();
	int querysize = vQuery.size() < 100 ? vQuery.size() : 100; // !
	std::vector<uint> vErrorCode;

//	time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
//	auto time_tm = localtime(&tt);
//	std::stringstream ss;
//	ss << std::put_time(time_tm, "%m%d%H%M");
//	std::ofstream outfile;
//	outfile.open("./" + ss.str() + "_" + input_method + "_" + input_k + "_" + std::to_string(querysize) + std::string("_result.txt"), std::ios::out | std::ios::app);
	// ===== 结果目录设置 =====
	// 取数据集名：从 input_graph_folder 中抽出最后的文件名再去掉 .txt
	std::string dataset_name;
	{
	    auto pos_slash = input_graph_folder.find_last_of("/\\");
	    std::string filename =
	        (pos_slash == std::string::npos)
	            ? input_graph_folder
	            : input_graph_folder.substr(pos_slash + 1);

	    auto pos_dot = filename.find_last_of('.');
	    dataset_name =
	        (pos_dot == std::string::npos)
	            ? filename
	            : filename.substr(0, pos_dot);
	}

	std::string base_result_dir = "../../result/";

	//  加上数据集子目录：result/Slashdot0902/
	std::string dataset_dir = base_result_dir + dataset_name;

	// 如果目录不存在就创建（第一次跑某个数据集会用到）
	struct stat st;
	if (stat(dataset_dir.c_str(), &st) != 0) {
	    // 目录不存在，创建它
	    mkdir(dataset_dir.c_str(), 0775);
	}
    // ===== 生成时间戳 =====
    time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto time_tm = localtime(&tt);
    std::stringstream ss_time;
    ss_time << std::put_time(time_tm, "%m%d%H%M");   // 例如 12081030

    // 文件名：method_k_time_result.txt
    std::string filename =
        input_method + "_" + input_k + "_" + ss_time.str() + "_result.txt";

    // 完整路径：./result/数据集名/文件名
    std::string outfile_path = dataset_dir + "/" + filename;

    std::ofstream outfile;
    outfile.open(outfile_path, std::ios::out | std::ios::app);


	if (!if_compare)
	{
		for (uint32_t i = 0; i < querysize; ++i)
		{
			auto &query = vQuery[i];
			uint bOverLarge = 0;
			bool bCountOnly = false;
			std::string query_status = "Complete";

			if (!execute_within_time_limit(&oHcPE, query.first, query.second, bCountOnly, method_type, per_query_time_limit, bOverLarge))
			{
				vErrorCode.push_back(1);
				terminated_cnt += 1;
				switch (bOverLarge)
				{
				case 0:
					query_status = "Time Out";
					break;
				case 1:
					query_status = "FPGA small batch";
					break;
				case 2:
					query_status = "FPGA small DRAM";
					break;
				case 3:
					query_status = "FPGA large tmp";
					break;
				default:
					query_status = "FPGA err";
					break;
				}
			}
			else
			{
				vErrorCode.push_back(bOverLarge);
				if (bOverLarge != 0)
				{
					terminated_cnt += 1;
					// oHcPE.vResultCount_.back() = 0;
					// oHcPE.vPreprocessTime_.back() = 0;
					// oHcPE.vQueryTime_.back() = 0;
					// oHcPE.vSubGraphNodeCount_.back() = 0;
					// oHcPE.vSubGraphEdgeCount_.back() = 0;
					// oHcPE.vDDR2BRAMTimes_.back() = 0;
					// oHcPE.vBRAM2DDRTimes_.back() = 0;
					// oHcPE.vDDR2BRAMDatas_.back() = 0;
					// oHcPE.vBRAM2DDRDatas_.back() = 0;
					// oHcPE.vTempPathsCount_.back() = 0;
				}
				else
				{
					run_cnt++;
				}
				switch (bOverLarge)
				{
				case 0:
					query_status = "Complete";
					break;
				case 1:
					query_status = "FPGA small batch";
					break;
				case 2:
					query_status = "FPGA small DRAM";
					break;
				case 3:
					query_status = "FPGA large tmp";
					break;
				default:
					query_status = "FPGA err";
					break;
				}
			}
			log_info("NO. %u, src %u, dst %u, status %s, result count %lu, preprocessing time %.6lf s, query time %.6lf s, b2d %lu, d2b %lu, b2dd %lu, dbbd %lu, tp %lu", i, query.first, query.second, query_status.c_str(), oHcPE.vResultCount_.back(), oHcPE.vPreprocessTime_.back() / (double)1000000000, oHcPE.vQueryTime_.back() / (double)1000000000, oHcPE.vBRAM2DDRTimes_.back(), oHcPE.vDDR2BRAMTimes_.back(), oHcPE.vBRAM2DDRDatas_.back(), oHcPE.vDDR2BRAMDatas_.back(), oHcPE.vTempPathsCount_.back());
		}
	}
	else
	{
		// outfile << "1t/2q	NO	s	t	comp_time	test_time	Accel" << std::endl;
		outfile << "NO	s	t	comp_t	comp_q	test_t	test_q	Accel_t	Accel_q" << std::endl;

		for (uint32_t i = 0; i < querysize; ++i)
		{
			auto &query = vQuery[i];
			// compare
			uint bOverLarge = 0;
			bool bCountOnly = false;
			std::string query_status = "Complete";
			if (!execute_within_time_limit(&oHcPE1, query.first, query.second, bCountOnly, compare_method_type, compare_time_limit, bOverLarge))
			{
				terminated_cnt += 1;
				log_info("NO. %u, src %u, dst %u Time Out", i, query.first, query.second);
				query_status = "Time Out";
				oHcPE.update_performance_counter1();
				continue;
			}
			// test
			bOverLarge = false;
			query_status = "Complete";
			if (!execute_within_time_limit(&oHcPE, query.first, query.second, bCountOnly, method_type, per_query_time_limit, bOverLarge))
			{
				if (bOverLarge)
					query_status = "FPGA Error";
				else
					query_status = "Time Out";
			}
			else
			{
				if (bOverLarge)
				{
					query_status = "FPGA Error";
					oHcPE.vPreprocessTime_.back() = 0;
				}
				else
					run_cnt++;
			}
			if (bOverLarge)
			{
				log_info("NO. %u, src %u, dst %u, status %s, sub-n: %u, sub-e: %u", i, query.first, query.second, query_status.c_str(), oHcPE.vSubGraphNodeCount_.back(), oHcPE.vSubGraphEdgeCount_.back());
				continue;
			}
			// result
			if (oHcPE.vResultCount_.back() == oHcPE1.vResultCount_.back())
			{
				log_info("NO. %u, src %u, dst %u, result count %lu, preprocessing time %.6lf s, query time %.6lf s", i, query.first, query.second, oHcPE1.vResultCount_.back(), oHcPE1.vPreprocessTime_.back() / (double)1000000000, oHcPE1.vQueryTime_.back() / (double)1000000000);
				log_info("NO. %u, src %u, dst %u, status %s, result count %lu, preprocessing time %.6lf s, query time %.6lf s", i, query.first, query.second, query_status.c_str(), oHcPE.vResultCount_.back(), oHcPE.vPreprocessTime_.back() / (double)1000000000, oHcPE.vQueryTime_.back() / (double)1000000000);
				double comp_p = (double)oHcPE1.vPreprocessTime_.back();
				double comp_q = (double)oHcPE1.vQueryTime_.back();
				double comp_t = (double)(comp_p + comp_q);
				double test_p = (double)oHcPE.vPreprocessTime_.back();
				double test_q = (double)oHcPE.vQueryTime_.back();
				double test_t = (double)(test_p + test_q);

				// if (test_q != 0 && test_t != 0)
				// {
				// 	if ((double)comp_t / test_t > accel_t)
				// 	{
				// 		good_tal.insert(i);
				// 		double comp = comp_t / 1000000000.0;
				// 		double test = test_t / 1000000000.0;
				// 		double ratio = comp / test;
				// 		outfile << 1 << "	" << i << "	" << vQuery[i].first << "	" << vQuery[i].second << "	" << std::fixed << std::setprecision(6) << comp << "	" << std::fixed << std::setprecision(6) << test << "	" << std::fixed << std::setprecision(2) << ratio << std::endl;
				// 	}
				// 	if ((double)comp_q / test_q > accel_q)
				// 	{
				// 		good_query.insert(i);
				// 		double comp = comp_q / 1000000000.0;
				// 		double test = test_q / 1000000000.0;
				// 		double ratio = comp / test;
				// 		outfile << 2 << "	" << i << "	" << vQuery[i].first << "	" << vQuery[i].second << "	" << std::fixed << std::setprecision(6) << comp << "	" << std::fixed << std::setprecision(6) << test << "	" << std::fixed << std::setprecision(2) << ratio << std::endl;
				// 	}
				// 	if ((double)comp_t / test_t > accel_t || (double)comp_q / test_q > accel_q)
				// 	{
				// 		// log_info("NO. %u, src %u, dst %u, result count %lu, preprocessing time %.6lf s, query time %.6lf s", i, query.first, query.second, oHcPE1.vResultCount_.back(), oHcPE1.vPreprocessTime_.back() / (double)1000000000, oHcPE1.vQueryTime_.back() / (double)1000000000);
				// 		// log_info("NO. %u, src %u, dst %u, status %s, result count %lu, preprocessing time %.6lf s, query time %.6lf s", i, query.first, query.second, query_status.c_str(), oHcPE.vResultCount_.back(), oHcPE.vPreprocessTime_.back() / (double)1000000000, oHcPE.vQueryTime_.back() / (double)1000000000);
				// 		log_info("Query Accelerate Ratio: %.2lf, Taltol Accelerate Ratio: %.2lf.", (double)comp_q / test_q, (double)comp_t / test_t);
				// 	}
				// 	if (good_tal.size() > iAccelSelect && good_query.size() > iAccelSelect)
				// 		break;
				// }

				if (test_q != 0 && test_t != 0)
				{
					if ((double)comp_t / test_t > accel_t && (double)comp_q / test_q > accel_q)
					{
						good_good.insert(i);
						comp_t = (double)comp_t / 1000000000.0;
						comp_q = (double)comp_q / 1000000000.0;
						test_t = (double)test_t / 1000000000.0;
						test_q = (double)test_q / 1000000000.0;
						// log_info("NO. %u, src %u, dst %u, result count %lu, preprocessing time %.6lf s, query time %.6lf s", i, query.first, query.second, oHcPE1.vResultCount_.back(), oHcPE1.vPreprocessTime_.back() / (double)1000000000, oHcPE1.vQueryTime_.back() / (double)1000000000);
						// log_info("NO. %u, src %u, dst %u, status %s, result count %lu, preprocessing time %.6lf s, query time %.6lf s", i, query.first, query.second, query_status.c_str(), oHcPE.vResultCount_.back(), oHcPE.vPreprocessTime_.back() / (double)1000000000, oHcPE.vQueryTime_.back() / (double)1000000000);
						log_info("Query Accelerate Ratio: %.2lf, Taltol Accelerate Ratio: %.2lf.", (double)comp_q / test_q, (double)comp_t / test_t);
						outfile << i << "	" << vQuery[i].first << "	" << vQuery[i].second << "	"
								<< std::fixed << std::setprecision(6) << comp_t << "	" << std::fixed << std::setprecision(6) << comp_q << "	"
								<< std::fixed << std::setprecision(6) << test_t << "	" << std::fixed << std::setprecision(6) << test_q << "	"
								<< std::fixed << std::setprecision(2) << comp_t / test_t << std::fixed << std::setprecision(2) << comp_q / test_q << std::endl;
					}
					if (good_good.size() > iAccelSelect)
						break;
				}
			}
		}
	}

	ulonglong total_query_time = std::accumulate(oHcPE.vQueryTime_.begin(), oHcPE.vQueryTime_.end(), 0ull);
	ulonglong total_preprocess_time = std::accumulate(oHcPE.vPreprocessTime_.begin(), oHcPE.vPreprocessTime_.end(), 0ull);
	ulonglong total_result_count = std::accumulate(oHcPE.vResultCount_.begin(), oHcPE.vResultCount_.end(), 0ull);
	ulonglong total_subgraph_node_count = std::accumulate(oHcPE.vSubGraphNodeCount_.begin(), oHcPE.vSubGraphNodeCount_.end(), 0ull);
	ulonglong total_subgraph_edge_count = std::accumulate(oHcPE.vSubGraphEdgeCount_.begin(), oHcPE.vSubGraphEdgeCount_.end(), 0ull);
	ulonglong max_subgraph_node_count = *std::max_element(oHcPE.vSubGraphNodeCount_.begin(), oHcPE.vSubGraphNodeCount_.end());
	ulonglong max_subgraph_edge_count = *std::max_element(oHcPE.vSubGraphEdgeCount_.begin(), oHcPE.vSubGraphEdgeCount_.end());
	ulonglong total_bram2ddr_time = std::accumulate(oHcPE.vBRAM2DDRTimes_.begin(), oHcPE.vBRAM2DDRTimes_.end(), 0ull);
	ulonglong total_ddr2bram_time = std::accumulate(oHcPE.vDDR2BRAMTimes_.begin(), oHcPE.vDDR2BRAMTimes_.end(), 0ull);
	ulonglong total_bram2ddr_data = std::accumulate(oHcPE.vBRAM2DDRDatas_.begin(), oHcPE.vBRAM2DDRDatas_.end(), 0ull);
	ulonglong total_ddr2bram_data = std::accumulate(oHcPE.vDDR2BRAMDatas_.begin(), oHcPE.vDDR2BRAMDatas_.end(), 0ull);
	ulonglong total_temp_path_count = std::accumulate(oHcPE.vTempPathsCount_.begin(), oHcPE.vTempPathsCount_.end(), 0ull);

	// uint iTrueQuerySize = run_cnt;
	uint iTrueQuerySize = querysize;

#ifdef USE_LOG
	log_info("graph: %s", input_graph_folder.c_str());
	log_info("query: %s", input_query_folder.c_str());
	log_info("k: %s", input_k.c_str());
	log_info("methed: %s", input_method.c_str());
	log_info("terminated query: %u", terminated_cnt);
	log_info("run query: %u", run_cnt);
	log_info("total query: %u", querysize);
	log_info("avg_pre time: %.6lf s", (double)total_preprocess_time / 1000000000.0 / (iTrueQuerySize));
	log_info("avg_query time: %.6lf s", (double)total_query_time / 1000000000.0 / (iTrueQuerySize));

	log_info("avg count: %.2lf", (double)total_result_count / (double)(iTrueQuerySize));
	log_info("avg_subgraph_node_size: %.2lf", (double)total_subgraph_node_count / (iTrueQuerySize));
	log_info("avg_subgraph_edge_size: %.2lf", (double)total_subgraph_edge_count / (iTrueQuerySize));
	log_info("max_subgraph_node_size: %lu", max_subgraph_node_count);
	log_info("max_subgraph_edge_size: %lu", max_subgraph_edge_count);
	log_info("avg_bram2ddr_times: %.2lf", (double)total_bram2ddr_time / (iTrueQuerySize));
	log_info("avg_ddr2bram_times: %.2lf", (double)total_ddr2bram_time / (iTrueQuerySize));
	log_info("avg_bram2ddr_datas: %.2lf", (double)total_bram2ddr_data / (iTrueQuerySize));
	log_info("avg_ddr2bram_datas: %.2lf", (double)total_ddr2bram_data / (iTrueQuerySize));
	log_info("avg_temp_path_count: %.2lf", (double)total_temp_path_count / (iTrueQuerySize));
#endif

	outfile << "graph file: " << input_graph_folder << std::endl;
	outfile << "queries file: " << input_query_folder << std::endl;
	outfile << "Constaint : " << input_k << std::endl;
	outfile << "method : " << input_method << std::endl;
	outfile << "num of terminated queries : " << terminated_cnt << std::endl;
	outfile << "num of run queries : " << run_cnt << std::endl;
	outfile << "average preprocess time: " << std::fixed << std::setprecision(6) << (double)total_preprocess_time / 1000000000.0 / (iTrueQuerySize) << "s" << std::endl;
	outfile << "average query time: " << std::fixed << std::setprecision(6) << (double)total_query_time / 1000000000.0 / (iTrueQuerySize) << "s" << std::endl;
	outfile << "average total query time: " << std::fixed << std::setprecision(6) << ((double)total_query_time+(double)total_preprocess_time) / 1000000000.0 / (iTrueQuerySize) << "s" << std::endl;
	outfile << "average result count: " << std::fixed << std::setprecision(2) << (double)total_result_count / (double)(iTrueQuerySize) << std::endl;
	outfile << "avg_subgraph_node_size: " << std::fixed << std::setprecision(2) << (double)total_subgraph_node_count / (double)(iTrueQuerySize) << std::endl;
	outfile << "avg_subgraph_edge_size: " << std::fixed << std::setprecision(2) << (double)total_subgraph_edge_count / (double)(iTrueQuerySize) << std::endl;
	outfile << "max_subgraph_node_size: " << max_subgraph_node_count << std::endl;
	outfile << "max_subgraph_edge_size: " << max_subgraph_edge_count << std::endl;
	outfile << "avg_bram2ddr_times: " << std::fixed << std::setprecision(2) << (double)total_bram2ddr_time / (double)(iTrueQuerySize) << std::endl;
	outfile << "avg_ddr2bram_times: " << std::fixed << std::setprecision(2) << (double)total_ddr2bram_time / (double)(iTrueQuerySize) << std::endl;
	outfile << "avg_bram2ddr_datas: " << std::fixed << std::setprecision(2) << (double)total_bram2ddr_data / (double)(iTrueQuerySize) << std::endl;
	outfile << "avg_ddr2bram_datas: " << std::fixed << std::setprecision(2) << (double)total_ddr2bram_data / (double)(iTrueQuerySize) << std::endl;
	outfile << "avg_temp_path_count: " << std::fixed << std::setprecision(2) << (double)total_temp_path_count / (double)(iTrueQuerySize) << std::endl
			<< std::endl;
	outfile << "NO	s	t	err	count	pre_time	query_time	sub_node	sub_edge	b2d	d2b	b2dD	d2bD	tp_c" << std::endl;
	for (auto i = 0; i < querysize; i++)
	{
		outfile << i << "	" << vQuery[i].first << "	" << vQuery[i].second << "	" << vErrorCode[i] << "	" << oHcPE.vResultCount_[i] << "	"
				<< std::fixed << std::setprecision(6) << (double)oHcPE.vPreprocessTime_[i] / 1000000000.0 << "	"
				<< std::fixed << std::setprecision(6) << (double)oHcPE.vQueryTime_[i] / 1000000000.0 << "	"
				<< oHcPE.vSubGraphNodeCount_[i] << "	" << oHcPE.vSubGraphEdgeCount_[i] << "	"
				<< oHcPE.vBRAM2DDRTimes_[i] << "	" << oHcPE.vDDR2BRAMTimes_[i] << "	" << oHcPE.vBRAM2DDRDatas_[i] << "	" << oHcPE.vDDR2BRAMDatas_[i] << "	"
				<< oHcPE.vTempPathsCount_[i] << std::endl;
	}

	// if (if_compare)
	// {
	// 	outfile << "NO	s	t	comp_time	test_time	Accel" << std::endl;
	// 	outfile << "good_tal : " << std::endl;
	// 	for (auto i : good_tal)
	// 	{
	// 		double comp = (oHcPE1.vQueryTime_[i] + oHcPE1.vPreprocessTime_[i]) / 1000000000.0;
	// 		double test = (oHcPE.vQueryTime_[i] + oHcPE.vPreprocessTime_[i]) / 1000000000.0;
	// 		double ratio = comp / test;
	// 		outfile << i << "	" << vQuery[i].first << "	" << vQuery[i].second << "	" << std::fixed << std::setprecision(6) << comp << "	" << std::fixed << std::setprecision(6) << test << "	" << std::fixed << std::setprecision(2) << ratio << std::endl;
	// 	}
	// 	outfile << "good_query : " << std::endl;
	// 	for (auto i : good_query)
	// 	{
	// 		double comp = (double)(oHcPE1.vQueryTime_[i]) / 1000000000.0;
	// 		double test = (double)(oHcPE.vQueryTime_[i]) / 1000000000.0;
	// 		double ratio = comp / test;
	// 		outfile << i << "	" << vQuery[i].first << "	" << vQuery[i].second << "	" << std::fixed << std::setprecision(6) << comp << "	" << std::fixed << std::setprecision(6) << test << "	" << std::fixed << std::setprecision(2) << ratio << std::endl;
	// 	}
	// }

	outfile.close();

	return 0;
}
