
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "utils.h"
#include "gl_utils.h"
#include "r_utils/r_string_utils.h"
#include <stdio.h>
#ifdef IS_WINDOWS
#include <windows.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include "roboto.c"
#include <string>
#include <thread>
#include <tray.hpp>

using namespace std;
using namespace r_utils;

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

extern ImGuiContext *GImGui;

struct font_catalog
{
    ImFont* roboto_black;
    ImFont* roboto_black_italic;
    ImFont* roboto_bold;
    ImFont* roboto_bold_italic;
    ImFont* roboto_italic;
    ImFont* roboto_light;
    ImFont* roboto_light_italic;
    ImFont* roboto_medium;
    ImFont* roboto_medium_italic;
    ImFont* roboto_regular;
    ImFont* roboto_thin;
    ImFont* roboto_thin_italic;
};

font_catalog load_fonts(ImGuiIO& io)
{
    font_catalog f;
    ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = false;
    f.roboto_black = io.Fonts->AddFontFromMemoryTTF(Roboto_Black_ttf, Roboto_Black_ttf_len, 18.0, &font_cfg);
    f.roboto_black_italic = io.Fonts->AddFontFromMemoryTTF(Roboto_BlackItalic_ttf, Roboto_BlackItalic_ttf_len, 18.0, &font_cfg);
    f.roboto_bold = io.Fonts->AddFontFromMemoryTTF(Roboto_Bold_ttf, Roboto_Bold_ttf_len, 18.0, &font_cfg);
    f.roboto_bold_italic = io.Fonts->AddFontFromMemoryTTF(Roboto_BoldItalic_ttf, Roboto_BoldItalic_ttf_len, 18.0, &font_cfg);
    f.roboto_italic = io.Fonts->AddFontFromMemoryTTF(Roboto_Italic_ttf, Roboto_Italic_ttf_len, 18.0, &font_cfg);
    f.roboto_light = io.Fonts->AddFontFromMemoryTTF(Roboto_Light_ttf, Roboto_Light_ttf_len, 18.0, &font_cfg);
    f.roboto_light_italic = io.Fonts->AddFontFromMemoryTTF(Roboto_LightItalic_ttf, Roboto_LightItalic_ttf_len, 18.0, &font_cfg);
    f.roboto_medium = io.Fonts->AddFontFromMemoryTTF(Roboto_Medium_ttf, Roboto_Medium_ttf_len, 18.0, &font_cfg);
    f.roboto_medium_italic = io.Fonts->AddFontFromMemoryTTF(Roboto_MediumItalic_ttf, Roboto_MediumItalic_ttf_len, 18.0, &font_cfg);
    f.roboto_regular = io.Fonts->AddFontFromMemoryTTF(Roboto_Regular_ttf, Roboto_Regular_ttf_len, 18.0, &font_cfg);
    f.roboto_thin = io.Fonts->AddFontFromMemoryTTF(Roboto_Thin_ttf, Roboto_Thin_ttf_len, 18.0, &font_cfg);
    f.roboto_thin_italic = io.Fonts->AddFontFromMemoryTTF(Roboto_ThinItalic_ttf, Roboto_ThinItalic_ttf_len, 18.0, &font_cfg);
    return f;
}

