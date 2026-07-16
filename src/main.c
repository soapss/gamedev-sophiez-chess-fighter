#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SCREEN_WIDTH 1080
#define SCREEN_HEIGHT 720
#define WORLD_HEIGHT 4000
#define SKY_HEIGHT 140
#define MAX_ROCKS 900
#define MAX_GATES 6
#define MAX_SHRIMPS 24
#define SAVE_MAX 128
#define SHRIMPS_NEEDED 5
#define TIDE_COUNT 5
#define LOW_PHASE_DURATION 42.0f
#define RISING_DURATION 10.0f
// wave timings - slower waves as requested
#define WAVE_PAUSE_LOW_DUR 2.0f
#define WAVE_IN_DUR 2.5f
#define WAVE_PAUSE_HIGH_DUR 1.0f
#define WAVE_OUT_DUR 2.5f

// Dig-Dug palette - pixel art solid
#define COL_SKY_DAY (Color){113,188,225,255}
#define COL_SKY_NIGHT (Color){28,32,64,255}
#define COL_SAND_LIGHT (Color){228,206,148,255}
#define COL_SAND_MID (Color){194,178,128,255}
#define COL_SAND_DARK (Color){160,138,96,255}
#define COL_SAND_STRIPE (Color){178,156,110,255}
#define COL_ROCK (Color){148,126,98,255}
#define COL_ROCK_DARK (Color){92,76,58,255}
#define COL_ROCK_LIGHT (Color){192,172,142,255}
#define COL_ROCK_GATE (Color){165,85,60,255}
#define COL_ROCK_GATE_DARK (Color){115,50,40,255}
#define COL_WATER (Color){32,96,192,200}
#define COL_WATER_DEEP (Color){24,64,160,210}
#define COL_WAVE_FOAM (Color){210,234,255,255}
#define COL_CRAB_BODY (Color){214,80,32,255}
#define COL_CRAB_BODY_DARK (Color){160,50,20,255}
#define COL_CRAB_LEG (Color){160,48,24,255}
#define COL_CRAB_CLAW (Color){180,60,32,255}
#define COL_CRAB_CLAW_LIGHT (Color){220,100,60,255}
#define COL_SHRIMP (Color){255,160,140,255}
#define COL_SHRIMP_DARK (Color){200,90,80,255}
#define COL_SHELL_1 (Color){238,232,192,255}
#define COL_SHELL_2 (Color){212,192,148,255}
#define COL_SHELL_3 (Color){184,160,120,255}
#define COL_SHELL_4 (Color){162,140,108,255}
#define COL_SHELL_5 (Color){142,124,100,255}
#define COL_WAVE (Color){64,140,220,190}
#define COL_UI_BG (Color){0,0,0,100}
#define COL_UI_PANEL (Color){200,184,132,255}
#define COL_UI_PANEL_DARK (Color){120,100,68,255}

typedef enum { STATE_TITLE, STATE_PLAYING, STATE_GAMEOVER, STATE_WIN, STATE_LEVEL_SELECT, STATE_BETWEEN } GameState;
typedef enum { PHASE_LOW_WAVES, PHASE_RISING } TidePhase;
typedef enum { WAVE_PAUSE_LOW, WAVE_IN, WAVE_PAUSE_HIGH, WAVE_OUT } WaveState;

typedef struct { float x,y,w,h; Color color; } Rock;
typedef struct { float x,y,w,h; int requiredSize; bool locked; Color color; } Gate;
typedef struct { float x,y; bool alive; float anim; } Shrimp;
typedef struct { float x,y; int sizeIdx; bool exists; bool collected; } Shell;
typedef struct { float x,y; int sizeIdx; float size; float speed; bool moving; float animTime; bool upsideDown; } Crab;
typedef struct { unsigned int seed; bool completed; } SaveEntry;

static Rock rocks[MAX_ROCKS]; static int rockCount=0;
static Gate gates[MAX_GATES]; static int gateCount=0;
static Shrimp shrimps[MAX_SHRIMPS]; static int shrimpCount=0; static int shrimpsEaten=0;
static Shell currentShell;
static Crab crab;
static float safeTargets[TIDE_COUNT] = {2100,1500,900,300,50};
static float highWaters[TIDE_COUNT] = {2250,1650,1050,450,200};
static float lowWater = 3800;
static float waveHigh = 3420;
static float waterLevel = 3800;
static float waveLevel = 3800;
static float tideTimer=0; static float lowTimer=0;
static int tideIndex=0;
static TidePhase tidePhase=PHASE_LOW_WAVES;
static WaveState waveState=WAVE_PAUSE_LOW;
static float waveStateTimer=0; static int waveCount=0;
static bool waitingForShell=false;
static bool shellSpawnedThisTide=false;
static bool shellRidingWave=false;
static unsigned int currentSeed=12345;
static GameState gameState=STATE_TITLE;
static SaveEntry saves[SAVE_MAX]; static int saveCount=0;
static Crab autoCrab; static float autoWaterLevel=3800; static float autoWaveLevel=3800;
static float autoLowTimer=0; static int autoWaveCount=0; static WaveState autoWaveState=WAVE_PAUSE_LOW; static float autoWaveStateTimer=0;
static Shrimp autoShrimps[12]; static int autoShrimpCount=0;
static float g_crabWorldY=2700; static float g_crabScreenY=SCREEN_HEIGHT*0.65f; static float g_horizonLimit=0; static float g_k=0.0035f;
static int menuSelection=0;

static unsigned int g_rng=12345;
static void SeedRand(unsigned int s){ g_rng=s; }
static unsigned int RandU(){ g_rng=g_rng*1664525u+1013904223u; return g_rng; }
static int RandInt(int min,int max){ if(max<=min) return min; return min + (int)(RandU() % (unsigned int)(max-min)); }
static float RandFloat(float min,float max){ return min + ((float)RandU()/(float)0xFFFFFFFF)*(max-min); }

static void LoadSaves(){ saveCount=0; FILE *f=fopen("save.dat","r"); if(!f) f=fopen("./save.dat","r"); if(!f) return; char line[128]; while(fgets(line,sizeof(line),f) && saveCount<SAVE_MAX){ unsigned int s; int comp; if(sscanf(line,"%u %d",&s,&comp)==2){ saves[saveCount].seed=s; saves[saveCount].completed=comp!=0; saveCount++; } } fclose(f); }
static void SaveSaves(){ FILE *f=fopen("save.dat","w"); if(!f) return; for(int i=0;i<saveCount;i++) fprintf(f,"%u %d\n",saves[i].seed,saves[i].completed?1:0); fclose(f); }
static int FindSave(unsigned int seed){ for(int i=0;i<saveCount;i++) if(saves[i].seed==seed) return i; return -1; }
static void AddOrUpdateSave(unsigned int seed,bool completed){ int idx=FindSave(seed); if(idx>=0){ if(completed) saves[idx].completed=true; } else if(saveCount<SAVE_MAX){ saves[saveCount].seed=seed; saves[saveCount].completed=completed; saveCount++; } SaveSaves(); }

static float WorldToScreenY(float worldY){ float delta=worldY-g_crabWorldY; if(delta>=0) return g_crabScreenY+delta; else { float dist=-delta; float comp=g_horizonLimit*(1.0f-expf(-dist*g_k)); return g_crabScreenY-comp; } }
static bool AABB(float x1,float y1,float w1,float h1,float x2,float y2,float w2,float h2){ return (x1<x2+w2 && x1+w1>x2 && y1<y2+h2 && y1+h1>y2); }

