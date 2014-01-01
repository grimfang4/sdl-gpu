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





#define BEGIN \
	if(target == NULL) \
                return; \
        if(renderer != target->renderer) \
                return; \
        float z = ((RENDERER_DATA*)renderer->data)->z;  \
         \
        renderer->FlushBlitBuffer(renderer); \
        makeTargetCurrent(renderer, target); \
        if(bindFramebuffer(renderer, target)) \
        { \
        /*glPushAttrib(GL_COLOR_BUFFER_BIT);*/ \
        if(target->useClip) \
        { \
                glEnable(GL_SCISSOR_TEST); \
		int y = (renderer->current_target == target? renderer->current_target->h - (target->clipRect.y + target->clipRect.h) : target->clipRect.y); \
		float xFactor = ((float)renderer->current_target->window_w)/renderer->current_target->w; \
		float yFactor = ((float)renderer->current_target->window_h)/renderer->current_target->h; \
		glScissor(target->clipRect.x * xFactor, y * yFactor, target->clipRect.w * xFactor, target->clipRect.h * yFactor); \
	} \
	 \
	glDisable( GL_TEXTURE_2D ); \
	\
    if(target->current_shader_program == target->default_textured_shader_program) \
        renderer->ActivateShaderProgram(renderer, target->default_untextured_shader_program); \
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
        makeTargetCurrent(renderer, target); \
        if(bindFramebuffer(renderer, target)) \
        { \
        /*glPushAttrib(GL_COLOR_BUFFER_BIT);*/ \
        if(target->useClip) \
        { \
                glEnable(GL_SCISSOR_TEST); \
		int y = (renderer->current_target == target? renderer->current_target->h - (target->clipRect.y + target->clipRect.h) : target->clipRect.y); \
		float xFactor = ((float)renderer->current_target->window_w)/renderer->current_target->w; \
		float yFactor = ((float)renderer->current_target->window_h)/renderer->current_target->h; \
		glScissor(target->clipRect.x * xFactor, y * yFactor, target->clipRect.w * xFactor, target->clipRect.h * yFactor); \
	} \
	 \
	glEnable( GL_TEXTURE_2D ); \
	\
    if(target->current_shader_program == target->default_untextured_shader_program) \
        renderer->ActivateShaderProgram(renderer, target->default_textured_shader_program); \
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



	
	
static inline void set_vertex(float* verts, int index, float x, float y, float z)
{
    verts[index*3] = x;
    verts[index*3 + 1] = y;
    verts[index*3 + 2] = z;
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
        float mvp[16];
        GPU_GetModelViewProjection(mvp);
        GPU_SetUniformMatrixfv(data->current_shader_block.modelViewProjection_loc, 1, 4, 4, 0, mvp);
    
        // Update the vertex array object's buffers
        glBindVertexArray(data->blit_VAO);
        
        // Upload blit buffer to a single buffer object
        glBindBuffer(GL_ARRAY_BUFFER, data->blit_VBO);
        
        // Copy the whole blit buffer to the GPU
        glBufferData(GL_ARRAY_BUFFER, buffer_stride * num_vertices, glverts, GL_STREAM_DRAW);  // Creates space on the GPU and fills it with data.
        
        // Specify the formatting of the blit buffer
        glEnableVertexAttribArray(data->current_shader_block.position_loc);  // Tell GL to use client-side attribute data
        glVertexAttribPointer(data->current_shader_block.position_loc, 3, GL_FLOAT, GL_FALSE, buffer_stride, 0);  // Tell how the data is formatted
        glEnableVertexAttribArray(data->current_shader_block.color_loc);
        glVertexAttribPointer(data->current_shader_block.color_loc, 4, GL_FLOAT, GL_FALSE, buffer_stride, (void*)(3 * sizeof(float)));
        
        glDrawArrays(prim_type, 0, num_vertices);
        
        glBindVertexArray(0);
    #endif
}


static inline void draw_vertices_textured(GLfloat* glverts, GLfloat* gltexcoords, int num_vertices, GLenum prim_type)
{
    #ifdef SDL_GPU_USE_GL_TIER1
        glBegin(prim_type);
        int size = 3*num_vertices;
        int i, j;
        for(i = 0, j = 0; i < size; i += 3, j+=2)
        {
            glTexCoord2f(gltexcoords[j], gltexcoords[j+1]);
            glVertex3f(glverts[i], glverts[i+1], glverts[i+2]);
        }
        glEnd();
    #elif defined(SDL_GPU_USE_GL_TIER2)
        glVertexPointer(3, GL_FLOAT, 0, glverts);
        glTexCoordPointer(2, GL_FLOAT, 0, gltexcoords);
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
        
        int floats_per_vertex = 9;  // position (3), texcoord (2), color (4)
        int buffer_stride = floats_per_vertex * sizeof(float);
        
        // Upload our modelviewprojection matrix
        float mvp[16];
        GPU_GetModelViewProjection(mvp);
        GPU_SetUniformMatrixfv(data->current_shader_block.modelViewProjection_loc, 1, 4, 4, 0, mvp);
    
        // Update the vertex array object's buffers
        glBindVertexArray(data->blit_VAO);
        
        // Upload blit buffer to a single buffer object
        glBindBuffer(GL_ARRAY_BUFFER, data->blit_VBO);
        
        // Copy the whole blit buffer to the GPU
        glBufferData(GL_ARRAY_BUFFER, buffer_stride * num_vertices, glverts, GL_STREAM_DRAW);  // Creates space on the GPU and fills it with data.
        
        // Specify the formatting of the blit buffer
        glEnableVertexAttribArray(data->current_shader_block.position_loc);  // Tell GL to use client-side attribute data
        glVertexAttribPointer(data->current_shader_block.position_loc, 3, GL_FLOAT, GL_FALSE, buffer_stride, 0);  // Tell how the data is formatted
        glEnableVertexAttribArray(data->current_shader_block.texcoord_loc);
        glVertexAttribPointer(data->current_shader_block.texcoord_loc, 2, GL_FLOAT, GL_FALSE, buffer_stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(data->current_shader_block.color_loc);
        glVertexAttribPointer(data->current_shader_block.color_loc, 4, GL_FLOAT, GL_FALSE, buffer_stride, (void*)(5 * sizeof(float)));
        
        glDrawArrays(prim_type, 0, num_vertices);
        
        glBindVertexArray(0);
    #endif
}


static float SetThickness(GPU_Renderer* renderer, float thickness)
{
    if(renderer->current_target == NULL)
        return 1.0f;
    
	float old = ((TARGET_DATA*)renderer->current_target->data)->line_thickness;
	((TARGET_DATA*)renderer->current_target->data)->line_thickness = thickness;
	glLineWidth(thickness);
	return old;
}

static float GetThickness(GPU_Renderer* renderer)
{
    return ((TARGET_DATA*)renderer->current_target->data)->line_thickness;
}

static void Pixel(GPU_Renderer* renderer, GPU_Target* target, float x, float y, SDL_Color color)
{
	BEGIN;
	
        
        #ifdef SDL_GPU_USE_GL_TIER3
		GLfloat glverts[7];

		glverts[0] = x;
		glverts[1] = y;
		glverts[2] = z;
		glverts[3] = color.r/255.0f;
		glverts[4] = color.g/255.0f;
		glverts[5] = color.b/255.0f;
		glverts[6] = GET_ALPHA(color)/255.0f;
        #else
		renderer->SetRGBA(renderer, color.r, color.g, color.b, GET_ALPHA(color));
		
		GLfloat glverts[3];

		glverts[0] = x;
		glverts[1] = y;
		glverts[2] = z;
		#endif
        
        draw_vertices(glverts, 1, GL_POINTS);
        
        renderer->SetRGBA(renderer, 255, 255, 255, 255);

    END;
}

static void Line(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color)
{
    BEGIN;

        #ifdef SDL_GPU_USE_GL_TIER3
		float r = color.r/255.0f;
		float g = color.g/255.0f;
		float b = color.b/255.0f;
		float a = GET_ALPHA(color)/255.0f;
		GLfloat glverts[14];

		glverts[0] = x1;
		glverts[1] = y1;
		glverts[2] = z;
		glverts[3] = r;
		glverts[4] = g;
		glverts[5] = b;
		glverts[6] = a;
		glverts[7] = x2;
		glverts[8] = y2;
		glverts[9] = z;
		glverts[10] = r;
		glverts[11] = g;
		glverts[12] = b;
		glverts[13] = a;
        #else
        renderer->SetRGBA(renderer, color.r, color.g, color.b, GET_ALPHA(color));

        GLfloat glverts[6];

        glverts[0] = x1;
        glverts[1] = y1;
        glverts[2] = z;
        glverts[3] = x2;
        glverts[4] = y2;
        glverts[5] = z;
        #endif

        draw_vertices(glverts, 2, GL_LINES);
        
        renderer->SetRGBA(renderer, 255, 255, 255, 255);

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

    renderer->SetRGBA(renderer, color.r, color.g, color.b, GET_ALPHA(color));
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
        
    renderer->SetRGBA(renderer, 255, 255, 255, 255);

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

    renderer->SetRGBA(renderer, color.r, color.g, color.b, GET_ALPHA(color));
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
    
    renderer->SetRGBA(renderer, 255, 255, 255, 255);

    END;
}

static void Circle(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, SDL_Color color)
{
    BEGIN;

    renderer->SetRGBA(renderer, color.r, color.g, color.b, GET_ALPHA(color));
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
    
    renderer->SetRGBA(renderer, 255, 255, 255, 255);

    END;
}

static void CircleFilled(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, SDL_Color color)
{
    BEGIN;

    renderer->SetRGBA(renderer, color.r, color.g, color.b, GET_ALPHA(color));
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
    
    renderer->SetRGBA(renderer, 255, 255, 255, 255);

    END;
}

static void Tri(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color)
{
    BEGIN;

    renderer->SetRGBA(renderer, color.r, color.g, color.b, GET_ALPHA(color));

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
    
    renderer->SetRGBA(renderer, 255, 255, 255, 255);

    END;
}

static void TriFilled(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color)
{
    BEGIN;

    #ifdef SDL_GPU_USE_GL_TIER3
    float r = color.r/255.0f;
    float g = color.g/255.0f;
    float b = color.b/255.0f;
    float a = GET_ALPHA(color)/255.0f;
    GLfloat glverts[21];

    glverts[0] = x1;
    glverts[1] = y1;
    glverts[2] = z;
    glverts[3] = r;
    glverts[4] = g;
    glverts[5] = b;
    glverts[6] = a;
    glverts[7] = x2;
    glverts[8] = y2;
    glverts[9] = z;
    glverts[10] = r;
    glverts[11] = g;
    glverts[12] = b;
    glverts[13] = a;
    glverts[14] = x3;
    glverts[15] = y3;
    glverts[16] = z;
    glverts[17] = r;
    glverts[18] = g;
    glverts[19] = b;
    glverts[20] = a;
    #else
    renderer->SetRGBA(renderer, color.r, color.g, color.b, GET_ALPHA(color));

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
    #endif

    draw_vertices(glverts, 3, GL_TRIANGLE_STRIP);
    
    renderer->SetRGBA(renderer, 255, 255, 255, 255);

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

    renderer->SetRGBA(renderer, color.r, color.g, color.b, GET_ALPHA(color));

    GLfloat glverts[30];
    
    float t = thickness/2;
    
    glverts[0] = x1 - t;
    glverts[1] = y1 - t;
    glverts[2] = z;
    
    glverts[3] = x1 + t;
    glverts[4] = y1 + t;
    glverts[5] = z;
    
    glverts[6] = x2 + t;
    glverts[7] = y1 - t;
    glverts[8] = z;
    
    glverts[9] = x2 - t;
    glverts[10] = y1 + t;
    glverts[11] = z;
    
    glverts[12] = x2 + t;
    glverts[13] = y2 + t;
    glverts[14] = z;
    
    glverts[15] = x2 - t;
    glverts[16] = y2 - t;
    glverts[17] = z;
    
    glverts[18] = x1 - t;
    glverts[19] = y2 + t;
    glverts[20] = z;
    
    glverts[21] = x1 + t;
    glverts[22] = y2 - t;
    glverts[23] = z;
    
    glverts[24] = x1 - t;
    glverts[25] = y1 - t;
    glverts[26] = z;
    
    glverts[27] = x1 + t;
    glverts[28] = y1 + t;
    glverts[29] = z;
    
    draw_vertices(glverts, 10, GL_TRIANGLE_STRIP);
    
    renderer->SetRGBA(renderer, 255, 255, 255, 255);

    END;
}

static void RectangleFilled(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color)
{
    BEGIN;

    renderer->SetRGBA(renderer, color.r, color.g, color.b, GET_ALPHA(color));

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
    
    renderer->SetRGBA(renderer, 255, 255, 255, 255);

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

    renderer->SetRGBA(renderer, color.r, color.g, color.b, GET_ALPHA(color));
    
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
    
    renderer->SetRGBA(renderer, 255, 255, 255, 255);

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

    renderer->SetRGBA(renderer, color.r, color.g, color.b, GET_ALPHA(color));
    
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
    
    renderer->SetRGBA(renderer, 255, 255, 255, 255);
    
    END;
}

static void Polygon(GPU_Renderer* renderer, GPU_Target* target, Uint16 n, float* vertices, SDL_Color color)
{
    BEGIN;

    renderer->SetRGBA(renderer, color.r, color.g, color.b, GET_ALPHA(color));
    
    int numIndices = 2*n;
    float glverts[numIndices*3];
    
    int i, j;
    for(i = 0, j = 0; i < numIndices; i+=2, j++)
    {
        set_vertex(glverts, j, vertices[i], vertices[i+1], z);
    }
    
    draw_vertices(glverts, n, GL_LINE_LOOP);
    
    renderer->SetRGBA(renderer, 255, 255, 255, 255);

    END;
}

static void PolygonFilled(GPU_Renderer* renderer, GPU_Target* target, Uint16 n, float* vertices, SDL_Color color)
{
    BEGIN;

    renderer->SetRGBA(renderer, color.r, color.g, color.b, GET_ALPHA(color));
    
    int numIndices = 2*n;
    float glverts[numIndices*3];
    
    int i, j;
    for(i = 0, j = 0; i < numIndices; i+=2, j++)
    {
        set_vertex(glverts, j, vertices[i], vertices[i+1], z);
    }
    
    draw_vertices(glverts, n, GL_TRIANGLE_FAN);
    
    renderer->SetRGBA(renderer, 255, 255, 255, 255);

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
    float glverts[numIndices*3];
    float gltexcoords[numIndices*2];
    
    int i, j;
    for(i = 0, j = 0; i < numIndices; i+=2, j++)
    {
        float x = vertices[i];
    	float y = vertices[i+1];

    	gltexcoords[i] = (x - textureX)*scaleX/src->w;
    	gltexcoords[i+1] = (y - textureY)*scaleY/src->h;
    	
        set_vertex(glverts, j, vertices[i], vertices[i+1], z);
    }
    
    draw_vertices_textured(glverts, gltexcoords, n, GL_TRIANGLE_FAN);

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    
    END;
}

