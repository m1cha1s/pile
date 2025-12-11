#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define DRAW_GL
#define DRAW_IMPLEMENTATION
#include "draw.h"

float offx = 0;

int up=0, down=0, left=0, right=0;

typedef struct Entity {
    vec2_t pos;
    vec2_t vel;
    vec2_t dims;
    uint32_t color;
    float cooldown;
} Entity;

Entity *players = NULL;
Entity *rocks = NULL;

typedef struct Puff {
    vec2_t pos;
    vec2_t vel;
    float lifetime;
} Puff;

#define PUFF_COUNT 256

int currentPuff = 0;
Puff jetpuffs[PUFF_COUNT] = {0};

typedef enum {
    NOTE_C,
    NOTE_CS,
    NOTE_D,
    NOTE_DS,
    NOTE_E,
    NOTE_F,
    NOTE_FS,
    NOTE_G,
    NOTE_GS,
    NOTE_A,
    NOTE_AS,
    NOTE_B,
    NOTE_OCTAVE_COUNT,
} Notes;

const float NoteFreq[NOTE_OCTAVE_COUNT] = {
    16.32f,
    17.32f,
    18.35f,
    19.45f,
    20.6f,
    21.83f,
    23.12f,
    24.5f,
    25.96f,
    27.5f,
    29.14f,
    30.87f,
};

typedef enum {
    ADSR_STBY,
    ADSR_ATTACK,
    ADSR_DECAY,
    ADSR_SUSTAIN,
    ADSR_RELEASE,
} ADSRPhase;

typedef struct {
    ADSRPhase phase;
    float attack, decay, sustain, release;
    float attackStep, decayStep, releaseStep;
    float value;
} ADSR;

void ADSRPlay(ADSR *adsr)
{
    adsr->phase = ADSR_ATTACK;
}

void ADSRRelease(ADSR *adsr)
{
    adsr->phase = ADSR_RELEASE;
}

int ADSRIsPlaying(ADSR *adsr)
{
    return adsr->phase != ADSR_RELEASE && adsr->phase != ADSR_STBY;
}

void ADSRProcess(ADSR *adsr, float step)
{
    adsr->attackStep = 1/adsr->attack;
    adsr->decayStep = (1-adsr->sustain)/adsr->decay;
    adsr->releaseStep = adsr->sustain/adsr->release;

    switch (adsr->phase) 
    {
        case ADSR_STBY: 
            adsr->value = 0;
            return;
        case ADSR_ATTACK: {
            adsr->value += adsr->attackStep*step;
            if (adsr->value >= 1)
            {
                adsr->value = 1;
                adsr->phase = ADSR_DECAY;
            }
        } break;
        case ADSR_DECAY: {
            adsr->value -= adsr->decayStep*step;
            if (adsr->value <= adsr->sustain)
            {
                adsr->value = adsr->sustain;
                adsr->phase = ADSR_SUSTAIN;
            }
        } break;
        case ADSR_SUSTAIN:
            adsr->value = adsr->sustain;
            return;
        case ADSR_RELEASE: {
            adsr->value -= adsr->releaseStep*step;
            if (adsr->value <= 0)
            {
                adsr->value = 0;
                adsr->phase = ADSR_STBY;
            }
        } break;
    }
}

float phase = 0;
Notes note = NOTE_A;
ADSR adsr = {0};

typedef struct {
    Notes note;
    int octave;
    float duration;
    float advance;
} Note;

typedef struct {
    Note *notes;
    size_t noteCount;
    size_t currentNote;
    float timePassed;
} NoteSequence;

Note marioNotes[] = {
    {NOTE_E, 4, 0.1, 0.1},
    {NOTE_E, 4, 0.1, 0.2},
    {NOTE_E, 4, 0.1, 0.1},
    {NOTE_C, 4, 0.1, 0.1},
    {NOTE_E, 4, 0.1, 0.1},
    {NOTE_G, 4, 0.4, 0.1},
};

NoteSequence marioSeq = {
    .notes = marioNotes,
    .noteCount = ArrayCount(marioNotes),
    .currentNote = 0,
    .timePassed = 0,
};

