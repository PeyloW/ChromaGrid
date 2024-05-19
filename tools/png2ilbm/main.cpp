//
//  main.cpp
//  png2ilbm
//
//  Created by Fredrik on 2024-03-11.
//

#include <iostream>
#include "types.hpp"
#include "image.hpp"
#include "canvas.hpp"

#include "arguments.hpp"
#include <png.h>

using namespace toybox;

static void handle_help(arguments_t &args);
static void handle_palette(arguments_t &args);
static void handle_masked(arguments_t &args);
static void handle_compressed(arguments_t &args);

static bool save_palette = true;
static bool save_masked = false;
static uint8_t masked_idx = image_c::MASKED_CIDX;
static compression_type_e compression = compression_type_none;
//static point_s grab_point = {0,0};

const arg_handlers_t arg_handlers {
    {"-h",          {"Show this help and exit.", &handle_help}},
    {"-np",         {"Do not save palette.", [] (arguments_t &) { save_palette = false; }}},
    {"-m",          {"Save masked.", [] (arguments_t &) { save_masked = true; }}},
    {"-mi index",   {"Masked index.", [] (arguments_t &args) {
        masked_idx = atoi(args.front());
        args.pop_front();
    }}},
    {"-c type",          {"Save compressed.", [] (arguments_t &args) {
        compression = (compression_type_e)atoi(args.front());
        args.pop_front();
    }}},
/*    {"-g x,y",      {"Add grab point.", [] (arguments_t &args) {
        auto split = split_string(args.front(), ',');
        grab_point = {(int16_t)atoi(split[0].c_str()), (int16_t)atoi(split[1].c_str())};
    }}},*/
};

static void handle_help(arguments_t &args) {
    do_print_help("png2ilbm - A utility for converting png images to iff ilbm.\nusage: png2ilbm [options] image.png image.iff", arg_handlers);
    exit(0);
}

static int convert_png_to_ilbm(const std::string &png_file, const std::string &ilbm_file) {
    int16_t width, height;
    png_byte color_type;
    png_byte bit_depth;
    png_bytep *row_pointers = NULL;

    FILE *fp = fopen(png_file.c_str(), "rb");

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png) abort();

    png_infop info = png_create_info_struct(png);
    if (!info) {
        printf("Could not open open png file.");
        exit(-1);
    }
    png_init_io(png, fp);
    png_read_info(png, info);

    width      = png_get_image_width(png, info);
    height     = png_get_image_height(png, info);
    color_type = png_get_color_type(png, info);
    if (color_type != PNG_COLOR_TYPE_PALETTE) {
        printf("Only indexed images supported.\n");
        exit(-1);
    }
    bit_depth  = png_get_bit_depth(png, info);
    if(bit_depth != 8) {
        printf("Only 8 bit depth supported");
        exit(-1);
    }
    png_set_packing(png);
    
    int num_palette;
    png_colorp palette;
    if (png_get_PLTE(png, info, &palette, &num_palette) == 0) {
        printf("No palette.\n");
        exit(-1);
    }
    
    palette_c *cgpalette = nullptr;
    if (num_palette > 0 && save_palette) {
        cgpalette = new palette_c((uint8_t *)palette);
    }

    image_c cgimage((size_s){width, height}, save_masked, cgpalette);
    canvas_c cgcanvas(cgimage);
    
    png_bytep row = (png_byte*)malloc(png_get_rowbytes(png,info));
    point_s at;
    bool has_warned = false;
    for(at.y = 0; at.y < height; at.y++) {
        png_read_row(png, row, nullptr);
        for (at.x = 0; at.x < width; at.x++) {
            uint8_t c = row[at.x];
            if (c == masked_idx) {
                cgcanvas.put_pixel(image_c::MASKED_CIDX, at);
            } else if (c > 15) {
                if (!has_warned) {
                    printf("WARNING: Color index > 15 found and ignored.\n");
                    has_warned = true;
                }
            } else {
                cgcanvas.put_pixel(c, at);
            }
        }
    }
    
    // cgimage.set_offset(grab_point);
    
    cgimage.save(ilbm_file.c_str(), compression, save_masked);
    
    return 0;
}

int main(int argc, const char * argv[]) {
    arguments_t args(&argv[1], &argv[argc]);
    if (args.empty()) {
        handle_help(args);
    } else {
        do_handle_args(args, arg_handlers);
        if (args.size() < 1) {
            printf("No input png file.\n");
            exit(-1);
        }
        auto png_file = args.front(); args.pop_front();
        std::string ilbm_file;
        if (args.size() > 0) {
            ilbm_file = args.front();
            args.pop_front();
        }
        if (args.size() > 0) {
            printf("%zu extra unknown arguments.\n", args.size());
            exit(-1);
        }
        return convert_png_to_ilbm(png_file, ilbm_file);
    }
    return 0;
}
