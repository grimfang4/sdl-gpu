/* This is an implementation file to be included after certain #defines have been set.
These defines determine the code paths:
SDL_GPU_USE_OPENGL   // "Desktop" OpenGL
SDL_GPU_USE_GLES // "Embedded" OpenGL
SDL_GPU_USE_GL_TIER1 // Fixed-function, glBegin, etc.
SDL_GPU_USE_GL_TIER2 // Fixed-function, glDrawArrays, etc.
SDL_GPU_USE_GL_TIER3 // Shader pipeline, manual transforms
CONTEXT_DATA  // Appropriate type for the context data (via pointer)
IMAGE_DATA  // Appropriate type for the image data (via pointer)
TARGET_DATA  // Appropriate type for the target data (via pointer)
*/

#ifndef DEGPERRAD
#define DEGPERRAD 57.2957795f
#endif

#ifndef RADPERDEG
#define RADPERDEG 0.0174532925f
#endif



static void Circle(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, SDL_Color color);



#ifdef SDL_GPU_USE_GL_TIER3

#define SDL_GPU_SHAPE_FLOATS_PER_VERTEX 6

#define SET_VERTEX(_x, _y) \
do { \
    glverts[_vertex_array_index++] = (_x); \
    glverts[_vertex_array_index++] = (_y); \
    glverts[_vertex_array_index++] = r; \
    glverts[_vertex_array_index++] = g; \
    glverts[_vertex_array_index++] = b; \
    glverts[_vertex_array_index++] = a; \
} while(0);

#define SET_VERTEX_TEXTURED(_x, _y, _s, _t) \
do { \
    glverts[_vertex_array_index++] = (_x); \
    glverts[_vertex_array_index++] = (_y); \
    glverts[_vertex_array_index++] = r; \
    glverts[_vertex_array_index++] = g; \
    glverts[_vertex_array_index++] = b; \
    glverts[_vertex_array_index++] = a; \
    glverts[_vertex_array_index++] = (_s); \
    glverts[_vertex_array_index++] = (_t); \
} while(0);

#define DECLARE_COLOR_RGBA \
if(target->use_color) \
{ \
    SDL_Color c = MIX_COLORS(target->color, color); \
    color = c; \
} \
float r = color.r/255.0f; \
float g = color.g/255.0f; \
float b = color.b/255.0f; \
float a = GET_ALPHA(color)/255.0f;

        
#define RESET_COLOR

#else

#define SDL_GPU_SHAPE_FLOATS_PER_VERTEX 2

#define SET_VERTEX(_x, _y) \
do { \
    glverts[_vertex_array_index++] = (_x); \
    glverts[_vertex_array_index++] = (_y); \
} while(0);

#define SET_VERTEX_TEXTURED(_x, _y, _s, _t) \
do { \
    glverts[_vertex_array_index++] = (_x); \
    glverts[_vertex_array_index++] = (_y); \
    glverts[_vertex_array_index++] = (_s); \
    glverts[_vertex_array_index++] = (_t); \
} while(0);

#ifdef SDL_GPU_USE_GLES
#define DECLARE_COLOR_RGBA \
if(target->use_color) \
{ \
    SDL_Color c = MIX_COLORS(target->color, color); \
    color = c; \
} \
glColor4f(color.r/255.01f, color.g/255.01f, color.b/255.01f, GET_ALPHA(color)/255.01f);
#else
#define DECLARE_COLOR_RGBA \
if(target->use_color) \
{ \
    SDL_Color c = MIX_COLORS(target->color, color); \
    color = c; \
} \
glColor4ub(color.r, color.g, color.b, GET_ALPHA(color));
#endif

#define RESET_COLOR \
glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

#endif


#define DECLARE_VERTEX_ARRAY(num_vertices) \
const int _num_vertices = (num_vertices); \
const int _vertex_array_size = _num_vertices*SDL_GPU_SHAPE_FLOATS_PER_VERTEX; \
GLfloat glverts[_vertex_array_size]; \
int _vertex_array_index = 0;

#define DECLARE_TEXTURED_VERTEX_ARRAY(num_vertices) \
const int _num_vertices = (num_vertices); \
const int _vertex_array_size = _num_vertices*(SDL_GPU_SHAPE_FLOATS_PER_VERTEX + 2); \
GLfloat glverts[_vertex_array_size]; \
int _vertex_array_index = 0;

