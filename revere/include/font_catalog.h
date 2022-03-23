
#ifndef __revere_font_catalog_h
#define __revere_font_catalog_h

#include "imgui.h"

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

font_catalog load_fonts(ImGuiIO& io);

#endif
