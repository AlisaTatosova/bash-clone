# bash-clone
Bash clone is command-line shell implemented in C++. It supports common shell commands and features such as
* cd: Change the current working directory.
* ls: List all the file and directory names in current directory
* pwd: Present Current Directory
* echo: Display text or variable values on the screen.
* exit: Terminate the shell.
* help: Provide information about available commands.
* history: Provides the history of used commands.

## Features
- Command execution: Run common shell commands like `cd`, `ls`, `pwd`, `help`, `exit`, and more.
- Tab completion: Get suggestions for commands, paths, and filenames as you type.
- Command history: Navigate through command history using arrow keys or a history command.
- Environment variable management: Set, modify, and retrieve environment variables.
- External program execution: Execute external programs and scripts from system directories or specified paths.
- Pipe functionality: for example: ls | grep a | grep c
- Redirections:
  *  (Output Redirection): Redirects the output of a command to a file, creating the file if    it doesn't exist or overwriting its content if it does.
  Example: Redirect ls command output to a file named "list.txt"
  ```bash
  ls > list.txt
  ```
  *  (Append Output): Appends the output of a command to a file, creating the file if it doesn't exist.
   Example: Append the output of the ls command to a file named "log.txt"
   ```bash
    ls >> log.txt
    ```
  * < (Input Redirection): Takes input for a command from a file.
  Example: Use the contents of a file named "input.txt" as input for the cat command
  ```bash
  cat < input.txt
   ```
  * << (Here Document): Allows input of multiple lines directly into a command.
  Example: Using a here document to input multiple lines to the cat command
  ```bash
  cat << EOF
  This is line 1
  This is line 2
  EOF
  ```
  In this example, EOF is a delimiter indicating the end of the input.
  
 * | (Piping):
  Sends the output of one command as input to another command.
  Example: List the files in the current directory and pipe the output to the 'grep'   command to filter for files containing 'example'
  ```bash
  ls | grep example
   ```

## Usage
1. **Compile the Shell:**
    Warning: for use of readline header file there is need to install

    ```bash
    sudo apt-get install libreadline-dev
    ```

    During compiling there is need to add -lreadline flag telling the compiler to link your program with the Readline library. 
    ```bash
    g++ -o shell main.cpp -lreadline
    ```

2. **Run the Shell:**
    ```bash
    ./shell
    ```
3. **Commands:**
    - `cd [directory]`: Change the current directory.
    - `ls`: List files in the current directory.
    - `pwd`: Print the current working directory.
    - `help`: Display information about supported commands.
    - `exit`: Exit the shell.
    - `export VAR_NAME=VAR_VALUE`: Set an environment variable.
    - `echo $VAR_NAME`: Display the value of an environment variable.
    - Other commands: Execute external programs or scripts.

4. **Tab Completion:**

    The shell provides tab completion for commands, paths, and filenames. Press the `Tab` key to see suggestions.

5. **Command History:**

    Navigate through command history using arrow keys or type `history` to see a list of previous commands.

## Examples

- Set an environment variable:

    ```bash
    export MY_VARIABLE=Hello
    ```

- Print the value of an environment variable:

    ```bash
    echo $MY_VARIABLE
    ```

## Contributing

Contributions are welcome! Please follow the [contribution guidelines](CONTRIBUTING.md) when submitting pull requests.

## License

This project is licensed under the [MIT License](LICENSE).
