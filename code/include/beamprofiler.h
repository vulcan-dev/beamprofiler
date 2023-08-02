#ifndef BP_OUTGAUGE_H
#define BP_OUTGAUGE_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include "common/bp_vector.h"
#include "common/bp_mutex.h"
#include "config.h"

void interface_thread(void* udata);

//typedef struct profiler_wheel_thermal_t
//{
//    int id;
//    char name[2];
//    float core_temperature;
//    float surface_temperature;
//    float thermal_efficiency;
//} profiler_wheel_thermal_t;

typedef struct profiler_thermals_t
{
    float water;
    float engine_block;
    float engine_wall;
    float coolant;
    float oil;
    float exhaust;
    float radiator_speed;
} profiler_thermals_t;

typedef struct profiler_inputs_t
{
    float throttle;
    float brake;
    float clutch;
} profiler_inputs_t;

typedef struct profiler_t
{
    profiler_thermals_t thermals;
    profiler_inputs_t inputs;

    float exhaust_flow;
    float engine_load;
    float driveshaft;
    float boost;

    float bullet_time;

    float rpm;
    float turbo;
    float airspeed;
    float wheelspeed;
    float fuel;
} profiler_t;

typedef struct render_thread_data_t
{
#if defined(_WIN32)
    HWND foreground_window;
#endif

    bool can_read;
    bool requested_connect; // Maybe use flags if I end up adding more ways to communicate from render -> main

    bool can_connected;
    config_t config;

    profiler_t data;
    bp::vec<char> queue;
    bp::mutex queue_mtx;

    bool connected;
    bool request_close;
} render_data_t;

#endif