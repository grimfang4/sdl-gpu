/* This is an implementation file to be included after certain #defines have been set.
See a particular renderer's *.c file for specifics. */


// All shapes start this way for setup and so they can access the blit buffer properly
#define BEGIN_UNTEXTURED(function_name, shape, num_additional_vertices, num_additional_indices) \
	GPU_CONTEXT_DATA* cdata; \
	float* blit_buffer; \
	unsigned short* index_buffer; \
	int vert_index; \
	int color_index; \
	float r, g, b, a; \
	unsigned short blit_buffer_starting_index; \
    if(target == NULL) \
    { \
        GPU_PushErrorCode(function_name, GPU_ERROR_NULL_ARGUMENT, "target"); \
        return; \
    } \
    if(renderer != target->renderer) \
    { \
        GPU_PushErrorCode(function_name, GPU_ERROR_USER_ERROR, "Mismatched renderer"); \
        return; \
    } \
     \
    makeContextCurrent(renderer, target); \
    if(renderer->current_context_target == NULL) \
    { \
        GPU_PushErrorCode(function_name, GPU_ERROR_USER_ERROR, "NULL context"); \
        return; \
    } \
     \
    if(!SetActiveTarget(renderer, target)) \
    { \
        GPU_PushErrorCode(function_name, GPU_ERROR_BACKEND_ERROR, "Failed to bind framebuffer."); \
        return; \
    } \
     \
    prepareToRenderToTarget(renderer, target); \
    prepareToRenderShapes(renderer, shape); \
     \
    cdata = (GPU_CONTEXT_DATA*)renderer->current_context_target->context->data; \
     \
    if(cdata->blit_buffer_num_vertices + (num_additional_vertices) >= cdata->blit_buffer_max_num_vertices) \
    { \
        if(!growBlitBuffer(cdata, cdata->blit_buffer_num_vertices + (num_additional_vertices))) \
            renderer->impl->FlushBlitBuffer(renderer); \
    } \
    if(cdata->index_buffer_num_vertices + (num_additional_indices) >= cdata->index_buffer_max_num_vertices) \
    { \
        if(!growIndexBuffer(cdata, cdata->index_buffer_num_vertices + (num_additional_indices))) \
            renderer->impl->FlushBlitBuffer(renderer); \
    } \
     \
    blit_buffer = cdata->blit_buffer; \
    index_buffer = cdata->index_buffer; \
     \
    vert_index = GPU_BLIT_BUFFER_VERTEX_OFFSET + cdata->blit_buffer_num_vertices*GPU_BLIT_BUFFER_FLOATS_PER_VERTEX; \
    color_index = GPU_BLIT_BUFFER_COLOR_OFFSET + cdata->blit_buffer_num_vertices*GPU_BLIT_BUFFER_FLOATS_PER_VERTEX; \
     \
    if(target->use_color) \
    { \
        r = MIX_COLOR_COMPONENT_NORMALIZED_RESULT(target->color.r, color.r); \
        g = MIX_COLOR_COMPONENT_NORMALIZED_RESULT(target->color.g, color.g); \
        b = MIX_COLOR_COMPONENT_NORMALIZED_RESULT(target->color.b, color.b); \
        a = MIX_COLOR_COMPONENT_NORMALIZED_RESULT(GET_ALPHA(target->color), GET_ALPHA(color)); \
    } \
    else \
    { \
        r = color.r/255.0f; \
        g = color.g/255.0f; \
        b = color.b/255.0f; \
        a = GET_ALPHA(color)/255.0f; \
    } \
    blit_buffer_starting_index = cdata->blit_buffer_num_vertices; \
    (void)blit_buffer_starting_index;





static float SetLineThickness(GPU_Renderer* renderer, float thickness)
{
	float old;

    if(renderer->current_context_target == NULL)
        return 1.0f;
    
	old = renderer->current_context_target->context->line_thickness;
	if(old != thickness)
        renderer->impl->FlushBlitBuffer(renderer);
    
	renderer->current_context_target->context->line_thickness = thickness;
	#ifndef SDL_GPU_SKIP_LINE_WIDTH
	glLineWidth(thickness);
	#endif
	return old;
}

static float GetLineThickness(GPU_Renderer* renderer)
{
    return renderer->current_context_target->context->line_thickness;
}

static void Pixel(GPU_Renderer* renderer, GPU_Target* target, float x, float y, SDL_Color color)
{
    BEGIN_UNTEXTURED("GPU_Pixel", GL_POINTS, 1, 1);
    
    SET_UNTEXTURED_VERTEX(x, y, r, g, b, a);
}

static void Line(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color)
{
	float thickness = GetLineThickness(renderer);

    float t = thickness/2;
    float line_angle = atan2f(y2 - y1, x2 - x1);
    float tc = t*cosf(line_angle);
    float ts = t*sinf(line_angle);

    BEGIN_UNTEXTURED("GPU_Line", GL_TRIANGLES, 4, 6);
    
    SET_UNTEXTURED_VERTEX(x1 + ts, y1 - tc, r, g, b, a);
    SET_UNTEXTURED_VERTEX(x1 - ts, y1 + tc, r, g, b, a);
    SET_UNTEXTURED_VERTEX(x2 + ts, y2 - tc, r, g, b, a);
    
    SET_INDEXED_VERTEX(1);
    SET_INDEXED_VERTEX(2);
    SET_UNTEXTURED_VERTEX(x2 - ts, y2 + tc, r, g, b, a);
}

