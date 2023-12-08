#include <iostream>
#include <iomanip>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>

// 函数用于输出文件的权限信息
void printPermissions(mode_t mode) {
    // 输出文件类型（d表示目录，-表示文件）
    std::cout << ((S_ISDIR(mode)) ? "d" : "-");
    // 输出用户权限
    std::cout << ((mode & S_IRUSR) ? "r" : "-");
    std::cout << ((mode & S_IWUSR) ? "w" : "-");
    std::cout << ((mode & S_IXUSR) ? "x" : "-");
    // 输出组权限
    std::cout << ((mode & S_IRGRP) ? "r" : "-");
    std::cout << ((mode & S_IWGRP) ? "w" : "-");
    std::cout << ((mode & S_IXGRP) ? "x" : "-");
    // 输出其他用户权限
    std::cout << ((mode & S_IROTH) ? "r" : "-");
    std::cout << ((mode & S_IWOTH) ? "w" : "-");
    std::cout << ((mode & S_IXOTH) ? "x" : "-");
}

// 函数用于列出指定目录下的文件信息
void listFiles(const char *path) {
    DIR *dir;
    struct dirent *entry;

    // 打开指定目录
    if ((dir = opendir(path)) == nullptr) {
        perror("opendir");
        return;
    }

    // 读取目录中的文件信息
    while ((entry = readdir(dir)) != nullptr) {
        struct stat fileStat;
        char filePath[PATH_MAX];
        snprintf(filePath, sizeof(filePath), "%s/%s", path, entry->d_name);

        // 获取文件信息
        if (stat(filePath, &fileStat) == -1) {
            perror("stat");
            continue;
        }

        // 输出文件权限信息
        printPermissions(fileStat.st_mode);
        // 输出链接数、所有者、组信息
        std::cout << " " << std::setw(2) << fileStat.st_nlink;
        std::cout << " " << std::left << std::setw(8) << getpwuid(fileStat.st_uid)->pw_name;
        std::cout << " " << std::left << std::setw(8) << getgrgid(fileStat.st_gid)->gr_name;
        // 输出文件大小
        std::cout << " " << std::right << std::setw(8) << fileStat.st_size;
        // 输出文件名
        std::cout << " " << entry->d_name << std::endl;
    }

    // 关闭目录
    closedir(dir);
}

int main(int argc, char *argv[]) {
    const char *path;

    // 检查是否指定了目录，如果没有，默认使用当前目录
    if (argc > 1) {
        path = argv[1];
    } else {
        path = ".";
    }

    // 列出目录下的文件信息
    listFiles(path);

    return 0;
}
