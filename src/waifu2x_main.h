#ifndef WAIFU2X_MAIN_H
#define WAIFU2X_MAIN_H
#include <stdio.h>
#include <algorithm>
#include <queue>
#include <vector>
#include <clocale>
#include <wchar.h>
#include <set>
#include "cpu.h"
#include "gpu.h"
#include "platform.h"
#include <comutil.h> 
#pragma comment(lib, "comsuppw.lib")
#include "waifu2x.h"
#include <time.h>
#include "filesystem_utils.h"


class Task
{
public:
    int id = 0;
    int webp;

    path_t inpath;
    path_t outpath;
    void* fileDate;
    unsigned long fileSize;
    std::string file = "jpg";
    bool isSuc = true;
    ncnn::Mat inimage;
    ncnn::Mat outimage;
    int callBack = 0;
    int modelIndex;
    unsigned long toW;
    unsigned long toH;
    int scale = 2;

    clock_t startTick;
    clock_t encodeTick;
    clock_t procTick;
    clock_t saveTick;
    void* out = 0;
    unsigned long outSize = 0;
};

class TaskQueue
{
public:
    TaskQueue()
    {
    }

    void put(const Task& v)
    {
        lock.lock();

        tasks.push(v);

        lock.unlock();

        condition.signal();
    }

    void get(Task& v, int timeout = 0)
    {
        lock.lock();
        if (timeout == 0)
        {
            while (tasks.size() == 0)
            {
                condition.wait(lock);
            }
        }
        else {
            if (tasks.size() == 0)
            {
                lock.unlock();
                condition.signal();
                return;
            }
        }

        v = tasks.front();
        tasks.pop();

        lock.unlock();

        condition.signal();
    }

    void clear()
    {
        lock.lock();

        while (!tasks.empty())
        {

            Task v = tasks.front();
            tasks.pop();
            if (v.fileDate) free(v.fileDate);
            v.fileDate = NULL;
            if (v.inimage.data) { free(v.inimage.data); v.inimage.data = NULL; }
        }

        lock.unlock();

        condition.signal();
    }

    void remove(std::set<int>& taskIds)
    {
        lock.lock();
        std::set<int>::iterator ite1 = taskIds.begin();
        std::set<int>::iterator ite2 = taskIds.end();
        std::queue<Task> newData;
        while (!tasks.empty())
        {
            Task v = tasks.front();
            tasks.pop();
            ite1 = taskIds.find(v.callBack);
            if (ite1 == ite2)
            {
                newData.push(v);
            }
            else
            {
                if (v.fileDate) free(v.fileDate);
                v.fileDate = NULL;
                if (v.inimage.data) { free(v.inimage.data); v.inimage.data = NULL; }
            }
        }
        tasks = newData;
        lock.unlock();
        condition.signal();
    }
private:
    ncnn::Mutex lock;
    ncnn::ConditionVariable condition;
    std::queue<Task> tasks;
};

int waifu2x_addData(const unsigned char* data, unsigned int size, int callBack, int modelIndex, const char* format, unsigned long toW, unsigned long toH);
int waifu2x_getData(void*& out, unsigned long& outSize, double& tick, int& callBack, unsigned int timeout);
int waifu2x_init();
int waifu2x_init_set(int gpuId2, int threadNum, const char * model);
int waifu2x_stop();
int waifu2x_clear();
int waifu2x_remove(std::set<int>&);

static int GpuId;
static int TotalJobsProc = 0;
static std::vector<ncnn::Thread*> ProcThreads;
static std::vector<Waifu2x*> Waifu2xList;
static int TaskId = 1;


#endif // WAIFU2X_MAIN_H