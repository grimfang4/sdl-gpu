// Hacks to fix compile errors due to polluted namespace
#ifdef _WIN32
#define _WINUSER_H
#define _WINGDI_H
#endif

#include "SDL_gpu_OpenGL_internal.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265f
#endif

#ifndef DEGPERRAD
#define DEGPERRAD 57.2957795f
#endif

#ifndef RADPERDEG
#define RADPERDEG 0.0174532925f
#endif


static void Circle(GPU_ShapeRenderer* renderer, GPU_Target* target, float x, float y, float radius, SDL_Color color);



static inline void bindTexture(GPU_Renderer* renderer, GPU_Image* image)
{
    // Bind the texture to which subsequent calls refer
    if(image != ((RendererData_OpenGL*)renderer->data)->last_image)
    {
        GLuint handle = ((ImageData_OpenGL*)image->data)->handle;
        renderer->FlushBlitBuffer(renderer);
        
        glBindTexture( GL_TEXTURE_2D, handle );
        ((RendererData_OpenGL*)renderer->data)->last_image = image;
    }
}

static inline void bindFramebuffer(GPU_Renderer* renderer, GPU_Target* target)
{
    // Bind the FBO
    if(target != ((RendererData_OpenGL*)renderer->data)->last_target)
    {
        GLuint handle = ((TargetData_OpenGL*)target->data)->handle;
        renderer->FlushBlitBuffer(renderer);
        
        glBindFramebuffer(GL_FRAMEBUFFER, handle);
        ((RendererData_OpenGL*)renderer->data)->last_target = target;
    }
}


#ifdef SDL_GPU_USE_SDL2
    #define GET_WINDOW(shape_renderer) ((GPU_RendererData_OpenGL*)shape_renderer->renderer->data)->window
    #define GET_ALPHA(sdl_color) (sdl_color.a)
#else
	#define GET_WINDOW(shape_renderer) SDL_GetVideoSurface()
    #define GET_ALPHA(sdl_color) (sdl_color.unused)
#endif


#define BEGIN \
	if(target == NULL) \
                return; \
        if(renderer->renderer != target->renderer) \
                return; \
        float z = ((RendererData_OpenGL*)renderer->renderer->data)->z;  \
         \
        renderer->renderer->FlushBlitBuffer(renderer->renderer); \
        bindFramebuffer(renderer->renderer, target); \
        /*glPushAttrib(GL_COLOR_BUFFER_BIT);*/ \
        if(target->useClip) \
        { \
                glEnable(GL_SCISSOR_TEST); \
		int y = (renderer->renderer->display == target? renderer->renderer->display->h - (target->clipRect.y + target->clipRect.h) : target->clipRect.y); \
		float xFactor = ((float)renderer->renderer->window_w)/renderer->renderer->display->w; \
		float yFactor = ((float)renderer->renderer->window_h)/renderer->renderer->display->h; \
		glScissor(target->clipRect.x * xFactor, y * yFactor, target->clipRect.w * xFactor, target->clipRect.h * yFactor); \
	} \
	 \
	glDisable( GL_TEXTURE_2D ); \
	GLint vp[4]; \
    if(renderer->renderer->display != target) \
    { \
        glGetIntegerv(GL_VIEWPORT, vp); \
        \
        unsigned int w = target->w; \
        unsigned int h = target->h; \
        \
        glViewport( 0, 0, w, h); \
        \
        glMatrixMode( GL_PROJECTION ); \
        glPushMatrix(); \
        glLoadIdentity(); \
        \
        glOrtho(0.0f, w, 0.0f, h, -1.0f, 1.0f); /* Special inverted orthographic projection because tex coords are inverted already. */ \
        \
        glMatrixMode( GL_MODELVIEW ); \
    }

