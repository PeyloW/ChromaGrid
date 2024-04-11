//
//  canvas.cpp
//  toybox
//
//  Created by Fredrik on 2024-04-10.
//

#include "canvas.hpp"

using namespace toybox;

canvas_c::canvas_c(image_c &image) :
    _image(image), _dirtymap(nullptr), _stencil(nullptr), _clipping(true)
{}

canvas_c::~canvas_c() {}
