#include "server_manager.hpp"
#include <iostream>
#include <exception>

/**
 * 现代C++ HTTP服务器
 * 重构后的主程序 - 简洁、清晰、易维护
 */
int main() {
    try {
        // 创建服务器管理器
        server::ServerManager manager(8080);
        
        // 初始化服务器
        manager.initialize();
        
        // 启动服务器
        manager.start();
        
        // 运行服务器主循环
        manager.run();
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 服务器运行出错: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ 未知错误发生" << std::endl;
        return 1;
    }
    
    return 0;
}