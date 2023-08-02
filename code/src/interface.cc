// Fun fact, this file got corrupted because shitty windows decided to freeze. Had to use the debug symbols and IDA to piece it back together... fun, right??? no.
// It will probably happen again because I'm using Windows, all of my life issues are because of Windows.

#include "beamprofiler.h"
#include "interface.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <SDL_opengl.h>
#include <SDL.h>
#include <imgui.h>
#include <implot.h>
#include <cstdio>

// Utility
//------------------------------------------------------------------------
ImVec2 find_window_pos(HWND foreground_window, const uint32_t width, const uint32_t height)
{
    // On Windows, we want to find the previous window and put it on that monitor.
    // Say you have the game on the left monitor and file explorer on the right,
    // if you double click this in the file explorer, you want it to open on that monitor. Well, I do anyways.
    if (!foreground_window)
        return ImVec2(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    HMONITOR monitor = MonitorFromWindow(foreground_window, MONITOR_DEFAULTTONEAREST);

    MONITORINFO monitor_info;
    monitor_info.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(monitor, &monitor_info);

    float center_x = (float)(monitor_info.rcWork.left + monitor_info.rcWork.right - width) / 2;
    float center_y = (float)(monitor_info.rcWork.top + monitor_info.rcWork.bottom - height) / 2;

    return ImVec2(center_x, center_y);
}

// Graph Functions
//------------------------------------------------------------------------
void graph_add_line(graph_element_t* elem, graph_element_line_t* line, const bp::string& name, float* value)
{
    line->name = name;
    line->value = value;
    elem->lines.push_back(line);
}

void draw_graph_line(graph_element_line_t* line, float history, float time)
{
    line->rb.span = history;
    line->rb.add_point(time, *line->value);
    ImPlot::PlotLine(line->name.c_str(), &line->rb.data[0].x, &line->rb.data[0].y, line->rb.data.size(), 0, 0, 8);
}

const int flags = 6145; // TODO: Figure out the flags I used, had to grab them from IDA when this file got corrupted.

void draw_graph_element(graph_element_t elem, float history, float time, ImVec2 size = ImVec2(0, 0), int xflags = flags, int yflags = flags, int cond = ImGuiCond_None)
{
    if (ImPlot::BeginPlot(elem.graph_name.c_str(), size, ImPlotFlags_NoMouseText))
    {
        ImPlot::SetupAxes(nullptr, nullptr, xflags, yflags);
        ImPlot::SetupAxisLimits(ImAxis_X1, 0, history, ImGuiCond_Always);
        ImPlot::SetupAxisLimits(ImAxis_Y1, elem.min, elem.max, 2);

        for (auto& line : elem.lines)
            draw_graph_line(line, history, time);

        ImPlot::EndPlot();
    }
}

// UI Thread
//------------------------------------------------------------------------
void interface_thread(void* udata)
{
    render_thread_data_t* render_data = (render_thread_data_t*)udata;
    profiler_t* profiler_data = &render_data->data;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    int window_width = 800;
    int window_height = 600;

    ImVec2 pos;
#if defined(_WIN32)
    pos = find_window_pos(render_data->foreground_window, window_width, window_height);
#else
    pos ImVec2(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
#endif

    SDL_Window* window = SDL_CreateWindow("BeamProfiler", (int)pos.x, (int)pos.y, window_width, window_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext gl_ctx = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_ctx);
    SDL_GL_SetSwapInterval(1);

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    ImGui::StyleColorsClassic();

    ImGui_ImplSDL2_InitForOpenGL(window, gl_ctx);
    ImGui_ImplOpenGL2_Init();

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Render variables
    uint32_t target_fps = render_data->config.fps_limit;
    uint32_t target_frame_time = 1000 / target_fps;
    Uint32 frame_time = 0;
    int frame_count = 0;
    float fps = 0;
    Uint32 prev_time = SDL_GetTicks();

    // RPM
    graph_element_line_t rpm_line;
    graph_element_t elem_rpm("RPM");
    graph_add_line(&elem_rpm, &rpm_line, "RPM", &profiler_data->rpm);

    // Temperatures
    graph_element_line_t temp_line_eng, temp_line_oil, temp_line_engine_block, temp_line_engine_wall, temp_coolant /*, temp_radiator_speed , temp_exhaust*/;
    graph_element_t elem_temps("Temperatures");
    graph_add_line(&elem_temps, &temp_line_eng, "Water", &profiler_data->thermals.water);
    graph_add_line(&elem_temps, &temp_line_oil, "Engine Block", &profiler_data->thermals.engine_block);
    graph_add_line(&elem_temps, &temp_line_engine_block, "Engine Wall", &profiler_data->thermals.engine_wall);
    graph_add_line(&elem_temps, &temp_line_engine_wall, "Oil", &profiler_data->thermals.oil);
    graph_add_line(&elem_temps, &temp_coolant, "Coolant", &profiler_data->thermals.coolant);

    // Inputs
    graph_element_line_t input_line_throttle, input_line_brake, input_line_clutch;
    graph_element_t elem_inputs("Inputs");

    graph_add_line(&elem_inputs, &input_line_throttle, "Throttle", &profiler_data->inputs.throttle);
    graph_add_line(&elem_inputs, &input_line_brake, "Clutch", &profiler_data->inputs.clutch);
    graph_add_line(&elem_inputs, &input_line_clutch, "Brake", &profiler_data->inputs.brake);

    // Speed
    graph_element_line_t line_wheelspeed, line_airspeed;
    graph_element_t elem_speed("Speed");
    graph_add_line(&elem_speed, &line_wheelspeed, "Wheelspeed (m/s)", &profiler_data->wheelspeed);
    graph_add_line(&elem_speed, &line_airspeed, "Airspeed (m/s)", &profiler_data->airspeed);

    float time = 0;
    float old_time = 0.0f; // old_history is used when resetting or changing vehicles
    float history = render_data->config.history;
    float paused_time = 0.0f;

    ImPlotContext* plot_ctx = ImPlot::CreateContext();
    while (!render_data->request_close)
    {
        Uint32 frame_start = SDL_GetTicks();

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
            {
                render_data->request_close = true;
            } else if (event.type == SDL_WINDOWEVENT)
            {
                if (event.window.type == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    window_width = event.window.data1;
                    window_height = event.window.data2;
                }
            }
        }

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        if (render_data->connected)
            time += ImGui::GetIO().DeltaTime * profiler_data->bullet_time;

        // Check queue
        {
            render_data->queue_mtx.lock();
            const size_t queue_len = render_data->queue.size();
            if (queue_len > 0)
            {
                const char op = render_data->queue.back();
                switch ((operation_e)op)
                {
                    case operation_e::op_reset:
                    {
                        // Reset graphs
                        old_time = time;
                        time = history;
                        render_data->queue.pop_back();
                        break;
                    }
                }
            }
            render_data->queue_mtx.unlock();
        }

        {
            ImGui::SetNextWindowSize(ImVec2((float)window_width, (float)window_height));
            ImGui::SetNextWindowPos(ImVec2(0, 0));

            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

            if (ImGui::Begin("##beam_profiler_root", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus))
            {
                ImGui::PopStyleVar(); // ImGuiStyleVar_WindowBorderSize

                if (ImGui::BeginMainMenuBar())
                {
                    if (ImGui::MenuItem("Exit"))
                        render_data->request_close = true;

                    if (ImGui::BeginMenu("Options"))
                    {
                        if (ImGui::SliderFloat("History", &history, 1, 30, "%.1f s"))
                        {
                            // I could actually limit this (ctrl+left click to bypass), but there's no need.
                            // I had to do it with the FPS limit though, setting it to 0 = crash

                            old_time = time; // Reset the current graphs
                            time = history;

                            render_data->config.history = history;
                            config_save(&render_data->config);
                        }

                        if (ImGui::SliderInt("FPS Limit", &(int)target_fps, 15, 240))
                        {
                            if (target_fps >= 15 && target_fps <= 240)
                            {
                                target_frame_time = 1000 / target_fps;

                                render_data->config.fps_limit = target_fps;
                                config_save(&render_data->config);
                            } else
                            {
                                target_fps = render_data->config.fps_limit;
                            }
                        }

                        { // Connection
                            ImGui::Text("Connection");
                            ImGui::Separator();

                            { // Input Address
                                ImGui::Text("Address");
                                ImGui::PushItemWidth(40);
                                ImGui::InputScalar("##ip_part1", ImGuiDataType_U8, &render_data->config.ip[0], nullptr, nullptr, "%u");
                                ImGui::SameLine();
                                ImGui::Text(".");
                                ImGui::SameLine();

                                ImGui::InputScalar("##ip_part2", ImGuiDataType_U8, &render_data->config.ip[1], nullptr, nullptr, "%u");
                                ImGui::SameLine();
                                ImGui::Text(".");
                                ImGui::SameLine();

                                ImGui::InputScalar("##ip_part3", ImGuiDataType_U8, &render_data->config.ip[2], nullptr, nullptr, "%u");
                                ImGui::SameLine();
                                ImGui::Text(".");
                                ImGui::SameLine();

                                ImGui::InputScalar("##ip_part4", ImGuiDataType_U8, &render_data->config.ip[3], nullptr, nullptr, "%u");
                                ImGui::PopItemWidth();
                            }

                            { // Input Port
                                ImGui::InputInt("Port", &render_data->config.port);
                            }

                            if (ImGui::Button("Connect"))
                            {
                                render_data->requested_connect = true;
                                config_save(&render_data->config);
                            }
                        }

                        ImGui::EndMenu();
                    }

                    { // Connected
                        bool connected = render_data->connected;
                        if (!connected)
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));

                        ImGui::MenuItem(connected ? "Connected" : "Not Connected");
                        if (!connected)
                            ImGui::PopStyleColor();
                    }

                    { // Time Elapsed
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.49f, 0.68f, 0.64f, 1.0f));
                        ImGui::MenuItem(bp::string::format("Elapsed: %.2fs", time).c_str());
                        ImGui::MenuItem(bp::string::format("Frame Time: %d", frame_time).c_str());
                        ImGui::MenuItem(bp::string::format("Framerate: %d", (int)fps).c_str());
                        ImGui::PopStyleColor();
                    }

                    if (profiler_data->bullet_time > 0.0f && profiler_data->bullet_time < 1.0f)
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.49f, 0.68f, 0.64f, 1.0f));
                        ImGui::MenuItem(bp::string::format("Bullet Time: %.2f", profiler_data->bullet_time).c_str());
                        ImGui::PopStyleColor();
                    }

                    ImGui::EndMainMenuBar();
                }

                ImGui::SetCursorPosY(30);

                {
                    const float avail_x = ImGui::GetContentRegionAvail().x;

                    // Draw graphs
                    draw_graph_element(elem_rpm, history, time, ImVec2(avail_x, 0));
                    draw_graph_element(elem_temps, history, time, ImVec2(avail_x / 2, 0));

                    ImGui::SameLine();

                    draw_graph_element(elem_inputs, history, time, ImVec2(avail_x / 2, 0));
                    draw_graph_element(elem_speed, history, time, ImVec2(avail_x, 0));
                }

                if (time == history) // We reset or changed vehicle
                {
                    time = old_time;
                    if (time > history)
                    {
                        int num_resets = (int)(time / history);
                        time = num_resets * history;
                    } else
                    {
                        time = 0;
                    }
                }
                ImGui::End();
            }
        }

        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);

        { // FPS Lock
            Uint32 frame_end = SDL_GetTicks();
            frame_time = frame_end - frame_start;
            if (frame_time < target_frame_time)
            {
                Uint32 delay_time = target_frame_time - frame_time;
                SDL_Delay(delay_time);
            }

            frame_count++;
            Uint32 currentTime = SDL_GetTicks();
            Uint32 deltaTime = currentTime - prev_time;
            if (deltaTime >= 500)
            {
                fps = (float)frame_count / (deltaTime / 1000.0f);
                frame_count = 0;
                prev_time = currentTime;
            }
        }
    }

    ImPlot::DestroyContext(plot_ctx);

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
}