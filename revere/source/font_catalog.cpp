
#include "font_catalog.h"

#include "roboto.c"

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
