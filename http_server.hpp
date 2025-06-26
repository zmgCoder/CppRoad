#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <vector>
#include <thread>
#include <atomic>

namespace http {

struct Request {
    std::string method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

struct Response {
    int status_code = 200;
    std::string status_text = "OK";
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    
    std::string to_string() const;
};

using RequestHandler = std::function<Response(const Request&)>;

class HttpServer {
public:
    explicit HttpServer(int port = 8080);
    ~HttpServer();
    
    // 禁用拷贝构造和赋值
    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;
    
    // 注册路由处理器
    void register_handler(const std::string& method, const std::string& path, RequestHandler handler);
    
    // 启动服务器
    void start();
    
    // 停止服务器
    void stop();
    
    // 检查服务器是否在运行
    bool is_running() const { return running_.load(); }

private:
    int port_;
    int server_socket_;
    std::atomic<bool> running_{false};
    std::vector<std::thread> worker_threads_;
    
    std::unordered_map<std::string, RequestHandler> handlers_;
    
    void setup_socket();
    void accept_connections();
    void handle_client(int client_socket);
    void cleanup_completed_threads();
    Request parse_request(const std::string& raw_request);
    Response handle_request(const Request& request);
    std::string create_handler_key(const std::string& method, const std::string& path);
};

} // namespace http