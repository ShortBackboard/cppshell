#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>

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

        // 处理cd命令
        if(input.substr(0, 2) == "cd")
        {
            std::string path = input.substr(3); // 获取 cd 后面的路径参数
            int ret = chdir(path.c_str()); // 调用系统的 chdir 函数

            if (ret == 0) {
                std::cout << "Changed directory to " << path << std::endl;
            } else {
                std::cerr << "Failed to change directory to " << path << std::endl;
            }

            continue;
        }

        // 处理mkdir命令
        if (input.substr(0, 5) == "mkdir") {
            std::string dir_name = input.substr(6); // 获取 mkdir 后面的目录名参数
            int ret = mkdir(dir_name.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); // 创建目录
            
            if (ret == 0) {
                std::cout << "Created directory: " << dir_name << std::endl;
            } else {
                std::cerr << "Failed to create directory: " << dir_name << std::endl;
            }
            
            continue; // 继续下一次循环
        }

        // 解析命令和参数
        std::vector<std::string> commands;
        std::vector<std::vector<std::string>> args_list;

        size_t pipe_pos = input.find('|');
        if (pipe_pos != std::string::npos) {
            std::string first_command = input.substr(0, pipe_pos);
            std::string second_command = input.substr(pipe_pos + 1);
            commands.push_back(first_command);
            commands.push_back(second_command);
        } else {
            commands.push_back(input);
        }

        for (const auto& command : commands) {
            args_list.push_back(split(command, ' '));
        }

        int pipes[2];
        int fd_in = 0;

        for (size_t i = 0; i < commands.size(); ++i) {
            pipe(pipes);
            pid_t pid = fork();

            if (pid == -1) {  // 出错处理
                std::cerr << "Fork failed\n";
                return 1;
            } else if (pid == 0) {  // 子进程中
                dup2(fd_in, 0);  // 将前一个命令的输出作为当前命令的输入

                if (i < commands.size() - 1) {
                    dup2(pipes[1], 1);  // 将当前命令的输出连接到管道写入端
                }

                close(pipes[0]);  // 关闭管道的读取端

                // 处理I/O重定向
                size_t redirect_in = args_list[i].size();
                size_t redirect_out = args_list[i].size();

                for(size_t j = 0; j < args_list[i].size(); ++j) {
                    if (args_list[i][j] == "<") {
                        redirect_in = j;
                    } else if (args_list[i][j] == ">") {
                        redirect_out = j;
                    }
                }

                if (redirect_in < args_list[i].size()) {
                    int in_fd = open(args_list[i][redirect_in + 1].c_str(), O_RDONLY);
                    if (in_fd == -1) {
                        std::cerr << "Failed to open file for input redirection\n";
                        return 1;
                    }
                    dup2(in_fd, 0);
                    close(in_fd);
                    args_list[i].erase(args_list[i].begin() + redirect_in, args_list[i].begin() + redirect_in + 2);
                }

                if (redirect_out < args_list[i].size()) {
                    int out_fd = open(args_list[i][redirect_out + 1].c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
                    if (out_fd == -1) {
                        std::cerr << "Failed to open file for output redirection\n";
                        return 1;
                    }
                    dup2(out_fd, 1);
                    close(out_fd);
                    args_list[i].erase(args_list[i].begin() + redirect_out, args_list[i].begin() + redirect_out + 2);
                }

                // 将args转换成char*数组，以便于execvp使用
                std::vector<char*> c_args;
                for (const auto& arg : args_list[i]) {
                    c_args.push_back(const_cast<char*>(arg.c_str()));
                }
                c_args.push_back(nullptr);  // execvp需要一个以nullptr结尾的char*数组

                // execvp(c_args[0], c_args.data());
                if (system(c_args[0]) != 0) {
                  std::cerr << "Failed to execute command\n";
                  return 1;
                } 

            } else {  // 父进程中
                waitpid(pid, nullptr, 0);
                close(pipes[1]);  // 关闭管道的写入端
                fd_in = pipes[0];  // 当前命令的输出作为下一个命令的输入
            }
        }
    }

    return 0;
}