void FakeList2(int& selected, float width, const std::string& buttontext)
{
    auto pos = ImGui::GetCursorPos();

    // selectable list
    for (int n = 0; n < 10; n++)
    {
        ImGui::PushID(n);

        auto name = r_string_utils::format("##Object %d", n);

        ImGui::SetCursorPos(ImVec2(pos.x, pos.y));
        if (ImGui::Selectable(name.c_str(), n == selected, 0, ImVec2((float)width, 50))) {
            selected = n;
        }
        ImGui::SetItemAllowOverlap();

        // Calculate width of Camera name label with CalcTextSize()
        // Calculate width of ip address label
        // Compute midpoint between rightmost text end and end of selectable
        // put button at that midpoint

        std::string cameralabel = "Camera " + std::to_string(n);
        std::string iplabel = "192.168.1." + std::to_string(n);

        auto cameralabel_size = ImGui::CalcTextSize(cameralabel.c_str());
        auto iplabel_size = ImGui::CalcTextSize(iplabel.c_str());

        auto max_text_width = (cameralabel_size.x > iplabel_size.x) ? cameralabel_size.x : iplabel_size.x;

        ImGui::SetCursorPos(ImVec2(pos.x+10, pos.y+5));
        ImGui::Text(cameralabel.c_str());

        const int button_width = 70;
        const int button_height = 30;

        auto button_x = (max_text_width + (((pos.x + width) - max_text_width) / 2)) - (button_width / 2);

        ImGui::SetCursorPos(ImVec2(button_x, pos.y+10));
        if(ImGui::Button(buttontext.c_str(), ImVec2(button_width, button_height)))
        {
            ImGui::OpenPopup("Setup?");
            selected = n;
            printf("SETUP CLICKED %d\n", n);
        }

        if (ImGui::BeginPopupModal("Setup?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGuiContext& g = *GImGui;
            ImGuiWindow* window = g.CurrentWindow;
            ImVec2 pos_before = window->DC.CursorPos;

            ImGui::Text("Setup Popup");
            if (ImGui::Button("OK", ImVec2(120, 0))) { printf("OK PRESSED!\n"); ImGui::CloseCurrentPopup(); }
            ImGui::EndPopup();
        }

        ImGui::SetCursorPos(ImVec2(pos.x+10, pos.y+25));
        ImGui::Text(iplabel.c_str());

        pos.y += 55;

        ImGui::PopID();
    }
}

int main(int, char**)
{
    printf("top_dir=%s\n", top_dir().c_str());
    fflush(stdout);

    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    Tray::Tray tray("My Tray", "lightbulb.ico");
    tray.addEntry(Tray::Button("Exit", [&]{
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        tray.exit();
    }));
    tray.addEntry(Tray::Button("Show", [&]{
        glfwShowWindow(window);
    }));
    tray.addEntry(Tray::Button("Hide", [&]{
        glfwHideWindow(window);
    }));

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    auto fc = load_fonts(io);

    //int my_image_width = 0;
    //int my_image_height = 0;
    //GLuint my_image_texture = 0;
    //load_texture_from_file("icons8-hammer-16.png", &my_image_texture, &my_image_width, &my_image_height);

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        tray.pump();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        auto window_size = ImGui::GetIO().DisplaySize;
        auto window_width = window_size.x;
        auto window_height = window_size.y;

        ImGui::PushFont(fc.roboto_regular);

        float client_top = 0.;

        if(ImGui::BeginMainMenuBar())
        {
            if(ImGui::BeginMenu("Filez"))
            {
                if(ImGui::MenuItem("Exit"))
                {
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                }
                ImGui::EndMenu();
            }

            client_top = ImGui::GetWindowHeight();
            ImGui::EndMainMenuBar();
        }

        float left_panel_width = 0.;

        {
            left_panel_width = (float)window_width * 0.25f;
            if(left_panel_width < 250)
                left_panel_width = 250;
            if(left_panel_width > 350)
                left_panel_width = 350;

            float recording_top = (float)client_top;
            ImGui::SetNextWindowPos(ImVec2((float)0, recording_top));
            ImGui::SetNextWindowSize(ImVec2(left_panel_width, (float)(window_height/2)-client_top));
            ImGui::Begin("Recording", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
            //ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Recording");
            static int recording_selected_item = 0;
            FakeList2(recording_selected_item, left_panel_width, "Remove");
            ImGui::End();

            float discovered_top = (float)window_height/2;
            ImGui::SetNextWindowPos(ImVec2((float)0, discovered_top));
            ImGui::SetNextWindowSize(ImVec2(left_panel_width, (float)window_height/2));
            ImGui::Begin("Discovered", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
            //ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Discovered");
            static int discovered_selected_item = 0;
            FakeList2(discovered_selected_item, left_panel_width, "Record");
            ImGui::End();
        }

        float main_width = window_width - left_panel_width;

        {
            ImGui::SetNextWindowPos(ImVec2((float)left_panel_width, (float)client_top));
            ImGui::SetNextWindowSize(ImVec2(main_width, (float)window_height));
            ImGui::Begin("ClientPanel", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
            ImGui::End();
        }

        ImGui::ShowDemoWindow(NULL);

        ImGui::PopFont();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
