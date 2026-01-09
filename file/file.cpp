#include <iostream>
#include <fcntl.h>
#include <unistd.h>

#include <string>

int main(int argc, char** argv) {
    int fd = open("/proc/self/cmdline", O_RDONLY);
    if(fd == -1) {
        std::cerr << "Failed to open /proc/self/cmdline\n";
        return 1;
    }

    char buffer[4096];
    ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
    if(bytesRead == -1) {
        std::cerr << "Failed to read from /proc/self/cmdline\n";
        close(fd);
        return 1;
    }
    std::cout << "Read " << bytesRead << " bytes from /proc/self/cmdline\n";

    const int maxNullCharNum = 2;
    int nullCharCount = 0;
    std::string cmdlineStr;
    for(ssize_t i = 0; i < bytesRead; ++i) {
        if(buffer[i] == '\0') {
            std::cout << "Argument: " << cmdlineStr << '\n';
            cmdlineStr.clear();
            nullCharCount++;
            if(nullCharCount >= maxNullCharNum) {
                std::cout << "enter end of cmdline parsing\n";
                break;
            }
        } else {
            cmdlineStr += buffer[i];
            nullCharCount = 0;
        }
    }

    buffer[bytesRead] = '\0';
    std::cout << "Contents of /proc/self/cmdline: " << buffer << '\n';

    close(fd);
    return 0;
}