#define END \
    if(renderer->renderer->display != target) \
    { \
        glViewport(vp[0], vp[1], vp[2], vp[3]); \
         \
        glMatrixMode( GL_PROJECTION ); \
        glPopMatrix(); \
        glMatrixMode( GL_MODELVIEW ); \
    } \
	if(target->useClip) \
	{ \
			glDisable(GL_SCISSOR_TEST); \
	} \
	/*glPopAttrib();*/ \
	glColor4ub(255, 255, 255, 255); \
	glEnable( GL_TEXTURE_2D );



	
	
static inline void set_vertex(float* verts, int index, float x, float y, float z)
{
    verts[index*3] = x;
    verts[index*3 + 1] = y;
    verts[index*3 + 2] = z;
}


static inline void draw_vertices(GLfloat* glverts, int num_vertices, GLenum prim_type)
{
    #ifdef SDL_GPU_USE_OPENGLv1
        glBegin(prim_type);
        int size = 3*num_vertices;
        for(int i = 0; i < size; i += 3)
        {
            glVertex3f(glverts[i], glverts[i+1], glverts[i+2]);
        }
        glEnd();
    #else
        glVertexPointer(3, GL_FLOAT, 0, glverts);
        glEnableClientState(GL_VERTEX_ARRAY);
        glDrawArrays(prim_type, 0, num_vertices);
        glDisableClientState(GL_VERTEX_ARRAY);
    #endif
}


static float SetThickness(GPU_ShapeRenderer* renderer, float thickness)
{
	float old = ((ShapeRendererData_OpenGL*)renderer->data)->line_thickness;
	((ShapeRendererData_OpenGL*)renderer->data)->line_thickness = thickness;
	glLineWidth(thickness);
	return old;
}

static float GetThickness(GPU_ShapeRenderer* renderer)
{
    return ((ShapeRendererData_OpenGL*)renderer->data)->line_thickness;
}

static void Pixel(GPU_ShapeRenderer* renderer, GPU_Target* target, float x, float y, SDL_Color color)
{
	BEGIN;
	
		glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, GET_ALPHA(color)/255.5f);

		GLfloat glverts[3];

		glverts[0] = x;
		glverts[1] = y;
		glverts[2] = z;
        
        draw_vertices(glverts, 1, GL_POINTS);

    END;
}

static void Line(GPU_ShapeRenderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color)
{
    BEGIN;

        glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, GET_ALPHA(color)/255.5f);

        GLfloat glverts[6];

        glverts[0] = x1;
        glverts[1] = y1;
        glverts[2] = z;
        glverts[3] = x2;
        glverts[4] = y2;
        glverts[5] = z;

        draw_vertices(glverts, 2, GL_LINES);

    END;
}


static void Arc(GPU_ShapeRenderer* renderer, GPU_Target* target, float x, float y, float radius, float startAngle, float endAngle, SDL_Color color)
{
    float originalSA = startAngle;

    if(startAngle > endAngle)
    {
        float swapa = endAngle;
        endAngle = startAngle;
        startAngle = swapa;
    }
    if(startAngle == endAngle)
        return;

    // Big angle
    if(endAngle - startAngle >= 360)
    {
        Circle(renderer, target, x, y, radius, color);
        return;
    }

    // Shift together
    while(startAngle < 0 && endAngle < 0)
    {
        startAngle += 360;
        endAngle += 360;
    }
    while(startAngle > 360 && endAngle > 360)
    {
        startAngle -= 360;
        endAngle -= 360;
    }

    // Check if the angle to be drawn crosses 0
    Uint8 crossesZero = (startAngle < 0 && endAngle > 0) || (startAngle < 360 && endAngle > 360);

    if(endAngle == 0)
        endAngle = 360;
    else if(crossesZero)
    {
        float sa = originalSA;
        // Render the left part
        while(sa < 0.0f)
            sa += 360;
        Arc(renderer, target, x, y, radius, sa, 359.9f, color);

        // Continue to render the right part
        startAngle = 0;
        while(endAngle >= 360)
            endAngle -= 360;
    }


    BEGIN;

    glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, GET_ALPHA(color)/255.5f);
    float t = startAngle;
    float dt = (1 - (endAngle - startAngle)/360) * 5;  // A segment every 5 degrees of a full circle
    float dx, dy;

    int numSegments = fabs(endAngle - startAngle)/dt;

    GLfloat glverts[(numSegments+2)*3];  // Extra vertex for endpoint

    int i;
    for(i = 0; i < numSegments+1; i++)
    {
        dx = radius*cos(t*RADPERDEG);
        dy = radius*sin(t*RADPERDEG);
        glverts[i*3] = x+dx;
        glverts[i*3+1] = y+dy;
        glverts[i*3+2] = z;
        t += dt;
    }
    
    dx = radius*cos(endAngle*RADPERDEG);
    dy = radius*sin(endAngle*RADPERDEG);
    glverts[i*3] = x+dx;
    glverts[i*3+1] = y+dy;
    glverts[i*3+2] = z;

    draw_vertices(glverts, numSegments+2, GL_LINE_STRIP);

    /*glBegin(GL_LINE_STRIP);
    dx = radius*cos(t*RADPERDEG);
    dy = radius*sin(t*RADPERDEG);
    glVertex3f(x+dx, y+dy, z);
    while(t < endAngle)
    {
    	t += dt;
    	dx = radius*cos(t*RADPERDEG);
        dy = radius*sin(t*RADPERDEG);
        glVertex3f(x+dx, y+dy, z);
    }
    glEnd();*/

    END;
}


