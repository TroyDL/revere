
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "utils.h"
#include <stdio.h>
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "roboto.c"
#include <string>
#include <thread>
#include <tray.hpp>

// Use Shell_NotifyIconA() to display a tray icon
// https://docs.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shell_notifyicona
//
// NOTIFYICONDATA structure sent in the call to Shell_NotifyIcon contains information that specifies both the notification
// area icon and the notification itself.
//
// Each icon in the notification area can be identified by the GUID with which the icon is declared in the registry. This is
// the preferred method on Windows 7 and later.
//
// Icons in the notification area can have a tooltip. The tooltip can be either a standard tooltip (preferred) or an
// application-drawn, pop-up UI.
//
// Notification area icons should be high-DPI aware. An application should provide both a 16x16 pixel icon and a 32x32 icon
// in its resource file, and then use LoadIconMetric to ensure that the correct icon is loaded and scaled appropriately.
//
// The application responsible for the notification area icon should handle a mouse click for that icon. When a user
// right-clicks the icon, it should bring up a normal shortcut menu.
//
// The icon can be added to the notification area without displaying a notification by defining only the icon-specific
// members of NOTIFYICONDATA (discussed above) and calling Shell_NotifyIcon as shown here:
//
//     NOTIFYICONDATA nid = {};
//     Do NOT set the NIF_INFO flag.
//     ...                    
//     Shell_NotifyIcon(NIM_ADD, &nid);
//
// 

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

extern ImGuiContext *GImGui;

namespace ImGui
{
bool BeginMainToolBar();
void EndMainToolBar();
}

bool ImGui::BeginMainToolBar()
{
    ImGuiContext& g = *GImGui;
    ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)GetMainViewport();

    // For the main menu bar, which cannot be moved, we honor g.Style.DisplaySafeAreaPadding to ensure text can be visible on a TV set.
    // FIXME: This could be generalized as an opt-in way to clamp window->DC.CursorStartPos to avoid SafeArea?
    // FIXME: Consider removing support for safe area down the line... it's messy. Nowadays consoles have support for TV calibration in OS settings.
    g.NextWindowData.MenuBarOffsetMinVal = ImVec2(g.Style.DisplaySafeAreaPadding.x, ImMax(g.Style.DisplaySafeAreaPadding.y - g.Style.FramePadding.y, 0.0f));
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
    float height = GetFrameHeight();
    bool is_open = BeginViewportSideBar("##MainToolBar", viewport, ImGuiDir_Up, height, window_flags);
    g.NextWindowData.MenuBarOffsetMinVal = ImVec2(0.0f, 0.0f);

    if (is_open)
        BeginMenuBar();
    else
        End();
    return is_open;
}

void ImGui::EndMainToolBar()
{
    EndMenuBar();

    // When the user has left the menu layer (typically: closed menus through activation of an item), we restore focus to the previous window
    // FIXME: With this strategy we won't be able to restore a NULL focus.
    ImGuiContext& g = *GImGui;
    if (g.CurrentWindow == g.NavWindow && g.NavLayer == ImGuiNavLayer_Main && !g.NavAnyRequest)
        FocusTopMostWindowUnderOne(g.NavWindow, NULL);

    End();
}

#define GL_CLAMP_TO_EDGE 0x812F 

// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

void FakeList(int& selected)
{
    // selectable list
    for (int n = 0; n < 25; n++)
    {
        // You may call Selectable() first which a non-visible label ("##title") and specify a custom size and then
        // draw your stuff over it. You'll need to know the size ahead and call the imgui_internal.h text drawing
        // functions that handle clipping.
        //
        // You can set a custom widget size with PushItemWidth() and PushItemHeight()


        ImGui::PushID(n);
        char buf[32];
        sprintf(buf, "Object %d", n);
        if (ImGui::Selectable(buf, selected == n, 0, ImVec2(0, 20)))
            selected = n;
        ImGui::SetItemAllowOverlap();
        ImGui::SameLine();
        static const ImVec4 main_color { 0.3828f, 0.0f, 0.6992f, 1.0f };
        static const ImVec4 hover_color { 0.2148f, 0.0f, 0.9333f, 1.0f };
        ImGui::PushStyleColor(ImGuiCol_Button, main_color);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hover_color);

        auto pos = ImGui::GetCursorPos();
        pos.y -= 2;
        ImGui::SetCursorPos(pos);

        if(ImGui::Button("do thing"))
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

        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopID();
    }
}

void FakeList2(int& selected, int width, const std::string& buttontext)
{
    auto pos = ImGui::GetCursorPos();

    // selectable list
    for (int n = 0; n < 10; n++)
    {
        ImGui::PushID(n);

        char buf[32];
        sprintf(buf, "##Object %d", n);

        ImGui::SetCursorPos(ImVec2(pos.x, pos.y));
        if (ImGui::Selectable(buf, n == selected, 0, ImVec2(width, 50))) {
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

//    auto tray_th = std::thread([&]{
//        tray.run();
//    });
//    tray_th.detach();

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

    ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = false;
    auto roboto_black = io.Fonts->AddFontFromMemoryTTF(Roboto_Black_ttf, Roboto_Black_ttf_len, 18.0, &font_cfg);
    auto roboto_black_italic = io.Fonts->AddFontFromMemoryTTF(Roboto_BlackItalic_ttf, Roboto_BlackItalic_ttf_len, 18.0, &font_cfg);
    auto roboto_bold = io.Fonts->AddFontFromMemoryTTF(Roboto_Bold_ttf, Roboto_Bold_ttf_len, 18.0, &font_cfg);
    auto roboto_bold_italic = io.Fonts->AddFontFromMemoryTTF(Roboto_BoldItalic_ttf, Roboto_BoldItalic_ttf_len, 18.0, &font_cfg);
    auto roboto_italic = io.Fonts->AddFontFromMemoryTTF(Roboto_Italic_ttf, Roboto_Italic_ttf_len, 18.0, &font_cfg);
    auto roboto_light = io.Fonts->AddFontFromMemoryTTF(Roboto_Light_ttf, Roboto_Light_ttf_len, 18.0, &font_cfg);
    auto roboto_light_italic = io.Fonts->AddFontFromMemoryTTF(Roboto_LightItalic_ttf, Roboto_LightItalic_ttf_len, 18.0, &font_cfg);
    auto roboto_medium = io.Fonts->AddFontFromMemoryTTF(Roboto_Medium_ttf, Roboto_Medium_ttf_len, 18.0, &font_cfg);
    auto roboto_medium_italic = io.Fonts->AddFontFromMemoryTTF(Roboto_MediumItalic_ttf, Roboto_MediumItalic_ttf_len, 18.0, &font_cfg);
    auto roboto_regular = io.Fonts->AddFontFromMemoryTTF(Roboto_Regular_ttf, Roboto_Regular_ttf_len, 18.0, &font_cfg);
    auto roboto_thin = io.Fonts->AddFontFromMemoryTTF(Roboto_Thin_ttf, Roboto_Thin_ttf_len, 18.0, &font_cfg);
    auto roboto_thin_italic = io.Fonts->AddFontFromMemoryTTF(Roboto_ThinItalic_ttf, Roboto_ThinItalic_ttf_len, 18.0, &font_cfg);

    //int my_image_width = 0;
    //int my_image_height = 0;
    //GLuint my_image_texture = 0;
    //bool ret = LoadTextureFromFile("icons8-hammer-16.png", &my_image_texture, &my_image_width, &my_image_height);
    //IM_ASSERT(ret);

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

        ImGui::PushFont(roboto_regular);

        double client_top = 0.;

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
