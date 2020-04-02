// Simple citro2d sprite drawing example
// Images borrowed from:
//   https://kenney.nl/assets/space-shooter-redux
#include <citro2d.h>

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_SPRITES   4
#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 240
#define GRAVITY 0.25
#define BLOBB_MASS 1
#define BALL_MASS 1
#define BLOBB_RADIOUS 32
#define BALL_RADIOUS 16
// Simple sprite struct
typedef struct {
    C2D_Sprite spr;
    float dx, dy; // velocity
} Sprite;

static C2D_SpriteSheet spriteSheet;
static Sprite sprites[MAX_SPRITES];
static size_t numSprites = 4;
Sprite *ball = &sprites[1];
Sprite *blobbl = &sprites[2];
Sprite *blobbr = &sprites[3];

int speed = 3;
int scoreL = 0;
int scoreR = 0;
int paused = -1;

FILE *mem;

//---------------------------------------------------------------------------------
static void initSprites() {
//---------------------------------------------------------------------------------
    size_t numImages = C2D_SpriteSheetCount(spriteSheet);
    srand(time(NULL));

    Sprite *background = &sprites[0];

    // Random image, position, rotation and speed
    C2D_SpriteFromSheet(&background->spr, spriteSheet, 0);
    C2D_SpriteSetCenter(&background->spr, 0.5f, 0.5f);
    C2D_SpriteSetPos(&background->spr, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
    C2D_SpriteSetRotation(&background->spr, 0);
    background->dx = 0;
    background->dy = 0;

    C2D_SpriteFromSheet(&ball->spr, spriteSheet, 2);
    C2D_SpriteSetCenter(&ball->spr, 0.5f, 0.5f);
    C2D_SpriteSetPos(&ball->spr, SCREEN_WIDTH / 2 - SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4);
    C2D_SpriteSetRotation(&ball->spr, 0);
    ball->dx = 0;
    ball->dy = 0;

    C2D_SpriteFromSheet(&blobbl->spr, spriteSheet, 1);
    C2D_SpriteSetCenter(&blobbl->spr, 0.5f, 0.5f);
    C2D_SpriteSetPos(&blobbl->spr, SCREEN_WIDTH / 2 - SCREEN_WIDTH / 4, SCREEN_HEIGHT * 0.8);
    C2D_SpriteSetRotation(&blobbl->spr, 0);
    blobbl->dx = 0;
    blobbl->dy = 0;

    C2D_SpriteFromSheet(&blobbr->spr, spriteSheet, 1);
    C2D_SpriteSetCenter(&blobbr->spr, 0.5f, 0.5f);
    C2D_SpriteSetPos(&blobbr->spr, SCREEN_WIDTH / 2 + SCREEN_WIDTH / 4, SCREEN_HEIGHT * 0.8);
    C2D_SpriteSetRotation(&blobbr->spr, 0);
    blobbr->dx = 0;
    blobbr->dy = 0;
}

void clearConsole(){
    for(int i = 0; i < 40; i++ )
        printf("\n");
}

int ballCollision() {

    for (int i = 2; i < MAX_SPRITES; i++) {
        float distance = sqrt(pow(sprites[i].spr.params.pos.x - ball->spr.params.pos.x, 2) +
                              pow(sprites[i].spr.params.pos.y - ball->spr.params.pos.y, 2));

        if (distance <= 48) return i;
    }
    return 0;
}
void saveScoreToFile(){
    mem = fopen("/mem.txt", "a");
    if (mem == NULL)
    {
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }
    printf("\x1b[30;2HPriting score to file");
    fprintf(mem, "%d:%d\n", scoreL, scoreR);
    fclose(mem);
}

void ballUpdate() {

    if(paused < 0){
        if (ball->spr.params.pos.y < SCREEN_HEIGHT * 0.8) {
            ball->dy += GRAVITY;
        } else {
            ball->dy = 0;
            paused *= -1;
        }

        float distance = sqrt(pow(sprites[2].spr.params.pos.x - ball->spr.params.pos.x, 2) +
                              pow(sprites[2].spr.params.pos.y - ball->spr.params.pos.y, 2));

        float blobb_V = sqrt(pow(blobbl->dx, 2) + pow(blobbl->dy, 2));
        float ball_V = sqrt(pow(ball->dx, 2) + pow(ball->dy, 2));
        float blobb_momentum = BLOBB_MASS * blobb_V - ball_V;

        int collision = ballCollision();

        printf("\x1b[3;18HSCORE");
        printf("\x1b[5;15H%d\t---\t%d", scoreL, scoreR);

        if (ball->spr.params.pos.y + BALL_RADIOUS >= SCREEN_HEIGHT * 0.85 ||
            (ball->spr.params.pos.y > SCREEN_HEIGHT / 2 + 32 &&
             (ball->spr.params.pos.x + 16 >= SCREEN_WIDTH / 2 && ball->spr.params.pos.x <= SCREEN_WIDTH / 2))) {
            paused *= -1;

            if (ball->spr.params.pos.x > SCREEN_WIDTH / 2) {
                scoreL++;
                ball->spr.params.pos.x = blobbl->spr.params.pos.x;
                ball->spr.params.pos.y = SCREEN_HEIGHT * 0.2;
                ball->dy = 0;
                ball->dx = 0;
            } else if (ball->spr.params.pos.x < SCREEN_WIDTH / 2) {
                scoreR++;
                ball->spr.params.pos.x = blobbr->spr.params.pos.x;
                ball->spr.params.pos.y = SCREEN_HEIGHT * 0.2;
                ball->dy = 0;
                ball->dx = 0;
            }

            if (scoreR == 6 || scoreL == 6) {
                saveScoreToFile();
                paused = 0;

                printf("\x1b[10;1H GAME OVER");
            }

        } else if (collision > 0) {

            Sprite *blobb = &sprites[collision];
            float blobb_V = sqrt(pow(blobb->dx, 2) + pow(blobb->dy, 2));
            float ball_V = sqrt(pow(ball->dx, 2) + pow(ball->dy, 2));
            float blobb_momentum = BLOBB_MASS * blobb_V + 0.5 * ball_V;

            float cosA = (ball->spr.params.pos.x - blobb->spr.params.pos.x) / (BLOBB_RADIOUS + BALL_RADIOUS);
            float sinA =
                    (ball->spr.params.pos.y - blobb->spr.params.pos.y) / (BLOBB_RADIOUS + BALL_RADIOUS); //cos(90 - B)

            float dx = cosA != 0 ? (blobb_momentum * cosA) : 0;
            float dy = sinA != 0 ? (blobb_momentum * sinA) : 0;

            ball->dx = dx; //cosA > 0 ? speed : (-1) * speed; // dx;
            ball->dy = dy;//sinA > 0 ? speed : (-1) * speed; // dy;

        }
        if (ball->spr.params.pos.x < 0 || ball->spr.params.pos.x + BALL_RADIOUS > SCREEN_WIDTH) {
            ball->dx *= -1;
        }

        if (ball->spr.params.pos.y < 0 && ball->dy < 0) {
            ball->dy *= -1;
        }

        ball->dx += ball->dx > 0 ? -0.1 : 0.1;
    } else {
        ball->dy = 0;
        ball->dx = 0;
    }

    C2D_SpriteMove(&ball->spr, ball->dx, ball->dy);
}

//---------------------------------------------------------------------------------


void moveSprites() {
    // Respond to user input
    u32 kDown = hidKeysDown();
    if (kDown & KEY_START) {
        return; // break in order to return to hbmenu
    }

    blobbr->dx *= 0.8;
    blobbl->dx *= 0.8;

    if (blobbr->spr.params.pos.y < SCREEN_HEIGHT * 0.8) {
        blobbr->dy += GRAVITY;
    } else {
        blobbr->dy = 0;
    }
    if (blobbl->spr.params.pos.y < SCREEN_HEIGHT * 0.8) {
        blobbl->dy += GRAVITY;
    } else {
        blobbl->dy = 0;
    }

    u32 kHeld = hidKeysHeld();
    if (kHeld & KEY_CPAD_UP && blobbr->spr.params.pos.y > SCREEN_HEIGHT * 0.2)
        blobbr->dy = -5;
    if (kHeld & KEY_CPAD_DOWN && blobbr->spr.params.pos.y < SCREEN_HEIGHT * 0.8)
        blobbr->dy += 2;
    if (kHeld & KEY_CPAD_RIGHT && blobbr->spr.params.pos.x < SCREEN_WIDTH * 0.93)
        blobbr->dx = 4;
    if (kHeld & KEY_CPAD_LEFT && blobbr->spr.params.pos.x > SCREEN_WIDTH * 0.61)
        blobbr->dx = -4;
    if (kHeld & KEY_DUP && blobbl->spr.params.pos.y > SCREEN_HEIGHT * 0.2)
        blobbl->dy = -5;
    if (kHeld & KEY_DDOWN && blobbl->spr.params.pos.y < SCREEN_HEIGHT * 0.8)
        blobbl->dy += 2;
    if (kHeld & KEY_DRIGHT && blobbl->spr.params.pos.x < SCREEN_WIDTH * 0.39)
        blobbl->dx = 4;
    if (kHeld & KEY_DLEFT && blobbl->spr.params.pos.x > SCREEN_WIDTH * 0.07)
        blobbl->dx = -4;
    C2D_SpriteMove(&blobbl->spr, blobbl->dx, blobbl->dy);
    C2D_SpriteMove(&blobbr->spr, blobbr->dx, blobbr->dy);

}

void play(){
    clearConsole();
    // Create screens
    C3D_RenderTarget *top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");
    if (!spriteSheet) svcBreak(USERBREAK_PANIC);

    C2D_Image background = C2D_SpriteSheetGetImage(spriteSheet, 0);
    C2D_DrawImageAt(background, 0, 0, 0, 0, 1, 1);

    // Initialize sprites
    initSprites();

    // Main loop
    while (aptMainLoop()) {
        hidScanInput();

        u32 kDown = hidKeysDown();
        if(kDown & KEY_X) break;
        if(kDown & KEY_Y) paused *= -1;

        moveSprites();
        ballUpdate();

        // Render the scene
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
        C2D_SceneBegin(top);
        for (size_t i = 0; i < numSprites; i++)
            C2D_DrawSprite(&sprites[i].spr);
        C3D_FrameEnd(0);
    }
    C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
    C2D_SceneBegin(top);
    clearConsole();

    // Delete graphics
    C2D_SpriteSheetFree(spriteSheet);


}

void scores(){
    clearConsole();
    mem = fopen("/mem.txt", "r");
    if (mem == NULL)
    {
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }


    char line[3];
    float temp = 0;
    int i = 1;
    int r;

    printf("\x1b[4;10HSCORES HISTORY:");
    while((r = fscanf(mem, "%s\n", line)) != EOF){
        printf("\x1b[%d;13H%d) %s", i+5, i, line);
        i++;
    }

    fclose(mem);
    while(1){
        hidScanInput();

        u32 kDown = hidKeysDown();
        if(kDown & KEY_X) break;
    }
    clearConsole();
}

//---------------------------------------------------------------------------------
int main(int argc, char *argv[]) {

//---------------------------------------------------------------------------------
    // Init libs
    char *categories[3] = {"Play Amazing Blobby Volley!", "Score from the last game", "Exit"};
    char *indent[3] = {"\x1b[1;1H", "\x1b[2;1H", "\x1b[3;1H"};
    romfsInit();
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    consoleInit(GFX_BOTTOM, NULL);

    u32 kDownOld = 0, kHeldOld = 0, kUpOld = 0;
    int i= 0;
    while(aptMainLoop()){

        int selectedMenu = 0;
        int selected = 0;

        int catCount = 3;
        int demoCount = 0;

        while (selected == 0) {
            hidScanInput();
            i++;
            u32 kDown = hidKeysDown();
            u32 kHeld = hidKeysHeld();
            u32 kUp = hidKeysUp();

            if( kDown != kDownOld ) {
                if (kDown & KEY_CPAD_UP) {
                    selectedMenu--;
                }
                if (kDown & KEY_CPAD_DOWN) {
                    selectedMenu++;
                }
                if (kDown & KEY_X) selected++;
            }
            if (selectedMenu < 0) selectedMenu = catCount - 1;
            if (selectedMenu >= catCount) selectedMenu = 0;

            for (int ci = 0; ci < catCount; ci++) {
                printf("%s %c%d: %s\n", indent[ci], ci == selectedMenu ? '*' : ' ', ci + 1, categories[ci]);
            }
        }
        selected = 0;

        if (selectedMenu == 0) {
            play();
        }
        if (selectedMenu == 1) scores();
        if (selectedMenu == 2) break;
    }

    // Load graphics


    // Deinit libs
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    romfsExit();
    return 0;
}