static void ArcFilled(GPU_ShapeRenderer* renderer, GPU_Target* target, float x, float y, float radius, float startAngle, float endAngle, SDL_Color color)
{
    float originalSA = startAngle;

    if(startAngle > endAngle)
    {
        float swapa = endAngle;
        endAngle = startAngle;
        startAngle = swapa;
    }
    if(startAngle == endAngle)
        return;

    // Big angle
    if(endAngle - startAngle >= 360)
    {
        Circle(renderer, target, x, y, radius, color);
        return;
    }

    // Shift together
    while(startAngle < 0 && endAngle < 0)
    {
        startAngle += 360;
        endAngle += 360;
    }
    while(startAngle > 360 && endAngle > 360)
    {
        startAngle -= 360;
        endAngle -= 360;
    }

    // Check if the angle to be drawn crosses 0
    Uint8 crossesZero = (startAngle < 0 && endAngle > 0) || (startAngle < 360 && endAngle > 360);

    if(endAngle == 0)
        endAngle = 360;
    else if(crossesZero)
    {
        float sa = originalSA;

        // Render the left part
        while(sa < 0.0f)
            sa += 360;
        ArcFilled(renderer, target, x, y, radius, sa, 359.9f, color);

        // Continue to render the right part
        startAngle = 0;
        while(endAngle >= 360)
            endAngle -= 360;
    }


    BEGIN;

    glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, GET_ALPHA(color)/255.5f);
    float t = startAngle;
    float dt = (1 - (endAngle - startAngle)/360) * 5;  // A segment every 5 degrees of a full circle
    float dx, dy;

    int numSegments = fabs(endAngle - startAngle)/dt;

    GLfloat glverts[(numSegments+3)*3];  // Extra vertex for the center and endpoint

    glverts[0] = x;
    glverts[1] = y;
    glverts[2] = z;
    int i;
    for(i = 1; i < numSegments+2; i++)
    {
        dx = radius*cos(t*RADPERDEG);
        dy = radius*sin(t*RADPERDEG);
        glverts[i*3] = x+dx;
        glverts[i*3+1] = y+dy;
        glverts[i*3+2] = z;
        t += dt;
    }

    dx = radius*cos(endAngle*RADPERDEG);
    dy = radius*sin(endAngle*RADPERDEG);
    glverts[i*3] = x+dx;
    glverts[i*3+1] = y+dy;
    glverts[i*3+2] = z;
    
    draw_vertices(glverts, numSegments+3, GL_TRIANGLE_FAN);

    /*glBegin(GL_TRIANGLE_FAN);
    glVertex3f(x, y, z);
    dx = radius*cos(t*RADPERDEG);
    dy = radius*sin(t*RADPERDEG);
    while(t < endAngle)
    {
    	glVertex3f(x+dx, y+dy, z);
    	t += dt;
    	dx = radius*cos(t*RADPERDEG);
        dy = radius*sin(t*RADPERDEG);
        glVertex3f(x+dx, y+dy, z);
    }
    glEnd();*/

    END;
}

