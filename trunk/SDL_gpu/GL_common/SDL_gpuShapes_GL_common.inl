/* This is an implementation file to be included after certain #defines have been set.
These defines determine the code paths:
SDL_GPU_USE_OPENGL   // "Desktop" OpenGL
SDL_GPU_USE_GLES // "Embedded" OpenGL
SDL_GPU_USE_GL_TIER1 // Fixed-function, glBegin, etc.
SDL_GPU_USE_GL_TIER2 // Fixed-function, glDrawArrays, etc.
SDL_GPU_USE_GL_TIER3 // Shader pipeline, manual transforms
RENDERER_DATA  // Appropriate type for the renderer data (via pointer)
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

#define SDL_GPU_SHAPE_FLOATS_PER_VERTEX 7

#define SET_VERTEX(_x, _y) \
do { \
    glverts[_vertex_array_index++] = (_x); \
    glverts[_vertex_array_index++] = (_y); \
    glverts[_vertex_array_index++] = z; \
    glverts[_vertex_array_index++] = r; \
    glverts[_vertex_array_index++] = g; \
    glverts[_vertex_array_index++] = b; \
    glverts[_vertex_array_index++] = a; \
} while(0);

#define SET_VERTEX_TEXTURED(_x, _y, _s, _t) \
do { \
    glverts[_vertex_array_index++] = (_x); \
    glverts[_vertex_array_index++] = (_y); \
    glverts[_vertex_array_index++] = z; \
    glverts[_vertex_array_index++] = r; \
    glverts[_vertex_array_index++] = g; \
    glverts[_vertex_array_index++] = b; \
    glverts[_vertex_array_index++] = a; \
    glverts[_vertex_array_index++] = (_s); \
    glverts[_vertex_array_index++] = (_t); \
} while(0);

#define DECLARE_COLOR_RGBA \
float r = color.r/255.0f; \
float g = color.g/255.0f; \
float b = color.b/255.0f; \
float a = GET_ALPHA(color)/255.0f;

        
#define RESET_COLOR

#else

#define SDL_GPU_SHAPE_FLOATS_PER_VERTEX 3

#define SET_VERTEX(_x, _y) \
do { \
    glverts[_vertex_array_index++] = (_x); \
    glverts[_vertex_array_index++] = (_y); \
    glverts[_vertex_array_index++] = z; \
} while(0);

#define SET_VERTEX_TEXTURED(_x, _y, _s, _t) \
do { \
    glverts[_vertex_array_index++] = (_x); \
    glverts[_vertex_array_index++] = (_y); \
    glverts[_vertex_array_index++] = z; \
    glverts[_vertex_array_index++] = (_s); \
    glverts[_vertex_array_index++] = (_t); \
} while(0);

#define DECLARE_COLOR_RGBA \
glColor4ub(color.r, color.g, color.b, GET_ALPHA(color));

#define RESET_COLOR \
glColor4ub(255, 255, 255, 255);

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



#define BEGIN \
	if(target == NULL) \
                return; \
        if(renderer != target->renderer) \
                return; \
        float z = ((RENDERER_DATA*)renderer->data)->z;  \
         \
        renderer->FlushBlitBuffer(renderer); \
        makeContextCurrent(renderer, target); \
        if(renderer->current_context_target == NULL) \
            return; \
        if(bindFramebuffer(renderer, target)) \
        { \
            prepareToRenderToTarget(renderer, target); \
            prepareToRenderImage(renderer, NULL); \
            /*glPushAttrib(GL_COLOR_BUFFER_BIT);*/ \
            if(target->useClip) \
            { \
                    glEnable(GL_SCISSOR_TEST); \
            int y = (renderer->current_context_target == target? renderer->current_context_target->h - (target->clipRect.y + target->clipRect.h) : target->clipRect.y); \
            float xFactor = ((float)renderer->current_context_target->window_w)/renderer->current_context_target->w; \
            float yFactor = ((float)renderer->current_context_target->window_h)/renderer->current_context_target->h; \
            glScissor(target->clipRect.x * xFactor, y * yFactor, target->clipRect.w * xFactor, target->clipRect.h * yFactor); \
            } \
	 \
            glDisable( GL_TEXTURE_2D ); \
            \
            if(renderer->current_context_target->image == NULL && renderer->current_context_target->current_shader_program == renderer->current_context_target->default_textured_shader_program) \
                renderer->ActivateShaderProgram(renderer, renderer->current_context_target->default_untextured_shader_program, NULL); \
            \
            GLint vp[4]; \
            if(target->image != NULL) \
            { \
                glGetIntegerv(GL_VIEWPORT, vp); \
                \
                unsigned int w = target->w; \
                unsigned int h = target->h; \
                \
                glViewport( 0, 0, w, h); \
                \
                GPU_MatrixMode( GPU_PROJECTION ); \
                GPU_PushMatrix(); \
                GPU_LoadIdentity(); \
                \
                GPU_Ortho(0.0f, w, 0.0f, h, -1.0f, 1.0f); /* Special inverted orthographic projection because tex coords are inverted already. */ \
                \
                GPU_MatrixMode( GPU_MODELVIEW ); \
            }


