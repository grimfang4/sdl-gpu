// Hacks to fix compile errors due to polluted namespace
#ifdef _WIN32
#define _WINUSER_H
#define _WINGDI_H
#endif

#include "SDL_gpu_OpenGLES_1_internal.h"
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


#include "vase_rend_draft_2.h"

static void Circle(GPU_ShapeRenderer* renderer, GPU_Target* target, float x, float y, float radius, SDL_Color color);



#ifdef SDL_GPU_USE_SDL2
#define GET_WINDOW(shape_renderer) ((GPU_RendererData_OpenGLES_1*)shape_renderer->renderer->data)->window
#else
#define GET_WINDOW(shape_renderer) SDL_GetVideoSurface()
#endif


#define BEGIN \
	if(target == NULL) \
		return; \
	if(renderer->renderer != target->renderer) \
		return; \
	float z = ((RendererData_OpenGLES_1*)renderer->renderer->data)->z;	\
	 \
	/* Bind the FBO */ \
	glBindFramebuffer(GL_FRAMEBUFFER, ((TargetData_OpenGLES_1*)target->data)->handle); \
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
	glDisable( GL_TEXTURE_2D );

#define END \
	if(target->useClip) \
	{ \
		glDisable(GL_SCISSOR_TEST); \
	} \
	/*glPopAttrib();*/ \
	glColor4ub(255, 255, 255, 255); \
	glBindFramebuffer(GL_FRAMEBUFFER, 0); \
	glEnable( GL_TEXTURE_2D );


#define INVERT_Y(y) \
	if(renderer->renderer->display != target) \
		(y) = renderer->renderer->display->h - (y);

static float SetThickness(GPU_ShapeRenderer* renderer, float thickness)
{
    float old = ((ShapeRendererData_OpenGLES_1*)renderer->data)->line_thickness;
    ((ShapeRendererData_OpenGLES_1*)renderer->data)->line_thickness = thickness;
    glLineWidth(thickness);
    return old;
}

static float GetThickness(GPU_ShapeRenderer* renderer)
{
    return ((ShapeRendererData_OpenGLES_1*)renderer->data)->line_thickness;
}

static void Pixel(GPU_ShapeRenderer* renderer, GPU_Target* target, float x, float y, SDL_Color color)
{
    BEGIN;

    INVERT_Y(y);

    glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, color.unused/255.5f);

    GLfloat glverts[3];
    glVertexPointer(3, GL_FLOAT, 0, glverts);
    glEnableClientState(GL_VERTEX_ARRAY);

    glverts[0] = x;
    glverts[1] = y;
    glverts[2] = z;

    glDrawArrays(GL_POINTS, 0, 1);
    glDisableClientState(GL_VERTEX_ARRAY);

    END;
}

static void Line(GPU_ShapeRenderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color)
{
    BEGIN;

    INVERT_Y(y1);
    INVERT_Y(y2);

    (void)z;
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    line ( x1,y1,x2,y2,
           renderer->GetThickness(renderer),
           color.r/255.5f, color.g/255.5f, color.b/255.5f, color.unused/255.5f,
           0,0,
           1);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

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

    INVERT_Y(y);

    glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, color.unused/255.5f);
    float t = startAngle;
    float dt = (1 - (endAngle - startAngle)/360) * 5;  // A segment every 5 degrees of a full circle
    float dx, dy;

    int numSegments = fabs(endAngle - startAngle)/dt;

    GLfloat glverts[numSegments*3];
    glVertexPointer(3, GL_FLOAT, 0, glverts);
    glEnableClientState(GL_VERTEX_ARRAY);

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

    glDrawArrays(GL_LINE_STRIP, 0, numSegments);
    glDisableClientState(GL_VERTEX_ARRAY);

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

    INVERT_Y(y);

    glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, color.unused/255.5f);
    float t = startAngle;
    float dt = (1 - (endAngle - startAngle)/360) * 5;  // A segment every 5 degrees of a full circle
    float dx, dy;

    int numSegments = fabs(endAngle - startAngle)/dt+1;

    GLfloat glverts[(1+numSegments)*3];
    glVertexPointer(3, GL_FLOAT, 0, glverts);
    glEnableClientState(GL_VERTEX_ARRAY);

    glverts[0] = x;
    glverts[1] = y;
    glverts[2] = z;
    int i;
    for(i = 1; i < numSegments+1; i++)
    {
        dx = radius*cos(t*RADPERDEG);
        dy = radius*sin(t*RADPERDEG);
        glverts[i*3] = x+dx;
        glverts[i*3+1] = y+dy;
        glverts[i*3+2] = z;
        t += dt;
    }

    glDrawArrays(GL_TRIANGLE_FAN, 0, 1+numSegments);
    glDisableClientState(GL_VERTEX_ARRAY);

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

    INVERT_Y(y);

    glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, color.unused/255.5f);
    float t = 0;
    float dt = 5;  // A segment every 5 degrees of a full circle
    float dx, dy;
    int numSegments = 360/dt+1;

    GLfloat glverts[numSegments*3];
    glVertexPointer(3, GL_FLOAT, 0, glverts);
    glEnableClientState(GL_VERTEX_ARRAY);

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

    glDrawArrays(GL_LINE_LOOP, 0, numSegments);
    glDisableClientState(GL_VERTEX_ARRAY);

    END;
}