static void Circle(GPU_ShapeRenderer* renderer, GPU_Target* target, float x, float y, float radius, SDL_Color color)
{
    BEGIN;

    glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, GET_ALPHA(color)/255.5f);
    float t = 0;
    float dt = 5;  // A segment every 5 degrees of a full circle
    float dx, dy;
    int numSegments = 360/dt+1;

    GLfloat glverts[numSegments*3];

    int i;
    for(i = 0; i < numSegments; i++)
    {
        dx = radius*cos(t*RADPERDEG);
        dy = radius*sin(t*RADPERDEG);
        glverts[i*3] = x+dx;
        glverts[i*3+1] = y+dy;
        glverts[i*3+2] = z;
        t += dt;
    }

    draw_vertices(glverts, numSegments, GL_LINE_LOOP);

    END;
}

static void CircleFilled(GPU_ShapeRenderer* renderer, GPU_Target* target, float x, float y, float radius, SDL_Color color)
{
    BEGIN;

    glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, GET_ALPHA(color)/255.5f);
    float t = 0;
    float dt = 5;  // A segment every 5 degrees of a full circle
    float dx, dy;

    int numSegments = 360/dt+1;

    GLfloat glverts[(1+numSegments)*3];

    glverts[0] = x;
    glverts[1] = y;
    glverts[2] = z;
    int i;
    for(i = 1; i < 1+numSegments; i++)
    {
        dx = radius*cos(t*RADPERDEG);
        dy = radius*sin(t*RADPERDEG);
        glverts[i*3] = x+dx;
        glverts[i*3+1] = y+dy;
        glverts[i*3+2] = z;
        t += dt;
    }

    draw_vertices(glverts, 1+numSegments, GL_TRIANGLE_FAN);

    /*glBegin(GL_TRIANGLE_FAN);
    dx = radius*cos(t*RADPERDEG);
    dy = radius*sin(t*RADPERDEG);
    glVertex3f(x+dx, y+dy, z);
    while(t < 360)
    {
    	t += dt;
    	dx = radius*cos(t*RADPERDEG);
        dy = radius*sin(t*RADPERDEG);
        glVertex3f(x+dx, y+dy, z);
    }
    glEnd();*/

    END;
}

static void Tri(GPU_ShapeRenderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color)
{
    BEGIN;

    glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, GET_ALPHA(color)/255.5f);

    GLfloat glverts[9];

    glverts[0] = x1;
    glverts[1] = y1;
    glverts[2] = z;
    glverts[3] = x2;
    glverts[4] = y2;
    glverts[5] = z;
    glverts[6] = x3;
    glverts[7] = y3;
    glverts[8] = z;

    draw_vertices(glverts, 3, GL_LINE_LOOP);

    END;
}

static void TriFilled(GPU_ShapeRenderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color)
{
    BEGIN;

    glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, GET_ALPHA(color)/255.5f);

    GLfloat glverts[9];

    glverts[0] = x1;
    glverts[1] = y1;
    glverts[2] = z;
    glverts[3] = x2;
    glverts[4] = y2;
    glverts[5] = z;
    glverts[6] = x3;
    glverts[7] = y3;
    glverts[8] = z;

    draw_vertices(glverts, 3, GL_TRIANGLE_STRIP);

    END;
}

