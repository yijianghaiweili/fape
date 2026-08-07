#pragma once
#ifndef COMMON_H
#define COMMON_H

#include <sstream>       // 流输入输出
#include <fstream>       // 文件输入输出
#include <iostream>      // IO输入输出
#include <iomanip>       // IO输入输出格式
#include <chrono>        // 时间
#include <string>        // 字符串
#include <vector>        // 动态数组
#include <queue>         // 队列
#include <deque>         // 双端队列
#include <stack>         // 栈
#include <set>           // 集合 红黑树
#include <unordered_set> // 集合 哈希表
#include <map>           // 字典映射 红黑树
#include <unordered_map> // 字典映射 哈希表
#include <future>        // 异步
#include <thread>        // 多线程
#include <algorithm>     // 算法库
#include <numeric>       // 算法库
// #include <ctime>         //
#include "log/log.h"
#include "sparsepp/spp.h"

typedef uint16_t ushort;
typedef uint32_t uint;
typedef uint64_t ulonglong;

typedef std::vector<ushort> MyVecShort;
typedef std::vector<uint> MyVec;
typedef std::vector<ulonglong> MyVecLong;

struct pair_hash
{
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2> &p) const
    {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ h2;
    }
};

#define MAX_K (15)

#endif
