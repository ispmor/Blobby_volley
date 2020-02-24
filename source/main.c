//
// Created by puszkarb on 19.02.2020.
//

#include <nds.h>
#include <stdio.h>
#include <assert.h>
#include <maxmod9.h>

#include "sprites.h"
//-----------------------------------------------
// graphic references
//-----------------------------------------------
/*#include "ball.h"*/
#include "background.h"
#include "blobb.h"


#define FRAMES_PER_ANIMATION 1

typedef struct
{
    int x;
    int y;
    int height;
    int xvel;
    int yvel;

    u8 sprite_index;
    u8 sprite_affine_index;

    u16* sprite_gfx_mem;
    u8*  frame_gfx;
    int anim_frame;


}Blobb;


void initBlobb(Blobb *sprite, u8* gfx, int spriteIndex, int affineIndex)
{
    sprite->sprite_gfx_mem = oamAllocateGfx(&oamMain, SpriteSize_64x32, SpriteColorFormat_256Color);

    sprite->frame_gfx = (u8*)gfx;

    // use sprite index 0 (0->127)
    sprite->sprite_index = spriteIndex;

    // use affine matrix 0 (0->31)
    sprite->sprite_affine_index = affineIndex;

    // X = 128.0
    sprite->x = 0 ;

    // Y = 64.0
    sprite->y = 64;

    // start X velocity a bit to the right
    sprite->xvel = 0;

    // reset Y velocity
    sprite->yvel = 0;
}

void animateBlobb(Blobb *sprite)
{
    int frame = sprite->anim_frame;

    u8* offset = sprite->frame_gfx + frame * 64*64;

    dmaCopy(offset, sprite->sprite_gfx_mem, 64*64);
}
// The state of the sprite (which way it is walking)
//---------------------------------------------------------------------
//enum SpriteState {W_UP = 0, W_RIGHT = 1, W_DOWN = 2, W_LEFT = 3};

//---------------------------------------------------------------------
// Screen dimentions
//---------------------------------------------------------------------
enum {SCREEN_TOP = 0, SCREEN_BOTTOM = 192, SCREEN_LEFT = 0, SCREEN_RIGHT = 256};


#define c_radius (8) // the radius of the ball in *.8 fixed point
#define c_diam   16     // the diameter of the ball (integer)
#define c_gravity 1         // gravity constant (add to vertical velocity) (*.8 fixed)
#define c_air_friction 1        //  friction in the air... multiply X velocity by (256-f)/256
#define c_ground_friction 15    //  friction when the ball hits the ground, multiply X by (256-f)/256
#define c_platform_level ((154) )    //  the level of the brick platform in *.8 fixed point
#define c_bounce_damper 100      //  the amount of Y velocity that is absorbed when you hit the ground

#define min_height  (64)   // the minimum height of the ball (when it gets squished) (*.8)
#define min_yvel (5) // the minimum Y velocity (*.8)
#define max_xvel (1000<<4)  // the maximum X velocity (*.12)
//-----------------------------------------------------------------
// clamp integer to range
//-----------------------------------------------------------------
static inline int clampint( int value, int low, int high )
//-----------------------------------------------------------------
{
    if( value < low ) value = low;
    if( value > high) value = high;
    return value;
}


//-----------------------------------------------------------------
// update ball object (call once per frame)
//-----------------------------------------------------------------
void blobbUpdate( Blobb* b ) // Físicas
{

    // add X velocity to X position xvel is 20.12 while x is 24.8
    //b->x += (b->xvel>>2);

    //Gravetat:
    // apply air friction to X velocity
    //b->xvel = (b->xvel * (256-c_air_friction)) >> 16;

    // clamp X velocity to the limits
    //b->xvel = clampint( b->xvel, -max_xvel, max_xvel );

    // add gravity to Y velocity
    b->yvel += c_gravity;
    // add Y velocity to Y position
    b->y += (b->yvel);

    // Bounce on the platform
    if( b->y + c_radius >= c_platform_level )
    {
        // apply ground friction to X velocity
        //b->xvel = (b->xvel * (256-c_ground_friction)) >>   16;

        // check if the ball has been squished to minimum height
        if( b->y > c_platform_level - min_height )
        {
            // mount Y on platform
            b->y = c_platform_level - min_height;

            // negate Y velocity, also apply the bounce damper
            b->yvel = -(b->yvel * (168-c_bounce_damper)) >>  8;

            // clamp Y to mininum velocity (minimum after bouncing, so the ball does not settle)
            /*if( b->yvel > -min_yvel )
                b->yvel = -min_yvel;*/

        }

        // calculate the height
        b->height = (c_platform_level - b->y) * 2;
    }
    else {
        b->height = c_diam <<4;
    }
} // Fi ballUpdate


