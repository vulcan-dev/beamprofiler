local M = {}

local ffi = require("ffi")
local socket = require("socket")

local connection = {
    address = "127.0.0.1",
    port = 4444
}

local udp_socket = nil

local operations = {
    op_none = 0, -- TODO: Remove
    op_reset = 1
}

M.set_address = function(address)
    connection.address = address
end

M.set_ip = function(port)
    connection.port = port
end

-- TODO: Instead of doing the units on the C++ side, just do them here.
pcall(function()
    ffi.cdef [[
        typedef struct profiler_wheel_thermal_t
        {
            int id;
            char name[2];
            float core_temperature;
            float surface_temperature;
            float thermal_efficiency;
        } profiler_wheel_thermal_t;

        typedef struct profiler_thermals_t
        {
            // profiler_wheel_thermal_t* wheels;
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
    ]]
end)

local wheel_count = 0

-- Utilities
---------------------------------------
local function table_len(t)
    local c = 0
    for k, v in pairs(t) do c = c + 1 end
    return c
end

-- Send Function
---------------------------------------
local function dispatch_data(delta)
    local values = electrics.values

    if not values.watertemp then return end
    -- if wheel_count == 0 then
    --     wheel_count = table_len(values.wheelThermals)
    -- end

    -- Wheel Thermals
    -- local wheel_thermals = ffi.new("profiler_wheel_thermal_t[?]", wheel_count)
    -- local i = 0
    -- for name, data in pairs(values.wheelThermals) do
    --     wheel_thermals[i].id = i
    --     for j = 1, 2 do
    --         wheel_thermals[i].name[j - 1] = name:sub(j, j):byte()
    --     end
    --     wheel_thermals[i].core_temperature = data.brakeCoreTemperature
    --     wheel_thermals[i].surface_temperature = data.brakeSurfaceTemperature
    --     wheel_thermals[i].thermal_efficiency = data.brakeThermalEfficiency
    --     i = i + 1
    -- end

    -- General Thermals
    local thermals = ffi.new("profiler_thermals_t")
    -- thermals.wheels = wheel_thermals
    thermals.water = values.watertemp

    local engine = powertrain.getDevicesByType("combustionEngine")[1]
    if engine then
        thermals.engine_block = engine.thermals.engineBlockTemperature
        thermals.engine_wall = engine.thermals.cylinderWallTemperature
        thermals.coolant = engine.thermals.coolantTemperature
        thermals.oil = engine.thermals.oilTemperature
        thermals.exhaust = engine.thermals.exhaustTemperature
        thermals.radiator_speed = engine.thermals.radiatorAirSpeed or 0
    end

    -- Inputs
    local inputs = ffi.new("profiler_inputs_t")
    inputs.throttle = values.throttle * 100
    inputs.brake = values.brake * 100
    inputs.clutch = values.clutch * 100

    -- Profiler
    local profiler = ffi.new("profiler_t")
    profiler.thermals = thermals
    profiler.inputs = inputs
    
    profiler.exhaust_flow = values.exhaustFlow or 0
    profiler.engine_load = values.engineLoad
    profiler.driveshaft = values.driveshaft or 0
    profiler.boost = values.boost

    profiler.bullet_time = bullettime.get()

    profiler.rpm = values.rpm
    profiler.turbo = values.turboBoost or 0
    profiler.airspeed = values.airspeed
    profiler.wheelspeed = values.wheelspeed
    profiler.fuel = values.fuel or 0

    -- Send UDP Packet
    local packet = ffi.string(profiler, ffi.sizeof(profiler))
    udp_socket:sendto(packet, connection.address, connection.port)
end

-- Game Hooks
---------------------------------------
local function onExtensionLoaded()
    if udp_socket then udp:close() end
    udp_socket = socket.udp()

    udp_socket:sendto(tostring(operations.op_reset), connection.address, connection.port)
    log('I', "onExtensionLoaded", "Sending reset operation")
end

local function onReset()
    if not playerInfo.firstPlayerSeated then return end
    log('I', "onReset", "Sending reset operation")
    udp_socket:sendto(tostring(operations.op_reset), connection.address, connection.port)
end

local function updateGFX(delta)
    dispatch_data(delta)
end

local function onExtensionUnloaded()
    if udp_socket then udp_socket:close() end
end

-- Interface
---------------------------------------
M.onExtensionLoaded = onExtensionLoaded
M.onReset = onReset
M.onPlayersChanged = onReset
M.updateGFX = updateGFX
M.onExtensionUnloaded = onExtensionUnloaded

return M