#define DRAW_VERTICES(_prim_type) draw_vertices(glverts, _num_vertices, _prim_type);
#define DRAW_COUNTED_VERTICES(_prim_type) draw_vertices(glverts, _vertex_array_index/SDL_GPU_SHAPE_FLOATS_PER_VERTEX, _prim_type);
#define DRAW_VERTICES_TEXTURED(_prim_type) draw_vertices_textured(glverts, _num_vertices, _prim_type);


#ifdef SDL_GPU_APPLY_TRANSFORMS_TO_GL_STACK
    #define APPLY_TRANSFORMS \
    /*if(!renderer->IsFeatureEnabled(GPU_FEATURE_VERTEX_SHADER))*/ \
        applyTransforms();
#else
    #define APPLY_TRANSFORMS
#endif

#define BEGIN \
	if(target == NULL) \
                return; \
        if(renderer != target->renderer) \
                return; \
         \
        renderer->FlushBlitBuffer(renderer); \
        makeContextCurrent(renderer, target); \
        if(renderer->current_context_target == NULL) \
            return; \
        if(bindFramebuffer(renderer, target)) \
        { \
            prepareToRenderToTarget(renderer, target); \
            prepareToRenderShapes(renderer); \
            changeViewport(target); \
            /*glPushAttrib(GL_COLOR_BUFFER_BIT);*/ \
            \
            if(target->image != NULL) \
            { \
                GPU_MatrixMode( GPU_PROJECTION ); \
                GPU_PushMatrix(); \
                GPU_LoadIdentity(); \
                \
                GPU_Ortho(0.0f, target->viewport.w, 0.0f, target->viewport.h, -1.0f, 1.0f); /* Special inverted orthographic projection because tex coords are inverted already. */ \
                \
                GPU_MatrixMode( GPU_MODELVIEW ); \
            } \
            APPLY_TRANSFORMS;


#define BEGIN_TEXTURED \
	if(target == NULL) \
                return; \
        if(renderer != target->renderer) \
                return; \
         \
        renderer->FlushBlitBuffer(renderer); \
        makeContextCurrent(renderer, target); \
        if(renderer->current_context_target == NULL) \
            return; \
        if(bindFramebuffer(renderer, target)) \
        { \
            prepareToRenderToTarget(renderer, target); \
            prepareToRenderImage(renderer, target, src); \
            changeViewport(target); \
            /*glPushAttrib(GL_COLOR_BUFFER_BIT);*/ \
            \
            if(target->image != NULL) \
            { \
                GPU_MatrixMode( GPU_PROJECTION ); \
                GPU_PushMatrix(); \
                GPU_LoadIdentity(); \
                \
                GPU_Ortho(0.0f, target->viewport.w, 0.0f, target->viewport.h, -1.0f, 1.0f); /* Special inverted orthographic projection because tex coords are inverted already. */ \
                \
                GPU_MatrixMode( GPU_MODELVIEW ); \
            } \
            APPLY_TRANSFORMS;

#define END \
    RESET_COLOR; \
    if(target->image != NULL) \
    { \
        GPU_MatrixMode( GPU_PROJECTION ); \
        GPU_PopMatrix(); \
        GPU_MatrixMode( GPU_MODELVIEW ); \
    } \
	if(target->use_clip_rect) \
	{ \
			glDisable(GL_SCISSOR_TEST); \
	} \
	/*glPopAttrib();*/ \
	glEnable( GL_TEXTURE_2D ); \
    }





static inline void draw_vertices(GLfloat* glverts, int num_vertices, GLenum prim_type)
{
    #ifdef SDL_GPU_USE_GL_TIER1
        glBegin(prim_type);
        int size = 2*num_vertices;
        int i;
        for(i = 0; i < size; i += 2)
        {
            glVertex3f(glverts[i], glverts[i+1], 0.0f);
        }
        glEnd();
    #elif defined(SDL_GPU_USE_GL_TIER2)
        glVertexPointer(2, GL_FLOAT, 0, glverts);
        glEnableClientState(GL_VERTEX_ARRAY);
        glDrawArrays(prim_type, 0, num_vertices);
        glDisableClientState(GL_VERTEX_ARRAY);
    #elif defined(SDL_GPU_USE_GL_TIER3)
    
        GPU_Target* target = GPU_GetContextTarget();
        if(target == NULL)
            return;

        TARGET_DATA* data = ((TARGET_DATA*)target->data);
        
        int floats_per_vertex = 6;  // position (2), color (4)
        int buffer_stride = floats_per_vertex * sizeof(float);
        
        // Upload our modelviewprojection matrix
        if(data->current_shader_block.modelViewProjection_loc >= 0)
        {
            float mvp[16];
            GPU_GetModelViewProjection(mvp);
            GPU_SetUniformMatrixfv(data->current_shader_block.modelViewProjection_loc, 1, 4, 4, 0, mvp);
        }
    
        // Update the vertex array object's buffers
        #if !defined(SDL_GPU_USE_GLES) || SDL_GPU_GLES_MAJOR_VERSION != 2
        glBindVertexArray(data->blit_VAO);
        #endif
        
        // Upload blit buffer to a single buffer object
        glBindBuffer(GL_ARRAY_BUFFER, data->blit_VBO[data->blit_VBO_flop]);
        data->blit_VBO_flop = !data->blit_VBO_flop;
        
        // Copy the whole blit buffer to the GPU
        glBufferSubData(GL_ARRAY_BUFFER, 0, buffer_stride * num_vertices, glverts);
        
        // Specify the formatting of the blit buffer
        if(data->current_shader_block.position_loc >= 0)
        {
            glEnableVertexAttribArray(data->current_shader_block.position_loc);  // Tell GL to use client-side attribute data
            glVertexAttribPointer(data->current_shader_block.position_loc, 2, GL_FLOAT, GL_FALSE, buffer_stride, 0);  // Tell how the data is formatted
        }
        if(data->current_shader_block.color_loc >= 0)
        {
            glEnableVertexAttribArray(data->current_shader_block.color_loc);
            glVertexAttribPointer(data->current_shader_block.color_loc, 4, GL_FLOAT, GL_FALSE, buffer_stride, (void*)(2 * sizeof(float)));
        }
        
        glDrawArrays(prim_type, 0, num_vertices);
        
        #if !defined(SDL_GPU_USE_GLES) || SDL_GPU_GLES_MAJOR_VERSION != 2
        glBindVertexArray(0);
        #endif
    #endif
}


static inline void draw_vertices_textured(GLfloat* glverts, int num_vertices, GLenum prim_type)
{
    #ifdef SDL_GPU_USE_GL_TIER1
        glBegin(prim_type);
        int size = 4*num_vertices;
        int i;
        for(i = 0; i < size; i += 5)
        {
            glTexCoord2f(glverts[i+2], glverts[i+3]);
            glVertex3f(glverts[i], glverts[i+1], 0.0f);
        }
        glEnd();
    #elif defined(SDL_GPU_USE_GL_TIER2)
        glVertexPointer(2, GL_FLOAT, 5*sizeof(float), glverts);
        glTexCoordPointer(2, GL_FLOAT, 5*sizeof(float), (glverts + 2));
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glDrawArrays(prim_type, 0, num_vertices);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    #elif defined(SDL_GPU_USE_GL_TIER3)
    
        GPU_Target* target = GPU_GetContextTarget();
        if(target == NULL)
            return;

        TARGET_DATA* data = ((TARGET_DATA*)target->data);
        
        int floats_per_vertex = 8;  // position (2), color (4), texcoord (2)
        int buffer_stride = floats_per_vertex * sizeof(float);
        
        // Upload our modelviewprojection matrix
        if(data->current_shader_block.modelViewProjection_loc >= 0)
        {
            float mvp[16];
            GPU_GetModelViewProjection(mvp);
            GPU_SetUniformMatrixfv(data->current_shader_block.modelViewProjection_loc, 1, 4, 4, 0, mvp);
        }
    
        // Update the vertex array object's buffers
        #if !defined(SDL_GPU_USE_GLES) || SDL_GPU_GLES_MAJOR_VERSION != 2
        glBindVertexArray(data->blit_VAO);
        #endif
        
        // Upload blit buffer to a single buffer object
        glBindBuffer(GL_ARRAY_BUFFER, data->blit_VBO[data->blit_VBO_flop]);
        data->blit_VBO_flop = !data->blit_VBO_flop;
        
        // Copy the whole blit buffer to the GPU
        glBufferSubData(GL_ARRAY_BUFFER, 0, buffer_stride * num_vertices, glverts);
        
        // Specify the formatting of the blit buffer
        if(data->current_shader_block.position_loc >= 0)
        {
            glEnableVertexAttribArray(data->current_shader_block.position_loc);  // Tell GL to use client-side attribute data
            glVertexAttribPointer(data->current_shader_block.position_loc, 2, GL_FLOAT, GL_FALSE, buffer_stride, 0);  // Tell how the data is formatted
        }
        if(data->current_shader_block.color_loc >= 0)
        {
            glEnableVertexAttribArray(data->current_shader_block.color_loc);
            glVertexAttribPointer(data->current_shader_block.color_loc, 4, GL_FLOAT, GL_FALSE, buffer_stride, (void*)(2 * sizeof(float)));
        }
        if(data->current_shader_block.texcoord_loc >= 0)
        {
            glEnableVertexAttribArray(data->current_shader_block.texcoord_loc);
            glVertexAttribPointer(data->current_shader_block.texcoord_loc, 2, GL_FLOAT, GL_FALSE, buffer_stride, (void*)(6 * sizeof(float)));
        }
        
        glDrawArrays(prim_type, 0, num_vertices);
        
        #if !defined(SDL_GPU_USE_GLES) || SDL_GPU_GLES_MAJOR_VERSION != 2
        glBindVertexArray(0);
        #endif
    #endif
}


static float SetLineThickness(GPU_Renderer* renderer, float thickness)
{
    if(renderer->current_context_target == NULL)
        return 1.0f;
    
	float old = renderer->current_context_target->context->line_thickness;
	renderer->current_context_target->context->line_thickness = thickness;
	glLineWidth(thickness);
	return old;
}

static float GetLineThickness(GPU_Renderer* renderer)
{
    return renderer->current_context_target->context->line_thickness;
}

static void Pixel(GPU_Renderer* renderer, GPU_Target* target, float x, float y, SDL_Color color)
{
	BEGIN;
        
        DECLARE_VERTEX_ARRAY(1);
		DECLARE_COLOR_RGBA;
		
		SET_VERTEX(x, y);
        
        DRAW_VERTICES(GL_POINTS);

    END;
}

static void Line(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color)
{
    BEGIN;

        DECLARE_VERTEX_ARRAY(2);
		DECLARE_COLOR_RGBA;

		SET_VERTEX(x1, y1);
		SET_VERTEX(x2, y2);

        DRAW_VERTICES(GL_LINES);

    END;
}


static void Arc(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, float startAngle, float endAngle, SDL_Color color)
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

    float t = startAngle;
    float dt = (1 - (endAngle - startAngle)/360) * 5;  // A segment every 5 degrees of a full circle
    float dx, dy;

    int numSegments = fabs(endAngle - startAngle)/dt;

    DECLARE_VERTEX_ARRAY(numSegments+2); // Extra vertex for endpoint
    DECLARE_COLOR_RGBA;

    int i;
    for(i = 0; i < numSegments+1; i++)
    {
        dx = radius*cos(t*RADPERDEG);
        dy = radius*sin(t*RADPERDEG);
        SET_VERTEX(x+dx, y+dy);
        t += dt;
    }
    
    dx = radius*cos(endAngle*RADPERDEG);
    dy = radius*sin(endAngle*RADPERDEG);
    SET_VERTEX(x+dx, y+dy);

    DRAW_VERTICES(GL_LINE_STRIP);

    END;
}


