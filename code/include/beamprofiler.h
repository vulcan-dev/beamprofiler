#ifndef BP_OUTGAUGE_H
#define BP_OUTGAUGE_H

#include "common/bp_vector.h"

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

typedef struct render_data_t
{
    profiler_t data;
    bp::vec<char> queue;

    bool connected;
    bool request_close;
} render_data_t;

#endif