// Arc() might call Circle()
static void Circle(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, SDL_Color color);

static void Arc(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, float start_angle, float end_angle, SDL_Color color)
{
    float dx, dy;
    int i;
    
    float t = GetLineThickness(renderer)/2;
    float inner_radius = radius - t;
    float outer_radius = radius + t;
    
    float dt;
    int numSegments;
    
    float tempx;
    float c, s;
    
    if(inner_radius < 0.0f)
        inner_radius = 0.0f;

    if(start_angle > end_angle)
    {
        float swapa = end_angle;
        end_angle = start_angle;
        start_angle = swapa;
    }
    if(start_angle == end_angle)
        return;

    // Big angle
    if(end_angle - start_angle >= 360)
    {
        Circle(renderer, target, x, y, radius, color);
        return;
    }

    // Shift together
    while(start_angle < 0 && end_angle < 0)
    {
        start_angle += 360;
        end_angle += 360;
    }
    while(start_angle > 360 && end_angle > 360)
    {
        start_angle -= 360;
        end_angle -= 360;
    }


    dt = ((end_angle - start_angle)/360)*(1.25f/sqrtf(outer_radius));  // s = rA, so dA = ds/r.  ds of 1.25*sqrt(radius) is good, use A in degrees.

    numSegments = (int)((fabs(end_angle - start_angle)*PI/180)/dt);
    if(numSegments == 0)
		return;
    
	{
		BEGIN_UNTEXTURED("GPU_Arc", GL_TRIANGLES, 2*(numSegments), 6*(numSegments));
		
        c = cosf(dt);
        s = sinf(dt);
        
        // Rotate to start
        start_angle *= RAD_PER_DEG;
        dx = cosf(start_angle);
        dy = sinf(start_angle);

        BEGIN_UNTEXTURED_SEGMENTS(x+inner_radius*dx, y+inner_radius*dy, x+outer_radius*dx, y+outer_radius*dy, r, g, b, a);

		for (i = 1; i < numSegments; i++)
		{
            tempx = c * dx - s * dy;
            dy = s * dx + c * dy;
            dx = tempx;
            SET_UNTEXTURED_SEGMENTS(x+inner_radius*dx, y+inner_radius*dy, x+outer_radius*dx, y+outer_radius*dy, r, g, b, a);
		}

		// Last point
        end_angle *= RAD_PER_DEG;
        dx = cosf(end_angle);
        dy = sinf(end_angle);
        END_UNTEXTURED_SEGMENTS(x+inner_radius*dx, y+inner_radius*dy, x+outer_radius*dx, y+outer_radius*dy, r, g, b, a);
	}
}

// ArcFilled() might call CircleFilled()
static void CircleFilled(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, SDL_Color color);

static void ArcFilled(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, float start_angle, float end_angle, SDL_Color color)
{
    float dx, dy;
    int i;
    
    float dt;
    int numSegments;
    
    float tempx;
    float c, s;

    if(start_angle > end_angle)
    {
        float swapa = end_angle;
        end_angle = start_angle;
        start_angle = swapa;
    }
    if(start_angle == end_angle)
        return;

    // Big angle
    if(end_angle - start_angle >= 360)
    {
        CircleFilled(renderer, target, x, y, radius, color);
        return;
    }

    // Shift together
    while(start_angle < 0 && end_angle < 0)
    {
        start_angle += 360;
        end_angle += 360;
    }
    while(start_angle > 360 && end_angle > 360)
    {
        start_angle -= 360;
        end_angle -= 360;
    }
    
    dt = ((end_angle - start_angle)/360)*(1.25f/sqrtf(radius));  // s = rA, so dA = ds/r.  ds of 1.25*sqrt(radius) is good, use A in degrees.

    numSegments = (int)((fabs(end_angle - start_angle)*RAD_PER_DEG)/dt);
    if(numSegments == 0)
		return;

	{
		BEGIN_UNTEXTURED("GPU_ArcFilled", GL_TRIANGLES, 3 + (numSegments - 1) + 1, 3 + (numSegments - 1) * 3 + 3);
        
        c = cosf(dt);
        s = sinf(dt);
        
        // Rotate to start
        start_angle *= RAD_PER_DEG;
        dx = cosf(start_angle);
        dy = sinf(start_angle);

		// First triangle
		SET_UNTEXTURED_VERTEX(x, y, r, g, b, a);
		SET_UNTEXTURED_VERTEX(x + radius*dx, y + radius*dy, r, g, b, a); // first point
		
        tempx = c * dx - s * dy;
        dy = s * dx + c * dy;
        dx = tempx;
		SET_UNTEXTURED_VERTEX(x + radius*dx, y + radius*dy, r, g, b, a); // new point

		for (i = 2; i < numSegments + 1; i++)
		{
            tempx = c * dx - s * dy;
            dy = s * dx + c * dy;
            dx = tempx;
			SET_INDEXED_VERTEX(0);  // center
			SET_INDEXED_VERTEX(i);  // last point
			SET_UNTEXTURED_VERTEX(x + radius*dx, y + radius*dy, r, g, b, a); // new point
		}

		// Last triangle
        end_angle *= RAD_PER_DEG;
        dx = cosf(end_angle);
        dy = sinf(end_angle);
		SET_INDEXED_VERTEX(0);  // center
		SET_INDEXED_VERTEX(i);  // last point
		SET_UNTEXTURED_VERTEX(x + radius*dx, y + radius*dy, r, g, b, a); // new point
	}
}


