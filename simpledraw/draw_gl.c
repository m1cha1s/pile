#include <stddef.h>
#include <stdint.h>

#include "draw.h"
#include <math.h>

#define TRIANGLE_COUNT 1000
#define MULTILINE_STR(...) #__VA_ARGS__

const char *vertexShader = "#version 330\n"MULTILINE_STR(
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec4 aColor;

out vec2 uv;
out vec4 color;

uniform vec2 uResolution;

void main()
{
    vec2 halfRes = uResolution/2;
    vec2 pos = aPos.xy / halfRes;
    pos.x = pos.x-1;
    pos.y = 1-pos.y;

    uv = aUV;
    color = aColor;

    gl_Position = vec4(pos.xy, 0, 1.0);
}
);

const char *fragmentShader = "#version 330\n"MULTILINE_STR(
in vec2 uv;
in vec4 color;

out vec4 fragColor;

void main()
{
    fragColor = color;
}
);

typedef struct {
    float pos[3];
    float uv[2];
    float color[4];
} Vertex;

const int vertCap = 3*TRIANGLE_COUNT;
int vertCount = 0;
Vertex verts[3*TRIANGLE_COUNT];

GLuint vbo, vao, prog;

static vec4_t colors[D_COLOR_TYPE_COUNT] = {0};

float D_StrokeWidth = 3;

static GLuint CompileShader(const char *src, int type, int *err)
{
    *err = false;

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar* const*)&src, 0);
    glCompileShader(shader);

    int success;
    size_t infoLogSize;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, sizeof(infoLog), (GLsizei * ) & infoLogSize, (GLchar *) infoLog);
        printf("%.*s\n", (int) infoLogSize, infoLog); // TODO: replace
        *err = true;
    }

    return shader;
}

static GLuint CompileProg(const char *vert, const char *frag)
{
    GLuint v, f, p;
    int err = 0;

    if (vert)
    {
        v = CompileShader(vert, GL_VERTEX_SHADER, &err);
        if (err) return 0;
    }

    if (frag)
    {
        f = CompileShader(frag, GL_FRAGMENT_SHADER, &err);
        if (err) return 0;
    }

    p = glCreateProgram();
    if (vert) glAttachShader(p, v);
    if (frag) glAttachShader(p, f);

    glLinkProgram(p);

    if (vert) glDeleteShader(v);
    if (frag) glDeleteShader(f);

    int success;
    size_t infoLogSize;
    char infoLog[512];
    glGetProgramiv(p, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(p, sizeof(infoLog), (GLsizei*)&infoLogSize, (GLchar*)infoLog);
        printf("%.*s\n", (int)infoLogSize, infoLog);
        glDeleteProgram(p);
        return 0;
    }

    return p;
}

void D_GL_Init(void)
{
//    printf("Vertex shader: %s\n", vertexShader);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), NULL, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(2);

    prog = CompileProg(vertexShader, fragmentShader);
}

void D_SetColor(ColorType type, float r, float g, float b, float a)
{
    colors[type][0] = r;
    colors[type][1] = g;
    colors[type][2] = b;
    colors[type][3] = a;
}

void D_SetColorHex(ColorType type, uint32_t color)
{
    colors[type][0] = (float)((color >> 24)&0xFF)/255.f;
    colors[type][1] = (float)((color >> 16)&0xFF)/255.f;
    colors[type][2] = (float)((color >> 8)&0xFF)/255.f;
    colors[type][3] = (float)((color)&0xFF)/255.f;
}

void D_SetColorVec4(ColorType type, vec4_t color)
{
    colors[type][0] = color[0];
    colors[type][1] = color[1];
    colors[type][2] = color[2];
    colors[type][3] = color[3];
}

void D_ClearScreen(void)
{
    glClearColor(colors[D_COLOR_CLEAR][0], 
                 colors[D_COLOR_CLEAR][1], 
                 colors[D_COLOR_CLEAR][2], 
                 colors[D_COLOR_CLEAR][3]);
    glClear(GL_COLOR_BUFFER_BIT);
}

static void D_GL_Flush(GLenum mode)
{
    glUseProgram(prog);
    glBindVertexArray(vao);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(*verts)*vertCount, verts);

    int w, h;
    D_GetWindowDims(&w, &h);

    int resLoc = glGetUniformLocation(prog, "uResolution");
    glUniform2f(resLoc, w, h);

    glDrawArrays(mode, 0, vertCount);
    vertCount = 0;
}

static void D_GL_PushVertex(float x, float y, float z, float u, float v, float r, float g, float b, float a)
{
    if (vertCount >= vertCap)
    {
        D_GL_Flush(GL_TRIANGLES);
    }

    verts[vertCount++] = (Vertex){x,y,z,u,v,r,g,b,a};
}

