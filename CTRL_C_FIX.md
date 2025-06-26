# Ctrl+C 卡死问题修复说明

## 问题描述
程序运行后，按下 Ctrl+C 后程序出现卡死情况，无法正常退出。

## 问题根因分析

经过代码分析，发现以下几个主要问题：

### 1. 信号处理问题
**问题**: 在 `server_manager.hpp` 中，信号处理函数直接调用了 `exit(0)`
```cpp
static void signal_handler(int signal) {
    if (instance_) {
        instance_->stop();
    }
    exit(0);  // 这里直接退出，不给线程清理的机会
}
```

**影响**: 直接调用 `exit(0)` 会立即终止程序，不给线程足够的时间来正确清理资源。

### 2. accept() 阻塞问题
**问题**: 在 `http_server.cpp` 的 `accept_connections()` 方法中，`accept()` 调用会无限期阻塞
```cpp
int client_socket = accept(server_socket_, (struct sockaddr*)&client_address, &client_len);
```

**影响**: 即使 `running_` 标志被设置为 false，`accept()` 仍然在阻塞等待连接，只有当有新连接到来时才会检查 `running_` 状态。

### 3. 线程管理问题
**问题**: 线程清理机制不完善，可能导致资源泄漏或卡死。

## 修复方案

### 1. 改进信号处理
修改信号处理函数，移除 `exit(0)` 调用：
```cpp
static void signal_handler(int signal) {
    std::cout << "\n🛑 收到退出信号 (" << signal << ")..." << std::endl;
    if (instance_) {
        instance_->stop();
    }
    // 不要直接调用exit()，让程序自然退出
}
```

### 2. 使用非阻塞 accept() 和 select()
将服务器 socket 设置为非阻塞模式：
```cpp
// 设置socket为非阻塞模式
int flags = fcntl(server_socket_, F_GETFL, 0);
if (fcntl(server_socket_, F_SETFL, flags | O_NONBLOCK) == -1) {
    // 错误处理
}
```

使用 `select()` 实现带超时的连接等待：
```cpp
void accept_connections() {
    while (running_.load()) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(server_socket_, &read_fds);
        
        struct timeval timeout;
        timeout.tv_sec = 1;  // 1秒超时
        timeout.tv_usec = 0;
        
        int activity = select(server_socket_ + 1, &read_fds, nullptr, nullptr, &timeout);
        
        if (activity == 0) {
            // 超时，继续循环检查running_状态
            continue;
        }
        
        if (FD_ISSET(server_socket_, &read_fds)) {
            // 有新连接，处理accept()
        }
    }
}
```

### 3. 改进停止机制
增强 `stop()` 方法，确保正确的清理顺序：
```cpp
void stop() {
    running_.store(false);
    
    // 关闭服务器socket以中断accept()调用
    if (server_socket_ >= 0) {
        shutdown(server_socket_, SHUT_RDWR);
        close(server_socket_);
        server_socket_ = -1;
    }
    
    // 等待accept线程结束
    if (!worker_threads_.empty() && worker_threads_[0].joinable()) {
        worker_threads_[0].join();
    }
    
    // 给客户端处理线程时间完成，设置超时避免无限等待
    // ...
}
```

## 修复效果

修复后的程序能够：

1. **响应 Ctrl+C 信号**: 立即开始优雅关闭流程
2. **正确清理资源**: 所有线程都能得到适当的清理
3. **避免卡死**: 即使在高负载情况下也能快速响应退出信号
4. **超时保护**: 如果某些线程无法在合理时间内完成，会强制退出避免无限等待

## 测试验证

经过测试验证：
- ✅ 程序启动正常
- ✅ Ctrl+C 能够立即响应
- ✅ 服务器优雅关闭，显示清理信息
- ✅ 所有进程都能正确退出，没有僵尸进程

## 技术要点

1. **信号处理**: 避免在信号处理函数中调用 `exit()`
2. **非阻塞 I/O**: 使用 `select()` 和非阻塞 socket 避免无限期阻塞
3. **超时机制**: 为线程清理设置合理的超时时间
4. **资源管理**: 确保 socket 和线程资源得到正确释放

这些修复确保了程序能够响应系统信号并优雅地关闭，解决了 Ctrl+C 卡死的问题。