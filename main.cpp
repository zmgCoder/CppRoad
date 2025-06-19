#include "http_server.hpp"
#include <iostream>
#include <csignal>
#include <memory>

std::unique_ptr<http::HttpServer> server;

void signal_handler(int signal) {
    std::cout << "\n收到停止信号，正在关闭服务器..." << std::endl;
    if (server) {
        server->stop();
    }
    exit(0);
}

int main() {
    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    try {
        // 创建服务器实例
        server = std::make_unique<http::HttpServer>(8080);
        
        // 注册路由处理器
        server->register_handler("GET", "/", [](const http::Request& req) -> http::Response {
            http::Response response;
            response.body = R"(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>现代C++ HTTP服务器</title>
    <style>
        body { 
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            max-width: 800px; 
            margin: 50px auto; 
            padding: 20px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
        }
        .container {
            background: rgba(255, 255, 255, 0.1);
            padding: 30px;
            border-radius: 15px;
            backdrop-filter: blur(10px);
            box-shadow: 0 8px 32px 0 rgba(31, 38, 135, 0.37);
        }
        h1 { text-align: center; margin-bottom: 30px; }
        .feature { 
            margin: 15px 0; 
            padding: 15px;
            background: rgba(255, 255, 255, 0.1);
            border-radius: 8px;
        }
        .links { text-align: center; margin-top: 30px; }
        .links a { 
            color: #fff; 
            text-decoration: none; 
            margin: 0 15px;
            padding: 10px 20px;
            background: rgba(255, 255, 255, 0.2);
            border-radius: 25px;
            transition: all 0.3s ease;
        }
        .links a:hover {
            background: rgba(255, 255, 255, 0.3);
            transform: translateY(-2px);
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🚀 现代C++ HTTP服务器</h1>
        <div class="feature">
            <strong>✨ 现代C++特性:</strong> 使用智能指针、lambda函数、移动语义
        </div>
        <div class="feature">
            <strong>🔧 多线程支持:</strong> 异步处理客户端连接
        </div>
        <div class="feature">
            <strong>🛣️ 路由系统:</strong> 灵活的请求处理器注册
        </div>
        <div class="feature">
            <strong>⚡ 高性能:</strong> 基于socket的高效网络通信
        </div>
        
        <div class="links">
            <a href="/hello">问候页面</a>
            <a href="/json">JSON API</a>
            <a href="/info">服务器信息</a>
        </div>
    </div>
</body>
</html>
            )";
            return response;
        });
        
        server->register_handler("GET", "/hello", [](const http::Request& req) -> http::Response {
            http::Response response;
            response.body = R"(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <title>问候页面</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; margin-top: 100px; }
        .greeting { font-size: 2em; color: #4CAF50; }
    </style>
</head>
<body>
    <div class="greeting">👋 你好！欢迎使用现代C++ HTTP服务器</div>
    <p><a href="/">返回首页</a></p>
</body>
</html>
            )";
            return response;
        });
        
        server->register_handler("GET", "/json", [](const http::Request& req) -> http::Response {
            http::Response response;
            response.headers["Content-Type"] = "application/json; charset=utf-8";
            response.body = R"({
    "message": "Hello from Modern C++ HTTP Server!",
    "timestamp": ")" + std::to_string(time(nullptr)) + R"(",
    "features": [
        "Modern C++17/20",
        "Multi-threading",
        "Smart pointers",
        "Lambda functions",
        "RAII"
    ],
    "status": "running"
})";
            return response;
        });
        
        server->register_handler("GET", "/info", [](const http::Request& req) -> http::Response {
            http::Response response;
            response.body = R"(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <title>服务器信息</title>
    <style>
        body { 
            font-family: monospace; 
            background: #1e1e1e; 
            color: #d4d4d4; 
            padding: 20px; 
        }
        .info { background: #2d2d30; padding: 20px; border-radius: 8px; }
        .header { color: #569cd6; font-weight: bold; }
    </style>
</head>
<body>
    <div class="info">
        <h2 class="header">📊 服务器信息</h2>
        <p><span class="header">服务器:</span> 现代C++ HTTP服务器 v1.0</p>
        <p><span class="header">协议:</span> HTTP/1.1</p>
        <p><span class="header">端口:</span> 8080</p>
        <p><span class="header">编程语言:</span> C++17/20</p>
        <p><span class="header">架构:</span> 多线程异步处理</p>
        <p><a href="/" style="color: #4fc3f7;">← 返回首页</a></p>
    </div>
</body>
</html>
            )";
            return response;
        });
        
        // 启动服务器
        server->start();
        
        std::cout << "服务器正在运行... 按 Ctrl+C 停止" << std::endl;
        std::cout << "访问: http://localhost:8080" << std::endl;
        
        // 保持主线程运行
        while (server->is_running()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}