//
//  arguments.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-11.
//

#ifndef arguments_h
#define arguments_h

#include <vector>
#include <string>
#include <deque>
#include <map>
#include <functional>
#include <string>

typedef std::deque<const char *> arguments_t;
typedef std::function<void(arguments_t&)> handler_t;
typedef std::pair<const char *, handler_t> arg_handler_t;
typedef std::map<std::string, arg_handler_t> arg_handlers_t;

static void do_unknown_arg(const char *arg) {
    printf("Unknown %s '%s'.\n", arg[0] == '-' ? "option" : "command", arg);
    exit(-1);
}

static void do_print_help(const char *usage, const arg_handlers_t &arg_handlers) {
    size_t max_len = 0;
    for (const auto &com : arg_handlers) {
        max_len = std::max(max_len, com.first.length());
    }
    printf("%s\n", usage);
    const auto do_print = [&] (const arg_handlers_t::value_type &cmd) {
        printf("  %s", cmd.first.c_str());
        for (size_t i = cmd.first.length(); i < max_len + 2; i++) {
            printf(" ");
        }
        printf("%s\n", cmd.second.first);
    };
    arg_handlers_t options;
    for (auto &command : arg_handlers) {
        if (command.first[0] == '-') {
            options.insert(command);
        }
    }
    if (options.size() > 0) {
        printf("options:\n");
        for (auto &option : options) {
            do_print(option);
        }
    }
    
    arg_handlers_t commands;
    for (auto &command : arg_handlers) {
        if (command.first[0] != '-') {
            commands.insert(command);
        }
    }
    if (commands.size() > 0) {
        printf("command:\n");
        for (auto &option : commands) {
            do_print(option);
        }
    }
}

static void do_handle_args(arguments_t &args, const arg_handlers_t &arg_handlers, bool fail_on_unknown = false) {
    while (args.size() > 0) {
        auto arg = args.front();
        const auto command = arg_handlers.find(arg);
        if (command != arg_handlers.end()) {
            args.pop_front();
            command->second.second(args);
        } else if (fail_on_unknown) {
            do_unknown_arg(arg);
        } else {
            return;
        }
    }
}


#endif /* arguments_h */