// 5x7 font
typedef struct { char ch; const char* rows[7]; } CharPattern;
static CharPattern fontPatterns[] = {
 {'A', {"01110","10001","10001","11111","10001","10001","10001"}},
 {'B', {"11110","10001","10001","11110","10001","10001","11110"}},
 {'C', {"01111","10000","10000","10000","10000","10000","01111"}},
 {'D', {"11110","10001","10001","10001","10001","10001","11110"}},
 {'E', {"11111","10000","10000","11110","10000","10000","11111"}},
 {'F', {"11111","10000","10000","11110","10000","10000","10000"}},
 {'G', {"01111","10000","10000","10111","10001","10001","01111"}},
 {'H', {"10001","10001","10001","11111","10001","10001","10001"}},
 {'I', {"11111","00100","00100","00100","00100","00100","11111"}},
 {'J', {"00111","00001","00001","00001","10001","10001","01110"}},
 {'K', {"10001","10010","10100","11000","10100","10010","10001"}},
 {'L', {"10000","10000","10000","10000","10000","10000","11111"}},
 {'M', {"10001","11011","10101","10101","10001","10001","10001"}},
 {'N', {"10001","11001","10101","10011","10001","10001","10001"}},
 {'O', {"01110","10001","10001","10001","10001","10001","01110"}},
 {'P', {"11110","10001","10001","11110","10000","10000","10000"}},
 {'Q', {"01110","10001","10001","10001","10101","10010","01101"}},
 {'R', {"11110","10001","10001","11110","10100","10010","10001"}},
 {'S', {"01111","10000","10000","01110","00001","00001","11110"}},
 {'T', {"11111","00100","00100","00100","00100","00100","00100"}},
 {'U', {"10001","10001","10001","10001","10001","10001","01110"}},
 {'V', {"10001","10001","10001","10001","01010","01010","00100"}},
 {'W', {"10001","10001","10001","10101","10101","11011","10001"}},
 {'X', {"10001","01010","00100","00100","00100","01010","10001"}},
 {'Y', {"10001","10001","01010","00100","00100","00100","00100"}},
 {'Z', {"11111","00001","00010","00100","01000","10000","11111"}},
 {'0', {"01110","10001","10011","10101","11001","10001","01110"}},
 {'1', {"00100","01100","00100","00100","00100","00100","01110"}},
 {'2', {"01110","10001","00001","00010","00100","01000","11111"}},
 {'3', {"11111","00010","00100","00010","00001","10001","01110"}},
 {'4', {"00010","00110","01010","10010","11111","00010","00010"}},
 {'5', {"11111","10000","11110","00001","00001","10001","01110"}},
 {'6', {"01110","10000","10000","11110","10001","10001","01110"}},
 {'7', {"11111","00001","00010","00100","01000","01000","01000"}},
 {'8', {"01110","10001","10001","01110","10001","10001","01110"}},
 {'9', {"01110","10001","10001","01111","00001","00001","01110"}},
 {' ', {"00000","00000","00000","00000","00000","00000","00000"}},
 {'/', {"00001","00010","00010","00100","01000","01000","10000"}},
 {0, {0}}
};
static void DrawChar5x7(char ch, int x, int y, int px, Color col){
    if(ch>='a' && ch<='z') ch=ch-'a'+'A';
    const char** rows=NULL;
    for(int i=0;fontPatterns[i].ch!=0;i++) if(fontPatterns[i].ch==ch){ rows=fontPatterns[i].rows; break; }
    if(!rows) return;
    for(int r=0;r<7;r++) for(int c=0;c<5;c++) if(rows[r][c]=='1') DrawRectangle(x+c*px, y+r*px, px, px, col);
}
static void DrawPixelText(const char* txt, int x, int y, int px, Color col){
    int cx=x;
    for(int i=0;txt[i];i++){ if(txt[i]=='\n'){ y+=8*px; cx=x; continue; } DrawChar5x7(txt[i], cx, y, px, col); cx+=6*px; }
}
static int MeasurePixelText(const char* txt, int px){ return (int)strlen(txt)*6*px; }

// Logo with water fill, foam, bold outline, tight spacing - chunkier arcade style
static void DrawLogoWithWater(const char* txt, int x, int y, int px, Color topCol, Color waterCol, Color foamCol, Color outlineCol){
    int len=strlen(txt);
    // tight spacing: 6*px per char
    // Draw outline first - thicker chunkier (3px outline)
    for(int i=0;i<len;i++){
        char ch=txt[i]; if(ch>='a' && ch<='z') ch=ch-'a'+'A';
        const char** rows=NULL;
        for(int f=0;fontPatterns[f].ch!=0;f++) if(fontPatterns[f].ch==ch){ rows=fontPatterns[f].rows; break; }
        if(!rows) continue;
        int charX = x + i*6*px;
        for(int r=0;r<7;r++) for(int c=0;c<5;c++) if(rows[r][c]=='1'){
            // thick outline 3px: draw in 5x5 area around pixel
            for(int oy=-2; oy<=2; oy++) for(int ox=-2; ox<=2; ox++) if(ox!=0 || oy!=0){
                // draw chunkier outline blocks
                DrawRectangle(charX + c*px + ox*px, y + r*px + oy*px, px, px, outlineCol);
            }
            // extra outer for bold
            for(int oy=-3; oy<=3; oy+=3) for(int ox=-3; ox<=3; ox+=3) if(abs(ox)+abs(oy)>=3){
                DrawRectangle(charX + c*px + ox*px, y + r*px + oy*px, px, px, outlineCol);
            }
        }
    }
    // Draw main with water fill - chunkier by drawing each pixel as 2x2 overlapping for bold strokes
    for(int i=0;i<len;i++){
        char ch=txt[i]; if(ch>='a' && ch<='z') ch=ch-'a'+'A';
        const char** rows=NULL;
        for(int f=0;fontPatterns[f].ch!=0;f++) if(fontPatterns[f].ch==ch){ rows=fontPatterns[f].rows; break; }
        if(!rows) continue;
        int charX = x + i*6*px;
        for(int r=0;r<7;r++){
            for(int c=0;c<5;c++) if(rows[r][c]=='1'){
                Color col;
                if(r<3) col=topCol;
                else if(r==3) col=foamCol;
                else col=waterCol;
                // chunkier: draw main block plus one extra to right and down for bold arcade look
                DrawRectangle(charX + c*px, y + r*px, px, px, col);
                DrawRectangle(charX + c*px + px/2, y + r*px, px, px, col);
                DrawRectangle(charX + c*px, y + r*px + px/2, px, px, col);
                // highlight
                if(r==1 && c==2) DrawRectangle(charX + c*px, y + r*px, px, px, (Color){255,255,255,200});
            }
        }
    }
}

// Detailed pixel art assets -------------------------------------------------
static void DrawDetailedCrab(float worldX,float worldY,float size,int sizeIdx,float anim,bool moving,bool upsideDown,float overrideSy){
    float sy = (overrideSy>=0)? overrideSy : WorldToScreenY(worldY);
    int px = 2 + sizeIdx; // 2..6
    int cx=(int)worldX;
    int cy=(int)sy;
    int flip = upsideDown? -1:1;
    // shadow
    if(!upsideDown) DrawRectangle(cx-6*px, cy+5*px*flip, 12*px, 2*px, (Color){0,0,0,60});
    // claws - solid pincers left/right front
    // left claw outer 4x3
    DrawRectangle(cx-9*px, cy-8*px*flip, 4*px, 3*px, COL_CRAB_CLAW);
    DrawRectangle(cx-8*px, cy-7*px*flip, 2*px, px, COL_CRAB_CLAW_LIGHT); // highlight
    DrawRectangle(cx-7*px, cy-7*px*flip, px, px, COL_SAND_LIGHT); // pincer gap
    // right claw
    DrawRectangle(cx+5*px, cy-8*px*flip, 4*px, 3*px, COL_CRAB_CLAW);
    DrawRectangle(cx+6*px, cy-7*px*flip, 2*px, px, COL_CRAB_CLAW_LIGHT);
    DrawRectangle(cx+6*px, cy-7*px*flip, px, px, COL_SAND_LIGHT);
    // eye stalks - solid 1x3
    DrawRectangle(cx-4*px, cy-6*px*flip, px, 3*px, BLACK);
    DrawRectangle(cx+3*px, cy-6*px*flip, px, 3*px, BLACK);
    // eyes white 3x3
    DrawRectangle(cx-5*px, cy-9*px*flip, 3*px, 3*px, WHITE);
    DrawRectangle(cx+3*px, cy-9*px*flip, 3*px, 3*px, WHITE);
    DrawRectangle(cx-4*px, cy-8*px*flip, px, px, BLACK);
    DrawRectangle(cx+4*px, cy-8*px*flip, px, px, BLACK);
    // body - solid oval approximated with rects but detailed
    // main body 10x6 blocky with cut corners for rounded look
    DrawRectangle(cx-5*px, cy-3*px*flip, 10*px, 6*px, COL_CRAB_BODY);
    // cut corners for rounded
    DrawRectangle(cx-5*px, cy-3*px*flip, px, px, (Color){0,0,0,0}); // we overdraw with transparent? Instead draw body as filled with corners missing
    // Actually draw body as 3 layers for rounded effect
    DrawRectangle(cx-4*px, cy-4*px*flip, 8*px, 8*px, COL_CRAB_BODY);
    DrawRectangle(cx-5*px, cy-3*px*flip, 10*px, 6*px, COL_CRAB_BODY);
    // dark outline
    DrawRectangle(cx-5*px, cy-4*px*flip, 10*px, px, COL_CRAB_BODY_DARK);
    DrawRectangle(cx-5*px, cy+3*px*flip, 10*px, px, COL_CRAB_BODY_DARK);
    DrawRectangle(cx-5*px, cy-3*px*flip, px, 6*px, COL_CRAB_BODY_DARK);
    DrawRectangle(cx+4*px, cy-3*px*flip, px, 6*px, COL_CRAB_BODY_DARK);
    // shell - spiral shaped solid except opening where crab extends from - per spec
    Color shellCol;
    switch(sizeIdx){ case 0:shellCol=COL_SHELL_1; break; case 1:shellCol=COL_SHELL_2; break; case 2:shellCol=COL_SHELL_3; break; case 3:shellCol=COL_SHELL_4; break; default:shellCol=COL_SHELL_5; break; }
    // spiral shell 7x6 with opening at bottom
    for(int dy=-3; dy<=2; dy++){
        for(int dx=-3; dx<=3; dx++){
            if(dx*dx + dy*dy > 9) continue;
            // opening at bottom where crab extends
            if(dy==2 && abs(dx)<=1) continue; // opening
            DrawRectangle(cx+dx*px, cy+dy*px*flip - px*flip, px, px, shellCol);
        }
    }
    // highlight top
    DrawRectangle(cx-2*px, cy-3*px*flip, 2*px, px, (Color){shellCol.r+25,shellCol.g+25,shellCol.b+25,255});
    // shadow bottom sides
    DrawRectangle(cx-3*px, cy+1*px*flip, px, px, (Color){shellCol.r-30,shellCol.g-30,shellCol.b-30,255});
    DrawRectangle(cx+2*px, cy+1*px*flip, px, px, (Color){shellCol.r-30,shellCol.g-30,shellCol.b-30,255});
    // inner spiral darker
    DrawRectangle(cx-1*px, cy-1*px*flip, 2*px, px, (Color){shellCol.r-40,shellCol.g-40,shellCol.b-40,255});
    DrawRectangle(cx, cy, px, px, (Color){shellCol.r-60,shellCol.g-60,shellCol.b-60,255});
    // 4 legs (2 per side) + 2 arms with claws - per spec He should have 4 legs and 2 arms with claws
    // legs - 2 per side
    for(int side=-1; side<=1; side+=2){
        for(int leg=0; leg<2; leg++){
            int baseY = (leg==0)? -2 : 2; // 2 legs per side, top and bottom
            float animOff = moving? sinf(anim*6 + leg*2.0f + (side==-1?0:3.14f))*1.2f : 0;
            int lx = cx + side*6*px;
            int ly = cy + (int)((baseY + animOff)*px*flip);
            // upper leg segment solid
            DrawRectangle(lx, ly, 3*px* (side==-1? -1:1), px, COL_CRAB_LEG);
            // lower leg
            DrawRectangle(lx + side*3*px, ly+px*flip, 3*px* (side==-1? -1:1), px, COL_CRAB_LEG);
        }
    }
    // arms already drawn as claws above, but ensure 2 arms with claws distinct from legs - claws are arms
}

