#include "gui/Panorama.h"

static C3D_Tex panoramaTex[4];
float panoPositions[6] = {
	0,
	128,
	256,
	384,
	512,
	640
};
float PanoMoveSpeed = 2.5f;

void loadPanorama(){
    Texture_Load(&panoramaTex[0], "romfs:/textures/gui/title/background/panorama_0.png");
	Texture_Load(&panoramaTex[1], "romfs:/textures/gui/title/background/panorama_1.png");
	Texture_Load(&panoramaTex[2], "romfs:/textures/gui/title/background/panorama_2.png");
	Texture_Load(&panoramaTex[3], "romfs:/textures/gui/title/background/panorama_3.png");
}

void unloadPanorama(){
	C3D_TexDelete(&panoramaTex[0]);
	C3D_TexDelete(&panoramaTex[1]);
	C3D_TexDelete(&panoramaTex[2]);
	C3D_TexDelete(&panoramaTex[3]);
}

void renderPanorama(){
    SpriteBatch_BindTexture(&panoramaTex[0]);
	SpriteBatch_PushQuad(panoPositions[0], 0, -1, 128, 128, 0, 0, 256, 256);

	SpriteBatch_BindTexture(&panoramaTex[1]);
	SpriteBatch_PushQuad(panoPositions[1], 0, -1, 128, 128, 0, 0, 256, 256);

	SpriteBatch_BindTexture(&panoramaTex[2]);
	SpriteBatch_PushQuad(panoPositions[2], 0, -1, 128, 128, 0, 0, 256, 256);

	SpriteBatch_BindTexture(&panoramaTex[3]);
	SpriteBatch_PushQuad(panoPositions[3], 0, -1, 128, 128, 0, 0, 256, 256);
			
	SpriteBatch_BindTexture(&panoramaTex[0]);
	SpriteBatch_PushQuad(panoPositions[4], 0, -1, 128, 128, 0, 0, 256, 256);

	SpriteBatch_BindTexture(&panoramaTex[1]);
	SpriteBatch_PushQuad(panoPositions[5], 0, -1, 128, 128, 0, 0, 256, 256);

    

    
	if (PanoMoveSpeed <= 0){
		for (int p = 0; p < 6; p++){
            panoPositions[p] -= 1.f;
        }
		PanoMoveSpeed = 1.f;
	}
	else{
		PanoMoveSpeed -= 0.5f;
	}
    

	if (panoPositions[4] == 0){
		panoPositions[0] = 0;
		panoPositions[1] = 128;
		panoPositions[2] = 256;
		panoPositions[3] = 384;
		panoPositions[4] = 512;
		panoPositions[5] = 640;
	}
}