/*
Incremental rotation circle algorithm
*/

static void Circle(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, SDL_Color color)
{
	float thickness = GetLineThickness(renderer);
    float dx, dy;
    int i;
    float t = thickness/2;
    float inner_radius = radius - t;
    float outer_radius = radius + t;
    float dt = (1.25f/sqrtf(outer_radius));  // s = rA, so dA = ds/r.  ds of 1.25*sqrt(radius) is good, use A in degrees.
    int numSegments = (int)(2*PI/dt)+1;
    
    float tempx;
    float c = cosf(dt);
    float s = sinf(dt);
    
    BEGIN_UNTEXTURED("GPU_Circle", GL_TRIANGLES, 2*(numSegments), 6*(numSegments));
    
    if(inner_radius < 0.0f)
        inner_radius = 0.0f;
    
    dx = 1.0f;
    dy = 0.0f;
    
    BEGIN_UNTEXTURED_SEGMENTS(x+inner_radius*dx, y+inner_radius*dy, x+outer_radius*dx, y+outer_radius*dy, r, g, b, a);
    
    for(i = 1; i < numSegments; i++)
    {
        tempx = c * dx - s * dy;
        dy = s * dx + c * dy;
        dx = tempx;

        SET_UNTEXTURED_SEGMENTS(x+inner_radius*dx, y+inner_radius*dy, x+outer_radius*dx, y+outer_radius*dy, r, g, b, a);
    }
    
    LOOP_UNTEXTURED_SEGMENTS();  // back to the beginning
}

static void CircleFilled(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, SDL_Color color)
{
    float dt = (1.25f/sqrtf(radius));  // s = rA, so dA = ds/r.  ds of 1.25*sqrt(radius) is good, use A in degrees.
    float dx, dy;
    int numSegments = (int)(2*PI/dt)+1;
    int i;
    
    float tempx;
    float c = cosf(dt);
    float s = sinf(dt);
    
    BEGIN_UNTEXTURED("GPU_CircleFilled", GL_TRIANGLES, 3 + (numSegments-2), 3 + (numSegments-2)*3 + 3);
    
    // First triangle
    SET_UNTEXTURED_VERTEX(x, y, r, g, b, a);  // Center
    
    dx = 1.0f;
    dy = 0.0f;
    SET_UNTEXTURED_VERTEX(x+radius*dx, y+radius*dy, r, g, b, a); // first point
    
    tempx = c * dx - s * dy;
    dy = s * dx + c * dy;
    dx = tempx;
    SET_UNTEXTURED_VERTEX(x+radius*dx, y+radius*dy, r, g, b, a); // new point
    
    for(i = 2; i < numSegments; i++)
    {
        tempx = c * dx - s * dy;
        dy = s * dx + c * dy;
        dx = tempx;
        SET_INDEXED_VERTEX(0);  // center
        SET_INDEXED_VERTEX(i);  // last point
        SET_UNTEXTURED_VERTEX(x+radius*dx, y+radius*dy, r, g, b, a); // new point
    }
    
    SET_INDEXED_VERTEX(0);  // center
    SET_INDEXED_VERTEX(i);  // last point
    SET_INDEXED_VERTEX(1);  // first point
}

static void Ellipse(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float rx, float ry, float degrees, SDL_Color color)
{
	float thickness = GetLineThickness(renderer);
    float dx, dy;
    int i;
    float t = thickness/2;
    float rot_x = cosf(degrees*RAD_PER_DEG);
    float rot_y = sinf(degrees*RAD_PER_DEG);
    float inner_radius_x = rx - t;
    float outer_radius_x = rx + t;
    float inner_radius_y = ry - t;
    float outer_radius_y = ry + t;
    float dt = (1.25f/sqrtf(outer_radius_x > outer_radius_y? outer_radius_x : outer_radius_y));  // s = rA, so dA = ds/r.  ds of 1.25*sqrt(radius) is good, use A in degrees.
    int numSegments = (int)(2*M_PI/dt)+1;
    
    float tempx;
    float c = cosf(dt);
    float s = sinf(dt);
    float inner_trans_x, inner_trans_y;
    float outer_trans_x, outer_trans_y;
    
    BEGIN_UNTEXTURED("GPU_Ellipse", GL_TRIANGLES, 2*(numSegments), 6*(numSegments));
    
    if(inner_radius_x < 0.0f)
        inner_radius_x = 0.0f;
    if(inner_radius_y < 0.0f)
        inner_radius_y = 0.0f;
    
    dx = 1.0f;
    dy = 0.0f;
    
    inner_trans_x = rot_x * inner_radius_x*dx - rot_y * inner_radius_y*dy;
    inner_trans_y = rot_y * inner_radius_x*dx + rot_x * inner_radius_y*dy;
    outer_trans_x = rot_x * outer_radius_x*dx - rot_y * outer_radius_y*dy;
    outer_trans_y = rot_y * outer_radius_x*dx + rot_x * outer_radius_y*dy;
    BEGIN_UNTEXTURED_SEGMENTS(x+inner_trans_x, y+inner_trans_y, x+outer_trans_x, y+outer_trans_y, r, g, b, a);
    
    for(i = 1; i < numSegments; i++)
    {
        tempx = c * dx - s * dy;
        dy = (s * dx + c * dy);
        dx = tempx;

        inner_trans_x = rot_x * inner_radius_x*dx - rot_y * inner_radius_y*dy;
        inner_trans_y = rot_y * inner_radius_x*dx + rot_x * inner_radius_y*dy;
        outer_trans_x = rot_x * outer_radius_x*dx - rot_y * outer_radius_y*dy;
        outer_trans_y = rot_y * outer_radius_x*dx + rot_x * outer_radius_y*dy;
        SET_UNTEXTURED_SEGMENTS(x+inner_trans_x, y+inner_trans_y, x+outer_trans_x, y+outer_trans_y, r, g, b, a);
    }
    
    LOOP_UNTEXTURED_SEGMENTS();  // back to the beginning
}

