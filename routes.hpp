#pragma once
#include "http_server.hpp"
#include "templates.hpp"

namespace routes {
    
    /**
     * 路由配置管理器
     * 负责注册所有的路由处理器
     */
    class RouteManager {
    public:
        /**
         * 配置所有路由
         * @param server HTTP服务器实例
         */
        static void configure_routes(http::HttpServer& server) {
            register_home_route(server);
            register_hello_route(server);
            register_json_route(server);
            register_info_route(server);
        }
        
    private:
        /**
         * 注册主页路由
         */
        static void register_home_route(http::HttpServer& server) {
            server.register_handler("GET", "/", [](const http::Request& req) -> http::Response {
                http::Response response;
                response.headers["Content-Type"] = "text/html; charset=utf-8";
                response.body = templates::get_home_page();
                return response;
            });
        }
        
        /**
         * 注册问候页面路由
         */
        static void register_hello_route(http::HttpServer& server) {
            server.register_handler("GET", "/hello", [](const http::Request& req) -> http::Response {
                http::Response response;
                response.headers["Content-Type"] = "text/html; charset=utf-8";
                response.body = templates::get_hello_page();
                return response;
            });
        }
        
        /**
         * 注册JSON API路由
         */
        static void register_json_route(http::HttpServer& server) {
            server.register_handler("GET", "/json", [](const http::Request& req) -> http::Response {
                http::Response response;
                response.headers["Content-Type"] = "application/json; charset=utf-8";
                response.body = templates::get_json_response();
                return response;
            });
        }
        
        /**
         * 注册服务器信息路由
         */
        static void register_info_route(http::HttpServer& server) {
            server.register_handler("GET", "/info", [](const http::Request& req) -> http::Response {
                http::Response response;
                response.headers["Content-Type"] = "text/html; charset=utf-8";
                response.body = templates::get_info_page();
                return response;
            });
        }
    };
    
} // namespace routes