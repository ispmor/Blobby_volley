//
// Created by puszkarb on 19.02.2020.
//
#include <nds.h>
#include <nds/arm9/trig_lut.h>
#include "sprites.h"


void updateOAM(OAMTable *oam){
    DC_FlushAll();
    dmaCopyHalfWords(SPRITE_DMA_CHANNEL, oam->oamBuffer, OAM, SPRITE_COUNT * sizeof(SpriteEntry));
}

void initOAM(OAMTable *oam){
    for (int i = 0; i < SPRITE_COUNT; i++) {
        oam->oamBuffer[i].attribute[0] = ATTR0_DISABLED;
        oam->oamBuffer[i].attribute[1] = 0;
        oam->oamBuffer[i].attribute[2] = 0;
    }

    for( int i = 0; i < MATRIX_COUNT; i++) {
        oam->matrixBuffer[i].hdx = 1 << 8;
        oam->matrixBuffer[i].hdy = 0;
        oam->matrixBuffer[i].vdx = 0;
        oam->matrixBuffer[i].vdy = 1 << 8;
    }

    updateOAM(oam);
}

void rotateSprite(SpriteRotation *spriteRotation, int angle){
    s16 s = sinLerp(angle) >> 4;
    s16 c = cosLerp(angle) >> 4;

    spriteRotation->hdx = c;
    spriteRotation->hdy = s;
    spriteRotation->vdx = -s;
    spriteRotation->vdy = c;
}

void setSpriteVisibility( SpriteEntry *spriteEntry, bool hidden, bool affine, bool doubleBound) {
    if (hidden) {
        spriteEntry->isRotateScale = false; //Turn off bit 9
        spriteEntry->isHidden = true; //turn on bit 8
    } else {
        if(affine) {
            spriteEntry->isRotateScale = true;
            spriteEntry->isSizeDouble = doubleBound;
        } else {
            spriteEntry->isHidden = false;
        }
    }
}