static void EllipseFilled(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float rx, float ry, float degrees, SDL_Color color)
{
    float dx, dy;
    int i;
    float rot_x = cosf(degrees*RAD_PER_DEG);
    float rot_y = sinf(degrees*RAD_PER_DEG);
    float dt = (1.25f/sqrtf(rx > ry? rx : ry));  // s = rA, so dA = ds/r.  ds of 1.25*sqrt(radius) is good, use A in degrees.
    int numSegments = (int)(2*M_PI/dt)+1;
    
    float tempx;
    float c = cosf(dt);
    float s = sinf(dt);
    float trans_x, trans_y;
    
    BEGIN_UNTEXTURED("GPU_EllipseFilled", GL_TRIANGLES, 3 + (numSegments-2), 3 + (numSegments-2)*3 + 3);
    
    // First triangle
    SET_UNTEXTURED_VERTEX(x, y, r, g, b, a);  // Center
    
    dx = 1.0f;
    dy = 0.0f;
    trans_x = rot_x * rx*dx - rot_y * ry*dy;
    trans_y = rot_y * rx*dx + rot_x * ry*dy;
    SET_UNTEXTURED_VERTEX(x+trans_x, y+trans_y, r, g, b, a); // first point
    
    tempx = c * dx - s * dy;
    dy = s * dx + c * dy;
    dx = tempx;
    
    trans_x = rot_x * rx*dx - rot_y * ry*dy;
    trans_y = rot_y * rx*dx + rot_x * ry*dy;
    SET_UNTEXTURED_VERTEX(x+trans_x, y+trans_y, r, g, b, a); // new point
    
    for(i = 2; i < numSegments; i++)
    {
        tempx = c * dx - s * dy;
        dy = (s * dx + c * dy);
        dx = tempx;

        trans_x = rot_x * rx*dx - rot_y * ry*dy;
        trans_y = rot_y * rx*dx + rot_x * ry*dy;
        
        SET_INDEXED_VERTEX(0);  // center
        SET_INDEXED_VERTEX(i);  // last point
        SET_UNTEXTURED_VERTEX(x+trans_x, y+trans_y, r, g, b, a); // new point
    }
    
    SET_INDEXED_VERTEX(0);  // center
    SET_INDEXED_VERTEX(i);  // last point
    SET_INDEXED_VERTEX(1);  // first point
}

static void Sector(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float inner_radius, float outer_radius, float start_angle, float end_angle, SDL_Color color)
{
	GPU_bool circled;
	float dx1, dy1, dx2, dy2, dx3, dy3, dx4, dy4;

    if(inner_radius < 0.0f)
        inner_radius = 0.0f;
    if(outer_radius < 0.0f)
        outer_radius = 0.0f;
    
    if(inner_radius > outer_radius)
    {
        float s = inner_radius;
        inner_radius = outer_radius;
        outer_radius = s;
    }
    
    if(start_angle > end_angle)
    {
        float swapa = end_angle;
        end_angle = start_angle;
        start_angle = swapa;
    }
    if(start_angle == end_angle)
        return;
    
    if(inner_radius == outer_radius)
    {
        Arc(renderer, target, x, y, inner_radius, start_angle, end_angle, color);
        return;
    }
    
    circled = (end_angle - start_angle >= 360);
    // Composited shape...  But that means error codes may be confusing. :-/
    Arc(renderer, target, x, y, inner_radius, start_angle, end_angle, color);
    
    if(!circled)
    {
        dx1 = inner_radius*cosf(end_angle*RAD_PER_DEG);
        dy1 = inner_radius*sinf(end_angle*RAD_PER_DEG);
        dx2 = outer_radius*cosf(end_angle*RAD_PER_DEG);
        dy2 = outer_radius*sinf(end_angle*RAD_PER_DEG);
        Line(renderer, target, x+dx1, y+dy1, x+dx2, y+dy2, color);
    }
    
    Arc(renderer, target, x, y, outer_radius, start_angle, end_angle, color);
    
    if(!circled)
    {
        dx3 = inner_radius*cosf(start_angle*RAD_PER_DEG);
        dy3 = inner_radius*sinf(start_angle*RAD_PER_DEG);
        dx4 = outer_radius*cosf(start_angle*RAD_PER_DEG);
        dy4 = outer_radius*sinf(start_angle*RAD_PER_DEG);
        Line(renderer, target, x+dx3, y+dy3, x+dx4, y+dy4, color);
    }
}

