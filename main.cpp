#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>

// 分割字符串函数
std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

int main() {
    std::string input;
    
    while (true) {
        std::cout << "cppshell> ";
        std::getline(std::cin, input);

        if (input == "exit") {
            break;
        }

        // 解析命令和参数
        std::vector<std::string> args = split(input, ' ');
        
        // 将args转换成char*数组，以便于execvp使用
        std::vector<char*> c_args;
        for (const auto& arg : args) {
            c_args.push_back(const_cast<char*>(arg.c_str()));
        }
        c_args.push_back(nullptr);  // execvp需要一个以nullptr结尾的char*数组

        pid_t pid = fork();

        if (pid == 0) {  // 子进程中
            execvp(c_args[0], c_args.data());
            std::cerr << "Failed to execute command\n";
            return 1;
        } else if (pid > 0) {  // 父进程中
            waitpid(pid, nullptr, 0);
        } else {  // 出错
            std::cerr << "Fork failed\n";
            return 1;
        }
    }

    return 0;
}