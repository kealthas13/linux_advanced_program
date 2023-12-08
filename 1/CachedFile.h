// CachedFile.h
#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <mutex>

class CachedFile {
public:
    // 构造函数，打开文件并初始化成员
    CachedFile(const std::string& filename, size_t cacheSize);

    // 析构函数，关闭文件和释放资源
    ~CachedFile();

    // 返回文件是否打开
    bool isOpen() const;

    // 读取数据函数
    ssize_t read(void* data, size_t size);

    // 写入数据函数
    ssize_t write(const void* data, size_t size);

    // 定位函数
    off_t lseek(off_t offset, int whence);

    // 关闭文件函数
    void close();

    // 刷新缓存函数
    void flush();

private:
    // 刷新缓存中的数据到文件
    void flushCache();

    // 文件名
    std::string filename;

    // 缓存大小
    size_t cacheSize;

    // 缓存数据的容器
    std::vector<char> buffer;

    // 缓存中已使用的数据大小
    size_t bufferIndex;

    // 文件读写位置
    std::streampos filePos;

    // 文件流对象，用于实际的文件读写
    std::fstream file;

    // 互斥锁，确保多线程环境下的安全性
    std::mutex mutex;
};
