#include "SDL.h"
#include "SDL_gpu.h"
#include "common.h"
#include <stdlib.h>


typedef Uint32 BlitFlagEnum;
static const BlitFlagEnum PASSTHROUGH_VERTICES = 0x1;
static const BlitFlagEnum PASSTHROUGH_TEXCOORDS = 0x2;
static const BlitFlagEnum PASSTHROUGH_COLORS = 0x4;
static const BlitFlagEnum USE_DEFAULT_POSITIONS = 0x8;
static const BlitFlagEnum USE_DEFAULT_SRC_RECTS = 0x10;
static const BlitFlagEnum USE_DEFAULT_COLORS = 0x20;

#define PASSTHROUGH_ALL (PASSTHROUGH_VERTICES | PASSTHROUGH_TEXCOORDS | PASSTHROUGH_COLORS)

void BlitBatch(GPU_Image* image, GPU_Target* target, unsigned int num_sprites, float* values, BlitFlagEnum flags)
{
	int src_position_floats_per_sprite;
	int src_rect_floats_per_sprite;
	int src_color_floats_per_sprite;

	Uint8 no_positions;
	Uint8 no_rects;
	Uint8 no_colors;
	Uint8 pass_vertices;
	Uint8 pass_texcoords;
	Uint8 pass_colors;

	int src_floats_per_sprite;

	int size;
	float* new_values;
	unsigned short* indices;
	unsigned int num_indices;
	
	unsigned int current_index;

	unsigned int n;  // The sprite number iteration variable.
	// Source indices (per sprite)
	int pos_n;
	int rect_n;
	int color_n;
	// Dest indices
	int vert_i;
	int texcoord_i;
	int color_i;
	// Dest float stride
	int floats_per_vertex;

	float w2;  // texcoord helpers for position expansion
	float h2;

	Uint32 tex_w;
	Uint32 tex_h;
    
	if(image == NULL)
        return;
	if(target == NULL)
        return;
    
    if(num_sprites == 0)
        return;
	
	// Conversion time...
	
	// Convert condensed interleaved format into full interleaved format for the renderer to use.
	// Condensed: Each vertex has 2 pos, 4 rect, 4 color
	
	// Default values: Each sprite is defined by a position, a rect, and a color.
	src_position_floats_per_sprite = 2;
	src_rect_floats_per_sprite = 4;
	src_color_floats_per_sprite = 4;
	
	no_positions = (Uint8)(flags & USE_DEFAULT_POSITIONS) || (values == NULL);
	no_rects = (Uint8)(flags & USE_DEFAULT_SRC_RECTS) || (values == NULL);
	no_colors = (Uint8)(flags & USE_DEFAULT_COLORS) || (values == NULL);
	pass_vertices = (Uint8)(flags & PASSTHROUGH_VERTICES);
	pass_texcoords = (Uint8)(flags & PASSTHROUGH_TEXCOORDS);
	pass_colors = (Uint8)(flags & PASSTHROUGH_COLORS);
	
	// Passthrough data is per-vertex.  Non-passthrough is per-sprite.  They can't interleave cleanly.
	if(flags & PASSTHROUGH_ALL && (flags & PASSTHROUGH_ALL) != PASSTHROUGH_ALL)
    {
        GPU_PushErrorCode(__func__, GPU_ERROR_USER_ERROR, "Cannot interpret interleaved data using partial passthrough");
        return;
    }
	
	if(pass_vertices)
        src_position_floats_per_sprite = 8; // 4 vertices of x, y
	if(pass_texcoords)
        src_rect_floats_per_sprite = 8; // 4 vertices of s, t
	if(pass_colors)
        src_color_floats_per_sprite = 16; // 4 vertices of r, g, b, a
	if(no_positions)
        src_position_floats_per_sprite = 0;
	if(no_rects)
        src_rect_floats_per_sprite = 0;
	if(no_colors)
        src_color_floats_per_sprite = 0;
    
	src_floats_per_sprite = src_position_floats_per_sprite + src_rect_floats_per_sprite + src_color_floats_per_sprite;
	
	size = num_sprites*(8 + 8 + 16);
	new_values = (float*)malloc(sizeof(float)*size);
	num_indices = num_sprites * 6;
	indices = (unsigned short*)malloc(sizeof(unsigned short)*num_indices);
	current_index = 0;
    
	// Source indices (per sprite)
	pos_n = 0;
	rect_n = src_position_floats_per_sprite;
	color_n = src_position_floats_per_sprite + src_rect_floats_per_sprite;
	// Dest indices
	vert_i = 0;
	texcoord_i = 2;
	color_i = 4;
	// Dest float stride
	floats_per_vertex = 8;
	
	w2 = 0.5f*image->w;  // texcoord helpers for position expansion
	h2 = 0.5f*image->h;
	
	tex_w = image->texture_w;
	tex_h = image->texture_h;
	
    for(n = 0; n < num_sprites; n++)
    {
        int i_n = n*6;
        indices[i_n] = current_index;
        indices[i_n+1] = current_index+1;
        indices[i_n+2] = current_index+2;
        indices[i_n+3] = current_index+2;
        indices[i_n+4] = current_index+3;
        indices[i_n+5] = current_index;
        current_index += 4;
        
        if(no_rects)
        {
            new_values[texcoord_i] = 0.0f;
            new_values[texcoord_i+1] = 0.0f;
            texcoord_i += floats_per_vertex;
            new_values[texcoord_i] = 1.0f;
            new_values[texcoord_i+1] = 0.0f;
            texcoord_i += floats_per_vertex;
            new_values[texcoord_i] = 1.0f;
            new_values[texcoord_i+1] = 1.0f;
            texcoord_i += floats_per_vertex;
            new_values[texcoord_i] = 0.0f;
            new_values[texcoord_i+1] = 1.0f;
            texcoord_i += floats_per_vertex;
        }
        else
        {
            if(!pass_texcoords)
            {
                float s1 = values[rect_n]/tex_w;
                float t1 = values[rect_n+1]/tex_h;
                float s3 = s1 + values[rect_n+2]/tex_w;
                float t3 = t1 + values[rect_n+3]/tex_h;
                rect_n += src_floats_per_sprite;
                
                new_values[texcoord_i] = s1;
                new_values[texcoord_i+1] = t1;
                texcoord_i += floats_per_vertex;
                new_values[texcoord_i] = s3;
                new_values[texcoord_i+1] = t1;
                texcoord_i += floats_per_vertex;
                new_values[texcoord_i] = s3;
                new_values[texcoord_i+1] = t3;
                texcoord_i += floats_per_vertex;
                new_values[texcoord_i] = s1;
                new_values[texcoord_i+1] = t3;
                texcoord_i += floats_per_vertex;
            
                if(!pass_vertices)
                {
                    w2 = 0.5f*(s3-s1)*image->w;
                    h2 = 0.5f*(t3-t1)*image->h;
                }
            }
            else
            {
                // 4 vertices all in a row
				float s1, t1, s3, t3;
                s1 = new_values[texcoord_i] = values[rect_n];
                t1 = new_values[texcoord_i+1] = values[rect_n+1];
                texcoord_i += floats_per_vertex;
                new_values[texcoord_i] = values[rect_n+2];
                new_values[texcoord_i+1] = values[rect_n+3];
                texcoord_i += floats_per_vertex;
                s3 = new_values[texcoord_i] = values[rect_n+4];
                t3 = new_values[texcoord_i+1] = values[rect_n+5];
                texcoord_i += floats_per_vertex;
                new_values[texcoord_i] = values[rect_n+6];
                new_values[texcoord_i+1] = values[rect_n+7];
                texcoord_i += floats_per_vertex;
                rect_n += src_floats_per_sprite;
            
                if(!pass_vertices)
                {
                    w2 = 0.5f*(s3-s1)*image->w;
                    h2 = 0.5f*(t3-t1)*image->h;
                }
            }
        }
        
        if(no_positions)
        {
            new_values[vert_i] = 0.0f;
            new_values[vert_i+1] = 0.0f;
            vert_i += floats_per_vertex;
            new_values[vert_i] = 0.0f;
            new_values[vert_i+1] = 0.0f;
            vert_i += floats_per_vertex;
            new_values[vert_i] = 0.0f;
            new_values[vert_i+1] = 0.0f;
            vert_i += floats_per_vertex;
            new_values[vert_i] = 0.0f;
            new_values[vert_i+1] = 0.0f;
            vert_i += floats_per_vertex;
        }
        else
        {
            if(!pass_vertices)
            {
                // Expand vertices from the position and dimensions
                float x = values[pos_n];
                float y = values[pos_n+1];
                pos_n += src_floats_per_sprite;
                
                new_values[vert_i] = x - w2;
                new_values[vert_i+1] = y - h2;
                vert_i += floats_per_vertex;
                new_values[vert_i] = x + w2;
                new_values[vert_i+1] = y - h2;
                vert_i += floats_per_vertex;
                new_values[vert_i] = x + w2;
                new_values[vert_i+1] = y + h2;
                vert_i += floats_per_vertex;
                new_values[vert_i] = x - w2;
                new_values[vert_i+1] = y + h2;
                vert_i += floats_per_vertex;
            }
            else
            {
                // 4 vertices all in a row
                new_values[vert_i] = values[pos_n];
                new_values[vert_i+1] = values[pos_n+1];
                vert_i += floats_per_vertex;
                new_values[vert_i] = values[pos_n+2];
                new_values[vert_i+1] = values[pos_n+3];
                vert_i += floats_per_vertex;
                new_values[vert_i] = values[pos_n+4];
                new_values[vert_i+1] = values[pos_n+5];
                vert_i += floats_per_vertex;
                new_values[vert_i] = values[pos_n+6];
                new_values[vert_i+1] = values[pos_n+7];
                vert_i += floats_per_vertex;
                pos_n += src_floats_per_sprite;
            }
        }
        
        if(no_colors)
        {
                new_values[color_i] = 1.0f;
                new_values[color_i+1] = 1.0f;
                new_values[color_i+2] = 1.0f;
                new_values[color_i+3] = 1.0f;
                color_i += floats_per_vertex;
                new_values[color_i] = 1.0f;
                new_values[color_i+1] = 1.0f;
                new_values[color_i+2] = 1.0f;
                new_values[color_i+3] = 1.0f;
                color_i += floats_per_vertex;
                new_values[color_i] = 1.0f;
                new_values[color_i+1] = 1.0f;
                new_values[color_i+2] = 1.0f;
                new_values[color_i+3] = 1.0f;
                color_i += floats_per_vertex;
                new_values[color_i] = 1.0f;
                new_values[color_i+1] = 1.0f;
                new_values[color_i+2] = 1.0f;
                new_values[color_i+3] = 1.0f;
                color_i += floats_per_vertex;
        }
        else
        {
            if(!pass_colors)
            {
                float r = values[color_n]/255.0f;
                float g = values[color_n+1]/255.0f;
                float b = values[color_n+2]/255.0f;
                float a = values[color_n+3]/255.0f;
                color_n += src_floats_per_sprite;
                
                new_values[color_i] = r;
                new_values[color_i+1] = g;
                new_values[color_i+2] = b;
                new_values[color_i+3] = a;
                color_i += floats_per_vertex;
                new_values[color_i] = r;
                new_values[color_i+1] = g;
                new_values[color_i+2] = b;
                new_values[color_i+3] = a;
                color_i += floats_per_vertex;
                new_values[color_i] = r;
                new_values[color_i+1] = g;
                new_values[color_i+2] = b;
                new_values[color_i+3] = a;
                color_i += floats_per_vertex;
                new_values[color_i] = r;
                new_values[color_i+1] = g;
                new_values[color_i+2] = b;
                new_values[color_i+3] = a;
                color_i += floats_per_vertex;
            }
            else
            {
                // 4 vertices all in a row
                new_values[color_i] = values[color_n];
                new_values[color_i+1] = values[color_n+1];
                new_values[color_i+2] = values[color_n+2];
                new_values[color_i+3] = values[color_n+3];
                color_i += floats_per_vertex;
                new_values[color_i] = values[color_n+4];
                new_values[color_i+1] = values[color_n+5];
                new_values[color_i+2] = values[color_n+6];
                new_values[color_i+3] = values[color_n+7];
                color_i += floats_per_vertex;
                new_values[color_i] = values[color_n+8];
                new_values[color_i+1] = values[color_n+9];
                new_values[color_i+2] = values[color_n+10];
                new_values[color_i+3] = values[color_n+11];
                color_i += floats_per_vertex;
                new_values[color_i] = values[color_n+12];
                new_values[color_i+1] = values[color_n+13];
                new_values[color_i+2] = values[color_n+14];
                new_values[color_i+3] = values[color_n+15];
                color_i += floats_per_vertex;
                color_n += src_floats_per_sprite;
            }
        }
    }
    
	GPU_TriangleBatch(image, target, num_sprites*4, new_values, num_indices, indices, GPU_BATCH_XY_ST_RGBA);
	
	free(indices);
	free(new_values);
}

