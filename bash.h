#ifndef BASH_H
#define BASH_H

#include <vector>
#include <string>

class Bash {
public:
    Bash();
    void execute(); // executing shell
    void execute_command(std::vector<std::string>& arg_list); // executing shell commands
    void redirections_check(std::vector<std::string>& arg_list, std::vector<std::pair<std::string, std::string>>& output_redirections, std::vector<std::pair<std::string, std::string>>& input_redirections, bool& redirection_exist, bool& is_double_input_redirect, std::stringstream& file_stream); // checking for rdirections
    void handle_double_input_redirection(std::stringstream& file_stream); // handling << redirection
    void handle_redirections(const std::vector<std::pair<std::string, std::string>>& output_redirections, const std::vector<std::pair<std::string, std::string>>& input_redirections); // handling >, >>, <
    void pipe_existence_check(const std::vector<std::string>& arg_list, bool& pipe_exist); // checking for pipe existence
    void handle_pipe(std::vector<std::string>& commands); // handling pipe

    static std::vector<std::string> tab_completions(const std::string& input); // tab completion
    static char** custom_completion(const char* text, int start, int end); // custom implementation for completion
private:
    std::vector<std::string> history;
};

#endif
