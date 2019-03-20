#ifndef _OPTIONS_HPP_
#define _OPTIONS_HPP_

#include <iostream>
#include <memory>
#include <string>

struct shader_file_program {
    std::string path;
    std::string source;

    bool empty() const;

    std::unique_ptr<std::istream> open() const;

    template <typename PathCallable, typename SourceCallable>
    void invoke(PathCallable if_path, SourceCallable if_source) const {
        if (!path.empty()) {
            if_path(path);
        } else if (!source.empty()) {
            if_source(source);
        }
    }
};

struct shader_program_options {
    shader_file_program shader;
    shader_file_program postprocess;

    bool use_make;
};

struct geometry_options {
    std::string path;
    std::string nff_source;

    template <typename PathCallable, typename SourceCallable>
    void invoke(PathCallable if_path, SourceCallable if_source) const {
        if (!path.empty()) {
            if_path(path);
        } else if (!nff_source.empty()) {
            if_source(nff_source);
        }
    }
};

struct frame_options {
    int width;
    int height;
};

struct server_options {
    std::string bind_addr;
};

struct log_options {
    bool debug;
    bool verbose;
};

struct viewer_options {
    shader_program_options program;
    geometry_options geometry;
    frame_options frame;
    server_options server;
    log_options log;
    bool headless_mode;
};

#endif /* _OPTIONS_HPP_ */