void BlitBatchSeparate(GPU_Image* image, GPU_Target* target, unsigned int num_sprites, float* positions, float* src_rects, float* colors, BlitFlagEnum flags)
{
	Uint8 pass_vertices;
	Uint8 pass_texcoords;
	Uint8 pass_colors;

	int size;  // 4 vertices of x, y...  s, t...  r, g, b, a
	float* values;
	unsigned short* indices;
	unsigned int num_indices;
	
	unsigned int current_index;

	unsigned int n;  // The sprite number iteration variable.
	// Source indices
	int pos_n;
	int rect_n;
	int color_n;
	// Dest indices
	int vert_i;
	int texcoord_i;
	int color_i;
	// Dest float stride
	int floats_per_vertex;

	float w2;  // texcoord helpers for position expansion
	float h2;

	Uint32 tex_w;
	Uint32 tex_h;
    
	if(image == NULL)
        return;
	if(target == NULL)
        return;
    
    if(num_sprites == 0)
        return;
	
	// Repack the given arrays into an interleaved array for more efficient access
	// Default values: Each sprite is defined by a position, a rect, and a color.
	
	pass_vertices = (Uint8)(flags & PASSTHROUGH_VERTICES);
	pass_texcoords = (Uint8)(flags & PASSTHROUGH_TEXCOORDS);
	pass_colors = (Uint8)(flags & PASSTHROUGH_COLORS);
	
	size = num_sprites*(8 + 8 + 16);  // 4 vertices of x, y...  s, t...  r, g, b, a
	values = (float*)malloc(sizeof(float)*size);
	num_indices = num_sprites * 6;
	indices = (unsigned short*)malloc(sizeof(unsigned short)*num_indices);
	current_index = 0;
	
	// Source indices
	pos_n = 0;
	rect_n = 0;
	color_n = 0;
	// Dest indices
	vert_i = 0;
	texcoord_i = 2;
	color_i = 4;
	// Dest float stride
	floats_per_vertex = 8;
	
	w2 = 0.5f*image->w;  // texcoord helpers for position expansion
	h2 = 0.5f*image->h;
	
	tex_w = image->texture_w;
	tex_h = image->texture_h;
    
	for(n = 0; n < num_sprites; n++)
    {
        int i_n = n*6;
        indices[i_n] = current_index;
        indices[i_n+1] = current_index+1;
        indices[i_n+2] = current_index+2;
        indices[i_n+3] = current_index+2;
        indices[i_n+4] = current_index+3;
        indices[i_n+5] = current_index;
        current_index += 4;
        
        // Unpack the arrays
        
        if(src_rects == NULL)
        {
            values[texcoord_i] = 0.0f;
            values[texcoord_i+1] = 0.0f;
            texcoord_i += floats_per_vertex;
            values[texcoord_i] = 1.0f;
            values[texcoord_i+1] = 0.0f;
            texcoord_i += floats_per_vertex;
            values[texcoord_i] = 1.0f;
            values[texcoord_i+1] = 1.0f;
            texcoord_i += floats_per_vertex;
            values[texcoord_i] = 0.0f;
            values[texcoord_i+1] = 1.0f;
            texcoord_i += floats_per_vertex;
        }
        else
        {
            if(!pass_texcoords)
            {
                float s1 = src_rects[rect_n++]/tex_w;
                float t1 = src_rects[rect_n++]/tex_h;
                float s3 = s1 + src_rects[rect_n++]/tex_w;
                float t3 = t1 + src_rects[rect_n++]/tex_h;
                
                values[texcoord_i] = s1;
                values[texcoord_i+1] = t1;
                texcoord_i += floats_per_vertex;
                values[texcoord_i] = s3;
                values[texcoord_i+1] = t1;
                texcoord_i += floats_per_vertex;
                values[texcoord_i] = s3;
                values[texcoord_i+1] = t3;
                texcoord_i += floats_per_vertex;
                values[texcoord_i] = s1;
                values[texcoord_i+1] = t3;
                texcoord_i += floats_per_vertex;
            
                if(!pass_vertices)
                {
                    w2 = 0.5f*(s3-s1)*image->w;
                    h2 = 0.5f*(t3-t1)*image->h;
                }
            }
            else
            {
                // 4 vertices all in a row
				float s1, t1, s3, t3;
                s1 = values[texcoord_i] = src_rects[rect_n++];
                t1 = values[texcoord_i+1] = src_rects[rect_n++];
                texcoord_i += floats_per_vertex;
                values[texcoord_i] = src_rects[rect_n++];
                values[texcoord_i+1] = src_rects[rect_n++];
                texcoord_i += floats_per_vertex;
                s3 = values[texcoord_i] = src_rects[rect_n++];
                t3 = values[texcoord_i+1] = src_rects[rect_n++];
                texcoord_i += floats_per_vertex;
                values[texcoord_i] = src_rects[rect_n++];
                values[texcoord_i+1] = src_rects[rect_n++];
                texcoord_i += floats_per_vertex;
            
                if(!pass_vertices)
                {
                    w2 = 0.5f*(s3-s1)*image->w;
                    h2 = 0.5f*(t3-t1)*image->h;
                }
            }
        }
        
        if(positions == NULL)
        {
            values[vert_i] = 0.0f;
            values[vert_i+1] = 0.0f;
            vert_i += floats_per_vertex;
            values[vert_i] = 0.0f;
            values[vert_i+1] = 0.0f;
            vert_i += floats_per_vertex;
            values[vert_i] = 0.0f;
            values[vert_i+1] = 0.0f;
            vert_i += floats_per_vertex;
            values[vert_i] = 0.0f;
            values[vert_i+1] = 0.0f;
            vert_i += floats_per_vertex;
        }
        else
        {
            if(!pass_vertices)
            {
                // Expand vertices from the position and dimensions
                float x = positions[pos_n++];
                float y = positions[pos_n++];
                values[vert_i] = x - w2;
                values[vert_i+1] = y - h2;
                vert_i += floats_per_vertex;
                values[vert_i] = x + w2;
                values[vert_i+1] = y - h2;
                vert_i += floats_per_vertex;
                values[vert_i] = x + w2;
                values[vert_i+1] = y + h2;
                vert_i += floats_per_vertex;
                values[vert_i] = x - w2;
                values[vert_i+1] = y + h2;
                vert_i += floats_per_vertex;
            }
            else
            {
                // 4 vertices all in a row
                values[vert_i] = positions[pos_n++];
                values[vert_i+1] = positions[pos_n++];
                vert_i += floats_per_vertex;
                values[vert_i] = positions[pos_n++];
                values[vert_i+1] = positions[pos_n++];
                vert_i += floats_per_vertex;
                values[vert_i] = positions[pos_n++];
                values[vert_i+1] = positions[pos_n++];
                vert_i += floats_per_vertex;
                values[vert_i] = positions[pos_n++];
                values[vert_i+1] = positions[pos_n++];
                vert_i += floats_per_vertex;
            }
        }
        
        if(colors == NULL)
        {
                values[color_i] = 1.0f;
                values[color_i+1] = 1.0f;
                values[color_i+2] = 1.0f;
                values[color_i+3] = 1.0f;
                color_i += floats_per_vertex;
                values[color_i] = 1.0f;
                values[color_i+1] = 1.0f;
                values[color_i+2] = 1.0f;
                values[color_i+3] = 1.0f;
                color_i += floats_per_vertex;
                values[color_i] = 1.0f;
                values[color_i+1] = 1.0f;
                values[color_i+2] = 1.0f;
                values[color_i+3] = 1.0f;
                color_i += floats_per_vertex;
                values[color_i] = 1.0f;
                values[color_i+1] = 1.0f;
                values[color_i+2] = 1.0f;
                values[color_i+3] = 1.0f;
                color_i += floats_per_vertex;
        }
        else
        {
            if(!pass_colors)
            {
                float r = colors[color_n++]/255.0f;
                float g = colors[color_n++]/255.0f;
                float b = colors[color_n++]/255.0f;
                float a = colors[color_n++]/255.0f;
                
                values[color_i] = r;
                values[color_i+1] = g;
                values[color_i+2] = b;
                values[color_i+3] = a;
                color_i += floats_per_vertex;
                values[color_i] = r;
                values[color_i+1] = g;
                values[color_i+2] = b;
                values[color_i+3] = a;
                color_i += floats_per_vertex;
                values[color_i] = r;
                values[color_i+1] = g;
                values[color_i+2] = b;
                values[color_i+3] = a;
                color_i += floats_per_vertex;
                values[color_i] = r;
                values[color_i+1] = g;
                values[color_i+2] = b;
                values[color_i+3] = a;
                color_i += floats_per_vertex;
            }
            else
            {
                // 4 vertices all in a row
                values[color_i] = colors[color_n++];
                values[color_i+1] = colors[color_n++];
                values[color_i+2] = colors[color_n++];
                values[color_i+3] = colors[color_n++];
                color_i += floats_per_vertex;
                values[color_i] = colors[color_n++];
                values[color_i+1] = colors[color_n++];
                values[color_i+2] = colors[color_n++];
                values[color_i+3] = colors[color_n++];
                color_i += floats_per_vertex;
                values[color_i] = colors[color_n++];
                values[color_i+1] = colors[color_n++];
                values[color_i+2] = colors[color_n++];
                values[color_i+3] = colors[color_n++];
                color_i += floats_per_vertex;
                values[color_i] = colors[color_n++];
                values[color_i+1] = colors[color_n++];
                values[color_i+2] = colors[color_n++];
                values[color_i+3] = colors[color_n++];
                color_i += floats_per_vertex;
            }
        }
    }
	
	GPU_TriangleBatch(image, target, num_sprites*4, values, num_indices, indices, GPU_BATCH_XY_ST_RGBA);
	
	free(indices);
	free(values);
}