static void CircleFilled(GPU_ShapeRenderer* renderer, GPU_Target* target, float x, float y, float radius, SDL_Color color)
{
    BEGIN;

    INVERT_Y(y);

    glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, color.unused/255.5f);
    float t = 0;
    float dt = 5;  // A segment every 5 degrees of a full circle
    float dx, dy;

    int numSegments = 360/dt+1;

    GLfloat glverts[(1+numSegments)*3];
    glVertexPointer(3, GL_FLOAT, 0, glverts);
    glEnableClientState(GL_VERTEX_ARRAY);

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

    glDrawArrays(GL_TRIANGLE_FAN, 0, 1+numSegments);
    glDisableClientState(GL_VERTEX_ARRAY);

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

    INVERT_Y(y1);
    INVERT_Y(y2);
    INVERT_Y(y3);

    glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, color.unused/255.5f);

    GLfloat glverts[9];
    glVertexPointer(3, GL_FLOAT, 0, glverts);
    glEnableClientState(GL_VERTEX_ARRAY);

    glverts[0] = x1;
    glverts[1] = y1;
    glverts[2] = z;
    glverts[3] = x2;
    glverts[4] = y2;
    glverts[5] = z;
    glverts[6] = x3;
    glverts[7] = y3;
    glverts[8] = z;

    glDrawArrays(GL_LINE_LOOP, 0, 3);
    glDisableClientState(GL_VERTEX_ARRAY);

    END;
}

static void TriFilled(GPU_ShapeRenderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color)
{
    BEGIN;

    INVERT_Y(y1);
    INVERT_Y(y2);
    INVERT_Y(y3);

    glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, color.unused/255.5f);

    GLfloat glverts[9];
    glVertexPointer(3, GL_FLOAT, 0, glverts);
    glEnableClientState(GL_VERTEX_ARRAY);

    glverts[0] = x1;
    glverts[1] = y1;
    glverts[2] = z;
    glverts[3] = x2;
    glverts[4] = y2;
    glverts[5] = z;
    glverts[6] = x3;
    glverts[7] = y3;
    glverts[8] = z;

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
    glDisableClientState(GL_VERTEX_ARRAY);

    END;
}

static void Rect(GPU_ShapeRenderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color)
{
    BEGIN;

    INVERT_Y(y1);
    INVERT_Y(y2);

    glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, color.unused/255.5f);

    GLfloat glverts[12];
    glVertexPointer(3, GL_FLOAT, 0, glverts);
    glEnableClientState(GL_VERTEX_ARRAY);

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

    glDrawArrays(GL_LINE_LOOP, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);

    END;
}

