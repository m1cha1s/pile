#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "draw.h"

#define MEM_IMPLEMENTATION
#include <mem.h>

float offx = 0;

int up=0, down=0, left=0, right=0;

typedef struct Entity {
    Vector2 pos;
    Vector2 vel;
    Vector2 dims;
    uint32_t color;
    float cooldown;
} Entity;

Entity *players = NULL;
Entity *rocks = NULL;

typedef struct Puff {
    Vector2 pos;
    Vector2 vel;
    float lifetime;
} Puff;

#define PUFF_COUNT 256

int currentPuff = 0;
Puff jetpuffs[PUFF_COUNT] = {0};

float Lerp(float a, float b, float t)
{
    return a+t*(b-a);
}

void AddJetPuff(float x, float y, float vx, float vy, float lifetime)
{
    Puff jp = {
        .pos = {x,y},
        .vel = {vx, vy},
        .lifetime = lifetime,
    };
    jetpuffs[currentPuff++] = jp;
    currentPuff = currentPuff % PUFF_COUNT;
}

void UpdateJetPuffs(void)
{
    for (int i = 0; i < PUFF_COUNT; ++i)
    {
        if (jetpuffs[i].lifetime <= 0) continue;
        jetpuffs[i].lifetime -= DeltaTime;

        jetpuffs[i].pos.x += jetpuffs[i].vel.x * DeltaTime;
        jetpuffs[i].pos.y += jetpuffs[i].vel.y * DeltaTime;

        jetpuffs[i].vel.x -= jetpuffs[i].vel.x * DeltaTime * 4;
        jetpuffs[i].vel.y -= jetpuffs[i].vel.y * DeltaTime * 4;
    }
}

void DrawJetPuffs(void)
{
    for (int i = 0; i < PUFF_COUNT; ++i)
    {
        if (jetpuffs[i].lifetime <= 0) continue;

        float puffSize = jetpuffs[i].lifetime * jetpuffs[i].lifetime * 5;

        D_FillColor = 0xaaaaaaff;
        D_DrawFilledRect(jetpuffs[i].pos.x-puffSize/2-puffSize*0.1, jetpuffs[i].pos.y-puffSize/2+puffSize*0.1, puffSize, puffSize);

        D_FillColor = 0xddddddff;
        D_DrawFilledRect(jetpuffs[i].pos.x-puffSize/2, jetpuffs[i].pos.y-puffSize/2, puffSize, puffSize);
    }
}

void HandleKey(KeyboardKey key, int c, int pressed)
{
    switch (key)
    {
        case KEY_W: up=pressed; break;
        case KEY_S: down=pressed; break;
        case KEY_A: left=pressed; break;
        case KEY_D: right=pressed; break;
        default: break;
    }
}

const float playerSpeed = 500;

void FiLerp(float *a, float b, float decay) {
    *a = b+((*a)-b)*exp(-decay*DeltaTime);
}

float Random(void)
{
    return (float)((rand() % 200)-100)/100.f;
}

void UpdatePlayers(void)
{
    Vector2 wishdir = {0};
    if (up) wishdir.y -= 1;
    if (down) wishdir.y += 1;
    if (left) wishdir.x -= 1;
    if (right) wishdir.x += 1;

    Vector2 wishspd;
    wishspd.x = wishdir.x * playerSpeed;
    wishspd.y = wishdir.y * playerSpeed;

    FiLerp(&players[0].vel.x, wishspd.x, 5);
    FiLerp(&players[0].vel.y, wishspd.y, 5);

    players[0].pos.x += players[0].vel.x * DeltaTime;
    players[0].pos.y += players[0].vel.y * DeltaTime;

    players[0].cooldown -= DeltaTime;

    if ((wishdir.x != 0 || wishdir.y != 0) && players[0].cooldown <= 0)
    {
        AddJetPuff(
                players[0].pos.x + players[0].dims.x / 2 + Random() * players[0].dims.x - wishdir.x * players[0].dims.x,
                players[0].pos.y + players[0].dims.y / 2 + Random() * players[0].dims.y - wishdir.y * players[0].dims.y,
                -wishdir.x * 500 + Random() * 50,
                -wishdir.y * 500 + Random() * 50,
                3);
        players[0].cooldown = 0.002;
    }
}

void DrawPlayers(void)
{
    for (int i = 0; i < ArrayLen(players); ++i)
    {
        D_FillColor = players[i].color;
        D_DrawFilledRect(players[i].pos.x, players[i].pos.y, players[i].dims.x, players[i].dims.y);
    }
}

int main(int argc, char **argv)
{
    D_Init();

    Entity player = {
            .dims = {50, 50},
            .pos = {400, 300},
            .color = 0xccff00ff,
    };
    ArrayPush(players, player);

    D_SetWindowDims( 800, 600 );
    D_SetWindowTitle( "test" );

    while (D_Running())
    {
        D_ClearColor = 0x000000ff;
        D_ClearScreen();

        UpdatePlayers();
        UpdateJetPuffs();

        DrawJetPuffs();
        DrawPlayers();

        D_FillColor = 0xacacacff;
        D_StrokeWidth = 10;
        D_DrawLine(0, 0, mousePos.x, mousePos.y);

        float x1=10,y1=10,x2=10,y2=20,x3=20,y3=20,x4=20,y4=10;

        float t = 0.5;
        float xa = Lerp(x1, x2, t);
        float ya = Lerp(y1, y2, t);
        float xb = Lerp(x2, x3, t);

        D_Present();
    }

    D_Deinit();
}