int do_interleaved(GPU_Target* screen)
{
    GPU_Image* image;
	int return_value;
	float dt;
	Uint32 startTime;
	long frameCount;
	int maxSprites;
	int numSprites;
	int floats_per_sprite;
	float* sprite_values;
	float* velx;
	float* vely;
	int i;
    int val_n;
	Uint8 done;
	SDL_Event event;
    
	GPU_LogError("do_interleaved()\n");
	image = GPU_LoadImage("data/small_test.png");
	if(image == NULL)
		return -1;
	
	return_value = 0;
	
	dt = 0.010f;
	
	startTime = SDL_GetTicks();
	frameCount = 0;
	
	maxSprites = 50000;
	numSprites = 101;
	
	floats_per_sprite = 2 + 4 + 4;
	sprite_values = (float*)malloc(sizeof(float)*maxSprites*floats_per_sprite);
	velx = (float*)malloc(sizeof(float)*maxSprites);
	vely = (float*)malloc(sizeof(float)*maxSprites);
    val_n = 0;
	for(i = 0; i < maxSprites; i++)
	{
		sprite_values[val_n++] = rand()%screen->w;
		sprite_values[val_n++] = rand()%screen->h;
		sprite_values[val_n++] = 0;
		sprite_values[val_n++] = 0;
		sprite_values[val_n++] = image->w;
		sprite_values[val_n++] = image->h;
		sprite_values[val_n++] = rand()%256;
		sprite_values[val_n++] = rand()%256;
		sprite_values[val_n++] = rand()%256;
		sprite_values[val_n++] = rand()%256;
		velx[i] = 10 + rand()%screen->w/10;
		vely[i] = 10 + rand()%screen->h/10;
		if(rand()%2)
            velx[i] = -velx[i];
		if(rand()%2)
            vely[i] = -vely[i];
	}
	
	
	done = 0;
	while(!done)
	{
		while(SDL_PollEvent(&event))
		{
			if(event.type == SDL_QUIT)
				done = 1;
			else if(event.type == SDL_KEYDOWN)
			{
				if(event.key.keysym.sym == SDLK_ESCAPE)
					done = 1;
				else if(event.key.keysym.sym == SDLK_SPACE)
                {
					done = 1;
					return_value = 2;
                }
				else if(event.key.keysym.sym == SDLK_EQUALS || event.key.keysym.sym == SDLK_PLUS)
				{
					if(numSprites < maxSprites)
						numSprites += 100;
                    GPU_LogError("Sprites: %d\n", numSprites);
                    frameCount = 0;
                    startTime = SDL_GetTicks();
				}
				else if(event.key.keysym.sym == SDLK_MINUS)
				{
					if(numSprites > 1)
						numSprites -= 100;
					if(numSprites < 1)
                        numSprites = 1;
                    GPU_LogError("Sprites: %d\n", numSprites);
                    frameCount = 0;
                    startTime = SDL_GetTicks();
				}
			}
		}
		
		for(i = 0; i < numSprites; i++)
		{
		    val_n = floats_per_sprite*i;
			sprite_values[val_n] += velx[i]*dt;
			sprite_values[val_n+1] += vely[i]*dt;
			if(sprite_values[val_n] < 0)
			{
				sprite_values[val_n] = 0;
				velx[i] = -velx[i];
			}
			else if(sprite_values[val_n] > screen->w)
			{
				sprite_values[val_n] = screen->w;
				velx[i] = -velx[i];
			}
			
			if(sprite_values[val_n+1] < 0)
			{
				sprite_values[val_n+1] = 0;
				vely[i] = -vely[i];
			}
			else if(sprite_values[val_n+1] > screen->h)
			{
				sprite_values[val_n+1] = screen->h;
				vely[i] = -vely[i];
			}
		}
		
		GPU_Clear(screen);
		
        BlitBatch(image, screen, numSprites, sprite_values, 0);
		
		GPU_Flip(screen);
		
		frameCount++;
		if(SDL_GetTicks() - startTime > 5000)
        {
			printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
			frameCount = 0;
			startTime = SDL_GetTicks();
        }
	}
	
	printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	
	free(sprite_values);
	free(velx);
	free(vely);
	
	GPU_FreeImage(image);
	
	return return_value;
}

