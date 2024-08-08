#include <gui/GameSplash.h>

static C3D_Tex splashTex;
int duration = 500;
static GameState* gamestate;
bool unloaded = false;

void loadSplash(){
    Texture_Load(&splashTex, "romfs:/textures/gui/title/mojang.png");
}

void unloadSplash(){
    C3D_TexDelete(&splashTex);
}

void renderSplash(){
    if (duration <= 0){
        *gamestate = GameState_SelectWorld;
        unloadSplash();
    }
    else{
        SpriteBatch_BindTexture(&splashTex);
        SpriteBatch_PushQuad(35, 0, 0, 128, 128, 0, 0, 256, 256);

        duration -= 1;
    }
}