static void SectorFilled(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float inner_radius, float outer_radius, float start_angle, float end_angle, SDL_Color color)
{
	float t;
	float dt;
	float dx, dy;

	int numSegments;

    if(inner_radius < 0.0f)
        inner_radius = 0.0f;
    if(outer_radius < 0.0f)
        outer_radius = 0.0f;
    
    if(inner_radius > outer_radius)
    {
        float s = inner_radius;
        inner_radius = outer_radius;
        outer_radius = s;
    }
    
    if(inner_radius == outer_radius)
    {
        Arc(renderer, target, x, y, inner_radius, start_angle, end_angle, color);
        return;
    }
    
    
    if(start_angle > end_angle)
    {
        float swapa = end_angle;
        end_angle = start_angle;
        start_angle = swapa;
    }
    if(start_angle == end_angle)
        return;

    if(end_angle - start_angle >= 360)
        end_angle = start_angle + 360;
    
    
    t = start_angle;
    dt = ((end_angle - start_angle)/360)*(1.25f/sqrtf(outer_radius)) * DEG_PER_RAD;  // s = rA, so dA = ds/r.  ds of 1.25*sqrt(radius) is good, use A in degrees.

    numSegments = (int)(fabs(end_angle - start_angle)/dt);
    if(numSegments == 0)
		return;

	{
		int i;
		GPU_bool use_inner;
		BEGIN_UNTEXTURED("GPU_SectorFilled", GL_TRIANGLES, 3 + (numSegments - 1) + 1, 3 + (numSegments - 1) * 3 + 3);

		use_inner = GPU_FALSE;  // Switches between the radii for the next point

		// First triangle
		dx = inner_radius*cosf(t*RAD_PER_DEG);
		dy = inner_radius*sinf(t*RAD_PER_DEG);
		SET_UNTEXTURED_VERTEX(x + dx, y + dy, r, g, b, a);

		dx = outer_radius*cosf(t*RAD_PER_DEG);
		dy = outer_radius*sinf(t*RAD_PER_DEG);
		SET_UNTEXTURED_VERTEX(x + dx, y + dy, r, g, b, a);
		t += dt;
		dx = inner_radius*cosf(t*RAD_PER_DEG);
		dy = inner_radius*sinf(t*RAD_PER_DEG);
		SET_UNTEXTURED_VERTEX(x + dx, y + dy, r, g, b, a);
		t += dt;

		for (i = 2; i < numSegments + 1; i++)
		{
			SET_INDEXED_VERTEX(i - 1);
			SET_INDEXED_VERTEX(i);
			if (use_inner)
			{
				dx = inner_radius*cosf(t*RAD_PER_DEG);
				dy = inner_radius*sinf(t*RAD_PER_DEG);
			}
			else
			{
				dx = outer_radius*cosf(t*RAD_PER_DEG);
				dy = outer_radius*sinf(t*RAD_PER_DEG);
			}
			SET_UNTEXTURED_VERTEX(x + dx, y + dy, r, g, b, a); // new point
			t += dt;
			use_inner = !use_inner;
		}

		// Last quad
		t = end_angle;
		if (use_inner)
		{
			dx = inner_radius*cosf(t*RAD_PER_DEG);
			dy = inner_radius*sinf(t*RAD_PER_DEG);
		}
		else
		{
			dx = outer_radius*cosf(t*RAD_PER_DEG);
			dy = outer_radius*sinf(t*RAD_PER_DEG);
		}
		SET_INDEXED_VERTEX(i - 1);
		SET_INDEXED_VERTEX(i);
		SET_UNTEXTURED_VERTEX(x + dx, y + dy, r, g, b, a); // new point
		use_inner = !use_inner;
		i++;

		if (use_inner)
		{
			dx = inner_radius*cosf(t*RAD_PER_DEG);
			dy = inner_radius*sinf(t*RAD_PER_DEG);
		}
		else
		{
			dx = outer_radius*cosf(t*RAD_PER_DEG);
			dy = outer_radius*sinf(t*RAD_PER_DEG);
		}
		SET_INDEXED_VERTEX(i - 1);
		SET_INDEXED_VERTEX(i);
		SET_UNTEXTURED_VERTEX(x + dx, y + dy, r, g, b, a); // new point
	}
}

static void Tri(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color)
{
    BEGIN_UNTEXTURED("GPU_Tri", GL_LINES, 3, 6);
    
    SET_UNTEXTURED_VERTEX(x1, y1, r, g, b, a);
    SET_UNTEXTURED_VERTEX(x2, y2, r, g, b, a);
    
    SET_INDEXED_VERTEX(1);
    SET_UNTEXTURED_VERTEX(x3, y3, r, g, b, a);
    
    SET_INDEXED_VERTEX(2);
    SET_INDEXED_VERTEX(0);
}