int do_separate(GPU_Target* screen)
{
    GPU_Image* image;
	int return_value;
	float dt;
	Uint32 startTime;
	long frameCount;
	int maxSprites;
	int numSprites;
	float* positions;
	float* colors;
	float* velx;
	float* vely;
	int i;
    int val_n;
	Uint8 done;
	SDL_Event event;
    
	GPU_LogError("do_separate()\n");
	image = GPU_LoadImage("data/small_test.png");
	if(image == NULL)
		return -1;
	
	return_value = 0;
	
	dt = 0.010f;
	
	startTime = SDL_GetTicks();
	frameCount = 0;
	
	maxSprites = 50000;
	numSprites = 101;
	
	positions = (float*)malloc(sizeof(float)*maxSprites*2);
	colors = (float*)malloc(sizeof(float)*maxSprites*4);
	velx = (float*)malloc(sizeof(float)*maxSprites);
	vely = (float*)malloc(sizeof(float)*maxSprites);
    val_n = 0;
	for(i = 0; i < maxSprites; i++)
	{
		positions[val_n++] = rand()%screen->w;
		positions[val_n++] = rand()%screen->h;
		colors[i*4] = rand()%256;
		colors[i*4+1] = rand()%256;
		colors[i*4+2] = rand()%256;
		colors[i*4+3] = rand()%256;
		velx[i] = 10 + rand()%screen->w/10;
		vely[i] = 10 + rand()%screen->h/10;
		if(rand()%2)
            velx[i] = -velx[i];
		if(rand()%2)
            vely[i] = -vely[i];
	}
	
	
	done = 0;
	while(!done)
	{
		while(SDL_PollEvent(&event))
		{
			if(event.type == SDL_QUIT)
				done = 1;
			else if(event.type == SDL_KEYDOWN)
			{
				if(event.key.keysym.sym == SDLK_ESCAPE)
					done = 1;
				else if(event.key.keysym.sym == SDLK_SPACE)
                {
					done = 1;
					return_value = 3;
                }
				else if(event.key.keysym.sym == SDLK_EQUALS || event.key.keysym.sym == SDLK_PLUS)
				{
					if(numSprites < maxSprites)
						numSprites += 100;
                    GPU_LogError("Sprites: %d\n", numSprites);
                    frameCount = 0;
                    startTime = SDL_GetTicks();
				}
				else if(event.key.keysym.sym == SDLK_MINUS)
				{
					if(numSprites > 1)
						numSprites -= 100;
					if(numSprites < 1)
                        numSprites = 1;
                    GPU_LogError("Sprites: %d\n", numSprites);
                    frameCount = 0;
                    startTime = SDL_GetTicks();
				}
			}
		}
		
		for(i = 0; i < numSprites; i++)
		{
		    val_n = 2*i;
			positions[val_n] += velx[i]*dt;
			positions[val_n+1] += vely[i]*dt;
			if(positions[val_n] < 0)
			{
				positions[val_n] = 0;
				velx[i] = -velx[i];
			}
			else if(positions[val_n] > screen->w)
			{
				positions[val_n] = screen->w;
				velx[i] = -velx[i];
			}
			
			if(positions[val_n+1] < 0)
			{
				positions[val_n+1] = 0;
				vely[i] = -vely[i];
			}
			else if(positions[val_n+1] > screen->h)
			{
				positions[val_n+1] = screen->h;
				vely[i] = -vely[i];
			}
		}
		
		GPU_Clear(screen);
		
        BlitBatchSeparate(image, screen, numSprites, positions, NULL, colors, 0);
		
		GPU_Flip(screen);
		
		frameCount++;
		if(SDL_GetTicks() - startTime > 5000)
        {
			printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
			frameCount = 0;
			startTime = SDL_GetTicks();
        }
	}
	
	printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	
	free(positions);
	free(colors);
	free(velx);
	free(vely);
	
	GPU_FreeImage(image);
	
	return return_value;
}




