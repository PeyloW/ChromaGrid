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
#include <sstream>

typedef std::deque<const char *> arguments_t;
typedef std::function<void(arguments_t&)> handler_t;
typedef std::pair<const char *, handler_t> arg_handler_t;
typedef std::map<std::string, arg_handler_t> arg_handlers_t;

typedef std::deque<std::string> chunk_path_t;
typedef enum {
    visit_before_data,
    visit_after_data,
    visit_after_last_group_data,
} visit_time_e;
typedef enum {
    action_skip,
    action_traverse,
    action_traverse_copy
} visitor_action_e;
// iff_in, iff_out, read chunk/group, visit_time, is_matched
// Return true if chunk or group should be copied
typedef std::function<visitor_action_e(cgiff_file_c&, cgiff_file_c&, cgiff_chunk_t &chunk, visit_time_e, bool)> chunk_visitor_t;

static void handle_verbose(arguments_t &args);
static void handle_help(arguments_t &args);
static void handle_input(arguments_t &args);
static void handle_output(arguments_t &args);
static void handle_group(arguments_t &args);
static void handle_remove(arguments_t &args);
static void handle_list(arguments_t &args);
static void handle_extract(arguments_t &args);
static void handle_append(arguments_t &args);
static void handle_insert(arguments_t &args);
const arg_handlers_t top_arg_handlers {
    {"-g",     {"Add known group by C4ID.", &handle_group}},
    {"-h",     {"Show this help and exit.", &handle_help}},
    {"-i",     {"Input iff file.", &handle_input}},
    {"-o",     {"Output iff file.", &handle_output}},
    {"-v",     {"Verbose output.", &handle_verbose}},
    {"append", {"Append chunk to iff file.", &handle_append}},
    {"extract",{"Extract chunk from iff file.", &handle_extract}},
    {"insert", {"Insert chunk into iff file.", &handle_insert}},
    {"list",   {"List contents of iff file.", &handle_list}},
    {"remove", {"Remove chunk from iff file.", &handle_remove}},
};

static std::string iff_in_path;
static std::string iff_out_path;
static bool iff_out_path_is_temp = false;
static bool is_verbose = false;

static const std::string &get_iff_out_path() {
    if (iff_out_path.empty()) {
        iff_out_path = iff_in_path;
        iff_out_path.append(".tmp");
        iff_out_path_is_temp = true;
    }
    return iff_out_path;
}

static bool move_file(const char *dest, const char *src) {
    FILE *f_dest = fopen(dest, "w");
    FILE *f_src = fopen(src, "r");
    if (f_dest && f_src) {
        fseek(f_src, 0, SEEK_END);
        long size = ftell(f_src);
        fseek(f_src, 0, SEEK_SET);
        uint8_t buffer[size];
        if (fread(buffer, size, 1, f_src) == 1) {
            if (fwrite(buffer, size, 1, f_dest) == 1) {
                fclose(f_dest);
                fclose(f_src);
                return remove(src);
            }
        }
    }
    return false;
}

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

static void handle_verbose(arguments_t &args) {
    is_verbose = true;
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
            if (option.first[0] == '-') {
                do_print(option);
            }
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
        for (auto &option : top_arg_handlers) {
            if (option.first[0] != '-') {
                do_print(option);
            }
        }
    }
}

static chunk_path_t split_string(const std::string &str, char delimiter) {
    std::stringstream sstream(str);
    std::string segment;
    chunk_path_t segments;
    while(std::getline(sstream, segment, delimiter)) {
        segments.push_back(segment);
    }
    return segments;
}

static void do_copy_chunk_content(cgiff_file_c &iff_in, cgiff_file_c &iff_out, cgiff_chunk_t &chunk_in, bool is_group) {
    uint32_t size = chunk_in.size - (is_group ? 4 : 0);
    if (is_verbose) {
        printf("Copied %d bytes\n", size);
    }
    uint8_t buffer[size];
    if (!(iff_in.read(buffer, 1, size) && iff_out.write(buffer, 1, size))) {
        printf("Could not copy chunk.\n");
        exit(-1);
    }
}