static void TriFilled(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color)
{
    BEGIN_UNTEXTURED("GPU_TriFilled", GL_TRIANGLES, 3, 3);
    
    SET_UNTEXTURED_VERTEX(x1, y1, r, g, b, a);
    SET_UNTEXTURED_VERTEX(x2, y2, r, g, b, a);
    SET_UNTEXTURED_VERTEX(x3, y3, r, g, b, a);
}

static void Rectangle(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color)
{
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
    
	{
		float thickness = GetLineThickness(renderer);
        
        // Thickness offsets
		float outer = thickness / 2;
		float inner_x = outer;
		float inner_y = outer;

		// Thick lines via filled triangles

		BEGIN_UNTEXTURED("GPU_Rectangle", GL_TRIANGLES, 12, 24);
		
		// Adjust inner thickness offsets to avoid overdraw on narrow/small rects
		if(x1 + inner_x > x2 - inner_x)
            inner_x = (x2 - x1)/2;
		if(y1 + inner_y > y2 - inner_y)
            inner_y = (y2 - y1)/2;

		// First triangle
		SET_UNTEXTURED_VERTEX(x1 - outer, y1 - outer, r, g, b, a); // 0
		SET_UNTEXTURED_VERTEX(x1 - outer, y1 + inner_y, r, g, b, a); // 1
		SET_UNTEXTURED_VERTEX(x2 + outer, y1 - outer, r, g, b, a); // 2

		SET_INDEXED_VERTEX(2);
		SET_INDEXED_VERTEX(1);
		SET_UNTEXTURED_VERTEX(x2 + outer, y1 + inner_y, r, g, b, a); // 3

		SET_INDEXED_VERTEX(3);
		SET_UNTEXTURED_VERTEX(x2 - inner_x, y1 + inner_y, r, g, b, a); // 4
		SET_UNTEXTURED_VERTEX(x2 - inner_x, y2 - inner_y, r, g, b, a); // 5

		SET_INDEXED_VERTEX(3);
		SET_INDEXED_VERTEX(5);
		SET_UNTEXTURED_VERTEX(x2 + outer, y2 - inner_y, r, g, b, a); // 6

		SET_INDEXED_VERTEX(6);
		SET_UNTEXTURED_VERTEX(x1 - outer, y2 - inner_y, r, g, b, a); // 7
		SET_UNTEXTURED_VERTEX(x2 + outer, y2 + outer, r, g, b, a); // 8

		SET_INDEXED_VERTEX(7);
		SET_UNTEXTURED_VERTEX(x1 - outer, y2 + outer, r, g, b, a); // 9
		SET_INDEXED_VERTEX(8);

		SET_INDEXED_VERTEX(7);
		SET_UNTEXTURED_VERTEX(x1 + inner_x, y2 - inner_y, r, g, b, a); // 10
		SET_INDEXED_VERTEX(1);

		SET_INDEXED_VERTEX(1);
		SET_INDEXED_VERTEX(10);
		SET_UNTEXTURED_VERTEX(x1 + inner_x, y1 + inner_y, r, g, b, a); // 11
	}
}

static void RectangleFilled(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color)
{
    BEGIN_UNTEXTURED("GPU_RectangleFilled", GL_TRIANGLES, 4, 6);

    SET_UNTEXTURED_VERTEX(x1, y1, r, g, b, a);
    SET_UNTEXTURED_VERTEX(x1, y2, r, g, b, a);
    SET_UNTEXTURED_VERTEX(x2, y1, r, g, b, a);
    
    SET_INDEXED_VERTEX(1);
    SET_INDEXED_VERTEX(2);
    SET_UNTEXTURED_VERTEX(x2, y2, r, g, b, a);
}

