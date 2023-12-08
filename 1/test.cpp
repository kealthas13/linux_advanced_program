// integration_test.cpp
#include "CachedFile.h"
#include <iostream>
#include <cstring>

int main() {
    // Integration test
    CachedFile cachedFile("example.txt", 1024);

    if (cachedFile.isOpen()) {
        // Write data to file
        const char* dataToWrite = "Integration Test!";
        ssize_t bytesWritten = cachedFile.write(dataToWrite, std::strlen(dataToWrite));

        if (bytesWritten > 0) {
            std::cout << "Bytes written to file: " << bytesWritten << std::endl;
        }

        // Read data from file
        char buffer[50];
        ssize_t bytesRead = cachedFile.read(buffer, sizeof(buffer));

        if (bytesRead > 0) {
            std::cout << "Bytes read from file: " << bytesRead << std::endl;
            std::cout << "Data read from file: " << std::string(buffer, bytesRead) << std::endl;
        }

        // Seek and read data
        cachedFile.lseek(-5, SEEK_END);
        bytesRead = cachedFile.read(buffer, 5);

        if (bytesRead > 0) {
            std::cout << "Bytes read after seeking: " << bytesRead << std::endl;
            std::cout << "Data read after seeking: " << std::string(buffer, bytesRead) << std::endl;
        }

        // Close the file
        cachedFile.close();
    }

    return 0;
}
