# 现代C++ HTTP服务器

一个使用现代C++特性实现的简易HTTP服务器，支持多线程并发处理和灵活的路由系统。

## ✨ 特性

- **现代C++**: 使用C++17/20特性，包括智能指针、lambda函数、移动语义
- **多线程**: 异步处理客户端连接，支持并发请求
- **路由系统**: 简单灵活的HTTP路由注册和处理
- **跨平台**: 支持Linux和其他Unix系统
- **轻量级**: 不依赖第三方库，仅使用标准库和系统API

## 🛠️ 构建要求

- C++17或更高版本的编译器 (GCC 7+, Clang 5+)
- CMake 3.16+ 或 Make
- POSIX兼容系统 (Linux, macOS, Unix)

## 🚀 快速开始

### 使用Makefile构建

```bash
# 构建项目
make

# 运行服务器
make run

# 或者分步执行
./http_server
```

### 使用CMake构建

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 构建
make

# 运行
./bin/ModernCppHttpServer
```

## 📖 使用方法

### 基本用法

```cpp
#include "http_server.hpp"

int main() {
    // 创建服务器实例（默认端口8080）
    http::HttpServer server(8080);
    
    // 注册GET路由
    server.register_handler("GET", "/hello", [](const http::Request& req) {
        http::Response response;
        response.body = "<h1>Hello, World!</h1>";
        return response;
    });
    
    // 启动服务器
    server.start();
    
    // 保持运行
    while (server.is_running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    return 0;
}
```

### 高级功能

```cpp
// JSON API端点
server.register_handler("GET", "/api/data", [](const http::Request& req) {
    http::Response response;
    response.headers["Content-Type"] = "application/json";
    response.body = R"({"message": "Hello from API!"})";
    return response;
});

// 处理POST请求
server.register_handler("POST", "/submit", [](const http::Request& req) {
    http::Response response;
    // 处理请求体数据
    std::cout << "收到数据: " << req.body << std::endl;
    response.body = "<h1>数据已接收</h1>";
    return response;
});
```

## 🌐 默认路由

服务器启动后，访问以下URL查看示例页面：

- `http://localhost:8080/` - 主页
- `http://localhost:8080/hello` - 问候页面  
- `http://localhost:8080/json` - JSON API示例
- `http://localhost:8080/info` - 服务器信息

## 🏗️ 项目结构

```
├── http_server.hpp     # HTTP服务器类声明
├── http_server.cpp     # HTTP服务器实现
├── main.cpp           # 示例程序入口
├── CMakeLists.txt     # CMake构建配置
├── Makefile          # Make构建配置
└── README.md         # 项目说明
```

## 🔧 API参考

### HttpServer类

```cpp
class HttpServer {
public:
    explicit HttpServer(int port = 8080);
    
    // 注册路由处理器
    void register_handler(const std::string& method, 
                         const std::string& path, 
                         RequestHandler handler);
    
    void start();    // 启动服务器
    void stop();     // 停止服务器
    bool is_running() const;  // 检查运行状态
};
```

### Request结构

```cpp
struct Request {
    std::string method;      // HTTP方法 (GET, POST等)
    std::string path;        // 请求路径
    std::string version;     // HTTP版本
    std::unordered_map<std::string, std::string> headers;  // 请求头
    std::string body;        // 请求体
};
```

### Response结构

```cpp
struct Response {
    int status_code = 200;           // HTTP状态码
    std::string status_text = "OK";  // 状态文本
    std::unordered_map<std::string, std::string> headers;  // 响应头
    std::string body;                // 响应体
};
```

## 🛡️ 注意事项

- 这是一个简化的HTTP服务器实现，主要用于学习和演示
- 不建议在生产环境中使用，缺少安全性和错误处理机制
- 仅支持HTTP/1.1协议
- 目前不支持HTTPS/TLS加密

## 📝 许可证

本项目采用MIT许可证，详见LICENSE文件。

## 🤝 贡献

欢迎提交Issue和Pull Request来改进这个项目！