//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------
	//
    printf("it works?");
    Blobb blobb1 = {0,0};
    Blobb blobb2 = {64,64};
   // irqInit();
    //irqEnable( IRQ_VBLANK );

//  setupGraphics();
    //powerOn(POWER_ALL_2D);
    consoleDemoInit();
    //lcdMainOnBottom();
    initVideo();
    initBackgrounds();

    //SpriteInfo spriteInfo[SPRITE_COUNT];
    //OAMTable *oam;
    //initOAM(oam);
    //initSprites(oam, spriteInfo);*/

    videoSetMode(MODE_5_2D);
    videoSetModeSub(MODE_0_2D);

    vramSetBankA(VRAM_A_MAIN_SPRITE);
    vramSetBankB(VRAM_B_MAIN_BG_0x06000000);
    vramSetBankD(VRAM_D_SUB_SPRITE);

    int bg3 = bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 0,0);
    dmaCopy(backgroundBitmap, bgGetGfxPtr(bg3), 256*256);
    dmaCopy(backgroundPal, BG_PALETTE, 256*2);

    oamInit(&oamMain, SpriteMapping_1D_256, false);
    oamInit(&oamSub, SpriteMapping_1D_256, false);

    initBlobb(&blobb1, (u8*)blobbTiles, 0, 0);;
    dmaCopy(blobbPal, SPRITE_PALETTE, 512);


    while(1) {
        scanKeys();

        int keys = keysHeld();

        if(keys & KEY_START) break;

        if(keys)
        {
            if(keys & KEY_UP)
            {
                if(blobb1.y >= SCREEN_TOP) {
                    //blobb1.y--;
                    blobb1.yvel -= 1;
                }              //  blobb.state = W_UP;
            }
            if(keys & KEY_LEFT)
            {
                if(blobb1.x >= SCREEN_LEFT) blobb1.x -= 2;
                //blobb.state = W_LEFT;
            }
            if(keys & KEY_RIGHT)
            {
                if(blobb1.x <= SCREEN_RIGHT  / 2  - 64) blobb1.x += 2;

                //blobb.state = W_RIGHT;
            }
            if(keys & KEY_DOWN)
            {
                if(blobb1.y <= SCREEN_BOTTOM) {
                    blobb1.y++;
                    blobb1.yvel++;
                }

                //blobb.state = W_DOWN;
            }

            if(blobb1.anim_frame >= FRAMES_PER_ANIMATION) blobb1.anim_frame = 0;


        }
        blobbUpdate(&blobb1);

        animateBlobb(&blobb1);

        //-----------------------------------------------------------------
        // Set oam attributes, notice the only difference is in the sprite 
        // graphics memory pointer argument.  The blobb only has one pointer
        // while the women has an array of pointers
        //-----------------------------------------------------------------
        oamSet(&oamMain, 0, blobb1.x, blobb1.y, 0, 0, SpriteSize_64x64, SpriteColorFormat_256Color,
               blobb1.sprite_gfx_mem, -1, false, false, false, false, false);

        swiWaitForVBlank();

        oamUpdate(&oamMain);
        oamUpdate(&oamSub);
    }
    return 0;
}





