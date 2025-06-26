#include "http_server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <chrono>
#include <cstring>

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
    
    // 设置socket为非阻塞模式
    int flags = fcntl(server_socket_, F_GETFL, 0);
    if (flags == -1) {
        close(server_socket_);
        throw std::runtime_error("无法获取socket标志");
    }
    if (fcntl(server_socket_, F_SETFL, flags | O_NONBLOCK) == -1) {
        close(server_socket_);
        throw std::runtime_error("无法设置socket为非阻塞模式");
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
    
    std::cout << "正在停止HTTP服务器..." << std::endl;
    running_.store(false);
    
    // 关闭服务器socket以中断accept()调用
    if (server_socket_ >= 0) {
        shutdown(server_socket_, SHUT_RDWR);
        close(server_socket_);
        server_socket_ = -1;
    }
    
    // 等待accept线程结束（通常是第一个线程）
    if (!worker_threads_.empty() && worker_threads_[0].joinable()) {
        worker_threads_[0].join();
    }
    
    // 给其他客户端处理线程一些时间来完成
    std::cout << "等待客户端连接处理完成..." << std::endl;
    auto start_time = std::chrono::steady_clock::now();
    
    for (size_t i = 1; i < worker_threads_.size(); ++i) {
        if (worker_threads_[i].joinable()) {
            // 设置超时时间为5秒
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
            
            if (elapsed < 5) {
                worker_threads_[i].join();
            } else {
                std::cout << "超时等待线程完成，强制分离线程" << std::endl;
                worker_threads_[i].detach();
                break;
            }
        }
    }
    
    worker_threads_.clear();
    std::cout << "HTTP服务器已停止" << std::endl;
}

void HttpServer::accept_connections() {
    while (running_.load()) {
        // 使用select来检查是否有新连接，避免无限期阻塞
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(server_socket_, &read_fds);
        
        struct timeval timeout;
        timeout.tv_sec = 1;  // 1秒超时
        timeout.tv_usec = 0;
        
        int activity = select(server_socket_ + 1, &read_fds, nullptr, nullptr, &timeout);
        
        if (activity < 0) {
            if (running_.load()) {
                std::cerr << "select()调用出错: " << strerror(errno) << std::endl;
            }
            break;
        } else if (activity == 0) {
            // 超时，继续循环检查running_状态
            continue;
        }
        
        if (!running_.load()) {
            break;
        }
        
        if (FD_ISSET(server_socket_, &read_fds)) {
            sockaddr_in client_address{};
            socklen_t client_len = sizeof(client_address);
            
            int client_socket = accept(server_socket_, (struct sockaddr*)&client_address, &client_len);
            
            if (client_socket < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK && running_.load()) {
                    std::cerr << "接受连接时出错: " << strerror(errno) << std::endl;
                }
                continue;
            }
            
            // 清理已完成的线程
            cleanup_completed_threads();
            
            // 为每个客户端创建一个处理线程
            worker_threads_.emplace_back([this, client_socket]() {
                handle_client(client_socket);
            });
        }
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

void HttpServer::cleanup_completed_threads() {
    // 为了简化，我们限制最大并发线程数
    // 如果线程太多，我们让它们自然完成
    if (worker_threads_.size() > 100) {
        std::cout << "警告: 活跃线程数过多 (" << worker_threads_.size() << ")，建议检查连接处理" << std::endl;
    }
}

std::string HttpServer::create_handler_key(const std::string& method, const std::string& path) {
    return method + " " + path;
}

} // namespace http