float PlayNoteSequence(NoteSequence *seq, float step)
{
    if (seq->currentNote >= seq->noteCount) return 0;
    Note currNote = seq->notes[seq->currentNote];
    
    if (seq->timePassed > currNote.duration && ADSRIsPlaying(&adsr))
    {
        ADSRRelease(&adsr);
    }

    if (seq->timePassed > currNote.duration+currNote.advance)
    {
        seq->timePassed = 0;
        seq->currentNote++;
        if (seq->currentNote >= seq->noteCount) return 0;

        currNote = seq->notes[seq->currentNote];
        ADSRPlay(&adsr);
    }

    seq->timePassed += step;

    return NoteFreq[currNote.note] * powf(2.f, (float)currNote.octave);
}

void HandleAudio(float *buffer, int frameCount)
{
    for (uint32_t frame = 0; frame < frameCount; ++frame)
    {
        float freq = PlayNoteSequence(&marioSeq, 1/48000.f);
        ADSRProcess(&adsr, 1/48000.f);

        float m = sinf(freq/4 * 2*M_PI * phase);
        float mod = sinf(freq/8 * 2*M_PI * phase + m);

        buffer[2*frame] = 0.3*sinf(freq * 2*M_PI * phase + mod) * adsr.value;
        buffer[2*frame+1] = buffer[2*frame];//0.3*sinf(880 * 2*M_PI * phase + mod);

        phase += 1/48000.f;
        if (phase > 1) phase -= 1;
    }
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

        jetpuffs[i].pos[0] += jetpuffs[i].vel[0] * DeltaTime;
        jetpuffs[i].pos[1] += jetpuffs[i].vel[1] * DeltaTime;

        jetpuffs[i].vel[0] -= jetpuffs[i].vel[0] * DeltaTime * 4;
        jetpuffs[i].vel[1] -= jetpuffs[i].vel[1] * DeltaTime * 4;
    }
}

vec4_t colorPallette[5];

void DrawJetPuffs(void)
{
    for (int i = 0; i < PUFF_COUNT; ++i)
    {
        if (jetpuffs[i].lifetime <= 0) continue;

        float puffSize = jetpuffs[i].lifetime * jetpuffs[i].lifetime * 5;

        D_SetColorVec4(D_COLOR_FILL, colorPallette[0]);
        D_DrawFilledCircle(jetpuffs[i].pos[0]-puffSize*0.1, jetpuffs[i].pos[1]+puffSize*0.1, puffSize, puffSize/5);

        D_SetColorVec4(D_COLOR_FILL, colorPallette[1]);
        D_DrawFilledCircle(jetpuffs[i].pos[0], jetpuffs[i].pos[1], puffSize, puffSize/5);
    }
}

void GenerateRandomAnalogousPalette()
{
    float start = Random()*M_PI;
    float distH = fabsf(Random());
    distH = Max(0.2, distH);
    float distC = Random()*0.1;
    
    float distL = fabsf(Random());
    distL = Min(distL, 0.2);

    float L = 0.4;
    float C = Random()*0.4;
    float h = start;

    for (int i = 0; i < ArrayCount(colorPallette); ++i)
    {
        vec3_t c;
        OkLChToOkLab(c, L, C, h);
        h += distH;
        L += distL;
        L = Clamp(0, 1, L);
        OkLabToSRGB(c);
        Vector3Copy(c, colorPallette[i]);
        colorPallette[i][3] = 1;
    }
}

void GenerateRandomSquareTetradicPalette()
{
    float start = Random()*M_PI;

    float L = 1;
    float C = Random()*0.4;
    float h = start;

    for (int i = 0; i < ArrayCount(colorPallette); ++i)
    {
        vec3_t c;
        OkLChToOkLab(c, L, C, h);
        h += M_PI/2.f;
        OkLabToSRGB(c);
        Vector3Copy(c, colorPallette[i]);
        colorPallette[i][3] = 1;
    }
}