/*
void setupGraphics( void ) {
    vramSetBankE(VRAM_E_MAIN_BG);
    vramSetBankF(VRAM_F_MAIN_SPRITE);

    int n;
    for (n = 0; n < 16; n++){
        BG_GFX[n] = 0;
    }

    dmaCopyHalfWords( 3, backgroundTiles, tile2bgram(tile_brick), backgroundTilesLen);
    dmaCopyHalfWords( 3, blobbTiles, tile2bgram(tile_blobb), blobbTilesLen );

    // palettes goto palette memory
    dmaCopyHalfWords( 3, blobbPal, pal2bgram(pal_blobb), blobbPalLen );
    dmaCopyHalfWords( 3, backgroundPal, pal2bgram(pal_background), backgroundPalLen );
// set backdrop color
    BG_PALETTE[0] = backdrop_colour;

    // libnds prefixes the register names with REG_
    REG_BG0CNT = BG_MAP_BASE(1);
    REG_BG1CNT = BG_MAP_BASE(2);

}
*/

static const int BOUNDARY_VALUE = 32;
static const int OFFSET_MULTIPLIER = BOUNDARY_VALUE / sizeof(SPRITE_GFX[0]);

void initVideo(void ){
    vramSetMainBanks(VRAM_A_MAIN_BG_0x06000000,
                     VRAM_B_MAIN_BG_0x06020000,
                     VRAM_C_SUB_BG_0x06200000,
                     VRAM_D_LCD);

    /*  Set the video mode on the main screen. */
    videoSetMode(MODE_5_2D | // Set the graphics mode to Mode 5
                 DISPLAY_BG2_ACTIVE | // Enable BG2 for display
                 DISPLAY_BG3_ACTIVE | //Enable BG3 for display
                 DISPLAY_SPR_ACTIVE | //Enable Sprites display
//                 DISPLAY_BG0_ACTIVE |
                 DISPLAY_SPR_1D // enable 1D tiled sprites
                 );

    /*  Set the video mode on the sub screen. */
    videoSetModeSub(MODE_0_2D | // Set the graphics mode to Mode 5
                    DISPLAY_BG0_ACTIVE); // Enable BG3 for display
}

void initBackgrounds() {
    /*  Set up affine background 3 on main as a 16-bit color background. */
    REG_BG3CNT = BG_BMP16_256x256 |
                 BG_BMP_BASE(0) | // The starting place in memory
                 BG_PRIORITY(3); // A low priority

    /*  Set the affine transformation matrix for the main screen background 3
     *  to be the identity matrix.
     */
    REG_BG3PA = 1 << 8;
    REG_BG3PB = 0;
    REG_BG3PC = 0;
    REG_BG3PD = 1 << 8;

    /*  Place main screen background 3 at the origin (upper left of the
     *  screen).
     */
    REG_BG3X = 0;
    REG_BG3Y = 0;

    /*  Set up affine background 2 on main as a 16-bit color background. */
    REG_BG2CNT = BG_BMP16_128x128 |
                 BG_BMP_BASE(8) | // The starting place in memory
                 BG_PRIORITY(2);  // A higher priority

    /*  Set the affine transformation matrix for the main screen background 3
     *  to be the identity matrix.
     */
    REG_BG2PA = 1 << 8;
    REG_BG2PB = 0;
    REG_BG2PC = 0;
    REG_BG2PD = 1 << 8;

    /*  Place main screen background 2 in an interesting place. */
    REG_BG2X = -(SCREEN_WIDTH / 2 - 32) << 8;
    REG_BG2Y = -32 << 8;

    /*  Set up affine background 3 on the sub screen as a 16-bit color
     *  background.
     */
    REG_BG3CNT_SUB = BG_BMP16_256x256 |
                     BG_BMP_BASE(0) | // The starting place in memory
                     BG_PRIORITY(3); // A low priority

    /*  Set the affine transformation matrix for the sub screen background 3
     *  to be the identity matrix.
     */
    REG_BG3PA_SUB = 1 << 8;
    REG_BG3PB_SUB = 0;
    REG_BG3PC_SUB = 0;
    REG_BG3PD_SUB = 1 << 8;

    /*
     *  Place main screen background 3 at the origin (upper left of the screen)
     */
    REG_BG3X_SUB = 0;
    REG_BG3Y_SUB = 0;
}

