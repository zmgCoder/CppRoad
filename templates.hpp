#pragma once
#include <string>
#include <ctime>

namespace templates {
    
    /**
     * 生成主页HTML
     */
    inline std::string get_home_page() {
        return R"(
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
    }
    
    /**
     * 生成问候页面HTML
     */
    inline std::string get_hello_page() {
        return R"(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <title>问候页面</title>
    <style>
        body { 
            font-family: Arial, sans-serif; 
            text-align: center; 
            margin-top: 100px;
            background: linear-gradient(45deg, #ff6b6b, #4ecdc4);
            color: white;
            min-height: 100vh;
            padding: 20px;
            box-sizing: border-box;
        }
        .greeting { 
            font-size: 2.5em; 
            margin-bottom: 30px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        .back-link {
            display: inline-block;
            padding: 10px 20px;
            background: rgba(255,255,255,0.2);
            color: white;
            text-decoration: none;
            border-radius: 25px;
            transition: all 0.3s ease;
        }
        .back-link:hover {
            background: rgba(255,255,255,0.3);
            transform: translateY(-2px);
        }
    </style>
</head>
<body>
    <div class="greeting">👋 你好！欢迎使用现代C++ HTTP服务器</div>
    <a href="/" class="back-link">← 返回首页</a>
</body>
</html>
        )";
    }
    
    /**
     * 生成服务器信息页面HTML
     */
    inline std::string get_info_page() {
        return R"(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <title>服务器信息</title>
    <style>
        body { 
            font-family: 'Courier New', monospace; 
            background: #1e1e1e; 
            color: #d4d4d4; 
            padding: 20px;
            margin: 0;
            min-height: 100vh;
        }
        .info { 
            background: #2d2d30; 
            padding: 30px; 
            border-radius: 8px;
            max-width: 600px;
            margin: 50px auto;
            box-shadow: 0 4px 6px rgba(0,0,0,0.3);
        }
        .header { 
            color: #569cd6; 
            font-weight: bold; 
        }
        .info-item {
            margin: 15px 0;
            padding: 10px;
            border-left: 3px solid #569cd6;
            background: rgba(86, 156, 214, 0.1);
        }
        .back-link {
            display: inline-block;
            color: #4fc3f7;
            text-decoration: none;
            padding: 10px 15px;
            background: rgba(79, 195, 247, 0.1);
            border-radius: 5px;
            transition: all 0.3s ease;
        }
        .back-link:hover {
            background: rgba(79, 195, 247, 0.2);
        }
    </style>
</head>
<body>
    <div class="info">
        <h2 class="header">📊 服务器信息</h2>
        <div class="info-item">
            <span class="header">服务器:</span> 现代C++ HTTP服务器 v1.0
        </div>
        <div class="info-item">
            <span class="header">协议:</span> HTTP/1.1
        </div>
        <div class="info-item">
            <span class="header">端口:</span> 8080
        </div>
        <div class="info-item">
            <span class="header">编程语言:</span> C++17/20
        </div>
        <div class="info-item">
            <span class="header">架构:</span> 多线程异步处理
        </div>
        <div style="text-align: center; margin-top: 30px;">
            <a href="/" class="back-link">← 返回首页</a>
        </div>
    </div>
</body>
</html>
        )";
    }
    
    /**
     * 生成JSON API响应
     */
    inline std::string get_json_response() {
        return R"({
    "message": "Hello from Modern C++ HTTP Server!",
    "timestamp": ")" + std::to_string(std::time(nullptr)) + R"(",
    "server": {
        "name": "Modern C++ HTTP Server",
        "version": "1.0",
        "language": "C++17/20",
        "architecture": "Multi-threaded"
    },
    "features": [
        "Modern C++17/20",
        "Multi-threading",
        "Smart pointers",
        "Lambda functions",
        "RAII",
        "Template-based routing"
    ],
    "status": "running",
    "endpoints": [
        "/",
        "/hello", 
        "/json",
        "/info"
    ]
})";
    }
    
} // namespace templates