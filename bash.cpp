#include "bash.h"
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h> 

#include <iostream>
#include <algorithm>
#include <functional>
#include <sstream>
#include <filesystem>


void Bash::execute() {
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
}

void Bash::execute_command(std::vector<std::string>& arg_list) {
    if (arg_list.empty()) {
        std::cerr << "Error: no command" << std::endl;
        return;
    }

	bool pipe_exist = false;
    pipe_existence_check(arg_list, pipe_exist);

    if (pipe_exist) {
        return;
    }

    std::vector<std::pair<std::string, std::string>> output_redirections; 
    std::vector<std::pair<std::string, std::string>> input_redirections; 
    redirections(arg_list, output_redirections, input_redirections);       

    std::string command = arg_list[0];

    // converting array of strings to array of char* 's , because, execvp get as argument array of char* 's 
    char* arr[arg_list.size()];
    for (int i = 0; i < arg_list.size(); ++i) {
        arr[i] = strdup(arg_list[i].c_str()); // aranc strdup - i chi toxnum const char* -> char*
    }

    arr[arg_list.size()] = nullptr;

	//fork a child process
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // child process
	    for (const auto& redirection : input_redirections) {
           	// opening file for reading and duplicateing file descriptor to stdin
            int fd = open(redirection.first.c_str(), O_RDONLY);
            if (fd == -1) {
                std::cerr << "Error opening input file" << std::endl;
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
         }

        for (const auto &redirection : output_redirections) {
            int flags = O_WRONLY | O_CREAT;
            if (redirection.second == ">>") {
                flags |= O_APPEND; // verjic avelacnelu hamar flag
            } else {
                flags |= O_TRUNC; // flag overwrite anelu hamar
            }

            // open the file for writing and duplicate the file descriptor to stdout
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

void Bash::pipe_existence_check(const std::vector<std::string>& arg_list, bool& found_pipe) {
    std::vector<std::string> result;
	std::string current_string;

    for (const std::string& str : arg_list) {
        if (str == "|") {
            found_pipe = true;
            if (!current_string.empty()) {
                result.push_back(current_string);
                current_string.clear();
            }
        } else {
   	    current_string += " ";
            current_string += str;
        }
    }	

    if (found_pipe && !current_string.empty()) {
        result.push_back(current_string);
	handle_pipe(result);
    }
}

void Bash::redirections(std::vector<std::string>& arg_list, std::vector<std::pair<std::string, std::string>>& output_redirections, std::vector<std::pair<std::string, std::string>>& input_redirections) {
    for (size_t i = 0; i < arg_list.size(); ++i) {
        if (arg_list[i] == ">") {
            output_redirections.push_back({arg_list[i + 1], ">"});
            arg_list.erase(arg_list.begin() + i, arg_list.begin() + i + 2); // kjnji arg_list[i]-n u ira hajordy- arg_list[i + 1] - y , for example: ls > f1 > f2 => ls > f2
            --i; // erase - ic heto index - y -- anenq
        } else if (arg_list[i] == ">>") {
            output_redirections.push_back({arg_list[i + 1], ">>"});
            arg_list.erase(arg_list.begin() + i, arg_list.begin() + i + 2);
            --i; 
        } else if (arg_list[i] == "<") {
            input_redirections.push_back({arg_list[i + 1], "<"});
            arg_list.erase(arg_list.begin() + i, arg_list.begin() + i + 2);
            --i; 
        } 
    }
} 

void Bash::handle_pipe(std::vector<std::string>& commands) {
    int size = commands.size();
    int pipes[size - 1][2];

    for (int i = 0; i < size - 1; ++i) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return;
        }
    } 

    for (int i = 0; i < size; ++i) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            return;
        } else if (pid == 0) {
            // Child process
            // Connect input/output to pipes if not the first/last command
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO); // vor standart inputy current processi kardacvi pipei reading endic(0) (pipe-i vory vor asocacvuma naxord hramani het) : current processi standart inputy copya linum naxorrd hramani het assocacvox pipeic(reading endic) 
                close(pipes[i - 1][0]);
            }

            if (i < size - 1) {
                dup2(pipes[i][1], STDOUT_FILENO); // vor standart outputy current processi write arvi write endum(1) pipe-i vory assocacvuma hajord hramani het
                close(pipes[i][1]);
            }

            // close all other pipes
            for (int j = 0; j < size - 1; ++j) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // execute the command
            const char* command = commands[i].c_str();
            execlp("/bin/sh", "/bin/sh", "-c", command, nullptr); // stex "-c" nra hamara trvum vor vorpes hajord argument trvi commandy vory kara lini vorpes mi string pahvac orinak "grep a" => vor vorpes mi hraman haskacvi "-c" flagn enq talis
            //perror("execlp");
            return;
        }
    }

    // close all pipes in the parent process
    for (int i = 0; i < size - 1; ++i) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // wait for all child processes to finish
    for (int i = 0; i < size; ++i) {
        wait(nullptr);
    }
}

