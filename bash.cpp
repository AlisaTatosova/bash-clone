#include "bash.h"

#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h> 
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>

#include <iostream>
#include <sstream>
#include <fstream>

Bash::Bash() {
    /* The rl_attempted_completion_function is a variable used in readline/readline.h.
        By setting rl_attempted_completion_function we are specifying function that Readline should call when presses the Tab key for tab-completion.
        signature: char** function_name(const char* text, int start, int end); -> text: The word that the user is trying to complete.*/ 
    rl_attempted_completion_function = custom_completion;
    using_history(); // is set up to maintain a history of commands entered by user
}

void Bash::execute() {
    while (char* command_c = readline("Bash clone: ")) {
        std::string command = command_c;;
        history.push_back(command); // saving history

	add_history(command_c); // adds command to history (add_history(char* command) - function from <readline/history.h> header)

        if (command == "exit") { 
            break; 
        }

        std::vector<std::string> arg_list;
        std::istringstream iss(command);
        std::string token;

        while (iss >> token) {
            arg_list.push_back(token);
        }

        std::string cmd = arg_list[0]; // separating command

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
            std::cout << "Available commands:" << std::endl;
            std::cout << "  cd             : Change the current working directory" << std::endl;
            std::cout << "  ls             : List files in the current directory" << std::endl;
            std::cout << "  pwd            : Print the current working directory" << std::endl;
	    std::cout << "  export         : Setting environment variable" << std::endl;
	    std::cout << "  history        : Printing history of commands" << std::endl;
	    std::cout << "  cat            : Printing containment of file" << std::endl;
	    std::cout << "  echo           : Printing environment variable value" << std::endl;
            std::cout << "  help           : Display help message" << std::endl;
	    std::cout << "  exit           : Exit shell" << std::endl;
        } else if (cmd == "history") {
            std::cout << "The Bash command history" << std::endl;
            for (const auto& com : history) {
                std::cout << com << std::endl;
            }
        } else if (cmd == "export") {
            std::string second_arg = arg_list[1];
            int pos = second_arg.find('=');

            if (pos != std::string::npos) {
            	std::string name = second_arg.substr(0, pos);
            	std::string val = second_arg.substr(pos + 1);
            	if (setenv(name.c_str(), val.c_str(), 1) != 0) {
                	perror("setenv");
            	}
            } else {
                std::cerr << "Invalid syntax: Usage: export VAR_NAME=VAR_VALUE" << std::endl;
            }
    	} else if (cmd == "echo") {
            std::string name = arg_list[1];
	    if (name[0] == '$') {  // if it is envorinment variable to print its value 
            	char* value = getenv((name.substr(1, name.size())).c_str());
		if (value != nullptr) {
                    std::cout << value << std::endl;
		} else {
		    std::cout << std::endl;    
		}
            } else {
	        std::cout << name << std::endl;
            }
        } else if (cmd == "ls" || cmd == "pwd" || cmd == "g++" || cmd == "./a.out" || cmd == "cat") {
            execute_command(arg_list);
	} else {
	    std::cout << "There is no such command" << std::endl;
	}
    }
}

// function for executing command
void Bash::execute_command(std::vector<std::string>& arg_list) {
    if (arg_list.empty()) {
        std::cerr << "Error: no command" << std::endl;
        return;
    }

    bool pipe_exist = false;
    pipe_existence_check(arg_list, pipe_exist); // checking for pipe existence
    if (pipe_exist) {
        return;
    }

    std::stringstream file_stream; // this is for << redirection
    bool is_double_redirection = false; // also for << redirection
    std::vector<std::pair<std::string, std::string>> output_redirections; 
    std::vector<std::pair<std::string, std::string>> input_redirections; 

    bool redirection_exist = false;
    redirections_check(arg_list, output_redirections, input_redirections, redirection_exist, is_double_redirection, file_stream); // checking for redirections     

    // converting array of strings to array of char* 's , because, execvp get as argument array of char* 's
    char* arr[arg_list.size()];
    for (int i = 0; i < arg_list.size(); ++i) {
        arr[i] = strdup(arg_list[i].c_str()); // aranc strdup - i chi toxnum const char* -> char*
    }
    arr[arg_list.size()] = nullptr;

    std::string command = arg_list[0];
    //fork a child process
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // child process
	
	if (redirection_exist) {
            handle_redirections(output_redirections, input_redirections);
        }

        if (is_double_redirection) { // if it is << redirection
	    handle_double_input_redirection(file_stream);
	    return;
	}

        if (execvp(arr[0], arr) == -1) { // if execvp is successful, the current process will be replaced by arr[0] command
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    } else {
        // parent process
        int status;
        waitpid(pid, &status, 0);
    }
}

