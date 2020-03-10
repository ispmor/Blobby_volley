//
// Created by puszkarb on 19.02.2020.
//

#include <nds.h>
#include <stdio.h>
#include <assert.h>
#include <maxmod9.h>
#include <math.h>


#include "sprites.h"

#include "ball.h"
#include "background.h"
#include "blobb.h"


typedef struct {
    int x;
    int y;
    int height;
    int xvel;
    int yvel;

    u8 sprite_index;
    u8 sprite_affine_index;
    SpriteEntry *oam;

    u16 *sprite_gfx_mem;
    u8 *frame_gfx;
    int anim_frame;


} Sprite;


Sprite blobb1 = {0, 0};
Sprite blobb2 = {192, 64};

Sprite ball;
int paused = -1;

void initSprite(Sprite *sprite, int x, int y, u8 *gfx, int spriteIndex, int affineIndex) {
    sprite->sprite_gfx_mem = oamAllocateGfx(&oamMain, SpriteSize_64x64, SpriteColorFormat_256Color);

    sprite->frame_gfx = (u8 *) gfx;

    // use sprite index 0 (0->127)
    sprite->sprite_index = spriteIndex;

    // use affine matrix 0 (0->31)
    sprite->sprite_affine_index = affineIndex;

    // X = 128.0
    sprite->x = x;

    // Y = 64.0
    sprite->y = y;

    // start X velocity a bit to the right
    sprite->xvel = 0;

    // reset Y velocity
    sprite->yvel = 0;
}


void animateSprite(Sprite *sprite) {
    int frame = sprite->anim_frame;

    u8 *offset = sprite->frame_gfx + frame * 64 * 64;

    dmaCopy(offset, sprite->sprite_gfx_mem, 64 * 64);
}


// The state of the sprite (which way it is walking)
//---------------------------------------------------------------------
//enum SpriteState {W_UP = 0, W_RIGHT = 1, W_DOWN = 2, W_LEFT = 3};

//---------------------------------------------------------------------
// Screen dimentions
//---------------------------------------------------------------------
enum {
    SCREEN_TOP = 5, SCREEN_BOTTOM = 192, SCREEN_LEFT = 0, SCREEN_RIGHT = 256
};


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

#define blobb_radious 32
#define blobb_mass 4
#define ball_radious 16
#define ball_mass 1

//-----------------------------------------------------------------
// clamp integer to range
//-----------------------------------------------------------------
static inline int clampint(int value, int low, int high)
//-----------------------------------------------------------------
{
    if (value < low) value = low;
    if (value > high) value = high;
    return value;
}


//-----------------------------------------------------------------
// update ball object (call once per frame)
//-----------------------------------------------------------------
void blobbUpdate(Sprite *b) {
    b->yvel += c_gravity;

    b->y += (b->yvel) >> 2;
    b->x += (b->xvel) >> 4;

    // Bounce on the platform
    if (b->y + c_radius >= c_platform_level) {
        if (b->y > c_platform_level - min_height) {
            b->y = c_platform_level - min_height / 2;
            b->yvel = -(b->yvel * (256 - c_bounce_damper)) >> 8;

        }
        b->height = (c_platform_level - b->y) * 2;
    } else {
        b->height = c_diam << 4;
    }
}

Sprite *blobbs[2];


int ballCollision(Sprite *ball) {
    blobbs[0] = &blobb1;
    blobbs[1] = &blobb2;
    for (int i = 0; i < 2; i++) {
        int ball_x = ball->x + 16;
        int ball_y = ball->y + 16;

        int blobb_x = blobbs[i]->x + 32;
        int blobb_y = blobbs[i]->y + 32;
        int distance = sqrt32(pow((blobb_x - ball_x), 2) + pow((blobb_y - ball_y), 2));
        if (distance <= ball_radious + blobb_radious) {
            return i + 1;
        }
    }
    return 0;
}

void ballUpdate(Sprite *ball) {
    if (paused > 0) {
        ball->yvel = 0;
        ball->y = 10;
    } else {

        ball->yvel += c_gravity;

        int colision = ballCollision(ball);

        if (ball->y + ball_radious >= SCREEN_BOTTOM ||
            (ball->y > SCREEN_BOTTOM / 2 - 16 && (ball->x + 32 >= SCREEN_RIGHT / 2 && ball->x <= SCREEN_RIGHT / 2))) {
            paused *= -1;

        } else if (colision > 0) {

            Sprite *blobb = blobbs[colision - 1];
            int blobb_V = sqrt32(pow(blobb->xvel, 2) + pow(blobb->yvel, 2));
            int blobb_momentum = blobb_mass * blobb_V;

            double cosA = abs(ball->x - blobb->x) / (blobb_radious + ball_radious);
            double sinA = abs(ball->y - blobb->y) / (blobb_radious + ball_radious); //cos(90 - B)

            int dx = (int) (blobb_momentum / cosA);
            int dy = (int) (blobb_momentum / sinA);

            if (ball->xvel > 0) {
                ball->xvel -= 5; //dx;
            } else if (ball->xvel < 0) {
                ball->xvel += 5;//dx;
            } else {
                ball->xvel += dx;
            }

            ball->yvel -= 5;

        }
        if (ball->x < SCREEN_LEFT || ball->x + ball_radious > SCREEN_RIGHT) {
            ball->xvel *= -1;
        }

        if (ball->y < 0 && ball->yvel < 0) {
            ball->yvel *= -1;
        }

        if (ball->xvel > 10) {
            ball->xvel %= 10;
        }
        if (ball->yvel > 10) {
            ball->yvel %= 10;
        }
        ball->x += ball->xvel;
        ball->y += ball->yvel >> 2;
    }
}

