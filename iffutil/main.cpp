//
//  main.cpp
//  iffutil
//
//  Created by Fredrik on 2024-03-08.
//

#define __NO_GCSTDLIB__
#include "types.hpp"
#include "iff_file.hpp"

#include <vector>
#include <string>
#include <deque>
#include <set>
#include <map>
#include <functional>
#include <string>
#include <algorithm>

typedef std::deque<const char *> arguments_t;
typedef std::function<void(arguments_t&)> handler_t;
typedef std::pair<const char *, handler_t> command_t;
typedef std::map<std::string, command_t> commands_t;

static void handle_help(arguments_t &args);
static void handle_input(arguments_t &args);
static void handle_output(arguments_t &args);
static void handle_group(arguments_t &args);
static void handle_remove(arguments_t &args);
static void handle_list(arguments_t &args);
static void handle_append(arguments_t &args);
static void handle_insert(arguments_t &args);
const commands_t commands {
    {"-g",   {"Add known group by C4ID.", &handle_group}},
    {"-h",   {"Show this help and exit.", &handle_help}},
    {"-i",   {"Input iff file.", &handle_input}},
    {"-o",   {"Output iff file.", &handle_output}},
    {"append", {"Append chunk to iff file.", &handle_append}},
    {"insert", {"Insert chunk into iff file.", &handle_insert}},
    {"list",   {"List contents of iff file.", &handle_list}},
    {"remove", {"Remove chunk from iff file.", &handle_remove}},
};


static const char *iff_in_path = nullptr;
static const char *iff_out_path = nullptr;

static cgiff_file_c *iff_in = nullptr;
static cgiff_file_c *iff_out = nullptr;

static std::set<cgiff_id_t> known_groups = {
    cgiff_id_make(CGIFF_FORM),
    cgiff_id_make(CGIFF_LIST),
    cgiff_id_make(CGIFF_CAT)
};

static void do_unknown_arg(const char *arg) {
    printf("Unknown %s '%s'.\n", arg[0] == '-' ? "option" : "command", arg);
    arguments_t args;
    handle_help(args);
    exit(-1);
}

static void handle_input(arguments_t &args) {
    if (args.size() < 1) {
        printf("No input file.\n");
        exit(-1);
    }
    iff_in_path = args.front();
    args.pop_front();
}

static void handle_output(arguments_t &args) {
    if (args.size() < 1) {
        printf("No output file.\n");
        exit(-1);
    }
    iff_in_path = args.front();
    args.pop_front();
}

static void handle_group(arguments_t &args) {
    if (args.size() < 1) {
        printf("No group given.\n");
        exit(-1);
    }
    known_groups.emplace(cgiff_id_make(args.front()));
    args.pop_front();
}

static void handle_list(arguments_t &args) {
    cgiff_file_c iff(iff_in_path);
    
    const auto do_list_indentation = [] (int level) {
        for (int l = 0; l < level; l++) {
            printf("    ");
        }
    };
    const auto do_list_chunk = [&] (int level, cgiff_chunk_t &chunk) {
        do_list_indentation(level);
        char id[5]; cgiff_id_str(chunk.id, id);
        printf("%.4s %d@%ld\n", id, chunk.size, chunk.offset);
    };
    const auto do_list_group = [&] (int level, cgiff_group_t &group) {
        do_list_indentation(level);
        char id[5]; cgiff_id_str(group.id, id);
        char subtype[5]; cgiff_id_str(group.subtype, subtype);
        printf("%.4s %d@%ld { %.4s\n", id, group.size, group.offset, subtype);
        cgiff_chunk_t chunk;
        while (iff.next(group, "*", chunk)) {
            if (known_groups.contains(chunk.id)) {
                cgiff_group_t subgroup;
                iff.expand(chunk, subgroup);
                do_list_chunk(level + 1, subgroup);
            } else {
                do_list_chunk(level + 1, chunk);
            }
            iff.skip(chunk);
        }
        do_list_indentation(level);
        printf("}\n");
    };
    
    cgiff_group_t top_group;
    if (iff.first("*", "*", top_group)) {
        do_list_group(0, top_group);
        return;
    }
    printf("No top group.\n");
    exit(-1);
}

static void handle_remove(arguments_t &args) {
    printf("Not implemented.\n");
    exit(-1);
}

static void handle_append(arguments_t &args) {
    printf("Not implemented.\n");
    exit(-1);
}

static void handle_insert(arguments_t &args) {
    printf("Not implemented.\n");
    exit(-1);
}

static void handle_help(arguments_t &args) {
    size_t max_len = 0;
    for (const auto &com : commands) {
        max_len = std::max(max_len, com.first.length());
    }
    printf(
        "iffutil - A utility for managing EA IFF 85 files.\n"
        "usage: iffutil [options] commands...\n"
        "options:\n");
    const auto do_print = [&] (const commands_t::value_type &cmd) {
        printf("  %s", cmd.first.c_str());
        for (size_t i = cmd.first.length(); i < max_len + 2; i++) {
            printf(" ");
        }
        printf("%s\n", cmd.second.first);
    };
    for (auto &option : commands) {
        if (option.first[0] == '-') {
            do_print(option);
        }
    }
    printf("command:\n");
    for (auto &option : commands) {
        if (option.first[0] != '-') {
            do_print(option);
        }
    }
    exit(0);
}

int main(int argc, const char * argv[]) {
    arguments_t args(&argv[1], &argv[argc]);
    
    if (args.empty()) {
        handle_help(args);
    } else {
        restart:
        for (auto arg : args) {
            const auto command = commands.find(arg);
            if (command != commands.end()) {
                args.pop_front();
                command->second.second(args);
                goto restart;
            } else {
                do_unknown_arg(arg);
            }
        }
    }
    return 0;
}