static void ArcFilled(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, float startAngle, float endAngle, SDL_Color color)
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

    float t = startAngle;
    float dt = (1 - (endAngle - startAngle)/360) * 5;  // A segment every 5 degrees of a full circle
    float dx, dy;

    int numSegments = fabs(endAngle - startAngle)/dt;

    DECLARE_VERTEX_ARRAY(numSegments+3);  // Extra vertex for the center and endpoint
    DECLARE_COLOR_RGBA;

    SET_VERTEX(x, y);
    int i;
    for(i = 1; i < numSegments+2; i++)
    {
        dx = radius*cos(t*RADPERDEG);
        dy = radius*sin(t*RADPERDEG);
        SET_VERTEX(x+dx, y+dy);
        t += dt;
    }

    dx = radius*cos(endAngle*RADPERDEG);
    dy = radius*sin(endAngle*RADPERDEG);
    SET_VERTEX(x+dx, y+dy);
    
    DRAW_VERTICES(GL_TRIANGLE_FAN);
    
    END;
}

static void Circle(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, SDL_Color color)
{
    BEGIN;

    float t = 0;
    float dt = 5;  // A segment every 5 degrees of a full circle
    float dx, dy;
    int numSegments = 360/dt+1;

    DECLARE_VERTEX_ARRAY(numSegments);
    DECLARE_COLOR_RGBA;

    int i;
    for(i = 0; i < numSegments; i++)
    {
        dx = radius*cos(t*RADPERDEG);
        dy = radius*sin(t*RADPERDEG);
        SET_VERTEX(x+dx, y+dy);
        t += dt;
    }

    DRAW_VERTICES(GL_LINE_LOOP);

    END;
}

