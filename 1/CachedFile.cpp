// CachedFile.cpp
#include "CachedFile.h"

// 构造函数，打开文件并初始化成员
CachedFile::CachedFile(const std::string& filename, size_t cacheSize)
    : filename(filename), cacheSize(cacheSize), buffer(cacheSize, 0), bufferIndex(0), filePos(0) {
    file.open(filename, std::ios::in | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
    }
}

// 析构函数，关闭文件和释放资源
CachedFile::~CachedFile() {
    flushCache();
    file.close();
}

// 返回文件是否打开
bool CachedFile::isOpen() const {
    return file.is_open();
}

// 读取数据函数
ssize_t CachedFile::read(void* data, size_t size) {
    std::unique_lock<std::mutex> lock(mutex);
    if (!isOpen()) {
        return -1;
    }

    if (bufferIndex < cacheSize) {
        // 从缓存中读取数据
        size_t bytesToCopy = std::min(size, cacheSize - bufferIndex);
        std::memcpy(data, &buffer[bufferIndex], bytesToCopy);
        bufferIndex += bytesToCopy;
        size -= bytesToCopy;
        data = static_cast<char*>(data) + bytesToCopy;
    }

    if (size > 0) {
        // 从文件中读取数据
        file.seekg(filePos);
        file.read(static_cast<char*>(data), size);
        ssize_t bytesRead = file.gcount();
        filePos += bytesRead;
        return bytesRead;
    }

    return size;
}

// 写入数据函数
ssize_t CachedFile::write(const void* data, size_t size) {
    std::unique_lock<std::mutex> lock(mutex);
    if (!isOpen()) {
        return -1;
    }

    if (bufferIndex + size < cacheSize) {
        // 写入缓存
        std::memcpy(&buffer[bufferIndex], data, size);
        bufferIndex += size;
        return size;
    } else {
        // 刷新缓存并写入文件
        flushCache();
        file.seekp(filePos);
        file.write(static_cast<const char*>(data), size);
        ssize_t bytesWritten = file.tellp() - filePos;
        filePos += bytesWritten;
        return bytesWritten;
    }
}

// 定位函数
off_t CachedFile::lseek(off_t offset, int whence) {
    std::unique_lock<std::mutex> lock(mutex);
    if (!isOpen()) {
        return -1;
    }

    switch (whence) {
        case SEEK_SET:
            filePos = offset;
            break;
        case SEEK_CUR:
            filePos += offset;
            break;
        case SEEK_END:
            file.seekg(0, std::ios::end);
            filePos = file.tellg() + offset;
            break;
        default:
            return -1;
    }

    return filePos;
}

// 关闭文件函数
void CachedFile::close() {
    flushCache();
    file.close();
}

// 刷新缓存函数
void CachedFile::flush() {
    std::unique_lock<std::mutex> lock(mutex);
    flushCache();
    file.flush();
}

// 刷新缓存中的数据到文件
void CachedFile::flushCache() {
    if (bufferIndex > 0) {
        file.seekp(filePos);
        file.write(reinterpret_cast<const char*>(buffer.data()), bufferIndex);
        filePos += bufferIndex;
        bufferIndex = 0;
    }
}