#define INCREMENT_CIRCLE \
    tempx = c * dx - s * dy; \
    dy = s * dx + c * dy; \
    dx = tempx; \
    ++i;

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
    
    if(radius > (x2-x1)/2)
        radius = (x2-x1)/2;
    if(radius > (y2-y1)/2)
		radius = (y2 - y1) / 2;
    
    x1 += radius;
    y1 += radius;
    x2 -= radius;
    y2 -= radius;
    
    {
        float thickness = GetLineThickness(renderer);
        float dx, dy;
        int i = 0;
        float t = thickness/2;
        float inner_radius = radius - t;
        float outer_radius = radius + t;
        float dt = (1.25f/sqrtf(outer_radius));  // s = rA, so dA = ds/r.  ds of 1.25*sqrt(radius) is good, use A in degrees.
        int numSegments = (int)(2*PI/dt)+1;
        if(numSegments < 4)
            numSegments = 4;
        
        // Make a multiple of 4 so we can have even corners
        numSegments += numSegments % 4;
        
        dt = 2*PI/(numSegments-1);
        
        {
            float x, y;
            int go_to_second = numSegments / 4;
            int go_to_third = numSegments / 2;
            int go_to_fourth = 3*numSegments / 4;
            
            float tempx;
            float c = cosf(dt);
            float s = sinf(dt);
            
            // Add another 4 for the extra corner vertices
            BEGIN_UNTEXTURED("GPU_RectangleRound", GL_TRIANGLES, 2*(numSegments + 4), 6*(numSegments + 4));
            
            if(inner_radius < 0.0f)
                inner_radius = 0.0f;
            
            dx = 1.0f;
            dy = 0.0f;
            
            x = x2;
            y = y2;
            BEGIN_UNTEXTURED_SEGMENTS(x+inner_radius, y, x+outer_radius, y, r, g, b, a);
            while(i < go_to_second-1)
            {
                INCREMENT_CIRCLE;

                SET_UNTEXTURED_SEGMENTS(x+inner_radius*dx, y+inner_radius*dy, x+outer_radius*dx, y+outer_radius*dy, r, g, b, a);
            }
            INCREMENT_CIRCLE;
            
            SET_UNTEXTURED_SEGMENTS(x, y+inner_radius, x, y+outer_radius, r, g, b, a);
            x = x1;
            y = y2;
            SET_UNTEXTURED_SEGMENTS(x, y+inner_radius, x, y+outer_radius, r, g, b, a);
            while(i < go_to_third-1)
            {
                INCREMENT_CIRCLE;

                SET_UNTEXTURED_SEGMENTS(x+inner_radius*dx, y+inner_radius*dy, x+outer_radius*dx, y+outer_radius*dy, r, g, b, a);
            }
            INCREMENT_CIRCLE;
            
            SET_UNTEXTURED_SEGMENTS(x-inner_radius, y, x-outer_radius, y, r, g, b, a);
            x = x1;
            y = y1;
            SET_UNTEXTURED_SEGMENTS(x-inner_radius, y, x-outer_radius, y, r, g, b, a);
            while(i < go_to_fourth-1)
            {
                INCREMENT_CIRCLE;

                SET_UNTEXTURED_SEGMENTS(x+inner_radius*dx, y+inner_radius*dy, x+outer_radius*dx, y+outer_radius*dy, r, g, b, a);
            }
            INCREMENT_CIRCLE;
            
            SET_UNTEXTURED_SEGMENTS(x, y-inner_radius, x, y-outer_radius, r, g, b, a);
            x = x2;
            y = y1;
            SET_UNTEXTURED_SEGMENTS(x, y-inner_radius, x, y-outer_radius, r, g, b, a);
            while(i < numSegments-1)
            {
                INCREMENT_CIRCLE;

                SET_UNTEXTURED_SEGMENTS(x+inner_radius*dx, y+inner_radius*dy, x+outer_radius*dx, y+outer_radius*dy, r, g, b, a);
            }
            SET_UNTEXTURED_SEGMENTS(x+inner_radius, y, x+outer_radius, y, r, g, b, a);
            
            LOOP_UNTEXTURED_SEGMENTS();  // back to the beginning
        }
    }
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
    
    if(radius > (x2-x1)/2)
        radius = (x2-x1)/2;
    if(radius > (y2-y1)/2)
		radius = (y2 - y1) / 2;

	{
		float tau = 2 * PI;

		int verts_per_corner = 7;
		float corner_angle_increment = (tau / 4) / (verts_per_corner - 1);  // 0, 15, 30, 45, 60, 75, 90

		// Starting angle
		float angle = tau*0.75f;
		int last_index = 2;
		int i;

		BEGIN_UNTEXTURED("GPU_RectangleRoundFilled", GL_TRIANGLES, 6 + 4 * (verts_per_corner - 1) - 1, 15 + 4 * (verts_per_corner - 1) * 3 - 3);


		// First triangle
		SET_UNTEXTURED_VERTEX((x2 + x1) / 2, (y2 + y1) / 2, r, g, b, a);  // Center
		SET_UNTEXTURED_VERTEX(x2 - radius + cosf(angle)*radius, y1 + radius + sinf(angle)*radius, r, g, b, a);
		angle += corner_angle_increment;
		SET_UNTEXTURED_VERTEX(x2 - radius + cosf(angle)*radius, y1 + radius + sinf(angle)*radius, r, g, b, a);
		angle += corner_angle_increment;

		for (i = 2; i < verts_per_corner; i++)
		{
			SET_INDEXED_VERTEX(0);
			SET_INDEXED_VERTEX(last_index++);
			SET_UNTEXTURED_VERTEX(x2 - radius + cosf(angle)*radius, y1 + radius + sinf(angle)*radius, r, g, b, a);
			angle += corner_angle_increment;
		}

		SET_INDEXED_VERTEX(0);
		SET_INDEXED_VERTEX(last_index++);
		SET_UNTEXTURED_VERTEX(x2 - radius + cosf(angle)*radius, y2 - radius + sinf(angle)*radius, r, g, b, a);
		for (i = 1; i < verts_per_corner; i++)
		{
			SET_INDEXED_VERTEX(0);
			SET_INDEXED_VERTEX(last_index++);
			SET_UNTEXTURED_VERTEX(x2 - radius + cosf(angle)*radius, y2 - radius + sinf(angle)*radius, r, g, b, a);
			angle += corner_angle_increment;
		}

		SET_INDEXED_VERTEX(0);
		SET_INDEXED_VERTEX(last_index++);
		SET_UNTEXTURED_VERTEX(x1 + radius + cosf(angle)*radius, y2 - radius + sinf(angle)*radius, r, g, b, a);
		for (i = 1; i < verts_per_corner; i++)
		{
			SET_INDEXED_VERTEX(0);
			SET_INDEXED_VERTEX(last_index++);
			SET_UNTEXTURED_VERTEX(x1 + radius + cosf(angle)*radius, y2 - radius + sinf(angle)*radius, r, g, b, a);
			angle += corner_angle_increment;
		}

		SET_INDEXED_VERTEX(0);
		SET_INDEXED_VERTEX(last_index++);
		SET_UNTEXTURED_VERTEX(x1 + radius + cosf(angle)*radius, y1 + radius + sinf(angle)*radius, r, g, b, a);
		for (i = 1; i < verts_per_corner; i++)
		{
			SET_INDEXED_VERTEX(0);
			SET_INDEXED_VERTEX(last_index++);
			SET_UNTEXTURED_VERTEX(x1 + radius + cosf(angle)*radius, y1 + radius + sinf(angle)*radius, r, g, b, a);
			angle += corner_angle_increment;
		}

		// Last triangle
		SET_INDEXED_VERTEX(0);
		SET_INDEXED_VERTEX(last_index++);
		SET_INDEXED_VERTEX(1);
	}
}