void GenerateRandomTriadicPalette()
{
    float start = Random()*M_PI;

    float L = 1;
    float C = Random()*0.4;
    float h = start;

    for (int i = 0; i < ArrayCount(colorPallette); ++i)
    {
        vec3_t c;
        OkLChToOkLab(c, L, C, h);
        h += 2*M_PI/3.f;
        OkLabToSRGB(c);
        Vector3Copy(c, colorPallette[i]);
        colorPallette[i][3] = 1;
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
        case KEY_R: {
            if (pressed) {
                GenerateRandomAnalogousPalette();
            }
        } break;
        case KEY_T: {
            if (pressed) {
                GenerateRandomSquareTetradicPalette();
            }
        } break;
        case KEY_Y: {
            if (pressed) {
                GenerateRandomTriadicPalette();
            }
        } break;
        case KEY_P: {
            if (pressed) {
                D_StartAudio();
            }
        } break;
        case KEY_L: {
            if (pressed) {
                D_StopAudio();
            }
        } break;
        case KEY_Z: {
            if (pressed && (adsr.phase == ADSR_STBY || adsr.phase == ADSR_RELEASE)) {
                adsr.attack = 0.1;
                adsr.decay = 0.1;
                adsr.sustain = 0.9;
                adsr.release = 1;
                adsr.phase = ADSR_ATTACK;
            } else if (!pressed) {
                adsr.phase = ADSR_RELEASE;
            }
        } break;
        default: break;
    }
}

const float playerSpeed = 500;

void UpdatePlayers(void)
{
    vec2_t wishdir = {0};
    if (up) wishdir[1] -= 1;
    if (down) wishdir[1] += 1;
    if (left) wishdir[0] -= 1;
    if (right) wishdir[0] += 1;

    vec2_t wishspd;
    wishspd[0] = wishdir[0] * playerSpeed;
    wishspd[1] = wishdir[1] * playerSpeed;

    FiLerp(&players[0].vel[0], wishspd[0], 5);
    FiLerp(&players[0].vel[1], wishspd[1], 5);

    players[0].pos[0] += players[0].vel[0] * DeltaTime;
    players[0].pos[1] += players[0].vel[1] * DeltaTime;

    players[0].cooldown -= DeltaTime;

    if ((wishdir[0] != 0 || wishdir[1] != 0) && players[0].cooldown <= 0)
    {
        AddJetPuff(
                players[0].pos[0] + players[0].dims[0] / 2 + Random() * players[0].dims[0] - wishdir[0] * players[0].dims[0],
                players[0].pos[1] + players[0].dims[1] / 2 + Random() * players[0].dims[1] - wishdir[1] * players[0].dims[1],
                -wishdir[0] * 500 + Random() * 50,
                -wishdir[1] * 500 + Random() * 50,
                3);
        players[0].cooldown = 0.002;
    }
}

void DrawPlayers(void)
{
    for (int i = 0; i < ArrayLen(players); ++i)
    {
        D_SetColorVec4(D_COLOR_FILL, colorPallette[3]);
        D_DrawFilledCircleV(players[i].pos, players[i].dims[0]/2, 20);
    }
}

int main(int argc, char **argv)
{
    D_Init();

    D_InitAudio(48000);
    // D_StartAudio();

    Entity player = {
            .dims = {50, 50},
            .pos = {400, 300},
            .color = 0xccff00ff,
    };
    ArrayPush(players, player);

    D_SetWindowDims( 800, 600 );
    D_SetWindowTitle( "test" );

    GenerateRandomAnalogousPalette();

    while (D_Running())
    {
        D_SetColor( D_COLOR_CLEAR, 0, 0, 0, 1 );
        D_ClearScreen();

        UpdatePlayers();
        UpdateJetPuffs();

        DrawJetPuffs();
        DrawPlayers();

        for (int i = 0; i < ArrayCount(colorPallette); ++i)
        {
            D_SetColorVec4(D_COLOR_FILL, colorPallette[i]);
            D_DrawFilledCircle(50+i*20, 20, 10, 10);
        }

        D_SetColorVec4(D_COLOR_FILL, colorPallette[0]);
        D_DrawFilledCircle(10, 10, 5, 10);
        D_DrawFilledCircle(10, 100, 5, 10);
        
        float p3[2] = { mousePos[0], mousePos[1] };
        float p4[2] = { p3[0], p3[1]+100 }; 

        D_DrawFilledCircleV(p3, 5, 10);
        D_DrawFilledCircleV(p4, 5, 10);
        
        D_SetColorVec4(D_COLOR_FILL, colorPallette[1]);
        D_StrokeWidth = 2;
        D_DrawBezier(10, 10, 10, 100, p3[0], p3[1], p4[0], p4[1], 50);

        printf("adsr: %f\n", adsr.value);

        D_Present();
    }

    D_StopAudio();
    D_Deinit();
}