static void Rect(GPU_ShapeRenderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color)
{
    BEGIN;

    glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, GET_ALPHA(color)/255.5f);

    GLfloat glverts[12];

    glverts[0] = x1;
    glverts[1] = y1;
    glverts[2] = z;
    glverts[3] = x1;
    glverts[4] = y2;
    glverts[5] = z;
    glverts[6] = x2;
    glverts[7] = y2;
    glverts[8] = z;
    glverts[9] = x2;
    glverts[10] = y1;
    glverts[11] = z;

    draw_vertices(glverts, 4, GL_LINE_LOOP);

    END;
}

static void RectFilled(GPU_ShapeRenderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color)
{
    BEGIN;

    glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, GET_ALPHA(color)/255.5f);

    GLfloat glverts[12];

    glverts[0] = x1;
    glverts[1] = y1;
    glverts[2] = z;
    glverts[3] = x1;
    glverts[4] = y2;
    glverts[5] = z;
    glverts[6] = x2;
    glverts[7] = y1;
    glverts[8] = z;
    glverts[9] = x2;
    glverts[10] = y2;
    glverts[11] = z;

    draw_vertices(glverts, 4, GL_TRIANGLE_STRIP);

    END;
}

static void RectRound(GPU_ShapeRenderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color)
{
    if(y2 < y1)
    {
        float temp = y2;
        y2 = y1;
        y1 = temp;
    }
    if(x2 < x1)
    {
        float temp = x2;
        x2 = x1;
        x1 = temp;
    }

    BEGIN;

    glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, GET_ALPHA(color)/255.5f);
    
    //int numVerts = 8 + (int(M_PI*5)+1)*4;  // 8 + (15.7 + 1)*4
    float glverts[120*3];
    
    float i;
    int n = 0;
    for(i=(float)M_PI*1.5f;i<M_PI*2;i+=0.1f)
        set_vertex(glverts, n++, x2-radius+cos(i)*radius,y1+radius+sin(i)*radius, z);
    for(i=0;i<(float)M_PI*0.5f;i+=0.1f)
        set_vertex(glverts, n++, x2-radius+cos(i)*radius,y2-radius+sin(i)*radius, z);
    for(i=(float)M_PI*0.5f;i<M_PI;i+=0.1f)
        set_vertex(glverts, n++, x1+radius+cos(i)*radius,y2-radius+sin(i)*radius, z);
    for(i=(float)M_PI;i<M_PI*1.5f;i+=0.1f)
        set_vertex(glverts, n++, x1+radius+cos(i)*radius,y1+radius+sin(i)*radius, z);

    draw_vertices(glverts, n, GL_LINE_LOOP);

    END;
}

static void RectRoundFilled(GPU_ShapeRenderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color)
{
    if(y2 < y1)
    {
        float temp = y2;
        y2 = y1;
        y1 = temp;
    }
    if(x2 < x1)
    {
        float temp = x2;
        x2 = x1;
        x1 = temp;
    }

    BEGIN;

    glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, GET_ALPHA(color)/255.5f);
    
    //int numVerts = 8 + (int(M_PI*5)+1)*4;  // 8 + (15.7 + 1)*4
    float glverts[120*3];
    
    float i;
    int n = 0;
    set_vertex(glverts, n++, x1+radius,y1, z);
    set_vertex(glverts, n++, x2-radius,y1, z);
    for(i=(float)M_PI*1.5f;i<M_PI*2;i+=0.1f)
        set_vertex(glverts, n++, x2-radius+cos(i)*radius,y1+radius+sin(i)*radius, z);
    set_vertex(glverts, n++, x2,y1+radius, z);
    set_vertex(glverts, n++, x2,y2-radius, z);
    for(i=0;i<(float)M_PI*0.5f;i+=0.1f)
        set_vertex(glverts, n++, x2-radius+cos(i)*radius,y2-radius+sin(i)*radius, z);
    set_vertex(glverts, n++, x2-radius,y2, z);
    set_vertex(glverts, n++, x1+radius,y2, z);
    for(i=(float)M_PI*0.5f;i<M_PI;i+=0.1f)
        set_vertex(glverts, n++, x1+radius+cos(i)*radius,y2-radius+sin(i)*radius, z);
    set_vertex(glverts, n++, x1,y2-radius, z);
    set_vertex(glverts, n++, x1,y1+radius, z);
    for(i=(float)M_PI;i<M_PI*1.5f;i+=0.1f)
        set_vertex(glverts, n++, x1+radius+cos(i)*radius,y1+radius+sin(i)*radius, z);

    draw_vertices(glverts, n, GL_TRIANGLE_FAN);
    
    END;
}

