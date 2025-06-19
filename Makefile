# 现代C++ HTTP服务器 Makefile

# 编译器设置
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread
LDFLAGS = -pthread

# 目标文件名
TARGET = http_server

# 源文件
SOURCES = main.cpp http_server.cpp
HEADERS = http_server.hpp

# 对象文件
OBJECTS = $(SOURCES:.cpp=.o)

# 默认目标
all: $(TARGET)

# 构建目标
$(TARGET): $(OBJECTS)
	@echo "🔗 链接 $(TARGET)..."
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "✅ 构建完成!"

# 编译对象文件
%.o: %.cpp $(HEADERS)
	@echo "🔨 编译 $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理
clean:
	@echo "🧹 清理构建文件..."
	rm -f $(OBJECTS) $(TARGET)
	@echo "✅ 清理完成!"

# 运行
run: $(TARGET)
	@echo "🚀 启动HTTP服务器..."
	./$(TARGET)

# 调试构建
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

# 安装依赖（Ubuntu/Debian）
install-deps:
	@echo "📦 安装构建依赖..."
	sudo apt-get update
	sudo apt-get install -y build-essential g++ cmake

# 帮助
help:
	@echo "可用的命令:"
	@echo "  make          - 构建项目"
	@echo "  make clean    - 清理构建文件"
	@echo "  make run      - 构建并运行服务器"
	@echo "  make debug    - 构建调试版本"
	@echo "  make install-deps - 安装构建依赖"

.PHONY: all clean run debug install-deps help