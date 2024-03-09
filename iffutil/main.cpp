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
typedef std::pair<const char *, handler_t> command_t;
typedef std::map<std::string, command_t> commands_t;

typedef std::deque<std::string> chunk_path_t;
typedef enum {
    visit_before_data,
    visit_after_data,
    visit_after_last_group_data,
} visit_time_e;
// iff_in, iff_out, read chunk/group, visit_time, is_matched
// Return true if chunk or group should be copied
typedef std::function<bool(cgiff_file_c&, cgiff_file_c&, cgiff_chunk_t &chunk, visit_time_e, bool)> chunk_visitor_t;

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
const commands_t commands {
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
static bool is_verbose = false;

static const std::string &get_iff_out_path() {
    if (iff_out_path.empty()) {
        iff_out_path = iff_in_path;
        iff_out_path.append(".tmp");
    }
    return iff_out_path;
}

static bool iff_out_path_is_temp() {
    return iff_out_path.empty() || iff_out_path.ends_with(".tmp");
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
        printf("Copy %d bytes\n", size);
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
    cgiff_chunk_t chunk_out;
    cgiff_group_t group_out;
    bool matched_first = false;
    if (is_group && path.size() > 0) {
        iff_in.expand(chunk_in, group_in);
        chunk_path_t group_id = split_string(path.front(), '.');
        if (group_id.size() == 2) {
            matched_first = matched_first =  cgiff_id_match(group_in.id, group_id[0].c_str()) && cgiff_id_match(group_in.subtype, group_id[1].c_str());
        }
    } else if (path.size() > 0) {
        matched_first = cgiff_id_match(chunk_in.id, path.front().c_str());
    }
    if (matched_first) {
        path.pop_front();
    }
    if (is_verbose) {
        char id[5]; cgiff_id_str(chunk_in.id, id);
        if (is_group) {
            char subtype[5]; cgiff_id_str(group_in.subtype, subtype);
            printf("Processing %s:%s for %d bytes\n", id, subtype, chunk_in.size);
        } else {
            printf("Processing %s for %d bytes\n", id, chunk_in.size);
        }
    }
    bool matched_final = matched_first && path.size() == 0;
    if (is_verbose) {
        if (matched_first) {
            printf("Did match %spath.\n", matched_final ? "final " : "");
        }
    }
    bool do_copy = visitor(iff_in, iff_out, chunk_in, visit_before_data, matched_final);
    if (do_copy) {
        char buf[5]; cgiff_id_str(chunk_in.id, buf);
        if (!iff_out.begin(chunk_out, buf)) {
            printf("Write error.\n");
            exit(-1);
        }
        if (is_group) {
            if (!(iff_out.write(group_in.subtype) && iff_out.expand(chunk_out, group_out))) {
                printf("Write error.\n");
                exit(-1);
            }
            if (path.size() == 0) {
                do_copy_chunk_content(iff_in, iff_out, chunk_in, is_group);
            } else {
                cgiff_chunk_t next_chunk;
                while (iff_in.next(group_in, "*", next_chunk)) {
                    do_visit_chunks(iff_in, iff_out, next_chunk, path, visitor);
                }
            }
        } else {
            do_copy_chunk_content(iff_in, iff_out, chunk_in, is_group);
        }
    }
    (void)visitor(iff_in, iff_out, chunk_in, is_group ? visit_after_last_group_data : visit_after_data, matched_final);
    if (do_copy) {
        if (is_group) {
            iff_out.end(group_out);
            if (is_verbose) {
                char id[5], subtype[5];
                cgiff_id_str(group_in.id, id); cgiff_id_str(group_in.subtype, subtype);
                printf("Closed %s:%s with %d bytes.\n", id, subtype, group_out.size);
            }
        } else {
            iff_out.end(chunk_out);
        }
    }
}

static void handle_list(arguments_t &args) {
    cgiff_file_c iff(iff_in_path.c_str());
    
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
            do_visit_chunks(iff_in, iff_out, top_chunk, path, [&] (cgiff_file_c &iff_in, cgiff_file_c &iff_out, cgiff_chunk_t &chunk, visit_time_e time, bool matched) -> bool {
                if (matched && time == visit_before_data) {
                    if (is_verbose) {
                        printf("Skip %d bytes.\n", chunk.size);
                    }
                    iff_in.skip(chunk);
                }
                return !matched;
            });
        } else {
            printf("Could not find first chunk.\n");
        }
    }
    if (iff_out_path_is_temp()) {
        // Copy temp over original
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
        while (args.size() > 0) {
            auto arg = args.front(); args.pop_front();
            const auto command = commands.find(arg);
            if (command != commands.end()) {
                command->second.second(args);
            } else {
                do_unknown_arg(arg);
            }
        }
    }
    return 0;
}