static void RectFilled(GPU_ShapeRenderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color)
{
    BEGIN;

    INVERT_Y(y1);
    INVERT_Y(y2);

    glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, color.unused/255.5f);

    GLfloat glverts[12];
    glVertexPointer(3, GL_FLOAT, 0, glverts);
    glEnableClientState(GL_VERTEX_ARRAY);

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

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);

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

    INVERT_Y(y1);
    INVERT_Y(y2);

    glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, color.unused/255.5f);

    float i;
    /*glBegin(GL_LINE_LOOP);
    	glVertex3f(x1+radius,y1, z);
    	glVertex3f(x2-radius,y1, z);
    	for(i=(float)M_PI*1.5f;i<M_PI*2;i+=0.1f)
    		glVertex3f(x2-radius+cos(i)*radius,y1+radius+sin(i)*radius, z);
    	glVertex3f(x2,y1+radius, z);
    	glVertex3f(x2,y2-radius, z);
    	for(i=0;i<(float)M_PI*0.5f;i+=0.1f)
    		glVertex3f(x2-radius+cos(i)*radius,y2-radius+sin(i)*radius, z);
    	glVertex3f(x2-radius,y2, z);
    	glVertex3f(x1+radius,y2, z);
    	for(i=(float)M_PI*0.5f;i<M_PI;i+=0.1f)
    		glVertex3f(x1+radius+cos(i)*radius,y2-radius+sin(i)*radius, z);
    	glVertex3f(x1,y2-radius, z);
    	glVertex3f(x1,y1+radius, z);
    	for(i=(float)M_PI;i<M_PI*1.5f;i+=0.1f)
    		glVertex3f(x1+radius+cos(i)*radius,y1+radius+sin(i)*radius, z);
    glEnd();*/

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

    INVERT_Y(y1);
    INVERT_Y(y2);

    glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, color.unused/255.5f);

    float i;
    /*glBegin(GL_TRIANGLE_FAN);
    	glVertex3f(x1+radius,y1, z);
    	glVertex3f(x2-radius,y1, z);
    	for(i=(float)M_PI*1.5f;i<M_PI*2;i+=0.1f)
    		glVertex3f(x2-radius+cos(i)*radius,y1+radius+sin(i)*radius, z);
    	glVertex3f(x2,y1+radius, z);
    	glVertex3f(x2,y2-radius, z);
    	for(i=0;i<(float)M_PI*0.5f;i+=0.1f)
    		glVertex3f(x2-radius+cos(i)*radius,y2-radius+sin(i)*radius, z);
    	glVertex3f(x2-radius,y2, z);
    	glVertex3f(x1+radius,y2, z);
    	for(i=(float)M_PI*0.5f;i<M_PI;i+=0.1f)
    		glVertex3f(x1+radius+cos(i)*radius,y2-radius+sin(i)*radius, z);
    	glVertex3f(x1,y2-radius, z);
    	glVertex3f(x1,y1+radius, z);
    	for(i=(float)M_PI;i<M_PI*1.5f;i+=0.1f)
    		glVertex3f(x1+radius+cos(i)*radius,y1+radius+sin(i)*radius, z);
    glEnd();*/

    END;
}

static void Polygon(GPU_ShapeRenderer* renderer, GPU_Target* target, Uint16 n, float* vertices, SDL_Color color)
{
    BEGIN;

    glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, color.unused/255.5f);

    int i;
    /*glBegin(GL_LINE_LOOP);
    for(i = 0; i < 2*n; i+=2)
    {
    	float y = vertices[i+1];

    	INVERT_Y(y);

    	glVertex3f(vertices[i], y, z);
    }
    glEnd();*/

    END;
}

static void PolygonFilled(GPU_ShapeRenderer* renderer, GPU_Target* target, Uint16 n, float* vertices, SDL_Color color)
{
    BEGIN;

    glColor4f(color.r/255.5f, color.g/255.5f, color.b/255.5f, color.unused/255.5f);
    int i;
    /*glBegin(GL_TRIANGLE_FAN);
    for(i = 0; i < 2*n; i+=2)
    {
    	float y = vertices[i+1];

    	INVERT_Y(y);

    	glVertex3f(vertices[i], y, z);
    }
    glEnd();*/

    END;
}

static void PolygonBlit(GPU_ShapeRenderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* target, Uint16 n, float* vertices, float textureX, float textureY, float angle, float scaleX, float scaleY)
{
    BEGIN;

    glEnable( GL_TEXTURE_2D );

    // Bind the texture to which subsequent calls refer
    glBindTexture( GL_TEXTURE_2D, ((ImageData_OpenGLES_1*)src->data)->handle );

    // Set repeat mode
    // FIXME: Save old mode and reset it later
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    // TODO: Add rotation of texture using 'angle'
    // TODO: Use 'srcrect'

    int i;
    /*glBegin(GL_TRIANGLE_FAN);
    for(i = 0; i < 2*n; i+=2)
    {
    	float x = vertices[i];
    	float y = vertices[i+1];

    	glTexCoord2f((x - textureX)*scaleX/src->w, (y - textureY)*scaleY/src->h);

    	INVERT_Y(y);
    	glVertex3f(x, y, z);
    }
    glEnd();*/

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    END;
}









GPU_ShapeRenderer* GPU_CreateShapeRenderer_OpenGLES_1(void)
{
    GPU_ShapeRenderer* renderer = (GPU_ShapeRenderer*)malloc(sizeof(GPU_ShapeRenderer));

    renderer->data = (ShapeRendererData_OpenGLES_1*)malloc(sizeof(ShapeRendererData_OpenGLES_1));

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

void GPU_FreeShapeRenderer_OpenGLES_1(GPU_ShapeRenderer* renderer)
{
    if(renderer == NULL)
        return;

    free(renderer->data);
    free(renderer);
}
