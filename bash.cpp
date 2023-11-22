#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <iostream>
#include <vector>
#include <sstream>

void execute_command(const std::vector<std::string>& arg_list, const std::vector<std::string>& history) {
    if (arg_list.empty()) {
        std::cerr << "Error: no command" << std::endl;
        return;
    }

    std::string command = arg_list[0];

    if (command == "cd") {
	    if (arg_list.size() == 1) {
		return;
	    } else if (arg_list[1] == ".") {
		return;
	    } else if (arg_list[1] == "..") {
                chdir("..");
		return;
            } else if (arg_list[1][0] == '/') {
		    if (chdir(arg_list[1].c_str()) != 0) {
                	std::cerr << "Error changing directory: No such file or directory" << std::endl;
		    }
            } else if (chdir(arg_list[1].c_str()) != 0) {
                std::cerr << "Error changing directory" << std::endl;
            }
        
    } else if (command == "help") {
        std::cout << "Available commands:\n";
        std::cout << "  cd             : Change the current working directory" << std::endl;
        std::cout << "  ls             : List files in the current directory" << std::endl;
        std::cout << "  pwd            : Print the current working directory" << std::endl;
        std::cout << "  help           : Display help message" << std::endl;
    } else if (command == "history") {
	std::cout << "The Bash command history" << std::endl;
	for (const auto& com : history) {
	    std::cout << com << std::endl;
	}
    } else if (command == "ls" || command == "pwd" || command == "echo" || command == "g++" || command == "./a.out" || command == "grep") {
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
   // else {
   //     std::cerr << "Error: Unknown command\n";
   // }
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

        std::vector<std::string> tokens;
        std::istringstream iss(command);
        std::string token;

        while (iss >> token) {
            tokens.push_back(token);
        }

	execute_command(tokens, history);

    }

    return 0;
}