static void DrawDetailedShrimp(Shrimp s){
    if(!s.alive) return;
    float sy=WorldToScreenY(s.y);
    if(sy<SKY_HEIGHT||sy>SCREEN_HEIGHT+40) return;
    float bob=sinf(GetTime()*3 + s.anim)*2;
    int px=3;
    int x=(int)s.x; int y=(int)(sy+bob);
    // antennae
    DrawRectangle(x-10,y-2,2,1,COL_SHRIMP_DARK);
    DrawRectangle(x-10,y+2,2,1,COL_SHRIMP_DARK);
    // head 3x2 solid
    DrawRectangle(x-8,y-1,3*px,2*px,COL_SHRIMP);
    DrawRectangle(x-8,y-1,3*px,px,COL_SHRIMP_DARK); // top darker
    // eye
    DrawRectangle(x-9,y-1,px,px,BLACK);
    // body segments 3
    DrawRectangle(x-2,y-1,2*px,2*px,COL_SHRIMP);
    DrawRectangle(x+2,y-1,2*px,2*px,COL_SHRIMP);
    DrawRectangle(x+6,y-1,2*px,2*px,COL_SHRIMP);
    // segments shading
    DrawRectangle(x-2,y+1,2*px,px,COL_SHRIMP_DARK);
    DrawRectangle(x+2,y+1,2*px,px,COL_SHRIMP_DARK);
    // tail fan
    DrawRectangle(x+10,y-1,2*px,2*px,COL_SHRIMP);
    DrawRectangle(x+12,y,px,px,COL_SHRIMP_DARK);
}

static void DrawDetailedShell(Shell sh){
    if(!sh.exists||sh.collected) return;
    float sy=WorldToScreenY(sh.y);
    if(sy<SKY_HEIGHT||sy>SCREEN_HEIGHT+40) return;
    int px=3;
    int x=(int)sh.x; int y=(int)sy;
    Color col;
    switch(sh.sizeIdx){ case 0:col=COL_SHELL_1; break; case 1:col=COL_SHELL_2; break; case 2:col=COL_SHELL_3; break; case 3:col=COL_SHELL_4; break; default:col=COL_SHELL_5; break; }
    int sz=14 + sh.sizeIdx*3;
    // spiral shaped solid except opening at bottom
    for(int dy=-sz/2; dy<sz/2; dy++) for(int dx=-sz/2; dx<sz/2; dx++){
        float d2 = dx*dx+dy*dy;
        float r = sz/2.0f;
        if(d2 >= r*r) continue;
        // opening at bottom where crab extends - small gap
        if(dy> r*0.3f && fabsf(dx) < r*0.3f) continue;
        DrawRectangle(x+dx*px, y+dy*px, px, px, col);
    }
    // inner spiral layers
    for(int k=1;k<3;k++){
        int rs = sz/2 - k*3;
        if(rs<=0) continue;
        Color inner = (Color){col.r - k*20, col.g - k*20, col.b - k*20, 255};
        for(int dy=-rs; dy<rs; dy++) for(int dx=-rs; dx<rs; dx++){
            float d2 = dx*dx+dy*dy;
            if(d2 < (rs-1)*(rs-1) && d2 >= rs*rs-6){
                if(dy> rs*0.2f && fabsf(dx) < rs*0.3f) continue; // keep opening
                DrawRectangle(x+dx*px, y+dy*px, px, px, inner);
            }
        }
    }
    // highlight
    DrawRectangle(x-4*px,y-4*px,2*px,2*px,WHITE);
    DrawRectangle(x-3*px,y-5*px,px,px,WHITE);
    // outline
    for(int dy=-sz/2; dy<sz/2; dy++) for(int dx=-sz/2; dx<sz/2; dx++){
        float dist=sqrtf(dx*dx+dy*dy);
        if(fabsf(dist - sz/2) < 1.2f){
            if(dy> sz/2*0.3f && fabsf(dx) < sz/2*0.3f) continue; // opening no outline
            DrawRectangle(x+dx*px, y+dy*px, px, px, COL_ROCK_DARK);
        }
    }
}

static void DrawDetailedRock(Rock r){
    float top=WorldToScreenY(r.y);
    float bot=WorldToScreenY(r.y+r.h);
    float h=bot-top; if(h<0) h=-h;
    if(bot<SKY_HEIGHT) return; if(top>SCREEN_HEIGHT) return; if(h<1) h=1;
    float sy=top; if(sy<SKY_HEIGHT){ float d=SKY_HEIGHT-sy; sy=SKY_HEIGHT; h-=d; if(h<=0) return; }
    int px=4;
    // solid base with border
    DrawRectangle((int)r.x,(int)sy,(int)r.w,(int)h,r.color);
    DrawRectangle((int)r.x,(int)sy,(int)r.w,px,COL_ROCK_DARK);
    DrawRectangle((int)r.x,(int)sy+(int)h-px,(int)r.w,px,COL_ROCK_DARK);
    DrawRectangle((int)r.x,(int)sy,px,(int)h,COL_ROCK_DARK);
    DrawRectangle((int)r.x+(int)r.w-px,(int)sy,px,(int)h,COL_ROCK_DARK);
    // highlight
    DrawRectangle((int)r.x+px,(int)sy+px,px*2,px,COL_ROCK_LIGHT);
    // cracks / texture pixel
    for(int yy=0; yy<(int)h; yy+=12) for(int xx=0; xx<(int)r.w; xx+=14) if(((int)(r.x+xx+yy*3) % 7)==0) DrawRectangle((int)r.x+xx,(int)sy+yy,px,px,COL_ROCK_DARK);
}

static void DrawDetailedGate(Gate g){
    if(!g.locked) return;
    float top=WorldToScreenY(g.y);
    float bot=WorldToScreenY(g.y+g.h);
    float h=bot-top; if(h<0) h=-h;
    if(bot<SKY_HEIGHT) return; if(top>SCREEN_HEIGHT) return; if(h<1) h=1;
    float sy=top; if(sy<SKY_HEIGHT){ float d=SKY_HEIGHT-sy; sy=SKY_HEIGHT; h-=d; if(h<=0) return; }
    int px=4;
    // rock boulder look - irregular, not door, per feedback gates don't look like rocks
    // base boulder with earth tones, textured like rock
    DrawRectangle((int)g.x,(int)sy,(int)g.w,(int)h,COL_ROCK);
    // irregular edges - add small bumps
    DrawRectangle((int)g.x-2,(int)sy+4,(int)g.w+4, (int)h-8, COL_ROCK);
    DrawRectangle((int)g.x+4,(int)sy-2,(int)g.w-8, (int)h+4, COL_ROCK);
    // dark outline irregular
    DrawRectangle((int)g.x,(int)sy,(int)g.w,px,COL_ROCK_DARK);
    DrawRectangle((int)g.x,(int)sy+(int)h-px,(int)g.w,px,COL_ROCK_DARK);
    DrawRectangle((int)g.x,(int)sy,px,(int)h,COL_ROCK_DARK);
    DrawRectangle((int)g.x+(int)g.w-px,(int)sy,px,(int)h,COL_ROCK_DARK);
    // highlight top for boulder 3D
    DrawRectangle((int)g.x+px,(int)sy+px, (int)g.w/2, px, COL_ROCK_LIGHT);
    // cracks for rock texture
    DrawRectangle((int)g.x+10,(int)sy+8, 20,2, COL_ROCK_DARK);
    DrawRectangle((int)g.x+30,(int)sy+12, 15,2, COL_ROCK_DARK);
    DrawRectangle((int)g.x+5,(int)sy+h/2, 12,2, COL_ROCK_DARK);
    // small lock indicator subtle - keep size number for gameplay
    // make lock smaller and more integrated as carving
    int lockX=(int)(g.x+g.w/2-4); int lockY=(int)(sy+h/2-6);
    // required size number
    DrawPixelText(TextFormat("%d", g.requiredSize+1), (int)g.x+4, (int)sy+4, 1, WHITE);
}

static void DrawRocksAndGates(){ for(int i=0;i<rockCount;i++) DrawDetailedRock(rocks[i]); for(int i=0;i<gateCount;i++) DrawDetailedGate(gates[i]); }

