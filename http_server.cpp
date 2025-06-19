#include "http_server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdexcept>

namespace http {

std::string Response::to_string() const {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";
    
    // 添加默认头部
    auto headers_copy = headers;
    if (headers_copy.find("Content-Length") == headers_copy.end()) {
        headers_copy["Content-Length"] = std::to_string(body.length());
    }
    if (headers_copy.find("Content-Type") == headers_copy.end()) {
        headers_copy["Content-Type"] = "text/html; charset=utf-8";
    }
    headers_copy["Connection"] = "close";
    
    for (const auto& [key, value] : headers_copy) {
        oss << key << ": " << value << "\r\n";
    }
    
    oss << "\r\n" << body;
    return oss.str();
}

HttpServer::HttpServer(int port) : port_(port), server_socket_(-1) {
    setup_socket();
}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::setup_socket() {
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ < 0) {
        throw std::runtime_error("无法创建socket");
    }
    
    // 设置socket选项，允许重用地址
    int opt = 1;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(server_socket_);
        throw std::runtime_error("无法设置socket选项");
    }
    
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);
    
    if (bind(server_socket_, (struct sockaddr*)&address, sizeof(address)) < 0) {
        close(server_socket_);
        throw std::runtime_error("无法绑定端口 " + std::to_string(port_));
    }
    
    if (listen(server_socket_, 10) < 0) {
        close(server_socket_);
        throw std::runtime_error("无法监听端口");
    }
}

void HttpServer::register_handler(const std::string& method, const std::string& path, RequestHandler handler) {
    std::string key = create_handler_key(method, path);
    handlers_[key] = std::move(handler);
}

void HttpServer::start() {
    if (running_.load()) {
        return;
    }
    
    running_.store(true);
    std::cout << "HTTP服务器启动在端口 " << port_ << std::endl;
    
    // 启动连接接受线程
    worker_threads_.emplace_back([this]() { accept_connections(); });
}

void HttpServer::stop() {
    if (!running_.load()) {
        return;
    }
    
    running_.store(false);
    
    if (server_socket_ >= 0) {
        close(server_socket_);
        server_socket_ = -1;
    }
    
    // 等待所有工作线程结束
    for (auto& thread : worker_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    worker_threads_.clear();
    
    std::cout << "HTTP服务器已停止" << std::endl;
}

void HttpServer::accept_connections() {
    while (running_.load()) {
        sockaddr_in client_address{};
        socklen_t client_len = sizeof(client_address);
        
        int client_socket = accept(server_socket_, (struct sockaddr*)&client_address, &client_len);
        
        if (client_socket < 0) {
            if (running_.load()) {
                std::cerr << "接受连接时出错" << std::endl;
            }
            continue;
        }
        
        // 为每个客户端创建一个处理线程
        worker_threads_.emplace_back([this, client_socket]() {
            handle_client(client_socket);
        });
    }
}

void HttpServer::handle_client(int client_socket) {
    char buffer[4096];
    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        std::string raw_request(buffer);
        
        try {
            Request request = parse_request(raw_request);
            Response response = handle_request(request);
            
            std::string response_str = response.to_string();
            send(client_socket, response_str.c_str(), response_str.length(), 0);
        } catch (const std::exception& e) {
            Response error_response;
            error_response.status_code = 500;
            error_response.status_text = "Internal Server Error";
            error_response.body = "<h1>500 Internal Server Error</h1><p>" + std::string(e.what()) + "</p>";
            
            std::string response_str = error_response.to_string();
            send(client_socket, response_str.c_str(), response_str.length(), 0);
        }
    }
    
    close(client_socket);
}

Request HttpServer::parse_request(const std::string& raw_request) {
    Request request;
    std::istringstream iss(raw_request);
    std::string line;
    
    // 解析请求行
    if (std::getline(iss, line)) {
        std::istringstream request_line(line);
        request_line >> request.method >> request.path >> request.version;
    }
    
    // 解析头部
    while (std::getline(iss, line) && line != "\r") {
        if (line.back() == '\r') {
            line.pop_back();
        }
        
        auto colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            
            // 去除前后空格
            key.erase(0, key.find_first_not_of(' '));
            key.erase(key.find_last_not_of(' ') + 1);
            value.erase(0, value.find_first_not_of(' '));
            value.erase(value.find_last_not_of(' ') + 1);
            
            request.headers[key] = value;
        }
    }
    
    // 解析请求体
    std::string remaining_content;
    std::string body_line;
    while (std::getline(iss, body_line)) {
        remaining_content += body_line + "\n";
    }
    request.body = remaining_content;
    
    return request;
}

Response HttpServer::handle_request(const Request& request) {
    std::string handler_key = create_handler_key(request.method, request.path);
    
    auto it = handlers_.find(handler_key);
    if (it != handlers_.end()) {
        return it->second(request);
    }
    
    // 默认404响应
    Response response;
    response.status_code = 404;
    response.status_text = "Not Found";
    response.body = "<h1>404 Not Found</h1><p>请求的资源未找到: " + request.path + "</p>";
    
    return response;
}

std::string HttpServer::create_handler_key(const std::string& method, const std::string& path) {
    return method + " " + path;
}

} // namespace http