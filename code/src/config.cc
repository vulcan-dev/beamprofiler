#include "config.h"
#include "third_party/json.h"
#include "common/bp_string.h"
#include <string>
using json = nlohmann::json;

#include <fstream>

// TODO: Actually create a nice config class (or struct).
//       You can then create the config with an initializer list and set what everything is and needs to be.
//       This one is crap and rushed, but should be safe from crashes.

static bp::string address_arr_to_string(unsigned char* uaddr)
{
    return bp::string::format("%d.%d.%d.%d",
                              uaddr[0],
                              uaddr[1],
                              uaddr[2],
                              uaddr[3]);
}

static unsigned char* address_string_to_arr(const char* str)
{
    unsigned char* ipArr = new unsigned char[4]{ 0, 0, 0, 0 };
    char* str_copy = _strdup(str);
    char* next_token = nullptr;

    char* token = strtok_s(str_copy, ".", &next_token);
    int i = 0;
    while (token != nullptr && i < 4)
    {
        int num = atoi(token);
        if (num < 0) num = 0;
        if (num > 255) num = 255;
        ipArr[i] = static_cast<unsigned char>(num);
        token = strtok_s(nullptr, ".", &next_token);
        i++;
    }

    free(str_copy);
    return ipArr;
}

static bool validate_json(json data)
{
    if (!data.is_object())
        return false;

    // Connection
    if (!data.contains("connection") || !data["connection"].is_object())
        return false;

    json connection = data["connection"];

    if (!connection.contains("address") || !connection["address"].is_string())
        return false;

    if (!connection.contains("port") || !connection["port"].is_number_integer())
        return false;


    // Profiler
    if (!data.contains("profiler") || !data["profiler"].is_object())
        return false;

    json profiler = data["profiler"];

    if (!profiler.contains("fps_limit") || !profiler["fps_limit"].is_number_integer())
        return false;

    if (!profiler.contains("history") || !profiler["history"].is_number())
        return false;

    return true;
}

static void config_create(config_t* config)
{
    config->ip[0] = 127;
    config->ip[1] = 0;
    config->ip[2] = 0;
    config->ip[3] = 1;

    config->port = 4444;
    config->fps_limit = 30;
    config->history = 10;

    std::ofstream f("config.json");

    json data;
    json connection, profiler;

    connection["address"] = address_arr_to_string(config->ip).c_str();
    connection["port"] = config->port;

    profiler["fps_limit"] = config->fps_limit;
    profiler["history"] = config->history;

    data["connection"] = connection;
    data["profiler"] = profiler;

    f << data.dump(4);
    f.close();
}

bool config_load(config_t* config)
{
    std::fstream f("config.json");
    if (!f.is_open())
    {
        config_create(config);
        return true;
    }

    json data = json::parse(f);
    if (!validate_json(data))
        return false;

    json connection = data["connection"];
    json profiler = data["profiler"];

    std::string addr_str = connection["address"];
    unsigned char* ip_data = address_string_to_arr(addr_str.c_str());

    config->ip[0] = ip_data[0];
    config->ip[1] = ip_data[1];
    config->ip[2] = ip_data[2];
    config->ip[3] = ip_data[3];
    config->port = connection["port"].get<uint16_t>();

    config->fps_limit = profiler["fps_limit"].get<int>();
    config->history = profiler["history"].get<float>();

    f.close();
    return true;
}

void config_save(config_t* config)
{
    std::ofstream f("config.json");

    json data;
    json connection, profiler;

    connection["address"] = address_arr_to_string(config->ip).c_str();
    connection["port"] = config->port;

    profiler["fps_limit"] = config->fps_limit;
    profiler["history"] = config->history;

    data["connection"] = connection;
    data["profiler"] = profiler;

    f << data.dump(4);
    f.close();
}

bp::string config_get_ip(config_t* config)
{
    return address_arr_to_string(config->ip);
}