static void CircleFilled(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, SDL_Color color)
{
    BEGIN;

    float t = 0;
    float dt = 5;  // A segment every 5 degrees of a full circle
    float dx, dy;

    int numSegments = 360/dt+1;

    DECLARE_VERTEX_ARRAY(numSegments+1);
    DECLARE_COLOR_RGBA;

    SET_VERTEX(x, y);
    
    int i;
    for(i = 1; i < numSegments+1; i++)
    {
        dx = radius*cos(t*RADPERDEG);
        dy = radius*sin(t*RADPERDEG);
        SET_VERTEX(x+dx, y+dy);
        t += dt;
    }

    DRAW_VERTICES(GL_TRIANGLE_FAN);
    
    END;
}

static void Tri(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color)
{
    BEGIN;

    DECLARE_VERTEX_ARRAY(3);
    DECLARE_COLOR_RGBA;

    SET_VERTEX(x1, y1);
    SET_VERTEX(x2, y2);
    SET_VERTEX(x3, y3);

    DRAW_VERTICES(GL_LINE_LOOP);

    END;
}

static void TriFilled(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color)
{
    BEGIN;
    
    DECLARE_VERTEX_ARRAY(3);
    DECLARE_COLOR_RGBA;

    SET_VERTEX(x1, y1);
    SET_VERTEX(x2, y2);
    SET_VERTEX(x3, y3);

    DRAW_VERTICES(GL_TRIANGLE_STRIP);

    END;
}