static void Polygon(GPU_ShapeRenderer* renderer, GPU_Target* target, Uint16 n, float* vertices, SDL_Color color)
{
    BEGIN;

    glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, GET_ALPHA(color)/255.5f);
    
    int numIndices = 2*n;
    float glverts[numIndices*3];
    
    int i, j;
    for(i = 0, j = 0; i < numIndices; i+=2, j++)
    {
        set_vertex(glverts, j, vertices[i], vertices[i+1], z);
    }
    
    draw_vertices(glverts, n, GL_LINE_LOOP);

    END;
}

static void PolygonFilled(GPU_ShapeRenderer* renderer, GPU_Target* target, Uint16 n, float* vertices, SDL_Color color)
{
    BEGIN;

    glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, GET_ALPHA(color)/255.5f);
    
    int numIndices = 2*n;
    float glverts[numIndices*3];
    
    int i, j;
    for(i = 0, j = 0; i < numIndices; i+=2, j++)
    {
        set_vertex(glverts, j, vertices[i], vertices[i+1], z);
    }
    
    draw_vertices(glverts, n, GL_TRIANGLE_FAN);

    END;
}

static void PolygonBlit(GPU_ShapeRenderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* target, Uint16 n, float* vertices, float textureX, float textureY, float angle, float scaleX, float scaleY)
{
    BEGIN;
    
    (void)z;
/*
    glEnable( GL_TEXTURE_2D );

    bindTexture( renderer->renderer, ((ImageData_OpenGL*)src->data)->handle );

    // Set repeat mode
    // FIXME: Save old mode and reset it later
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    // TODO: Add rotation of texture using 'angle'
    // TODO: Use 'srcrect'

    int i;
    glBegin(GL_TRIANGLE_FAN);
    for(i = 0; i < 2*n; i+=2)
    {
        float x = vertices[i];
    	float y = vertices[i+1];

    	glTexCoord2f((x - textureX)*scaleX/src->w, (y - textureY)*scaleY/src->h);

        glVertex3f(x, y, z);
    }
    glEnd();

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
*/
    END;
}









GPU_ShapeRenderer* GPU_CreateShapeRenderer_OpenGL(void)
{
    GPU_ShapeRenderer* renderer = (GPU_ShapeRenderer*)malloc(sizeof(GPU_ShapeRenderer));

        renderer->data = (ShapeRendererData_OpenGL*)malloc(sizeof(ShapeRendererData_OpenGL));

    renderer->SetThickness = &SetThickness;
    renderer->SetThickness(renderer, 1.0f);
    renderer->GetThickness = &GetThickness;
    renderer->Pixel = &Pixel;
    renderer->Line = &Line;
    renderer->Arc = &Arc;
    renderer->ArcFilled = &ArcFilled;
    renderer->Circle = &Circle;
    renderer->CircleFilled = &CircleFilled;
    renderer->Tri = &Tri;
    renderer->TriFilled = &TriFilled;
    renderer->Rect = &Rect;
    renderer->RectFilled = &RectFilled;
    renderer->RectRound = &RectRound;
    renderer->RectRoundFilled = &RectRoundFilled;
    renderer->Polygon = &Polygon;
    renderer->PolygonFilled = &PolygonFilled;
    renderer->PolygonBlit = &PolygonBlit;

    return renderer;
}

void GPU_FreeShapeRenderer_OpenGL(GPU_ShapeRenderer* renderer)
{
    if(renderer == NULL)
        return;

    free(renderer->data);
    free(renderer);
}
