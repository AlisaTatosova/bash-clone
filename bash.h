#ifndef BASH_H
#define BASH_H

#include <vector>
#include <string>

class Bash {
public:
    void execute();
    void execute_command(std::vector<std::string>& arg_list);
    void pipe_existence_check(const std::vector<std::string>& arg_list, bool& pipe_exist);
    void redirections(std::vector<std::string>& arg_list, std::vector<std::pair<std::string, std::string>>& output_redirections, std::vector<std::pair<std::string, std::string>>& input_redirections);
    void handle_pipe(std::vector<std::string>& commands);

private:
    std::vector<std::string> history;
};

#endif