static void Polygon(GPU_Renderer* renderer, GPU_Target* target, unsigned int num_vertices, float* vertices, SDL_Color color)
{
    if(num_vertices < 3)
		return;

	{
		int numSegments = 2 * num_vertices;
		int last_index = 0;
		int i;

		BEGIN_UNTEXTURED("GPU_Polygon", GL_LINES, num_vertices, numSegments);

		SET_UNTEXTURED_VERTEX(vertices[0], vertices[1], r, g, b, a);
		for (i = 2; i < numSegments; i += 2)
		{
			SET_UNTEXTURED_VERTEX(vertices[i], vertices[i + 1], r, g, b, a);
			last_index++;
			SET_INDEXED_VERTEX(last_index);  // Double the last one for the next line
		}
		SET_INDEXED_VERTEX(0);
	}
}

static void Polyline(GPU_Renderer* renderer, GPU_Target* target, unsigned int num_vertices, float* vertices, SDL_Color color, GPU_bool close_loop)
{
	if (num_vertices < 2) return;
	
	float t = GetLineThickness(renderer) * 0.5f;
	float x1, x2, y1, y2, line_angle, tc, ts;
	
	int num_v = num_vertices * 4;
	int num_i = num_v + 2;
	int last_vert = num_vertices;
	
	if ( !close_loop )
	{
		num_v -= 4;
		num_i = num_v;
		last_vert--;
	}
	
	BEGIN_UNTEXTURED("GPU_Polygon", GL_TRIANGLE_STRIP, num_v, num_i );
	
	int i = 0;
	do
	{
		x1 = vertices[ i * 2 ];
		y1 = vertices[ i * 2 + 1 ];
		i++;
		if ( i == (int)num_vertices )
		{
			x2 = vertices[ 0 ];
			y2 = vertices[ 1 ];
		}
		else
		{
			x2 = vertices[ i * 2 ];
			y2 = vertices[ i * 2 + 1 ];
		}
		
		line_angle = atan2f(y2 - y1, x2 - x1);
		tc = t * cosf(line_angle);
		ts = t * sinf(line_angle);
		
		SET_UNTEXTURED_VERTEX(x1 + ts, y1 - tc, r, g, b, a);
		SET_UNTEXTURED_VERTEX(x1 - ts, y1 + tc, r, g, b, a);
		SET_UNTEXTURED_VERTEX(x2 + ts, y2 - tc, r, g, b, a);
		SET_UNTEXTURED_VERTEX(x2 - ts, y2 + tc, r, g, b, a);
		
	} while ( i < last_vert );
	
	if ( close_loop ) // end cap for closed
	{
		SET_INDEXED_VERTEX(0);
		SET_INDEXED_VERTEX(1)
	}
	
}

static void PolygonFilled(GPU_Renderer* renderer, GPU_Target* target, unsigned int num_vertices, float* vertices, SDL_Color color)
{
    if(num_vertices < 3)
		return;

	{
		int numSegments = 2 * num_vertices;

		// Using a fan of triangles assumes that the polygon is convex
		BEGIN_UNTEXTURED("GPU_PolygonFilled", GL_TRIANGLES, num_vertices, 3 + (num_vertices - 3) * 3);

		// First triangle
		SET_UNTEXTURED_VERTEX(vertices[0], vertices[1], r, g, b, a);
		SET_UNTEXTURED_VERTEX(vertices[2], vertices[3], r, g, b, a);
		SET_UNTEXTURED_VERTEX(vertices[4], vertices[5], r, g, b, a);

		if (num_vertices > 3)
		{
			int last_index = 2;

			int i;
			for (i = 6; i < numSegments; i += 2)
			{
				SET_INDEXED_VERTEX(0);  // Start from the first vertex
				SET_INDEXED_VERTEX(last_index);  // Double the last one
				SET_UNTEXTURED_VERTEX(vertices[i], vertices[i + 1], r, g, b, a);
				last_index++;
			}
		}
	}
}

