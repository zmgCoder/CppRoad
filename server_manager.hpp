#pragma once
#include "http_server.hpp"
#include "routes.hpp"
#include <memory>
#include <iostream>
#include <csignal>
#include <thread>
#include <chrono>

namespace server {
    
    /**
     * 服务器管理器
     * 负责服务器的生命周期管理
     */
    class ServerManager {
    private:
        std::unique_ptr<http::HttpServer> server_;
        int port_;
        static ServerManager* instance_;
        
    public:
        /**
         * 构造函数
         * @param port 服务器端口，默认8080
         */
        explicit ServerManager(int port = 8080) : port_(port) {
            instance_ = this;
            setup_signal_handlers();
        }
        
        /**
         * 初始化服务器
         */
        void initialize() {
            try {
                server_ = std::make_unique<http::HttpServer>(port_);
                routes::RouteManager::configure_routes(*server_);
                std::cout << "✅ 服务器初始化完成" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "❌ 服务器初始化失败: " << e.what() << std::endl;
                throw;
            }
        }
        
        /**
         * 启动服务器
         */
        void start() {
            if (!server_) {
                throw std::runtime_error("服务器未初始化");
            }
            
            try {
                server_->start();
                print_startup_info();
            } catch (const std::exception& e) {
                std::cerr << "❌ 服务器启动失败: " << e.what() << std::endl;
                throw;
            }
        }
        
        /**
         * 运行服务器主循环
         */
        void run() {
            if (!server_) {
                throw std::runtime_error("服务器未启动");
            }
            
            while (server_->is_running()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        
        /**
         * 停止服务器
         */
        void stop() {
            if (server_) {
                std::cout << "\n🛑 正在关闭服务器..." << std::endl;
                server_->stop();
                std::cout << "✅ 服务器已关闭" << std::endl;
            }
        }
        
        /**
         * 检查服务器是否运行中
         */
        bool is_running() const {
            return server_ && server_->is_running();
        }
        
        /**
         * 获取服务器端口
         */
        int get_port() const {
            return port_;
        }
        
    private:
        /**
         * 设置信号处理器
         */
        void setup_signal_handlers() {
            signal(SIGINT, signal_handler);
            signal(SIGTERM, signal_handler);
        }
        
        /**
         * 信号处理函数
         */
        static void signal_handler(int signal) {
            std::cout << "\n🛑 收到退出信号 (" << signal << ")..." << std::endl;
            if (instance_) {
                instance_->stop();
            }
            // 不要直接调用exit()，让程序自然退出
        }
        
        /**
         * 打印启动信息
         */
        void print_startup_info() {
            std::cout << "\n" << std::string(50, '=') << std::endl;
            std::cout << "🚀 现代C++ HTTP服务器已启动！" << std::endl;
            std::cout << std::string(50, '=') << std::endl;
            std::cout << "📍 端口: " << port_ << std::endl;
            std::cout << "🌐 访问地址: http://localhost:" << port_ << std::endl;
            std::cout << "\n📋 可用路由:" << std::endl;
            std::cout << "  • http://localhost:" << port_ << "/ (主页)" << std::endl;
            std::cout << "  • http://localhost:" << port_ << "/hello (问候页面)" << std::endl;
            std::cout << "  • http://localhost:" << port_ << "/json (JSON API)" << std::endl;
            std::cout << "  • http://localhost:" << port_ << "/info (服务器信息)" << std::endl;
            std::cout << "\n⚡ 按 Ctrl+C 停止服务器" << std::endl;
            std::cout << std::string(50, '=') << std::endl;
        }
    };
    
    // 静态成员初始化
    ServerManager* ServerManager::instance_ = nullptr;
    
} // namespace server