static void DrawPixelSandBackground(){
    float feedTop=3300, feedBottom=3800;
    float sTop=WorldToScreenY(feedTop);
    float sBot=WorldToScreenY(feedBottom);
    if(sBot>SKY_HEIGHT){
        float y=sTop; if(y<SKY_HEIGHT) y=SKY_HEIGHT;
        float h=sBot-y;
        if(h>0){
            DrawRectangle(0,(int)y,SCREEN_WIDTH,(int)h+120,COL_SAND_MID);
            for(float wy=feedTop; wy<feedBottom; wy+=36){
                float sy=WorldToScreenY(wy);
                if(sy<SKY_HEIGHT||sy>SCREEN_HEIGHT) continue;
                for(int x=0;x<SCREEN_WIDTH;x+=24) DrawRectangle(x,(int)sy,12,3,COL_SAND_STRIPE);
            }
        }
    }
    for(float wy=0; wy<WORLD_HEIGHT; wy+=72){
        float sy=WorldToScreenY(wy);
        if(sy<SKY_HEIGHT||sy>SCREEN_HEIGHT) continue;
        for(int x=0;x<SCREEN_WIDTH;x+=40) DrawRectangle(x,(int)sy,20,3,COL_SAND_DARK);
    }
}

static void DrawSafeZoneMarkers(){
    float tiers[5]={300,900,1500,2100,2700};
    for(int i=0;i<5;i++){ float sy=WorldToScreenY(tiers[i]); if(sy<SKY_HEIGHT||sy>SCREEN_HEIGHT) continue; for(int x=0;x<SCREEN_WIDTH;x+=20) DrawRectangle(x,(int)sy,10,3,(Color){200,180,120,220}); }
}

static void DrawSky(float prog){
    Color sky = (prog<0.5f)? COL_SKY_DAY : COL_SKY_NIGHT;
    if(prog>=0.5f && prog<0.62f){ float t=(prog-0.5f)/0.12f; sky.r=(unsigned char)(113 + (28-113)*t); sky.g=(unsigned char)(188 + (32-188)*t); sky.b=(unsigned char)(225 + (64-225)*t); }
    DrawRectangle(0,0,SCREEN_WIDTH,SKY_HEIGHT,sky);
    if(prog>=0.55f){ SeedRand(777); for(int i=0;i<50;i++){ int sx=RandInt(0,SCREEN_WIDTH); int sy=RandInt(5,SKY_HEIGHT-12); if(RandInt(0,3)==0){ DrawRectangle(sx,sy,2,2,WHITE); } } }
    DrawRectangle(0,SKY_HEIGHT-4,SCREEN_WIDTH,4,(Color){110,90,60,255});
}

static void DrawSunMoon(float prog){
    float cx=SCREEN_WIDTH*(1.0f-prog);
    float cy=22 + sinf(prog*PI)*52;
    int px=4;
    if(prog<0.5f){
        int rad=10;
        for(int dy=-rad; dy<=rad; dy++) for(int dx=-rad; dx<=rad; dx++) if(dx*dx+dy*dy <= rad*rad) DrawRectangle((int)cx+dx*px,(int)cy+dy*px,px,px,(Color){255,232,90,255});
        for(int dy=-rad+2; dy<=rad-2; dy++) for(int dx=-rad+2; dx<=rad-2; dx++) if(dx*dx+dy*dy <= (rad-3)*(rad-3)) DrawRectangle((int)cx+dx*px,(int)cy+dy*px,px,px,(Color){255,210,20,255});
        for(int i=0;i<8;i++){ float ang=i*PI/4; int rx=(int)(cx + cosf(ang)*(rad+4)*px); int ry=(int)(cy + sinf(ang)*(rad+4)*px); DrawRectangle(rx,ry,px*2,px,(Color){255,220,100,200}); }
    }else{
        int rad=9;
        for(int dy=-rad; dy<=rad; dy++) for(int dx=-rad; dx<=rad; dx++) if(dx*dx+dy*dy <= rad*rad) DrawRectangle((int)cx+dx*px,(int)cy+dy*px,px,px,(Color){225,225,210,255});
        DrawRectangle((int)cx-6,(int)cy-3,px*2,px*2,(Color){200,200,190,255});
        DrawRectangle((int)cx+4,(int)cy+3,px,px,(Color){200,200,190,255});
    }
}

static void DrawWaterAndWaves(float eff){
    float sy=WorldToScreenY(eff);
    if(sy>SCREEN_HEIGHT) return;
    if(sy<SKY_HEIGHT) sy=SKY_HEIGHT;
    DrawRectangle(0,(int)sy,SCREEN_WIDTH,SCREEN_HEIGHT-(int)sy,COL_WATER_DEEP);
    DrawRectangle(0,(int)sy,SCREEN_WIDTH,14,COL_WATER);
    float t=GetTime()*3;
    for(int x=0;x<SCREEN_WIDTH;x+=16){
        float wave=sinf(x*0.06f + t)*4 + sinf(x*0.03f + t*0.7f)*2;
        int fx=x+(int)wave;
        DrawRectangle(fx,(int)sy-7,10,6,COL_WAVE_FOAM);
        DrawRectangle(fx+2,(int)sy-9,6,3,COL_WAVE_FOAM);
    }
}

static void DrawUI(){
    DrawRectangle(0,SKY_HEIGHT,SCREEN_WIDTH,40,(Color){0,0,0,120});
    int px=2;
    DrawPixelText(TextFormat("TIDE %d/5", tideIndex+1), 12, SKY_HEIGHT+8, px, WHITE);
    DrawPixelText(TextFormat("SIZE %d", crab.sizeIdx+1), 160, SKY_HEIGHT+8, px, WHITE);
    DrawPixelText(TextFormat("SHRIMP %d/%d", shrimpsEaten, SHRIMPS_NEEDED), 270, SKY_HEIGHT+8, px, WHITE);
    float timeLeft = (tidePhase==PHASE_LOW_WAVES)? (LOW_PHASE_DURATION - lowTimer) : (RISING_DURATION - tideTimer);
    if(timeLeft<0) timeLeft=0;
    DrawPixelText(TextFormat("TIME %.0f", timeLeft), 490, SKY_HEIGHT+8, px, WHITE);
    DrawPixelText(TextFormat("WAVE %d/5", waveCount+1), 620, SKY_HEIGHT+8, px, WHITE);
    if(currentShell.exists) DrawPixelText("SHELL!", 760, SKY_HEIGHT+8, px, YELLOW);
    DrawRectangle(900, SKY_HEIGHT+10, 160, 16, BLACK);
    DrawRectangle(902, SKY_HEIGHT+12, 156, 12, DARKGRAY);
    float prog = (tidePhase==PHASE_LOW_WAVES)? lowTimer/LOW_PHASE_DURATION : 1.0f;
    if(tidePhase==PHASE_RISING) prog = tideTimer / RISING_DURATION;
    if(prog>1) prog=1;
    DrawRectangle(902, SKY_HEIGHT+12, (int)(156*prog), 10, (tidePhase==PHASE_LOW_WAVES)? COL_WAVE : BLUE);
    if(crab.y > safeTargets[tideIndex]){ float safeSy=WorldToScreenY(safeTargets[tideIndex]); if(safeSy < SKY_HEIGHT+40) DrawPixelText("^ SAFE ABOVE ^", SCREEN_WIDTH/2-70, SKY_HEIGHT+42, 1, YELLOW); }
    DrawPixelText("WASD MOVE", 10, SCREEN_HEIGHT-18, 2, (Color){255,255,255,180});
}