static void do_visit_chunks(cgiff_file_c &iff_in, cgiff_file_c &iff_out, cgiff_chunk_t &chunk_in, chunk_path_t &path, chunk_visitor_t visitor) {
    const bool is_group = known_groups.contains(chunk_in.id);
    cgiff_group_t group_in;
    if (is_group) {
        iff_in.expand(chunk_in, group_in);
    }
    cgiff_chunk_t chunk_out;
    cgiff_group_t group_out;
    bool matched_first = false;
    std::string matched_path_id;
    if (is_group && path.size() > 0) {
        chunk_path_t group_id = split_string(path.front(), '.');
        if (group_id.size() == 2) {
            matched_first = matched_first =  cgiff_id_match(group_in.id, group_id[0].c_str()) && cgiff_id_match(group_in.subtype, group_id[1].c_str());
        }
    } else if (path.size() > 0) {
        matched_first = cgiff_id_match(chunk_in.id, path.front().c_str());
    }
    if (matched_first) {
        matched_path_id = path.front();
        path.pop_front();
    }
    if (is_verbose) {
        char id[5]; cgiff_id_str(chunk_in.id, id);
        if (is_group) {
            char subtype[5]; cgiff_id_str(group_in.subtype, subtype);
            printf("Started reading %s:%s for %d bytes\n", id, subtype, chunk_in.size);
        } else {
            printf("Started reading %s for %d bytes\n", id, chunk_in.size);
        }
    }
    bool matched_final = matched_first && path.size() == 0;
    if (is_verbose) {
        if (matched_first) {
            printf("Matched %s%s.\n", matched_final ? "final " : "", matched_path_id.c_str());
        }
    }
    visitor_action_e action = visitor(iff_in, iff_out, chunk_in, visit_before_data, matched_final);
    if (action == action_skip) {
        if (is_verbose) {
            printf("Skipping %d bytes", chunk_in.size);
        }
        iff_in.skip(chunk_in);
    } else {
        char buf[5]; cgiff_id_str(chunk_in.id, buf);
        if (action == action_traverse_copy) {
            if (is_verbose) {
                printf("Started writing %s.\n", buf);
            }
            if (!iff_out.begin(chunk_out, buf)) {
                printf("Write error.\n");
                exit(-1);
            }
        }
        if (is_group) {
            if (action == action_traverse_copy) {
                if (!(iff_out.write(group_in.subtype) && iff_out.expand(chunk_out, group_out))) {
                    printf("Write error.\n");
                    exit(-1);
                }
            }
            cgiff_chunk_t next_chunk;
            while (iff_in.next(group_in, "*", next_chunk)) {
                do_visit_chunks(iff_in, iff_out, next_chunk, path, visitor);
            }
        } else if (action == action_traverse_copy) {
            do_copy_chunk_content(iff_in, iff_out, chunk_in, is_group);
        } else {
            iff_in.skip(chunk_in);
        }
    }
    action = visitor(iff_in, iff_out, chunk_in, is_group ? visit_after_last_group_data : visit_after_data, matched_final);
    if (action == action_traverse_copy) {
        if (is_group) {
            iff_out.end(group_out);
            if (is_verbose) {
                char id[5], subtype[5];
                cgiff_id_str(group_in.id, id); cgiff_id_str(group_in.subtype, subtype);
                printf("Finished writing %s:%s with %d bytes.\n", id, subtype, group_out.size);
            }
        } else {
            if (is_verbose) {
                char id[5];
                cgiff_id_str(chunk_in.id, id);
                printf("Finished writing %s with %d bytes.\n", id, chunk_in.size);
            }
            iff_out.end(chunk_out);
        }
    }
}

static void handle_list(arguments_t &args) {
    cgiff_file_c iff_in(iff_in_path.c_str());
    cgiff_file_c iff_out(stdout);
    int level = 0;
    const auto do_list_indentation = [&] {
        for (int l = 0; l < level; l++) {
            printf("    ");
        }
    };
    cgiff_chunk_t top_chunk;
    if (iff_in.first("*", top_chunk)) {
        chunk_path_t path = {};
        do_visit_chunks(iff_in, iff_out, top_chunk, path, [&] (cgiff_file_c &iff_in, cgiff_file_c &iff_out, cgiff_chunk_t &chunk, visit_time_e time, bool matched) -> visitor_action_e {
            bool is_group = known_groups.contains(chunk.id);
            if (time == visit_before_data) {
                do_list_indentation();
                char id[5]; cgiff_id_str(chunk.id, id);
                printf("%s %d bytes", id, chunk.size);
                if (is_group) {
                    cgiff_group_t group;
                    iff_in.expand(chunk, group);
                    char subtype[5]; cgiff_id_str(group.subtype, subtype);
                    printf(" { %s", subtype);
                    level++;
                }
                printf("\n");
            } else if (is_group) {
                level--;
                do_list_indentation();
                printf("}\n");
           }
            return action_traverse;
        });
    } else {
        printf("Could not find first chunk.\n");
    }
}

static void handle_remove(arguments_t &args) {
    {
        cgiff_file_c iff_in(iff_in_path.c_str(), "r");
        cgiff_file_c iff_out(get_iff_out_path().c_str(), "w+");
        if (args.size() < 1) {
            printf("No path to remove given.\n");
            exit(-1);
        }
        auto path = chunk_path_t(split_string(args.front(), ':'));
        args.pop_front();
        
        cgiff_chunk_t top_chunk;
        if (iff_in.first("*", top_chunk)) {
            do_visit_chunks(iff_in, iff_out, top_chunk, path, [&] (cgiff_file_c &iff_in, cgiff_file_c &iff_out, cgiff_chunk_t &chunk, visit_time_e time, bool matched) -> visitor_action_e {
                if (matched) {
                    if (time != visit_before_data) {
                        printf("Removed matched chunk.\n");
                    }
                    return action_skip;
                }
                return action_traverse_copy;
            });
        } else {
            printf("Could not find first chunk.\n");
        }
    }
    if (iff_out_path_is_temp) {
        move_file(iff_in_path.c_str(), get_iff_out_path().c_str());
    }
}

static void handle_append(arguments_t &args) {
    printf("Not implemented.\n");
    exit(-1);
}

static void handle_extract(arguments_t &args) {
    printf("Not implemented.\n");
    exit(-1);
}

static void handle_insert(arguments_t &args) {
    printf("Not implemented.\n");
    exit(-1);
}

static void handle_help(arguments_t &args) {
    do_print_help("iffutil - A utility for managing EA IFF 85 files.\nusage: iffutil [options] commands...", top_arg_handlers);
    exit(0);
}

int main(int argc, const char * argv[]) {
    arguments_t args(&argv[1], &argv[argc]);
    
    if (args.empty()) {
        handle_help(args);
    } else {
        while (args.size() > 0) {
            auto arg = args.front(); args.pop_front();
            const auto command = top_arg_handlers.find(arg);
            if (command != top_arg_handlers.end()) {
                command->second.second(args);
            } else {
                do_unknown_arg(arg);
            }
        }
    }
    return 0;
}