// checking for redirections
void Bash::redirections_check(std::vector<std::string>& arg_list, std::vector<std::pair<std::string, std::string>>& output_redirections, std::vector<std::pair<std::string, std::string>>& input_redirections, bool& redirection_exist, bool& is_double_input_redirect, std::stringstream& file_stream) {
    for (size_t i = 0; i < arg_list.size(); ++i) {
        if (arg_list[i] == ">") { // redirects the output of a command to a file, creating the file if it doesn't exist or overwriting its content if it does.
	    redirection_exist = true;
            output_redirections.push_back({arg_list[i + 1], ">"});
            arg_list.erase(arg_list.begin() + i, arg_list.begin() + i + 2); // for example: ls > f1 > f2 => ls > f2
            --i; 
        } else if (arg_list[i] == ">>") { // appends the output of a command to a file, creating the file if it doesn't exist.
            redirection_exist = true;
            output_redirections.push_back({arg_list[i + 1], ">>"});
            arg_list.erase(arg_list.begin() + i, arg_list.begin() + i + 2);
            --i; 
        } else if (arg_list[i] == "<") { // takes input for a command from a file.
	    redirection_exist = true;
            input_redirections.push_back({arg_list[i + 1], "<"});
            arg_list.erase(arg_list.begin() + i, arg_list.begin() + i + 2);
            --i; 
        } else if (arg_list[i] == "<<") { // allows input of multiple lines directly into a command.
	    is_double_input_redirect = true;
            std::string eof = arg_list[i + 1];
            arg_list.erase(arg_list.begin() + i, arg_list.begin() + i + 2);
            --i; 
            // reading lines from user until the eof is encountered
            while (true) {
                std::cout << "> ";
                std::string line;
                std::getline(std::cin, line);
                if (line == eof) {
                    break;
                }
                file_stream << line << std::endl;
            }
        } 
    }
} 

// function for handling redirections
void Bash::handle_redirections(const std::vector<std::pair<std::string, std::string>>& output_redirections, const std::vector<std::pair<std::string, std::string>>& input_redirections) {
    for (const auto& redirection : input_redirections) {
        // opening file for reading and duplicateing file descriptor to stdin
        int fd = open(redirection.first.c_str(), O_RDONLY);
        if (fd == -1) {
            std::cerr << "Error opening input file" << std::endl;
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDIN_FILENO); // duplicates the file descriptor fd to the standard input file descriptor (STDIN_FILENO) // if for example "file.txt" file is associated with fd then both fd and standard input will refer to the same "file.txt"
        close(fd);
    }

    for (const auto& redirection : output_redirections) {
        int flags = O_WRONLY | O_CREAT;
        if (redirection.second == ">>") {
            flags |= O_APPEND; // flag for adding at the end of file
        } else {
            flags |= O_TRUNC; // flag for overwriting
        }

        // opening the file for writing and duplicate the file descriptor to stdout
        int fd = open(redirection.first.c_str(), flags, 0666);
        if (fd == -1) {
            std::cerr << "Error opening output file" << std::endl;
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO); // duplicating file descriptor to stdout
        close(fd);
    }
}

// handling << redirection
void Bash::handle_double_input_redirection(std::stringstream& file_stream) {
    // redirecting stdin to a temporary file
    std::ofstream temp("temp.txt");
    temp << file_stream.str();
    temp.close();

    // using the temporary file as input
    std::ifstream input_file("temp.txt");
   
    std::string line;
    while (std::getline(input_file, line)) {
        std::cout << line << std::endl;
    }

    // closing and removing the temporary file
    input_file.close();
    std::remove("temp.txt");
    file_stream.str(""); // clearing
}

// function for checking pipe existence
void Bash::pipe_existence_check(const std::vector<std::string>& arg_list, bool& found_pipe) {
    std::vector<std::string> result;
    std::string current_string;

    // separating commands between pipes
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

    // if pipe is found call handle pipe function
    if (found_pipe && !current_string.empty()) {
        result.push_back(current_string);
	handle_pipe(result);
    }
}

// function for handling pipe
void Bash::handle_pipe(std::vector<std::string>& commands) {
    int size = commands.size();
    int pipes[size - 1][2]; // creting size - 1 pipes,  pipe function creates a pair of file descriptors, and data written to one file descriptor can be read from the other

    // checking if pipe creation failes
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
                dup2(pipes[i - 1][0], STDIN_FILENO); // stdin of current process to be read from reading end of pipe, associated with previous command
                close(pipes[i - 1][0]);
            }

            if (i < size - 1) { 
                dup2(pipes[i][1], STDOUT_FILENO); // stdout of current process to be written to writing end of pipe, associated with next command
                close(pipes[i][1]);
            }

            // close all other pipes
            for (int j = 0; j < size - 1; ++j) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // execute the command
            const char* command = commands[i].c_str(); 
            execlp("/bin/sh", "/bin/sh", "-c", command, nullptr); // we give "-c" to indicate that next comming arg: command is string representing all command such as "grep a" => so we gave "c", such the command to be understood as one command 
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

// function for generating tab completions
std::vector<std::string> Bash::tab_completions(const std::string& input) {
    std::vector<std::string> available;
    std::vector<std::string> commands = {"cd", "ls", "pwd", "help", "exit", "export", "cat"};

    // adding commands to suggestions
    for (const auto& cmd : commands) {
        if (cmd.find(input) == 0) {
            available.push_back(cmd);
        }
    }

    // finding directories or files with starting input
    DIR* dir;
    struct dirent* entry;
    if ((dir = opendir(".")) != nullptr) { // starting search from current directory
        while ((entry = readdir(dir)) != nullptr) {
            std::string name(entry -> d_name);
            if (name.find(input) == 0) {
                available.push_back(name);
            }
        }
        closedir(dir);
    }

    return available;
}

//is a placeholder for custom completion logic to implement the logic to generate and return completion matches based on the provided text, start, and end parameters.
char** Bash::custom_completion(const char* input, int start, int end) {
    rl_attempted_completion_over = 1; // to indicate that the completion functions have been called and have finished attempting completion.
    std::vector<std::string> completions = tab_completions(input); // getting all completions
    if (completions.size() != 0) {
	    std::cout << std::endl;
    }
    int i = 0;
    for (const auto& comp : completions) {
        std::cout << comp << "\t";
        ++i;
	    if (i % 10 == 0) {
	        std::cout << std::endl;
        }
    }
    std::cout << std::endl;

    // continue with the next command
    rl_forced_update_display();

    return nullptr;    
}

