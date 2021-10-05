#include <gui/SplashText.h>

int randomSplash;
char* SPLASH_TEXT;

void loadSplashText(){
    srand ( time(NULL) );
    randomSplash = (rand() % 348);
    SPLASH_TEXT = SPLASH_ARRAY[randomSplash];
}

void renderSplashText(){
    Gui_Offset(1, 35);
    Gui_BeginRow(SpriteBatch_GetWidth(), 1);

    Gui_Label(0.f, true, 0xFFE0, true, SPLASH_TEXT);

    Gui_EndRow();
}