#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <chrono>

// 数据结构，表示一个文件中的数据块
struct DataBlock {
    std::vector<int64_t> data;
    std::string filename;
};

// 线程池类
class ThreadPool {
public:
    ThreadPool(size_t numThreads);
    ~ThreadPool();

    // 提交任务到线程池
    template <typename F>
    void enqueue(F&& func);

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};

ThreadPool::ThreadPool(size_t numThreads) : stop(false) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queueMutex);
                    // 使用条件变量等待任务或停止信号
                    this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                    if (this->stop && this->tasks.empty()) {
                        return;
                    }
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        // 设置停止标志，唤醒所有线程
        stop = true;
    }
    condition.notify_all();
    for (std::thread& worker : workers) {
        // 等待所有线程退出
        worker.join();
    }
}

template <typename F>
void ThreadPool::enqueue(F&& func) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        // 将任务加入队列
        tasks.emplace(std::forward<F>(func));
    }
    condition.notify_one();
}

// 归并排序函数
void mergeSort(std::vector<DataBlock>& blocks, std::vector<int64_t>& result) {
    for (auto& block : blocks) {
        // 将数据块中的数据合并到结果中
        result.insert(result.end(), block.data.begin(), block.data.end());
    }

    // 对结果进行排序
    std::sort(result.begin(), result.end());
}

// 从文本文件中读取包含正负数的数据块
DataBlock readDataBlock(const std::string& filename) {
    DataBlock block;
    block.filename = filename;

    std::ifstream inFile(filename);
    int64_t num;
    while (inFile >> num) {
        // 读取文件中的每个数并添加到数据块中
        block.data.push_back(num);
    }

    return block;
}

// 将数据块写入文本文件
void writeDataBlockText(const DataBlock& block) {
    std::ofstream outFile(block.filename + ".txt");
    for (int64_t num : block.data) {
        // 将数据块中的每个数写入文本文件
        outFile << num << " ";
    }
}

int main() {
    const size_t numThreads = 16;
    ThreadPool threadPool(numThreads);

    // 记录程序开始时间
    auto start_time = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());

    // 生成测试数据并写入文本文件
    const size_t totalFiles = 100;
    const size_t totalDataPerFile = 10000000;
    for (size_t i = 0; i < totalFiles; ++i) {
        std::string filename = "file" + std::to_string(i) + ".txt";  // 使用 .txt 扩展名
        std::ofstream outFile(filename);
        for (size_t j = 0; j < totalDataPerFile; ++j) {
            int64_t num = rand() % 2000000 - 1000000;  // 生成随机正负数
            outFile << num << " ";
        }
    }

    // 读取文本文件中的数据块
    std::vector<std::string> fileNames;
    for (size_t i = 0; i < totalFiles; ++i) {
        fileNames.push_back("file" + std::to_string(i) + ".txt");
    }

    std::vector<DataBlock> blocks;
    for (const std::string& fileName : fileNames) {
        blocks.push_back(readDataBlock(fileName));
    }

    // 将任务提交给线程池进行排序
    threadPool.enqueue([blocks,start_time]() {
        std::vector<int64_t> result;
        mergeSort(const_cast<std::vector<DataBlock>&>(blocks), result);

        // 记录程序结束时间
        auto end_time = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
        
        std::cout << "Sorting completed in " << duration << " seconds." << std::endl;

        // 将排序结果写入文本文件
        DataBlock resultBlock;
        resultBlock.data = result;
        resultBlock.filename = "output";
        writeDataBlockText(resultBlock);
    });

    // 主线程等待排序任务完成
    std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}

