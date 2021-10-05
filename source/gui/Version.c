#include <gui/Version.h>

void renderVersion(){
    Gui_Offset(-63.f, 110.f);
    Gui_BeginRow(SpriteBatch_GetWidth(), 1);

    Gui_Label(0.f, true, INT16_MAX, true, CRAFTUS_VERSION_STR);

    Gui_EndRow();
}