#ifndef OBFUSCATOR_H
#define OBFUSCATOR_H

#include <queue>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <random>
#include <thread>

using namespace std;

constexpr int OBFUSCATION_THREADS = 2;

enum FunctionID
{
    funcD_ii_enumidx,
    funcB_enumidx,
    funcE_ii_enumidx,
    funcC_enumidx,
    funcA_enumidx,
};

struct funcD_ii_values
{
    int a;
    int b;
    int funcD_ii_return;
    bool funcD_ii_done;
};


struct funcB_values
{
    bool funcB_done;
};


struct funcE_ii_values
{
    int a;
    int b;
    int funcE_ii_return;
    bool funcE_ii_done;
};


struct funcC_values
{
    bool funcC_done;
};


struct funcA_values
{
    bool funcA_done;
};

extern queue<int> funcD_ii_params_index_pool;
extern queue<int> funcB_params_index_pool;
extern queue<int> funcE_ii_params_index_pool;
extern queue<int> funcC_params_index_pool;
extern queue<int> funcA_params_index_pool;

extern vector<funcD_ii_values> funcD_ii_params;
extern vector<funcB_values> funcB_params;
extern vector<funcE_ii_values> funcE_ii_params;
extern vector<funcC_values> funcC_params;
extern vector<funcA_values> funcA_params;

extern thread threads[OBFUSCATION_THREADS];
extern queue<pair<int, int>> queues[OBFUSCATION_THREADS];
extern mutex mutexes[OBFUSCATION_THREADS];
extern condition_variable conditions[OBFUSCATION_THREADS];

extern bool stopThreads;
extern mutex stopMutex;

extern atomic<int> g_inFlightTasks;
extern condition_variable g_allTasksDoneCV;
extern mutex g_allTasksDoneMtx;

extern std::atomic<int> *vec;

void initialize();
void exit();
void taskFinished();
int getBalancedRandomIndex();
void pushToThread(int funcId, int line_no, int param_index);
void execute(int thread_idx);
void threadFunction(int thread_idx);

void funcD_ii(int thread_idx, int param_index);
void funcB(int thread_idx, int param_index);
void funcE_ii(int thread_idx, int param_index);
void funcC(int thread_idx, int param_index);
void funcA(int thread_idx, int param_index);

#endif