static void Rectangle(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color)
{
    BEGIN;
    
    if(y2 < y1)
    {
        float y = y1;
        y1 = y2;
        y2 = y;
    }
    if(x2 < x1)
    {
        float x = x1;
        x1 = x2;
        x2 = x;
    }
    
	float thickness = renderer->GetLineThickness(renderer);

    float t = thickness/2;
    
    // Thick lines via filled triangles
    DECLARE_VERTEX_ARRAY(10);
    DECLARE_COLOR_RGBA;
    
    SET_VERTEX(x1 - t, y1 - t);
    SET_VERTEX(x1 + t, y1 + t);
    SET_VERTEX(x2 + t, y1 - t);
    SET_VERTEX(x2 - t, y1 + t);
    SET_VERTEX(x2 + t, y2 + t);
    SET_VERTEX(x2 - t, y2 - t);
    SET_VERTEX(x1 - t, y2 + t);
    SET_VERTEX(x1 + t, y2 - t);
    SET_VERTEX(x1 - t, y1 - t);
    SET_VERTEX(x1 + t, y1 + t);
    
    DRAW_VERTICES(GL_TRIANGLE_STRIP);

    END;
}

static void RectangleFilled(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color)
{
    BEGIN;
    
    DECLARE_VERTEX_ARRAY(4);
    DECLARE_COLOR_RGBA;

    SET_VERTEX(x1, y1);
    SET_VERTEX(x1, y2);
    SET_VERTEX(x2, y1);
    SET_VERTEX(x2, y2);

    DRAW_VERTICES(GL_TRIANGLE_STRIP);

    END;
}

static void RectangleRound(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color)
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
    
    //int numVerts = 8 + (int(M_PI*5)+1)*4;  // 8 + (15.7 + 1)*4
    DECLARE_VERTEX_ARRAY(120);
    DECLARE_COLOR_RGBA;
    
    float i;
    for(i=(float)M_PI*1.5f;i<M_PI*2;i+=0.1f)
        SET_VERTEX(x2-radius+cos(i)*radius, y1+radius+sin(i)*radius);
    for(i=0;i<(float)M_PI*0.5f;i+=0.1f)
        SET_VERTEX(x2-radius+cos(i)*radius, y2-radius+sin(i)*radius);
    for(i=(float)M_PI*0.5f;i<M_PI;i+=0.1f)
        SET_VERTEX(x1+radius+cos(i)*radius, y2-radius+sin(i)*radius);
    for(i=(float)M_PI;i<M_PI*1.5f;i+=0.1f)
        SET_VERTEX(x1+radius+cos(i)*radius, y1+radius+sin(i)*radius);

    DRAW_COUNTED_VERTICES(GL_LINE_LOOP);

    END;
}