void D_DrawFilledRect(float x, float y, float w, float h)
{
    float r = colors[D_COLOR_FILL][0];
    float g = colors[D_COLOR_FILL][1];
    float b = colors[D_COLOR_FILL][2];
    float a = colors[D_COLOR_FILL][3];

    float x1 = x+w;
    float y1 = y+h;

    D_GL_PushVertex(x, y, 0, 0, 0, r, g, b, a);
    D_GL_PushVertex(x, y1, 0, 0, 1, r, g, b, a);
    D_GL_PushVertex(x1, y, 0, 1, 0, r, g, b, a);

    D_GL_PushVertex(x1, y, 0, 1, 0, r, g, b, a);
    D_GL_PushVertex(x, y1, 0, 0, 1, r, g, b, a);
    D_GL_PushVertex(x1, y1, 0, 1, 1, r, g, b, a);

    D_GL_Flush(GL_TRIANGLES);
}

void D_DrawFilledCircle(float x, float y, float rad, int segCount)
{
    float r = colors[D_COLOR_FILL][0];
    float g = colors[D_COLOR_FILL][1];
    float b = colors[D_COLOR_FILL][2];
    float a = colors[D_COLOR_FILL][3];

    D_GL_PushVertex(x, y, 0, 0, 0, r, g, b, a);

    for (int i = 0; i < segCount+1; ++i)
    {
        float a1 = i*2*M_PI/(float)segCount;
        float a2 = (i+1)*2*M_PI/(float)segCount;

        D_GL_PushVertex(x+rad*cos(a2), y+rad*sin(a2), 0, 0, 0, r, g, b, a);
    }

    D_GL_Flush(GL_TRIANGLE_FAN);
}

void D_DrawLine(float x1, float y1, float x2, float y2)
{
    if (x1 == x2 && y1 == y2) return;

    float r = colors[D_COLOR_FILL][0];
    float g = colors[D_COLOR_FILL][1];
    float b = colors[D_COLOR_FILL][2];
    float a = colors[D_COLOR_FILL][3];

    float xdir = x2 - x1;
    float ydir = y2 - y1;

    float len = sqrt(xdir*xdir + ydir*ydir);

    float xside = (ydir/len)*D_StrokeWidth/2.f;
    float yside = (-xdir/len)*D_StrokeWidth/2.f;

    D_GL_PushVertex(x1+xside, y1+yside, 0, 0, 0, r, g, b, a);
    D_GL_PushVertex(x2+xside, y2+yside, 0, 0, 0, r, g, b, a);
    D_GL_PushVertex(x1-xside, y1-yside, 0, 0, 0, r, g, b, a);
    D_GL_PushVertex(x2-xside, y2-yside, 0, 0, 0, r, g, b, a);

    D_GL_Flush(GL_TRIANGLE_STRIP);
}

static int segNum = 0;

void D_BeginSegmentedLine(void)
{
    segNum = 0;
}

void D_DrawSegment(float x1, float y1, float x2, float y2)
{
    if (x1 == x2 && y1 == y2) return;

    float r = colors[D_COLOR_FILL][0];
    float g = colors[D_COLOR_FILL][1];
    float b = colors[D_COLOR_FILL][2];
    float a = colors[D_COLOR_FILL][3];

    float xdir = x2 - x1;
    float ydir = y2 - y1;

    float len = sqrt(xdir*xdir + ydir*ydir);

    float xside = (ydir/len)*D_StrokeWidth/2.f;
    float yside = (-xdir/len)*D_StrokeWidth/2.f;

    if (segNum == 0) {
        D_GL_PushVertex(x1+xside, y1+yside, 0, 0, 0, r, g, b, a);
        D_GL_PushVertex(x1-xside, y1-yside, 0, 0, 0, r, g, b, a);
    }

    D_GL_PushVertex(x2+xside, y2+yside, 0, 0, 0, r, g, b, a);
    D_GL_PushVertex(x2-xside, y2-yside, 0, 0, 0, r, g, b, a);

    segNum++;
}

void D_EndSegmentedLine(void)
{
    D_GL_Flush(GL_TRIANGLE_STRIP);
}

void D_DrawBezier(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int segCount)
{
    D_BeginSegmentedLine();

        for (int i = 0; i < segCount; ++i)
        {
            float t = i/(float)segCount;
            float xa = Lerp(x1, x2, t);
            float ya = Lerp(y1, y2, t);
            float xb = Lerp(x2, x3, t);
            float yb = Lerp(y2, y3, t);
            float xc = Lerp(x3, x4, t);
            float yc = Lerp(y3, y4, t);
            float xd = Lerp(xa, xb, t);
            float yd = Lerp(ya, yb, t);
            float xe = Lerp(xb, xc, t);
            float ye = Lerp(yb, yc, t);
            float xf1 = Lerp(xd, xe, t);
            float yf1 = Lerp(yd, ye, t);

            t = (i+1)/(float)segCount;
            xa = Lerp(x1, x2, t);
            ya = Lerp(y1, y2, t);
            xb = Lerp(x2, x3, t);
            yb = Lerp(y2, y3, t);
            xc = Lerp(x3, x4, t);
            yc = Lerp(y3, y4, t);
            xd = Lerp(xa, xb, t);
            yd = Lerp(ya, yb, t);
            xe = Lerp(xb, xc, t);
            ye = Lerp(yb, yc, t);
            float xf2 = Lerp(xd, xe, t);
            float yf2 = Lerp(yd, ye, t);

            D_DrawSegment(xf1, yf1, xf2, yf2);
        }

        D_EndSegmentedLine();
}