int processInput(int keys) {
    if (keys & KEY_START) return 1;
    if (keys & KEY_B ) {
        paused *= -1;
    }

    if (keys) {
        if (keys & KEY_UP) {
            if (blobb1.y >= SCREEN_TOP) {
                blobb1.yvel -= 1;
            }
        }
        if (keys & KEY_LEFT) {
            if (blobb1.x >= SCREEN_LEFT) {
                blobb1.xvel -= 2;
            }
        }
        if (keys & KEY_RIGHT) {
            if (blobb1.x <= SCREEN_RIGHT / 2 - 66) blobb1.xvel += 2;
        }
        if (keys & KEY_DOWN) {
            if (blobb1.y <= SCREEN_BOTTOM) {
                blobb1.yvel++;
            }
        }

        if (keys & KEY_Y) {
            if (blobb2.y >= SCREEN_TOP) {
                blobb2.yvel -= 1;
            }
        }
        if (keys & KEY_L) {
            if (blobb2.x >= SCREEN_RIGHT / 2 + 2) {
                blobb2.x -= 2;
            }
        }
        if (keys & KEY_R) {
            if (blobb2.x + 64 <= SCREEN_RIGHT) blobb2.x += 2;
        }
        if (keys & KEY_X) {
            if (blobb2.y <= SCREEN_BOTTOM) {
                blobb2.y++;
                blobb2.yvel++;
            }
        }

        if (blobb1.anim_frame >= 0) blobb1.anim_frame = 0;

        return 0;
    }
}

void play() {


    consoleDemoInit();
    videoSetMode(MODE_5_2D);
    videoSetModeSub(MODE_0_2D);


    vramSetBankA(VRAM_A_MAIN_SPRITE);
    vramSetBankB(VRAM_B_MAIN_BG_0x06000000);
    vramSetBankD(VRAM_D_SUB_SPRITE);

    int bg3 = bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
    dmaCopy(backgroundBitmap, bgGetGfxPtr(bg3), 256 * 256);
    dmaCopy(backgroundPal, BG_PALETTE, 256 * 2);

    oamInit(&oamMain, SpriteMapping_1D_256, false);
    oamInit(&oamSub, SpriteMapping_1D_256, false);

    initSprite(&blobb1, 0, 0, (u8 *) blobbTiles, 0, 0);
    initSprite(&blobb2, 192, 0, (u8 *) blobbTiles, 1, 0);
    initSprite(&ball, 50, 10, (u8 *) ballTiles, 2, 0);
    dmaCopy(blobbPal, SPRITE_PALETTE, 512);

    while (1) {
        scanKeys();

        int keys = keysHeld();

        if (processInput(keys)) break;

        blobbUpdate(&blobb1);
        animateSprite(&blobb1);

        blobbUpdate(&blobb2);
        animateSprite(&blobb2);

        ballUpdate(&ball);
        animateSprite(&ball);


        oamSet(&oamMain,
               0,
               blobb1.x, blobb1.y,
               0,
               0,
               SpriteSize_64x64,
               SpriteColorFormat_256Color,
               blobb1.sprite_gfx_mem,
               0,
               false,
               false,
               false, false,
               false);

        oamSet(&oamMain,
               1,
               blobb2.x, blobb2.y,
               0,
               0,
               SpriteSize_64x64,
               SpriteColorFormat_256Color,
               blobb1.sprite_gfx_mem,
               0,
               false,
               false,
               false, false,
               false);

        oamSet(&oamMain,
               2,
               ball.x, ball.y,
               0,
               1,
               SpriteSize_64x64,
               SpriteColorFormat_256Color,
               ball.sprite_gfx_mem,
               0,
               false,
               false,
               false, false,
               false);


        printf("%d, %d", ball.x, ball.y);
        swiWaitForVBlank();

        oamUpdate(&oamMain);
        oamUpdate(&oamSub);
    }
}

//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------
    //

    consoleDemoInit();

    //videoSetModeSub(MODE_0_2D);

    char *categories[3] = {"Play Amazing Blobby Volley!", "Score from the last game", "Exit"};
    while (1) {
        int selectedMenu = 0;
        int selected = 0;

        int catCount = 3;
        int demoCount = 0;


        iprintf("it works?");
        while (selected == 0) {

            scanKeys();

            int keys = keysDown();

            if (keys & KEY_UP) selectedMenu--;
            if (keys & KEY_DOWN) selectedMenu++;
            if (keys & KEY_A) selected++;

            if (selectedMenu < 0) selectedMenu = catCount - 1;
            if (selectedMenu >= catCount) selectedMenu = 0;

            swiWaitForVBlank();
            consoleClear();
            for (int ci = 0; ci < catCount; ci++) {
                iprintf("%c%d: %s\n", ci == selectedMenu ? '*' : ' ', ci + 1, categories[ci]);
            }
        }
        selected = 0;

        printf("%d", selectedMenu);
        if (selectedMenu == 0) play();
        if (selectedMenu == 1) printf("Scores");
        if (selectedMenu == 2) break;

    }

    return 0;
}