#define BEGIN_TEXTURED \
	if(target == NULL) \
                return; \
        if(renderer != target->renderer) \
                return; \
        float z = ((RENDERER_DATA*)renderer->data)->z;  \
         \
        renderer->FlushBlitBuffer(renderer); \
        makeContextCurrent(renderer, target); \
        if(renderer->current_context_target == NULL) \
            return; \
        if(bindFramebuffer(renderer, target)) \
        { \
            prepareToRenderToTarget(renderer, target); \
            prepareToRenderImage(renderer, src); \
            /*glPushAttrib(GL_COLOR_BUFFER_BIT);*/ \
            if(target->useClip) \
            { \
                glEnable(GL_SCISSOR_TEST); \
                int y = (renderer->current_context_target == target? renderer->current_context_target->h - (target->clipRect.y + target->clipRect.h) : target->clipRect.y); \
                float xFactor = ((float)renderer->current_context_target->window_w)/renderer->current_context_target->w; \
                float yFactor = ((float)renderer->current_context_target->window_h)/renderer->current_context_target->h; \
                glScissor(target->clipRect.x * xFactor, y * yFactor, target->clipRect.w * xFactor, target->clipRect.h * yFactor); \
            } \
             \
            glEnable( GL_TEXTURE_2D ); \
            \
            if(renderer->current_context_target->image == NULL && renderer->current_context_target->current_shader_program == renderer->current_context_target->default_untextured_shader_program) \
                renderer->ActivateShaderProgram(renderer, renderer->current_context_target->default_textured_shader_program, NULL); \
            \
            GLint vp[4]; \
            if(target->image != NULL) \
            { \
                glGetIntegerv(GL_VIEWPORT, vp); \
                \
                unsigned int w = target->w; \
                unsigned int h = target->h; \
                \
                glViewport( 0, 0, w, h); \
                \
                GPU_MatrixMode( GPU_PROJECTION ); \
                GPU_PushMatrix(); \
                GPU_LoadIdentity(); \
                \
                GPU_Ortho(0.0f, w, 0.0f, h, -1.0f, 1.0f); /* Special inverted orthographic projection because tex coords are inverted already. */ \
                \
                GPU_MatrixMode( GPU_MODELVIEW ); \
            }

#define END \
    RESET_COLOR; \
    if(target->image != NULL) \
    { \
        glViewport(vp[0], vp[1], vp[2], vp[3]); \
         \
        GPU_MatrixMode( GPU_PROJECTION ); \
        GPU_PopMatrix(); \
        GPU_MatrixMode( GPU_MODELVIEW ); \
    } \
	if(target->useClip) \
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
        int size = 3*num_vertices;
        int i;
        for(i = 0; i < size; i += 3)
        {
            glVertex3f(glverts[i], glverts[i+1], glverts[i+2]);
        }
        glEnd();
    #elif defined(SDL_GPU_USE_GL_TIER2)
        glVertexPointer(3, GL_FLOAT, 0, glverts);
        glEnableClientState(GL_VERTEX_ARRAY);
        glDrawArrays(prim_type, 0, num_vertices);
        glDisableClientState(GL_VERTEX_ARRAY);
    #elif defined(SDL_GPU_USE_GL_TIER3)
    
        GPU_Target* target = GPU_GetCurrentTarget();
        if(target == NULL)
            return;

        TARGET_DATA* data = ((TARGET_DATA*)target->data);
        
        int floats_per_vertex = 7;  // position (3), color (4)
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
        glBindBuffer(GL_ARRAY_BUFFER, data->blit_VBO);
        
        // Copy the whole blit buffer to the GPU
        glBufferData(GL_ARRAY_BUFFER, buffer_stride * num_vertices, glverts, GL_STREAM_DRAW);  // Creates space on the GPU and fills it with data.
        
        // Specify the formatting of the blit buffer
        if(data->current_shader_block.position_loc >= 0)
        {
            glEnableVertexAttribArray(data->current_shader_block.position_loc);  // Tell GL to use client-side attribute data
            glVertexAttribPointer(data->current_shader_block.position_loc, 3, GL_FLOAT, GL_FALSE, buffer_stride, 0);  // Tell how the data is formatted
        }
        if(data->current_shader_block.color_loc >= 0)
        {
            glEnableVertexAttribArray(data->current_shader_block.color_loc);
            glVertexAttribPointer(data->current_shader_block.color_loc, 4, GL_FLOAT, GL_FALSE, buffer_stride, (void*)(3 * sizeof(float)));
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
        int size = 5*num_vertices;
        int i;
        for(i = 0; i < size; i += 5)
        {
            glTexCoord2f(glverts[i+3], glverts[i+4]);
            glVertex3f(glverts[i], glverts[i+1], glverts[i+2]);
        }
        glEnd();
    #elif defined(SDL_GPU_USE_GL_TIER2)
        glVertexPointer(3, GL_FLOAT, 5*sizeof(float), glverts);
        glTexCoordPointer(2, GL_FLOAT, 5*sizeof(float), (glverts + 3));
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glDrawArrays(prim_type, 0, num_vertices);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    #elif defined(SDL_GPU_USE_GL_TIER3)
    
        GPU_Target* target = GPU_GetCurrentTarget();
        if(target == NULL)
            return;

        TARGET_DATA* data = ((TARGET_DATA*)target->data);
        
        int floats_per_vertex = 9;  // position (3), color (4), texcoord (2)
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
        glBindBuffer(GL_ARRAY_BUFFER, data->blit_VBO);
        
        // Copy the whole blit buffer to the GPU
        glBufferData(GL_ARRAY_BUFFER, buffer_stride * num_vertices, glverts, GL_STREAM_DRAW);  // Creates space on the GPU and fills it with data.
        
        // Specify the formatting of the blit buffer
        if(data->current_shader_block.position_loc >= 0)
        {
            glEnableVertexAttribArray(data->current_shader_block.position_loc);  // Tell GL to use client-side attribute data
            glVertexAttribPointer(data->current_shader_block.position_loc, 3, GL_FLOAT, GL_FALSE, buffer_stride, 0);  // Tell how the data is formatted
        }
        if(data->current_shader_block.color_loc >= 0)
        {
            glEnableVertexAttribArray(data->current_shader_block.color_loc);
            glVertexAttribPointer(data->current_shader_block.color_loc, 4, GL_FLOAT, GL_FALSE, buffer_stride, (void*)(3 * sizeof(float)));
        }
        if(data->current_shader_block.texcoord_loc >= 0)
        {
            glEnableVertexAttribArray(data->current_shader_block.texcoord_loc);
            glVertexAttribPointer(data->current_shader_block.texcoord_loc, 2, GL_FLOAT, GL_FALSE, buffer_stride, (void*)(7 * sizeof(float)));
        }
        
        glDrawArrays(prim_type, 0, num_vertices);
        
        #if !defined(SDL_GPU_USE_GLES) || SDL_GPU_GLES_MAJOR_VERSION != 2
        glBindVertexArray(0);
        #endif
    #endif
}


static float SetThickness(GPU_Renderer* renderer, float thickness)
{
    if(renderer->current_context_target == NULL)
        return 1.0f;
    
	float old = ((TARGET_DATA*)renderer->current_context_target->data)->line_thickness;
	((TARGET_DATA*)renderer->current_context_target->data)->line_thickness = thickness;
	glLineWidth(thickness);
	return old;
}

static float GetThickness(GPU_Renderer* renderer)
{
    return ((TARGET_DATA*)renderer->current_context_target->data)->line_thickness;
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
    
	float thickness = renderer->GetThickness(renderer);

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
    ((RENDERER_DATA*)renderer->data)->last_image = src;
    
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