void initSprites(OAMTable *oam, SpriteInfo *spriteInfo){
    static const int BYTES_PER_16_COLOR_TILE = 32;
    static const int COLORS_PER_PALETTE = 16;

    int nextAvailableTileIdx = 0;

    static const int BLOBB1_OAM_ID = 0;
    assert(BLOBB1_OAM_ID < SPRITE_COUNT);
    SpriteInfo *blobb1Info = &spriteInfo[BLOBB1_OAM_ID];
    SpriteEntry * blobb1 = &oam->oamBuffer[BLOBB1_OAM_ID];

    blobb1Info->oamId = BLOBB1_OAM_ID;
    blobb1Info->width = 64;
    blobb1Info->height = 64;
    blobb1Info->angle = 462;
    blobb1Info->entry = blobb1;

    blobb1->y = SCREEN_HEIGHT / 2 - blobb1Info->height;
    blobb1->isRotateScale = true;

    assert(!blobb1->isRotateScale || (blobb1Info->oamId < MATRIX_COUNT));
    blobb1->isSizeDouble = false;
    blobb1->blendMode = OBJMODE_NORMAL;
    blobb1->isMosaic = false;
    blobb1->colorMode = OBJCOLOR_16;
    blobb1->shape = OBJSHAPE_SQUARE;

    blobb1->x = SCREEN_WIDTH / 2 - blobb1Info->width * 2 + blobb1Info->width / 2;
    blobb1->rotationIndex = blobb1Info->oamId;
    blobb1->size = OBJSIZE_64;
    blobb1->gfxIndex = nextAvailableTileIdx;
    nextAvailableTileIdx += blobbTilesLen / BYTES_PER_16_COLOR_TILE;
    blobb1->priority = OBJPRIORITY_0;
    blobb1->palette = blobb1Info->oamId;

    rotateSprite(&oam->matrixBuffer[blobb1Info->oamId], blobb1Info->angle);

    printf("x %d | y %d | hidden %d", blobb1->x, blobb1->y, blobb1->isHidden);

    dmaCopyHalfWords(SPRITE_DMA_CHANNEL, blobbPal, &SPRITE_PALETTE[blobb1Info->oamId * COLORS_PER_PALETTE], blobbPalLen);

    dmaCopyHalfWords(SPRITE_DMA_CHANNEL, blobbTiles, &SPRITE_GFX[blobb1->gfxIndex * OFFSET_MULTIPLIER],blobbTilesLen);

    setSpriteVisibility(spriteInfo->entry, false, false, false);
}


/*void blobbRender( SpriteInfo *b, int camera_x, int camera_y )
{
    u16* sprite = OAM + b->sprite_index * 4; // Cada entrà de la taula de OAM ocupa 4 u16

    int x, y;
    x = ((b->x - c_radius*2) >> 8) - camera_x;
    y = ((b->y - c_radius*2) >> 8) - camera_y;

    if( x <= -16 || y <= -16 || x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT )
    {
        oamSetHidden( &oamMain, b->sprite_index, true);
        //return;
    }else{
        sprite[0] = (y & 255) | ATTR0_ROTSCALE_DOUBLE;
        sprite[1] = (x & 511) | ATTR1_SIZE_16 | ATTR1_ROTDATA( b->sprite_affine_index );
        sprite[2] = 0;

        u16* affine;
        affine = OAM + b->sprite_affine_index * 16 + 3;
        affine[4] = 0;
        affine[8] = 0;

        int pa = (b->height * (65536/c_diam)) >> 16;
        int pd = 65536 / pa;
        affine[0] = pa;
        affine[12] = pd;
    }

}




/* Select a low priority DMA channel to perform our background
 * copying. */
static const int DMA_CHANNEL = 3;

void displayBackground() {
    dmaCopyHalfWords(DMA_CHANNEL,
                     backgroundBitmap, /* This variable is generated for us by
                                       * grit. */
                     (uint16 *)BG_BMP_RAM(0), /* Our address for main
                                               * background 3 */
                     backgroundBitmapLen); /* This length (in bytes) is generated
                                           * from grit. */
}