// Menu helpers - keyboard only, mouse optional
static bool IsActionPressed(){ return IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER); }
static bool DrawMenuButton(const char* label, int x,int y,int w,int h,bool selected){
    bool hover = CheckCollisionPointRec(GetMousePosition(), (Rectangle){x,y,w,h});
    // obvious highlight for selected: bright background + thick gold outline
    Color bg;
    Color outline = BLACK;
    int outer = 0;
    if(selected){
        bg = (Color){255,240,120,255}; // bright yellow for selected - obvious
        outline = (Color){255,220,20,255}; // gold outline
        outer = 4;
    } else if(hover){
        bg = (Color){230,214,162,255};
    } else {
        bg = COL_UI_PANEL;
    }
    // obvious outline for selected
    if(selected){
        // thick gold outer outline
        DrawRectangle(x-outer, y-outer, w+outer*2, h+outer*2, outline);
        // inner black border
        DrawRectangle(x, y, w, h, BLACK);
        DrawRectangle(x+2, y+2, w-4, h-4, bg);
        // extra white inner highlight for selected
        DrawRectangle(x+4, y+4, w-8, 2, WHITE);
    } else {
        DrawRectangle(x,y,w,h,BLACK);
        DrawRectangle(x+2,y+2,w-4,h-4,bg);
        DrawRectangle(x+4,y+4,w-8,2,COL_ROCK_LIGHT);
        DrawRectangle(x+4,y+h-6,w-8,2,COL_ROCK_DARK);
    }
    int tw=MeasurePixelText(label,2);
    DrawPixelText(label, x+(w-tw)/2, y+(h-14)/2, 2, BLACK);
    // draw selector arrow for extra obviousness when selected
    if(selected){
        DrawPixelText(">", x-16, y+(h-14)/2, 2, (Color){255,220,20,255});
        DrawPixelText("<", x+w+4, y+(h-14)/2, 2, (Color){255,220,20,255});
    }
    if(hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return true;
    if(selected && IsActionPressed()) return true;
    return false;
}

static bool CollidesWorld(float x,float y,float w,float h){ for(int i=0;i<rockCount;i++) if(AABB(x,y,w,h,rocks[i].x,rocks[i].y,rocks[i].w,rocks[i].h)) return true; for(int i=0;i<gateCount;i++) if(gates[i].locked && AABB(x,y,w,h,gates[i].x,gates[i].y,gates[i].w,gates[i].h)) return true; return false; }
static int GetCollidingGateIdx(float x,float y,float w,float h){ for(int i=0;i<gateCount;i++) if(gates[i].locked && AABB(x,y,w,h,gates[i].x,gates[i].y,gates[i].w,gates[i].h)) return i; return -1; }

static void GenerateMaze(unsigned int seed){
    rockCount=0; gateCount=0;
    SeedRand(seed);
    float sectionTops[TIDE_COUNT] = {400,1000,1600,2200,2800};
    float sectionBottoms[TIDE_COUNT] = {800,1400,2000,2600,3200};
    float gapTops[4]={800,1400,2000,2600}; float gapBottoms[4]={1000,1600,2200,2800};
    int numChannels[TIDE_COUNT]={1,2,2,2,3};
    float chW[TIDE_COUNT][3]={{80,0,0},{80,35,0},{80,55,0},{80,35,0},{80,55,35}};
    float chX[TIDE_COUNT][3];
    for(int s=0;s<TIDE_COUNT;s++){ for(int c=0;c<3;c++) chX[s][c]=-1000; for(int c=0;c<numChannels[s];c++){ float w=chW[s][c]; int tries=0; float x; bool ok; do{ ok=true; x=(float)RandInt(80, SCREEN_WIDTH-80-(int)w); for(int k=0;k<c;k++){ float ox=chX[s][k]; float ow=chW[s][k]; if(fabsf((x+w/2)-(ox+ow/2)) < (w+ow)/2+40) ok=false; } tries++; }while(!ok && tries<100); chX[s][c]=x; } }
    for(int s=0;s<TIDE_COUNT;s++){
        float top=sectionTops[s]; float bottom=sectionBottoms[s];
        float ledgeYs[3];
        float ledgeChX[3][3];
        for(int c=0;c<3;c++) for(int l=0;l<3;l++) ledgeChX[l][c]=-1000;
        // generate per-ledge varying gaps for horizontal traversing and down-then-up complexity, progressive difficulty higher
        for(int ledge=0; ledge<3; ledge++){
            int varRange = 30 + (4 - s)*20; // top more variation (90) bottom less (30)
            float baseWide = chX[s][0];
            float offset = (float)RandInt(-varRange, varRange);
            float wx = baseWide + offset;
            if(wx<80) wx=80;
            if(wx>SCREEN_WIDTH-80-80) wx=SCREEN_WIDTH-160;
            // avoid overlapping narrow channels for this ledge
            for(int c=1;c<numChannels[s];c++){
                float nx=chX[s][c]; float nw=chW[s][c];
                if(fabsf((wx+40)-(nx+nw/2)) < 50){
                    if(wx < nx) wx = nx - 80 -20;
                    else wx = nx + nw +20;
                }
            }
            if(wx<80) wx=80; if(wx>SCREEN_WIDTH-160) wx=SCREEN_WIDTH-160;
            ledgeChX[ledge][0]=wx;
            for(int c=1;c<numChannels[s];c++) ledgeChX[ledge][c]=chX[s][c];
        }
        for(int ledge=0; ledge<3; ledge++){
            float ly=top+40+ledge*130+RandInt(-10,10);
            ledgeYs[ledge]=ly;
            for(float x=0;x<SCREEN_WIDTH;x+=30){
                float rw=30,rh=26;
                bool inside=false;
                for(int c=0;c<numChannels[s];c++){ float gx=ledgeChX[ledge][c]; float gw=chW[s][c]; if(x+rw>gx && x<gx+gw) {inside=true; break;} }
                if(inside) continue;
                if(rockCount<MAX_ROCKS){
                    rocks[rockCount].x=x;
                    rocks[rockCount].y=ly;
                    rocks[rockCount].w=rw+RandInt(-2,2);
                    rocks[rockCount].h=rh+RandInt(-2,2);
                    int v=RandInt(-8,8);
                    rocks[rockCount].color=(Color){(unsigned char)(148+v),(unsigned char)(126+v),(unsigned char)(98+v),255};
                    rockCount++;
                }
            }
        }
        int vertWallsPerGap = 2 + (4 - s)*3;
        for(int gap=0; gap<2; gap++){
            float gapTop = ledgeYs[gap] + 26;
            float gapBottom = ledgeYs[gap+1];
            float gapH = gapBottom - gapTop;
            if(gapH<10) continue;
            float upperX = ledgeChX[gap][0];
            float lowerX = ledgeChX[gap+1][0];
            float minX = fminf(upperX, lowerX);
            float maxX = fmaxf(upperX, lowerX) + 80;
            for(int v=0; v<vertWallsPerGap; v++){
                float rx; int tries=0; bool ok=false;
                do{
                    ok=true;
                    rx=(float)RandInt(20, SCREEN_WIDTH-60);
                    if(rx+26 > minX-15 && rx < maxX+15){
                        if(RandInt(0,10)<7) ok=false;
                    }
                    float wideXMid = (upperX+lowerX)/2;
                    if(fabsf(rx - wideXMid) < 50 && RandInt(0,3)!=0) ok=false;
                    tries++;
                }while(!ok && tries<80);
                if(!ok) continue;
                float rw = (float)RandInt(20,28);
                float rh = gapH;
                int openStyle = RandInt(0,4);
                float offsetY=0;
                if(openStyle==0){ rh = gapH - 24; offsetY=24; }
                else if(openStyle==1){ rh = gapH - 24; offsetY=0; }
                else if(openStyle==3){ rh = gapH - 48; offsetY=24; }
                else if(openStyle==4){ rh = gapH*0.6f; offsetY= (RandInt(0,1)==0)?0:gapH*0.4f; }
                if(rockCount<MAX_ROCKS){
                    rocks[rockCount].x=rx;
                    rocks[rockCount].y=gapTop + offsetY;
                    rocks[rockCount].w=rw;
                    rocks[rockCount].h=rh;
                    rocks[rockCount].color=COL_ROCK;
                    rockCount++;
                }
            }
            if(s<=2){
                float blockY = gapTop + gapH*0.5f - 12;
                float blockW = (float)RandInt(60,120);
                float blockX = minX + (maxX-minX)/2 - blockW/2 + RandInt(-30,30);
                if(blockX+blockW > minX-20 && blockX < maxX+20){
                    if(RandInt(0,1)==0) blockX = minX - blockW - 30;
                    else blockX = maxX + 30;
                }
                if(blockX>=0 && blockX+blockW < SCREEN_WIDTH && rockCount<MAX_ROCKS){
                    rocks[rockCount].x=blockX;
                    rocks[rockCount].y=blockY;
                    rocks[rockCount].w=blockW;
                    rocks[rockCount].h=24;
                    rocks[rockCount].color=COL_ROCK_DARK;
                    rockCount++;
                }
            }
        }
    }
    for(int gi=0; gi<4; gi++){
        int gapIdx=3-gi;
        float gTop=gapTops[gapIdx]; float gBot=gapBottoms[gapIdx];
        float wallY=(gTop+gBot)/2 -12;
        float doorX=(float)RandInt(220, SCREEN_WIDTH-300);
        if(gateCount<MAX_GATES){ gates[gateCount].x=doorX; gates[gateCount].y=wallY; gates[gateCount].w=80; gates[gateCount].h=32; gates[gateCount].requiredSize=gi+1; gates[gateCount].locked=true; gates[gateCount].color=COL_ROCK_GATE; gateCount++; }
        for(float x=0;x<SCREEN_WIDTH;x+=30){ float rw=30,rh=24; if(x+rw>doorX-8 && x<doorX+80+8) continue; if(rockCount<MAX_ROCKS){ rocks[rockCount].x=x; rocks[rockCount].y=wallY; rocks[rockCount].w=rw; rocks[rockCount].h=rh; rocks[rockCount].color=COL_ROCK_DARK; rockCount++; } }
    }
}



static void GenerateShrimps(){
    shrimpCount=0; shrimpsEaten=0; waitingForShell=false; shellSpawnedThisTide=false; shellRidingWave=false;
    SeedRand(currentSeed + tideIndex*1000 + 999);
    int attempts=0;
    while(shrimpCount<12 && attempts<400){
        float x=RandInt(60, SCREEN_WIDTH-60); float y=RandInt(3350,3760);
        bool coll=false; for(int i=0;i<rockCount;i++) if(AABB(x-8,y-8,16,16,rocks[i].x,rocks[i].y,rocks[i].w,rocks[i].h)) {coll=true; break;} for(int i=0;i<gateCount;i++) if(gates[i].locked && AABB(x-8,y-8,16,16,gates[i].x,gates[i].y,gates[i].w,gates[i].h)) {coll=true; break;}
        if(!coll){ shrimps[shrimpCount].x=x; shrimps[shrimpCount].y=y; shrimps[shrimpCount].alive=true; shrimps[shrimpCount].anim=RandFloat(0,6.28f); shrimpCount++; }
        attempts++;
    }
    currentShell.exists=false; currentShell.collected=false;
}

static void SpawnShellAboveWaterLine(){
    if(currentShell.exists) return;
    if(shellSpawnedThisTide) return;
    SeedRand(currentSeed + tideIndex*2000 + shrimpsEaten*10 + waveCount*100 + 5555);
    int tries=0;
    while(tries<150){
        float x=RandInt(80, SCREEN_WIDTH-80);
        // spawn at current wave edge, riding in with next wave
        float y = waveLevel - 40 + RandInt(-20,20);
        // ensure above feeding lower bound but near wave
        if(y<3300) y=3300+RandInt(0,80);
        if(y>3750) y=3750-20;
        bool coll=false; for(int i=0;i<rockCount;i++) if(AABB(x-12,y-12,24,24,rocks[i].x,rocks[i].y,rocks[i].w,rocks[i].h)) {coll=true; break;} for(int i=0;i<gateCount;i++) if(gates[i].locked && AABB(x-12,y-12,24,24,gates[i].x,gates[i].y,gates[i].w,gates[i].h)) {coll=true; break;}
        if(!coll){ currentShell.x=x; currentShell.y=y; currentShell.sizeIdx=tideIndex+1; if(currentShell.sizeIdx>4) currentShell.sizeIdx=4; currentShell.exists=true; currentShell.collected=false; shellSpawnedThisTide=true; shellRidingWave=true; break; }
        tries++;
    }
}

static void InitCrab(){
    float sizes[5]={22,32,42,52,62};
    crab.sizeIdx=tideIndex; if(crab.sizeIdx<0) crab.sizeIdx=0; if(crab.sizeIdx>4) crab.sizeIdx=4;
    crab.size=sizes[crab.sizeIdx]; crab.speed=200; crab.animTime=0; crab.moving=false; crab.upsideDown=false;
    if(tideIndex==0 && gateCount>0){ crab.x=gates[0].x+10; crab.y=gates[0].y+60; }
}

static void InitGame(unsigned int seed){
    currentSeed=seed; tideIndex=0; tideTimer=0; lowTimer=0; waterLevel=lowWater; waveLevel=lowWater;
    waveState=WAVE_PAUSE_LOW; waveStateTimer=0; waveCount=0; waitingForShell=false; tidePhase=PHASE_LOW_WAVES;
    GenerateMaze(seed); GenerateShrimps();
    crab.x=SCREEN_WIDTH/2; crab.y=2700;
    if(gateCount>0){ crab.x=gates[0].x+10; crab.y=gates[0].y+60; }
    InitCrab(); g_crabWorldY=crab.y; g_horizonLimit=g_crabScreenY - SKY_HEIGHT;
    AddOrUpdateSave(seed,false); menuSelection=0;
}

static void NextTide(){
    tideIndex++; if(tideIndex>=TIDE_COUNT) return;
    // size growth already handled on shell collection, don't double increment
    float sizes[5]={22,32,42,52,62};
    if(crab.sizeIdx<0) crab.sizeIdx=0;
    if(crab.sizeIdx>4) crab.sizeIdx=4;
    crab.size=sizes[crab.sizeIdx]; crab.speed=200;
    tideTimer=0; lowTimer=0; waterLevel=lowWater; waveLevel=lowWater;
    waveState=WAVE_PAUSE_LOW; waveStateTimer=0; waveCount=0; tidePhase=PHASE_LOW_WAVES; waitingForShell=false;
    GenerateShrimps();
    // place near next locked gate - start next to rock you cannot move yet
    bool placed=false;
    for(int i=0;i<gateCount;i++) if(gates[i].locked){ crab.x=gates[i].x+10; crab.y=gates[i].y+60; placed=true; break; }
    if(!placed){ crab.x=SCREEN_WIDTH/2; crab.y=safeTargets[tideIndex>0?tideIndex-1:0]; }
    AddOrUpdateSave(currentSeed,false); menuSelection=0;
}

static void InitAutoplay(){
    autoCrab.x=SCREEN_WIDTH/2; autoCrab.y=1500; autoCrab.sizeIdx=2; autoCrab.size=42; autoCrab.speed=120; autoCrab.animTime=0; autoCrab.moving=true; autoCrab.upsideDown=false;
    autoWaterLevel=lowWater; autoWaveLevel=lowWater; autoLowTimer=0; autoWaveCount=0; autoWaveState=WAVE_PAUSE_LOW; autoWaveStateTimer=0;
    autoShrimpCount=6; for(int i=0;i<autoShrimpCount;i++){ autoShrimps[i].x=RandInt(100,SCREEN_WIDTH-100); autoShrimps[i].y=RandInt(1200,3000); autoShrimps[i].alive=true; autoShrimps[i].anim=RandFloat(0,6.28f); }
}

static void UpdateWaves(float dt){
    const float pauseLow=WAVE_PAUSE_LOW_DUR, inDur=WAVE_IN_DUR, pauseHigh=WAVE_PAUSE_HIGH_DUR, outDur=WAVE_OUT_DUR;
    waveStateTimer+=dt;
    switch(waveState){
        case WAVE_PAUSE_LOW:
            waveLevel=lowWater;
            if(waveStateTimer>=pauseLow){
                // start of wave IN - shell should come in with next wave
                if(waitingForShell && !currentShell.exists && !shellSpawnedThisTide){
                    SpawnShellAboveWaterLine();
                    waitingForShell=false;
                }
                waveState=WAVE_IN; waveStateTimer=0;
            }
            break;
        case WAVE_IN: {
            float p=waveStateTimer/inDur; if(p>1)p=1;
            waveLevel=lowWater + (waveHigh-lowWater)*p;
            // shell rides in with wave
            if(shellRidingWave && currentShell.exists){
                currentShell.y = waveLevel - 40;
            }
            if(waveStateTimer>=inDur){ waveState=WAVE_PAUSE_HIGH; waveStateTimer=0; }
        } break;
        case WAVE_PAUSE_HIGH:
            waveLevel=waveHigh;
            if(shellRidingWave && currentShell.exists){
                currentShell.y = waveHigh - 40;
            }
            if(waveStateTimer>=pauseHigh){ waveState=WAVE_OUT; waveStateTimer=0; }
            break;
        case WAVE_OUT: {
            float p=waveStateTimer/outDur; if(p>1)p=1;
            waveLevel=waveHigh + (lowWater-waveHigh)*p;
            // when wave recedes, leave shell behind above water line
            if(shellRidingWave && currentShell.exists){
                // during OUT, keep shell at high position, not following down
                if(p>0.3f){
                    shellRidingWave=false; // stop riding, leave behind
                    currentShell.y = waveHigh - 50; // above water line
                }
            }
            if(waveStateTimer>=outDur){
                waveCount++;
                // if still waiting (e.g., missed spawn), try again next cycle
                if(waitingForShell && !currentShell.exists && !shellSpawnedThisTide){
                    // will spawn at next IN
                }
                waveState=WAVE_PAUSE_LOW; waveStateTimer=0;
            }
        } break;
    }
}

static void UpdateAutoplay(float dt){
    autoLowTimer+=dt; autoWaveStateTimer+=dt;
    const float pauseLow=WAVE_PAUSE_LOW_DUR,inDur=WAVE_IN_DUR,pauseHigh=WAVE_PAUSE_HIGH_DUR,outDur=WAVE_OUT_DUR;
    switch(autoWaveState){
        case WAVE_PAUSE_LOW: autoWaveLevel=lowWater; if(autoWaveStateTimer>=pauseLow){ autoWaveState=WAVE_IN; autoWaveStateTimer=0; } break;
        case WAVE_IN:{ float p=autoWaveStateTimer/inDur; if(p>1)p=1; autoWaveLevel=lowWater + (waveHigh-lowWater)*p; if(autoWaveStateTimer>=inDur){ autoWaveState=WAVE_PAUSE_HIGH; autoWaveStateTimer=0; } }break;
        case WAVE_PAUSE_HIGH: autoWaveLevel=waveHigh; if(autoWaveStateTimer>=pauseHigh){ autoWaveState=WAVE_OUT; autoWaveStateTimer=0; } break;
        case WAVE_OUT:{ float p=autoWaveStateTimer/outDur; if(p>1)p=1; autoWaveLevel=waveHigh + (lowWater-waveHigh)*p; if(autoWaveStateTimer>=outDur){ autoWaveCount++; autoWaveState=WAVE_PAUSE_LOW; autoWaveStateTimer=0; if(autoWaveCount>=5) autoWaveCount=0; } }break;
    }
    if(autoLowTimer>LOW_PHASE_DURATION+RISING_DURATION){ autoLowTimer=0; autoWaveCount=0; }
    float mvx=sinf(GetTime()*0.7f)*autoCrab.speed*dt; float mvy=cosf(GetTime()*0.5f)*autoCrab.speed*dt;
    autoCrab.x+=mvx; autoCrab.y+=mvy;
    if(autoCrab.x<20) autoCrab.x=20; if(autoCrab.x>SCREEN_WIDTH-20) autoCrab.x=SCREEN_WIDTH-20;
    if(autoCrab.y<200) autoCrab.y=200; if(autoCrab.y>3600) autoCrab.y=3600;
    autoCrab.animTime+=dt; g_crabWorldY=autoCrab.y; g_horizonLimit=g_crabScreenY - SKY_HEIGHT;
}

static void DrawAutoplayWorld(){
    g_crabWorldY=autoCrab.y; g_horizonLimit=g_crabScreenY - SKY_HEIGHT;
    float prog=fmodf(autoLowTimer, LOW_PHASE_DURATION+RISING_DURATION)/(LOW_PHASE_DURATION+RISING_DURATION);
    DrawSky(prog); DrawSunMoon(prog); DrawPixelSandBackground(); 
    float sy;
    for(int i=0;i<rockCount;i++){ float top=WorldToScreenY(rocks[i].y); float bot=WorldToScreenY(rocks[i].y+rocks[i].h); if(bot<SKY_HEIGHT||top>SCREEN_HEIGHT) continue; DrawRectangle((int)rocks[i].x,(int)top,(int)rocks[i].w,(int)(bot-top),rocks[i].color); }
    for(int i=0;i<autoShrimpCount;i++){ float sY=WorldToScreenY(autoShrimps[i].y); if(sY<SKY_HEIGHT||sY>SCREEN_HEIGHT+20) continue; DrawRectangle((int)autoShrimps[i].x,(int)sY,6,6,COL_SHRIMP); }
    DrawDetailedCrab(autoCrab.x,autoCrab.y,autoCrab.size,autoCrab.sizeIdx,autoCrab.animTime,true,false,-1);
    float eff=fminf(lowWater, autoWaveLevel); float wsy=WorldToScreenY(eff); if(wsy<SCREEN_HEIGHT){ if(wsy<SKY_HEIGHT) wsy=SKY_HEIGHT; DrawRectangle(0,(int)wsy,SCREEN_WIDTH,SCREEN_HEIGHT-(int)wsy,COL_WAVE); }
}

int main(){
    InitWindow(SCREEN_WIDTH,SCREEN_HEIGHT,"TidEscape");
    SetTargetFPS(60);
    g_horizonLimit=g_crabScreenY - SKY_HEIGHT;
    srand((unsigned int)time(NULL));
    LoadSaves();
    InitAutoplay();
    currentSeed=(unsigned int)time(NULL)%100000;
    GenerateMaze(currentSeed);
    InitGame(currentSeed);
    gameState=STATE_TITLE;
    float betweenTimer=0;

    while(!WindowShouldClose()){
        float dt=GetFrameTime();

        // Global menu nav handling for non-playing states
        if(gameState!=STATE_PLAYING && gameState!=STATE_BETWEEN){
            if(IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)){ menuSelection--; }
            if(IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)){ menuSelection++; }
        }

        if(gameState==STATE_TITLE){
            // wrap
            int opts = (saveCount>0)?2:1;
            if(menuSelection<0) menuSelection=opts-1;
            if(menuSelection>=opts) menuSelection=0;
            UpdateAutoplay(dt);
        } else if(gameState==STATE_PLAYING){
            if(tidePhase==PHASE_LOW_WAVES){
                lowTimer+=dt;
                UpdateWaves(dt);
                if(shrimpsEaten>=SHRIMPS_NEEDED && !currentShell.exists && !waitingForShell) waitingForShell=true;
                if(waveCount>=5 || lowTimer>=LOW_PHASE_DURATION){ tidePhase=PHASE_RISING; tideTimer=0; waveLevel=lowWater; }
            } else {
                tideTimer+=dt;
                float p=tideTimer / RISING_DURATION; if(p>1) p=1;
                waterLevel = lowWater + (highWaters[tideIndex]-lowWater)*p;
                waveLevel=lowWater;
            }
            float eff=fminf(waterLevel, waveLevel);
            // input
            float dx=0,dy=0;
            if(IsKeyDown(KEY_W)) dy=-1;
            if(IsKeyDown(KEY_S)) dy=1;
            if(IsKeyDown(KEY_A)) dx=-1;
            if(IsKeyDown(KEY_D)) dx=1;
            if(dx!=0 && dy!=0){ dx*=0.7071f; dy*=0.7071f; }
            crab.moving=(dx!=0||dy!=0);
            if(crab.moving) crab.animTime+=dt*6; else crab.animTime+=dt*1;
            float newX=crab.x + dx*crab.speed*dt;
            float newY=crab.y + dy*crab.speed*dt;
            if(newX<0) newX=0; if(newX>SCREEN_WIDTH-crab.size) newX=SCREEN_WIDTH-crab.size;
            if(newY<0) newY=0; if(newY>WORLD_HEIGHT-20) newY=WORLD_HEIGHT-20;
            int gateIdx=GetCollidingGateIdx(newX,newY,crab.size,crab.size);
            if(gateIdx>=0){
                Gate *g=&gates[gateIdx];
                if(crab.sizeIdx >= g->requiredSize){ g->locked=false; }
                else {
                    // blocked - try separate axis
                    bool collX=CollidesWorld(newX,crab.y,crab.size,crab.size);
                    bool collY=CollidesWorld(crab.x,newY,crab.size,crab.size);
                    if(!collX) crab.x=newX;
                    if(!collY) crab.y=newY;
                    goto afterMove;
                }
            }
            if(!CollidesWorld(newX,newY,crab.size,crab.size)){ crab.x=newX; crab.y=newY; }
            else {
                if(!CollidesWorld(newX,crab.y,crab.size,crab.size)) crab.x=newX;
                if(!CollidesWorld(crab.x,newY,crab.size,crab.size)) crab.y=newY;
            }
            afterMove:
            g_crabWorldY=crab.y; g_horizonLimit=g_crabScreenY - SKY_HEIGHT;
            for(int i=0;i<shrimpCount;i++) if(shrimps[i].alive && AABB(crab.x,crab.y,crab.size,crab.size, shrimps[i].x-8, shrimps[i].y-4,16,8)){ shrimps[i].alive=false; shrimpsEaten++; if(shrimpsEaten>=SHRIMPS_NEEDED && !currentShell.exists && !shellSpawnedThisTide) waitingForShell=true; }
            if(currentShell.exists && !currentShell.collected && AABB(crab.x,crab.y,crab.size,crab.size, currentShell.x-12, currentShell.y-12,24,24)){ currentShell.collected=true; currentShell.exists=false; shellRidingWave=false; if(crab.sizeIdx<4){ crab.sizeIdx++; float sizes[5]={22,32,42,52,62}; crab.size=sizes[crab.sizeIdx]; crab.speed=200; } }
            float crabBottom=crab.y + crab.size;
            if(crabBottom >= eff){ crab.upsideDown=true; gameState=STATE_GAMEOVER; menuSelection=0; }
            // win condition: reached safe with shell
            if(crab.y <= safeTargets[tideIndex] && currentShell.collected){
                if(tideIndex >= TIDE_COUNT-1){ AddOrUpdateSave(currentSeed,true); gameState=STATE_WIN; menuSelection=0; }
                else { gameState=STATE_BETWEEN; betweenTimer=0; }
            }
            // fix stall: if rising phase finished and still alive, tide goes back out and night ends (start next day)
            // This prevents night never ending if player is above water but hasn't hit exact safe pixel
            if(tidePhase==PHASE_RISING && tideTimer >= RISING_DURATION){
                // if still alive, go to between to make tide recede and night pass quickly
                if(gameState==STATE_PLAYING){
                    gameState=STATE_BETWEEN;
                    betweenTimer=0;
                }
            }
        } else if(gameState==STATE_BETWEEN){
            betweenTimer+=dt;
            // night passes quickly: accelerate sky
            if(betweenTimer>2.0f){
                NextTide();
                // place near next locked gate
                for(int i=0;i<gateCount;i++) if(gates[i].locked){ crab.x=gates[i].x+10; crab.y=gates[i].y+60; break; }
                float sizes[5]={22,32,42,52,62}; crab.size=sizes[crab.sizeIdx];
                g_crabWorldY=crab.y;
                if(tideIndex>=TIDE_COUNT){ AddOrUpdateSave(currentSeed,true); gameState=STATE_WIN; menuSelection=0; } else { gameState=STATE_PLAYING; }
            }
            g_crabWorldY=crab.y;
        } else if(gameState==STATE_LEVEL_SELECT){
            int total = saveCount+1; // +1 back
            if(menuSelection<0) menuSelection=total-1;
            if(menuSelection>=total) menuSelection=0;
            if(IsActionPressed()){
                if(menuSelection==saveCount){ gameState=STATE_TITLE; menuSelection=0; }
                else { InitGame(saves[menuSelection].seed); gameState=STATE_PLAYING; }
            }
        } else if(gameState==STATE_GAMEOVER){
            int opts=4;
            if(menuSelection<0) menuSelection=opts-1;
            if(menuSelection>=opts) menuSelection=0;
            if(IsActionPressed()){
                if(menuSelection==0){ unsigned int seed=currentSeed; InitGame(seed); gameState=STATE_PLAYING; }
                else if(menuSelection==1){ unsigned int seed=(unsigned int)time(NULL)+RandU(); InitGame(seed); gameState=STATE_PLAYING; }
                else if(menuSelection==2){ gameState=STATE_LEVEL_SELECT; menuSelection=0; }
                else { InitAutoplay(); gameState=STATE_TITLE; menuSelection=0; }
            }
        } else if(gameState==STATE_WIN){
            int opts=3;
            if(menuSelection<0) menuSelection=opts-1;
            if(menuSelection>=opts) menuSelection=0;
            if(IsActionPressed()){
                if(menuSelection==0){ unsigned int seed=(unsigned int)time(NULL)+RandU(); InitGame(seed); gameState=STATE_PLAYING; }
                else if(menuSelection==1){ gameState=STATE_LEVEL_SELECT; menuSelection=0; }
                else { InitAutoplay(); gameState=STATE_TITLE; menuSelection=0; }
            }
        }

        BeginDrawing();
        ClearBackground(COL_SAND_MID);

        if(gameState==STATE_TITLE){
            DrawAutoplayWorld();
            DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,(Color){0,0,0,90});
            DrawDetailedCrab(SCREEN_WIDTH/2, 110, 80, 3, GetTime()*2, true, false, 110);
            // logo should fill 80% width, cartoonish chunky per spec
            int logoPx = (int)((SCREEN_WIDTH*0.8f) / (9*6)); // 9 letters *6 width per char = 80% width
            if(logoPx<14) logoPx=14; // ensure chunky
            DrawLogoWithWater("TIDESCAPE", SCREEN_WIDTH/2 - MeasurePixelText("TIDESCAPE",logoPx)/2, 170, logoPx, WHITE, (Color){60,120,200,255}, COL_WAVE_FOAM, BLACK);
            DrawPixelText("A CRABS CLIMB AGAINST THE TIDE", SCREEN_WIDTH/2 - MeasurePixelText("A CRABS CLIMB AGAINST THE TIDE",2)/2, 250, 2, (Color){230,230,230,255});
            int bw=300,bh=48,bx=SCREEN_WIDTH/2-bw/2;
            // start - moved lower to accommodate 80% width logo
            bool sel0 = (menuSelection==0);
            if(DrawMenuButton("START NEW GAME", bx, 360, bw, bh, sel0)){ unsigned int seed=(unsigned int)time(NULL)+RandU(); InitGame(seed); gameState=STATE_PLAYING; }
            if(saveCount>0){
                bool sel1 = (menuSelection==1);
                if(DrawMenuButton("LEVEL SELECT", bx, 420, bw, bh, sel1)){ gameState=STATE_LEVEL_SELECT; menuSelection=0; }
            }
            DrawPixelText(TextFormat("SEED %u", currentSeed), 12, SCREEN_HEIGHT-20, 2, WHITE);
            DrawPixelText("WASD NAVIGATE  SPACE/ENTER SELECT", SCREEN_WIDTH/2 - MeasurePixelText("WASD NAVIGATE  SPACE/ENTER SELECT",1)/2, 440, 1, WHITE);
        } else if(gameState==STATE_PLAYING){
            float totalProg = (tidePhase==PHASE_LOW_WAVES)? lowTimer/(LOW_PHASE_DURATION+RISING_DURATION) : (LOW_PHASE_DURATION + tideTimer)/(LOW_PHASE_DURATION+RISING_DURATION);
            if(totalProg>1) totalProg=1;
            DrawSky(totalProg); DrawSunMoon(totalProg); DrawPixelSandBackground(); DrawSafeZoneMarkers(); DrawRocksAndGates();
            for(int i=0;i<shrimpCount;i++) if(shrimps[i].alive) DrawDetailedShrimp(shrimps[i]);
            DrawDetailedShell(currentShell);
            float eff=fminf(waterLevel, waveLevel);
            DrawWaterAndWaves(eff);
            DrawDetailedCrab(crab.x,crab.y,crab.size,crab.sizeIdx,crab.animTime,crab.moving,crab.upsideDown,-1);
            DrawUI();
            if(tidePhase==PHASE_LOW_WAVES) DrawPixelText(TextFormat("WAVE %d/5", waveCount+1), SCREEN_WIDTH/2-60, SKY_HEIGHT+42, 2, COL_WAVE_FOAM);
        } else if(gameState==STATE_BETWEEN){
            // night passes quickly
            float fastProg = 0.7f + (betweenTimer/2.0f)*0.6f; if(fastProg>1.0f) fastProg = fmodf(fastProg,1.0f);
            DrawSky(fastProg); DrawSunMoon(fastProg); DrawPixelSandBackground(); DrawSafeZoneMarkers(); DrawRocksAndGates();
            DrawWaterAndWaves(waterLevel);
            DrawDetailedCrab(crab.x,crab.y,crab.size,crab.sizeIdx,crab.animTime,crab.moving,crab.upsideDown,-1);
            DrawUI();
            DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,(Color){0,0,0,100});
            DrawPixelText(TextFormat("TIDE %d SURVIVED", tideIndex+1), SCREEN_WIDTH/2 - MeasurePixelText(TextFormat("TIDE %d SURVIVED", tideIndex+1),3)/2, SCREEN_HEIGHT/2-30, 3, WHITE);
            DrawPixelText("NIGHT PASSES QUICKLY...", SCREEN_WIDTH/2 - MeasurePixelText("NIGHT PASSES QUICKLY...",2)/2, SCREEN_HEIGHT/2+10, 2, WHITE);
        } else if(gameState==STATE_GAMEOVER){
            DrawSky(0.9f); DrawSunMoon(0.9f); DrawPixelSandBackground(); DrawSafeZoneMarkers(); DrawRocksAndGates();
            DrawWaterAndWaves(waterLevel);
            DrawDetailedCrab(crab.x,crab.y,crab.size,crab.sizeIdx,crab.animTime,false,true,-1);
            DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,(Color){0,0,0,130});
            DrawLogoWithWater("GAME OVER", SCREEN_WIDTH/2 - MeasurePixelText("GAME OVER",7)/2, 120, 7, RED, (Color){80,20,20,255}, COL_WAVE_FOAM, BLACK);
            DrawPixelText("THE TIDE TOUCHED YOU", SCREEN_WIDTH/2 - MeasurePixelText("THE TIDE TOUCHED YOU",2)/2, 180, 2, WHITE);
            int bw=220,bh=40,bx=SCREEN_WIDTH/2-bw/2;
            DrawMenuButton("RETRY", bx, 240, bw, bh, menuSelection==0);
            DrawMenuButton("NEW GAME", bx, 290, bw, bh, menuSelection==1);
            DrawMenuButton("LEVEL SELECT", bx, 340, bw, bh, menuSelection==2);
            DrawMenuButton("TITLE", bx, 390, bw, bh, menuSelection==3);
            // trigger handled in update
        } else if(gameState==STATE_WIN){
            ClearBackground(COL_SKY_DAY);
            DrawLogoWithWater("YOU WIN", SCREEN_WIDTH/2 - MeasurePixelText("YOU WIN",8)/2, 100, 8, (Color){240,200,20,255}, (Color){60,120,200,255}, COL_WAVE_FOAM, BLACK);
            DrawPixelText("YOU SURVIVED 5 HIGH TIDES", SCREEN_WIDTH/2 - MeasurePixelText("YOU SURVIVED 5 HIGH TIDES",2)/2, 170, 2, DARKGREEN);
            DrawDetailedCrab(SCREEN_WIDTH/2, 300, 70, 4, GetTime()*4, true, false, 300);
            int bw=220,bh=40,bx=SCREEN_WIDTH/2-bw/2;
            DrawMenuButton("NEW GAME", bx, 420, bw, bh, menuSelection==0);
            DrawMenuButton("LEVEL SELECT", bx, 470, bw, bh, menuSelection==1);
            DrawMenuButton("TITLE", bx, 520, bw, bh, menuSelection==2);
        } else if(gameState==STATE_LEVEL_SELECT){
            ClearBackground(COL_SAND_MID);
            DrawPixelText("LEVEL SELECT", 30, 20, 4, BLACK);
            DrawPixelText("SEED - COMPLETED", 30, 60, 2, DARKGRAY);
            int y=90;
            for(int i=0;i<saveCount && i<16;i++){
                char buf[64]; sprintf(buf,"%u - %s", saves[i].seed, saves[i].completed?"COMPLETED":"NOT");
                bool sel = (menuSelection==i);
                if(sel){
                    // obvious highlight - thick gold outline + bright yellow bg
                    DrawRectangle(26,y-4,368,36,(Color){255,220,20,255});
                    DrawRectangle(30,y,360,28,BLACK);
                    DrawRectangle(32,y+2,356,24,(Color){255,240,120,255});
                    DrawPixelText(">", 10, y+6, 2, (Color){255,220,20,255});
                } else {
                    DrawRectangle(30,y,360,28,BLACK);
                    DrawRectangle(32,y+2,356,24,WHITE);
                }
                DrawPixelText(buf, 36, y+6, 2, BLACK);
                y+=32;
            }
            bool backSel = (menuSelection==saveCount);
            DrawMenuButton("BACK", 30, SCREEN_HEIGHT-50, 120, 36, backSel);
            DrawPixelText("SEEDS ARE PROCEDURALLY GENERATED MAZES", 500, 90, 1, DARKBROWN);
            DrawPixelText("WASD NAVIGATE SPACE SELECT", 500, 110, 1, DARKBROWN);
            DrawPixelText("WIN: SURVIVE 5 TIDES", 500, 130, 1, DARKBROWN);
            DrawPixelText("GATES NEED GROWTH TO OPEN", 500, 150, 1, DARKBROWN);
            DrawPixelText("WAVES 5 TIMES AT LOW TIDE", 500, 170, 1, DARKBROWN);
            DrawPixelText("SHELL ABOVE WATER AFTER WAVE", 500, 190, 1, DARKBROWN);
        }

        EndDrawing();
    }

    SaveSaves();
    CloseWindow();
    return 0;
}
