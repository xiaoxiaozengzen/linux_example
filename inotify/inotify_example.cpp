#include <iostream>
#include <thread>
#include <chrono>

#include <sys/inotify.h>
#include <unistd.h>

std::string file_to_watch = "/mnt/workspace/cgz_workspace/Exercise/linux_example/inotify/input/file.txt";

/**
 * @brief 内核中使用inotify_event来监视文件系统事件
 * 
 * struct inotify_event {
 *   int      wd;       // 监视描述符
 *   uint32_t mask;     // 事件掩码
 *   uint32_t cookie;   // 用于连接相关事件的cookie
 *   uint32_t len;      // name字段的长度
 *   char     name[];   // 可选的文件名
 * };
 */

int main() {
    // 1. 创建inotify实例
    int fd = inotify_init();
    if (fd < 0) {
        std::cerr << "Failed to initialize inotify" << std::endl;
        return 1;
    }

    // 2. 添加监视器

    /**
     * int wd = inotify_add_watch(int fd, const char* pathname, uint32_t mask);
     * @brief 添加文件到监视列表
     * @param fd inotify实例的文件描述符
     * @param pathname 需要监视的文件路径
     * @param mask 监视事件的掩码，可以是以下值的组合：
     *             IN_ACCESS        文件被访问
     *             IN_MODIFY        文件被修改
     *             IN_ATTRIB        文件元数据被修改，例如权限、时间戳等
     *             IN_CLOSE_WRITE   可写文件被关闭
     *             IN_CLOSE_NOWRITE 不可写文件被关闭
     *             IN_OPEN          文件被打开
     *             IN_MOVED_FROM    文件被移动
     *             IN_MOVED_TO      文件被移动到监视目录
     *             IN_CREATE        目录中创建新文件
     *             IN_DELETE        目录中删除文件
     *             IN_DELETE_SELF   监视的文件被删除
     *             IN_MOVE_SELF     监视的文件被移动
     *             IN_ALL_EVENTS    监视所有事件
     * @return 返回监视描述符，失败返回-1
     * 
     */
    int wd = inotify_add_watch(fd, file_to_watch.c_str(), IN_MODIFY);
    if (wd < 0) {
        std::cerr << "Failed to add watch for " << file_to_watch << std::endl;
        close(fd);
        return 1;
    }

    // 检测事件是否发生，没有发生则read阻塞
    char buffer[512];
    int event_size = sizeof(struct inotify_event);
    std::size_t num = read(fd, buffer, sizeof(buffer));
    if(num < event_size) {
        // 小于一个事件的大小，说明读取失败
        std::cerr << "Read error" << std::endl;
        inotify_rm_watch(fd, wd);
        close(fd);
        return 1;
    }

    // 解析事件: read返回值是一个或多个事件的总字节数
    int pos = 0;
    struct inotify_event *event = reinterpret_cast<struct inotify_event *>(buffer + pos);
    while (num >= event_size) {
        std::cout << "Parse event: " << std::endl;
        if(event->len > 0) {
            if (event->mask & IN_MODIFY) {
                std::cout << "File " << event->name << " was modified." << std::endl;
            }
            if(event->mask & IN_ACCESS) {
                std::cout << "File " << event->name << " was accessed." << std::endl;
            }
        } else {
            std::cout << "Event len is 0" << std::endl;
        }

        
        int real_size = event_size + event->len;
        num -= real_size;
        pos += real_size;
        event = reinterpret_cast<struct inotify_event *>(buffer + pos);
    }

    // 3. 移除监视器
    /**
     * int inotify_rm_watch(int fd, int wd);
     * @brief 移除监视器
     * @param fd inotify实例的文件描述符
     * @param wd 监视描述符
     * 
     */
    int ret = inotify_rm_watch(fd, wd);
    if (ret < 0) {
        std::cerr << "Failed to remove watch" << std::endl;
    }

    return 0;
}