static void RectangleRoundFilled(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color)
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
    
    //int numVerts = 8 + (int(M_PI*5)+1)*4;  // 8 + (15.7 + 1)*4
    DECLARE_VERTEX_ARRAY(120);
    DECLARE_COLOR_RGBA;
    
    float i;
    SET_VERTEX(x1+radius,y1);
    SET_VERTEX(x2-radius,y1);
    for(i=(float)M_PI*1.5f;i<M_PI*2;i+=0.1f)
        SET_VERTEX(x2-radius+cos(i)*radius,y1+radius+sin(i)*radius);
    SET_VERTEX(x2,y1+radius);
    SET_VERTEX(x2,y2-radius);
    for(i=0;i<(float)M_PI*0.5f;i+=0.1f)
        SET_VERTEX(x2-radius+cos(i)*radius,y2-radius+sin(i)*radius);
    SET_VERTEX(x2-radius,y2);
    SET_VERTEX(x1+radius,y2);
    for(i=(float)M_PI*0.5f;i<M_PI;i+=0.1f)
        SET_VERTEX(x1+radius+cos(i)*radius,y2-radius+sin(i)*radius);
    SET_VERTEX(x1,y2-radius);
    SET_VERTEX(x1,y1+radius);
    for(i=(float)M_PI;i<M_PI*1.5f;i+=0.1f)
        SET_VERTEX(x1+radius+cos(i)*radius,y1+radius+sin(i)*radius);

    DRAW_COUNTED_VERTICES(GL_TRIANGLE_FAN);
    
    END;
}

static void Polygon(GPU_Renderer* renderer, GPU_Target* target, Uint16 n, float* vertices, SDL_Color color)
{
    BEGIN;
    
    int numIndices = 2*n;
    DECLARE_VERTEX_ARRAY(n);
    DECLARE_COLOR_RGBA;
    
    int i;
    for(i = 0; i < numIndices; i+=2)
        SET_VERTEX(vertices[i], vertices[i+1]);
    
    DRAW_VERTICES(GL_LINE_LOOP);

    END;
}

static void PolygonFilled(GPU_Renderer* renderer, GPU_Target* target, Uint16 n, float* vertices, SDL_Color color)
{
    BEGIN;
    
    int numIndices = 2*n;
    DECLARE_VERTEX_ARRAY(n);
    DECLARE_COLOR_RGBA;
    
    int i;
    for(i = 0; i < numIndices; i+=2)
        SET_VERTEX(vertices[i], vertices[i+1]);
    
    DRAW_VERTICES(GL_TRIANGLE_FAN);

    END;
}

static void PolygonBlit(GPU_Renderer* renderer, GPU_Image* src, GPU_Rect* srcrect, GPU_Target* target, Uint16 n, float* vertices, float textureX, float textureY, float angle, float scaleX, float scaleY)
{
    BEGIN_TEXTURED;
    
    GLuint handle = ((IMAGE_DATA*)src->data)->handle;
    renderer->FlushBlitBuffer(renderer);
    
    glBindTexture( GL_TEXTURE_2D, handle );
    ((CONTEXT_DATA*)renderer->current_context_target->context->data)->last_image = src;
    
    // Set repeat mode
    // FIXME: Save old mode and reset it later
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    
    // TODO: Add rotation of texture using 'angle'
    // TODO: Use 'srcrect'
    
    int numIndices = 2*n;
    DECLARE_TEXTURED_VERTEX_ARRAY(n);
    
    SDL_Color color = src->color;
    DECLARE_COLOR_RGBA;
    
    int i;
    for(i = 0; i < numIndices; i+=2)
    {
        float x = vertices[i];
    	float y = vertices[i+1];
    	
        SET_VERTEX_TEXTURED(x, y, (x - textureX)*scaleX/src->w, (y - textureY)*scaleY/src->h);
    }
    
    DRAW_VERTICES_TEXTURED(GL_TRIANGLE_FAN);
    
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    
    END;
}