int do_attributes(GPU_Target* screen)
{
    GPU_Image* image;
	int return_value;
	
	float dt;
	
	Uint32 startTime;
	long frameCount;
	
	int maxSprites;
	int numSprites;
	
	int floats_per_vertex;
	int floats_per_sprite;
	float* sprite_values;
	
	Uint32 program_object;
    
	GPU_LogError("do_attributes()\n");
	image = GPU_LoadImage("data/small_test.png");
	if(image == NULL)
		return -1;
	
	return_value = 0;
	
	dt = 0.010f;
	
	startTime = SDL_GetTicks();
	frameCount = 0;
	
	maxSprites = 50000;
	numSprites = 101;
	
	// 2 pos floats per vertex, 2 texcoords, 4 color components
	floats_per_vertex = 2 + 2 + 4;
	floats_per_sprite = floats_per_vertex*4;
	sprite_values = (float*)malloc(sizeof(float)*maxSprites*floats_per_sprite);
	
	// Load attributes for the textured shader
	program_object = 0;
	GPU_ActivateShaderProgram(program_object, NULL);
	// Disable the default shader's attributes (not a typical thing to do...)
	{
        GPU_ShaderBlock block = {-1,-1,-1,GPU_GetUniformLocation(program_object, "gpu_ModelViewProjectionMatrix")};
        GPU_ActivateShaderProgram(program_object, &block);
	}
	
	{
		GPU_Attribute attributes[3];
        
        float* velx = (float*)malloc(sizeof(float)*maxSprites);
        float* vely = (float*)malloc(sizeof(float)*maxSprites);
        int i;
        int val_n = 0;
        Uint8 done;
		SDL_Event event;

		attributes[0] = GPU_MakeAttribute(GPU_GetAttributeLocation(program_object, "gpu_Vertex"), sprite_values,
			GPU_MakeAttributeFormat(2, GPU_TYPE_FLOAT, 0, floats_per_vertex*sizeof(float), 0));
		attributes[1] = GPU_MakeAttribute(GPU_GetAttributeLocation(program_object, "gpu_TexCoord"), sprite_values,
			GPU_MakeAttributeFormat(2, GPU_TYPE_FLOAT, 0, floats_per_vertex*sizeof(float), 2 * sizeof(float)));
		attributes[2] = GPU_MakeAttribute(GPU_GetAttributeLocation(program_object, "gpu_Color"), sprite_values,
			GPU_MakeAttributeFormat(4, GPU_TYPE_FLOAT, 0, floats_per_vertex*sizeof(float), 4 * sizeof(float)));
        
        for(i = 0; i < maxSprites; i++)
        {
            float x = rand()%screen->w;
            float y = rand()%screen->h;
            sprite_values[val_n++] = x - image->w/2;
            sprite_values[val_n++] = y - image->h/2;
            
            sprite_values[val_n++] = 0;
            sprite_values[val_n++] = 0;
            
            sprite_values[val_n++] = rand()%101/100.0f;
            sprite_values[val_n++] = rand()%101/100.0f;
            sprite_values[val_n++] = rand()%101/100.0f;
            sprite_values[val_n++] = rand()%101/100.0f;
            
            sprite_values[val_n++] = x + image->w/2;
            sprite_values[val_n++] = y - image->h/2;
            
            sprite_values[val_n++] = 1;
            sprite_values[val_n++] = 0;
            
            sprite_values[val_n++] = rand()%101/100.0f;
            sprite_values[val_n++] = rand()%101/100.0f;
            sprite_values[val_n++] = rand()%101/100.0f;
            sprite_values[val_n++] = rand()%101/100.0f;
            
            sprite_values[val_n++] = x + image->w/2;
            sprite_values[val_n++] = y + image->h/2;
            
            sprite_values[val_n++] = 1;
            sprite_values[val_n++] = 1;
            
            sprite_values[val_n++] = rand()%101/100.0f;
            sprite_values[val_n++] = rand()%101/100.0f;
            sprite_values[val_n++] = rand()%101/100.0f;
            sprite_values[val_n++] = rand()%101/100.0f;
            
            sprite_values[val_n++] = x - image->w/2;
            sprite_values[val_n++] = y + image->h/2;
            
            sprite_values[val_n++] = 0;
            sprite_values[val_n++] = 1;
            
            sprite_values[val_n++] = rand()%101/100.0f;
            sprite_values[val_n++] = rand()%101/100.0f;
            sprite_values[val_n++] = rand()%101/100.0f;
            sprite_values[val_n++] = rand()%101/100.0f;
            
            velx[i] = 10 + rand()%screen->w/10;
            vely[i] = 10 + rand()%screen->h/10;
            if(rand()%2)
                velx[i] = -velx[i];
            if(rand()%2)
                vely[i] = -vely[i];
        }
        
        
        done = 0;
        while(!done)
        {
            while(SDL_PollEvent(&event))
            {
                if(event.type == SDL_QUIT)
                    done = 1;
                else if(event.type == SDL_KEYDOWN)
                {
                    if(event.key.keysym.sym == SDLK_ESCAPE)
                        done = 1;
                    else if(event.key.keysym.sym == SDLK_SPACE)
                    {
                        done = 1;
                        return_value = 1;
                    }
                    else if(event.key.keysym.sym == SDLK_EQUALS || event.key.keysym.sym == SDLK_PLUS)
                    {
                        if(numSprites < maxSprites)
                            numSprites += 100;
                        GPU_LogError("Sprites: %d\n", numSprites);
                        frameCount = 0;
                        startTime = SDL_GetTicks();
                    }
                    else if(event.key.keysym.sym == SDLK_MINUS)
                    {
                        if(numSprites > 1)
                            numSprites -= 100;
                        if(numSprites < 1)
                            numSprites = 1;
                        GPU_LogError("Sprites: %d\n", numSprites);
                        frameCount = 0;
                        startTime = SDL_GetTicks();
                    }
                }
            }
            
            GPU_Clear(screen);
            
            for(i = 0; i < numSprites; i++)
            {
				float x, y;
                val_n = floats_per_sprite*i;
                x = sprite_values[val_n] + image->w/2;
                y = sprite_values[val_n+1] + image->h/2;
                
                x += velx[i]*dt;
                y += vely[i]*dt;
                if(x < 0)
                {
                    x = 0;
                    velx[i] = -velx[i];
                }
                else if(x > screen->w)
                {
                    x = screen->w;
                    velx[i] = -velx[i];
                }
                
                if(y < 0)
                {
                    y = 0;
                    vely[i] = -vely[i];
                }
                else if(y > screen->h)
                {
                    y = screen->h;
                    vely[i] = -vely[i];
                }
                
                sprite_values[val_n] = x - image->w/2;
                sprite_values[val_n+1] = y - image->h/2;
                val_n += floats_per_vertex;
                sprite_values[val_n] = x + image->w/2;
                sprite_values[val_n+1] = y - image->h/2;
                val_n += floats_per_vertex;
                sprite_values[val_n] = x + image->w/2;
                sprite_values[val_n+1] = y + image->h/2;
                val_n += floats_per_vertex;
                sprite_values[val_n] = x - image->w/2;
                sprite_values[val_n+1] = y + image->h/2;
            }
            
            //float color[4] = {0.5f, 0, 0, 1.0f};
            //GPU_SetAttributefv(attributes[2].location, 4, color);
            GPU_SetAttributeSource(numSprites*4, attributes[0]);
            GPU_SetAttributeSource(numSprites*4, attributes[1]);
            GPU_SetAttributeSource(numSprites*4, attributes[2]);
            BlitBatch(image, screen, numSprites, NULL, 0);
            
            GPU_Flip(screen);
            
            frameCount++;
            if(SDL_GetTicks() - startTime > 5000)
            {
                printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
                frameCount = 0;
                startTime = SDL_GetTicks();
            }
        }
        
        printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
        
        free(velx);
        free(vely);
	}
	
	free(sprite_values);
	
	GPU_FreeImage(image);
	
	// Reset the default shader's block
	GPU_ActivateShaderProgram(screen->context->default_textured_shader_program, NULL);
	
	return return_value;
}

int main(int argc, char* argv[])
{
    GPU_Target* screen;
    int i;
    
	printRenderers();
	
	screen = GPU_Init(800, 600, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
		return -1;
	
	printCurrentRenderer();
	
	i = 1;
	while(i > 0)
    {
        if(i == 1)
            i = do_interleaved(screen);
        else if(i == 2)
            i = do_separate(screen);
        else if(i == 3)
            i = do_attributes(screen);
        else
            i = 0;
    }
	
	GPU_Quit();
	
	return i;
}


