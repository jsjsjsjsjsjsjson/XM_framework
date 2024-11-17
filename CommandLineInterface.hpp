#ifndef COMMAND_LINE_INTERFACE_H
#define COMMAND_LINE_INTERFACE_H

#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <functional>

class CommandLineInterface {
public:
    static const size_t CMD_BUF_SIZE = 64;
    static const size_t CMD_LIST_SIZE = 32;

    using CommandFunc = std::function<void(int argc, const char* argv[])>;

    struct Command {
        std::string name;
        CommandFunc func;
    };

    CommandLineInterface()
        : cmdBufferIndex(0), cmdListSize(0) {
        instance = this;
    }

    void begin(const std::string &termPrompt) {
        prompt = termPrompt;
        std::cout << "Command Line Interface Started" << std::endl;
        printPrompt();
        addCommand("help", helpCmd);
    }

    void update() {
        std::string inputLine;
        if (std::getline(std::cin, inputLine)) {
            if (!inputLine.empty()) {
                strncpy(cmdBuffer, inputLine.c_str(), CMD_BUF_SIZE);
                executeCommand(cmdBuffer);
                cmdBufferIndex = 0;
            }
            printPrompt();
        }
    }

    void addCommand(const std::string &name, CommandFunc func) {
        if (cmdListSize < CMD_LIST_SIZE) {
            cmdList[cmdListSize].name = name;
            cmdList[cmdListSize].func = func;
            cmdListSize++;
        }
    }

    static void helpCmd(int argc, const char* argv[]) {
        if (instance != nullptr) {
            std::cout << "Available commands:" << std::endl;
            for (size_t i = 0; i < instance->cmdListSize; i++) {
                std::cout << instance->cmdList[i].name << std::endl;
            }
        } else {
            std::cout << "Error: instance is not set." << std::endl;
        }
    }

    void executeCommand(const char* cmd) {
        const char* argv[CMD_BUF_SIZE / 2];
        int argc = 0;
        char cmdCopy[CMD_BUF_SIZE];
        strncpy(cmdCopy, cmd, CMD_BUF_SIZE);
        char* token = strtok(cmdCopy, " ");
        while (token != nullptr && argc < CMD_BUF_SIZE / 2) {
            argv[argc++] = token;
            token = strtok(nullptr, " ");
        }

        if (argc > 0) {
            for (size_t i = 0; i < cmdListSize; i++) {
                if (cmdList[i].name == argv[0]) {
                    cmdList[i].func(argc, argv);
                    return;
                }
            }
            std::cout << "Unknown command" << std::endl;
        }
    }

    static CommandLineInterface* instance;

private:
    void printPrompt() {
        std::cout << prompt << ">>> ";
    }
    std::string prompt;
    char cmdBuffer[CMD_BUF_SIZE];
    size_t cmdBufferIndex;
    Command cmdList[CMD_LIST_SIZE];
    size_t cmdListSize;
};

CommandLineInterface* CommandLineInterface::instance = nullptr;

#endif
