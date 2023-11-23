#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <iostream>
#include <algorithm>
#include <functional>
#include <vector>
#include <sstream>

void execute_command(std::vector<std::string>& arg_list) {
    if (arg_list.empty()) {
        std::cerr << "Error: no command" << std::endl;
        return;
    }

    std::vector<std::pair<std::string, std::string>> output_redirections;

    for (size_t i = 0; i < arg_list.size(); ++i) {
        if (arg_list[i] == ">") {
            output_redirections.push_back({arg_list[i + 1], ">"});
            arg_list.erase(arg_list.begin() + i, arg_list.begin() + i + 2);
            --i; 
        } else if (arg_list[i] == ">>") {
            output_redirections.push_back({arg_list[i + 1], ">>"});
            arg_list.erase(arg_list.begin() + i, arg_list.begin() + i + 2);
            --i; 
        }
    }


    std::string command = arg_list[0];


    char* arr[arg_list.size()];
    for (int i = 0; i < arg_list.size(); ++i) {
        arr[i] = strdup(arg_list[i].c_str()); // aranc strdup - i chi toxnum const char* -> char*
    }

    arr[arg_list.size()] = NULL;
    //fork a child process
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // child process

	for (const auto& redirection : output_redirections) {
            int flags = O_WRONLY | O_CREAT;
            if (redirection.second == ">>") {
                flags |= O_APPEND;
            } else {
                flags |= O_TRUNC;
            }

            int fd = open(redirection.first.c_str(), flags, 0666);
            if (fd == -1) {
                std::cerr << "Error opening output file" << std::endl;
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }


        if (execvp(arr[0], arr) == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
	}
    } else {
        // parent process
        int status;
       	waitpid(pid, &status, 0);
    }
}



int main() {
    std::vector<std::string> history;
    while (true) {
        std::cout << "Bash clone: ";

	std::string command;
        std::getline(std::cin, command);
	history.push_back(command);


	if (command == "exit") {
            break;
        } 

        std::vector<std::string> arg_list;
        std::istringstream iss(command);
        std::string token;

        while (iss >> token) {
            arg_list.push_back(token);
        }


	std::string cmd = arg_list[0];

    	if (cmd == "cd") {
            if (arg_list.size() == 1) {
                continue;
            } else if (arg_list[1] == ".") {
                continue;
            } else if (arg_list[1] == "..") {
                chdir("..");
                continue;
            } else if (arg_list[1][0] == '/') {
                    if (chdir(arg_list[1].c_str()) != 0) {
                        std::cerr << "Error changing directory: No such file or directory" << std::endl;
                    }
            } else if (chdir(arg_list[1].c_str()) != 0) {
                std::cerr << "Error changing directory" << std::endl;
            }

    	} else if (cmd == "help") {
            std::cout << "Available commands:\n";
            std::cout << "  cd             : Change the current working directory" << std::endl;
            std::cout << "  ls             : List files in the current directory" << std::endl;
            std::cout << "  pwd            : Print the current working directory" << std::endl;
            std::cout << "  help           : Display help message" << std::endl;
        } else if (cmd == "history") {
            std::cout << "The Bash command history" << std::endl;
            for (const auto& com : history) {
                std::cout << com << std::endl;
            }
        } else if (cmd == "ls" || cmd == "pwd" || cmd == "echo" || cmd == "g++" || cmd == "./a.out") {
	    execute_command(arg_list);
	}

    }

    return 0;
}
