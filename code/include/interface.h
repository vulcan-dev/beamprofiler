#ifndef BP_INTERFACE_H
#define BP_INTERFACE_H

#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl2.h>

#include "common/bp_string.h"

enum operation_e : char
{
    op_none = 0,
    op_reset = 1
};

typedef struct rolling_buffer {
    float span;
    ImVector<ImVec2> data;

    rolling_buffer() {
        span = 10.0f;
        data.reserve(2000);
    }

    void add_point(float x, float y) {
        float xmod = fmodf(x, span);
        if (!data.empty() && xmod < data.back().x)
            data.shrink(0);
        data.push_back(ImVec2(xmod, y));
    }
} rolling_buffer;

typedef struct graph_element_line_t
{
    bp::string name;
    float* value;
    rolling_buffer rb;
} graph_element_line_t;

typedef struct graph_element_t
{
    graph_element_t(const bp::string& name) : graph_name(name) {}

    bp::string graph_name;
    bp::vec<graph_element_line_t*> lines;
    float min, max;
} graph_element_t;

#endif