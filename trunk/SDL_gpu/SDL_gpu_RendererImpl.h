#ifndef _GPU_RENDERERIMPL_H__
#define _GPU_RENDERERIMPL_H__

#include "SDL_gpu.h"

/*! Private implementation of renderer members. */
typedef struct GPU_RendererImpl
{
	/*! \see GPU_Init()
	 *  \see GPU_InitRenderer()
	 *  \see GPU_InitRendererByID()
	 */
	GPU_Target* (*Init)(GPU_Renderer* renderer, GPU_RendererID renderer_request, Uint16 w, Uint16 h, GPU_WindowFlagEnum SDL_flags);
	
	/*! \see GPU_IsFeatureEnabled() */
	Uint8 (*IsFeatureEnabled)(GPU_Renderer* renderer, GPU_FeatureEnum feature);
	
    /*! \see GPU_CreateTargetFromWindow
     * The extra parameter is used internally to reuse/reinit a target. */
    GPU_Target* (*CreateTargetFromWindow)(GPU_Renderer* renderer, Uint32 windowID, GPU_Target* target);
    
    /*! \see GPU_CreateAliasTarget() */
    GPU_Target* (*CreateAliasTarget)(GPU_Renderer* renderer, GPU_Target* target);

    /*! \see GPU_MakeCurrent */
    void (*MakeCurrent)(GPU_Renderer* renderer, GPU_Target* target, Uint32 windowID);
	
	/*! Sets up this renderer to act as the current renderer.  Called automatically by GPU_SetCurrentRenderer(). */
	void (*SetAsCurrent)(GPU_Renderer* renderer);
	
	/*! \see GPU_SetWindowResolution() */
	Uint8 (*SetWindowResolution)(GPU_Renderer* renderer, Uint16 w, Uint16 h);
	
	/*! \see GPU_SetVirtualResolution() */
	void (*SetVirtualResolution)(GPU_Renderer* renderer, GPU_Target* target, Uint16 w, Uint16 h);
	
	/*! \see GPU_UnsetVirtualResolution() */
	void (*UnsetVirtualResolution)(GPU_Renderer* renderer, GPU_Target* target);
	
	/*! Clean up the renderer state. */
	void (*Quit)(GPU_Renderer* renderer);
	
	/*! \see GPU_SetFullscreen() */
	Uint8 (*SetFullscreen)(GPU_Renderer* renderer, Uint8 enable_fullscreen, Uint8 use_desktop_resolution);

	/*! \see GPU_SetCamera() */
	GPU_Camera (*SetCamera)(GPU_Renderer* renderer, GPU_Target* target, GPU_Camera* cam);
	
    /*! \see GPU_CreateImage() */
	GPU_Image* (*CreateImage)(GPU_Renderer* renderer, Uint16 w, Uint16 h, GPU_FormatEnum format);
	
	/*! \see GPU_LoadImage() */
	GPU_Image* (*LoadImage)(GPU_Renderer* renderer, const char* filename);
	
    /*! \see GPU_CreateAliasImage() */
	GPU_Image* (*CreateAliasImage)(GPU_Renderer* renderer, GPU_Image* image);
	
	/*! \see GPU_SaveImage() */
	Uint8 (*SaveImage)(GPU_Renderer* renderer, GPU_Image* image, const char* filename);
	
	/*! \see GPU_CopyImage() */
	GPU_Image* (*CopyImage)(GPU_Renderer* renderer, GPU_Image* image);
	
	/*! \see GPU_UpdateImage */
	void (*UpdateImage)(GPU_Renderer* renderer, GPU_Image* image, SDL_Surface* surface, const GPU_Rect* surface_rect);
	
	/*! \see GPU_UpdateSubImage */
	void (*UpdateSubImage)(GPU_Renderer* renderer, GPU_Image* image, const GPU_Rect* image_rect, SDL_Surface* surface, const GPU_Rect* surface_rect);
	
	/*! \see GPU_UpdateImageBytes */
	void (*UpdateImageBytes)(GPU_Renderer* renderer, GPU_Image* image, const GPU_Rect* image_rect, const unsigned char* bytes, int bytes_per_row);
	
	/*! \see GPU_CopyImageFromSurface() */
	GPU_Image* (*CopyImageFromSurface)(GPU_Renderer* renderer, SDL_Surface* surface);
	
	/*! \see GPU_CopyImageFromTarget() */
	GPU_Image* (*CopyImageFromTarget)(GPU_Renderer* renderer, GPU_Target* target);
	
	/*! \see GPU_CopySurfaceFromTarget() */
	SDL_Surface* (*CopySurfaceFromTarget)(GPU_Renderer* renderer, GPU_Target* target);
	
	/*! \see GPU_CopySurfaceFromImage() */
	SDL_Surface* (*CopySurfaceFromImage)(GPU_Renderer* renderer, GPU_Image* image);
	
	/*! \see GPU_FreeImage() */
	void (*FreeImage)(GPU_Renderer* renderer, GPU_Image* image);
	
	/*! \see GPU_LoadTarget() */
	GPU_Target* (*LoadTarget)(GPU_Renderer* renderer, GPU_Image* image);
	
	/*! \see GPU_FreeTarget() */
	void (*FreeTarget)(GPU_Renderer* renderer, GPU_Target* target);

	/*! \see GPU_Blit() */
	void (*Blit)(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y);
	
	/*! \see GPU_BlitRotate() */
	void (*BlitRotate)(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float degrees);
	
	/*! \see GPU_BlitScale() */
	void (*BlitScale)(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float scaleX, float scaleY);
	
	/*! \see GPU_BlitTransform */
	void (*BlitTransform)(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float degrees, float scaleX, float scaleY);
	
	/*! \see GPU_BlitTransformX() */
	void (*BlitTransformX)(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float pivot_x, float pivot_y, float degrees, float scaleX, float scaleY);
	
	/*! \see GPU_BlitTransformMatrix() */
	void (*BlitTransformMatrix)(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float* matrix3x3);
	
	/*! \see GPU_BlitBatch() */
	void (*BlitBatch)(GPU_Renderer* renderer, GPU_Image* image, GPU_Target* target, unsigned int num_sprites, float* values, GPU_BlitFlagEnum flags);
	
	/*! \see GPU_TriangleBatch() */
	void (*TriangleBatch)(GPU_Renderer* renderer, GPU_Image* image, GPU_Target* target, unsigned short num_vertices, float* values, unsigned int num_indices, unsigned short* indices, GPU_BlitFlagEnum flags);
	
	/*! \see GPU_GenerateMipmaps() */
	void (*GenerateMipmaps)(GPU_Renderer* renderer, GPU_Image* image);

	/*! \see GPU_SetClip() */
	GPU_Rect (*SetClip)(GPU_Renderer* renderer, GPU_Target* target, Sint16 x, Sint16 y, Uint16 w, Uint16 h);

	/*! \see GPU_UnsetClip() */
	void (*UnsetClip)(GPU_Renderer* renderer, GPU_Target* target);
	
	/*! \see GPU_GetPixel() */
	SDL_Color (*GetPixel)(GPU_Renderer* renderer, GPU_Target* target, Sint16 x, Sint16 y);
	
	/*! \see GPU_SetImageFilter() */
	void (*SetImageFilter)(GPU_Renderer* renderer, GPU_Image* image, GPU_FilterEnum filter);
	
	/*! \see GPU_SetWrapMode() */
	void (*SetWrapMode)(GPU_Renderer* renderer, GPU_Image* image, GPU_WrapEnum wrap_mode_x, GPU_WrapEnum wrap_mode_y);

	/*! \see GPU_Clear() */
	void (*Clear)(GPU_Renderer* renderer, GPU_Target* target);
	/*! \see GPU_ClearRGBA() */
	void (*ClearRGBA)(GPU_Renderer* renderer, GPU_Target* target, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
	/*! \see GPU_FlushBlitBuffer() */
	void (*FlushBlitBuffer)(GPU_Renderer* renderer);
	/*! \see GPU_Flip() */
	void (*Flip)(GPU_Renderer* renderer, GPU_Target* target);
	
	
    /*! \see GPU_CompileShader_RW() */
	Uint32 (*CompileShader_RW)(GPU_Renderer* renderer, GPU_ShaderEnum shader_type, SDL_RWops* shader_source);
	
    /*! \see GPU_CompileShader() */
    Uint32 (*CompileShader)(GPU_Renderer* renderer, GPU_ShaderEnum shader_type, const char* shader_source);

    /*! \see GPU_LinkShaderProgram() */
    Uint32 (*LinkShaderProgram)(GPU_Renderer* renderer, Uint32 program_object);

    /*! \see GPU_LinkShaders() */
    Uint32 (*LinkShaders)(GPU_Renderer* renderer, Uint32 shader_object1, Uint32 shader_object2);

    /*! \see GPU_FreeShader() */
    void (*FreeShader)(GPU_Renderer* renderer, Uint32 shader_object);

    /*! \see GPU_FreeShaderProgram() */
    void (*FreeShaderProgram)(GPU_Renderer* renderer, Uint32 program_object);

    /*! \see GPU_AttachShader() */
    void (*AttachShader)(GPU_Renderer* renderer, Uint32 program_object, Uint32 shader_object);

    /*! \see GPU_DetachShader() */
    void (*DetachShader)(GPU_Renderer* renderer, Uint32 program_object, Uint32 shader_object);
    
    /*! \see GPU_IsDefaultShaderProgram() */
    Uint8 (*IsDefaultShaderProgram)(GPU_Renderer* renderer, Uint32 program_object);

    /*! \see GPU_ActivateShaderProgram() */
    void (*ActivateShaderProgram)(GPU_Renderer* renderer, Uint32 program_object, GPU_ShaderBlock* block);

    /*! \see GPU_DeactivateShaderProgram() */
    void (*DeactivateShaderProgram)(GPU_Renderer* renderer);

    /*! \see GPU_GetShaderMessage() */
    const char* (*GetShaderMessage)(GPU_Renderer* renderer);

    /*! \see GPU_GetAttribLocation() */
    int (*GetAttributeLocation)(GPU_Renderer* renderer, Uint32 program_object, const char* attrib_name);

    /*! \see GPU_GetUniformLocation() */
    int (*GetUniformLocation)(GPU_Renderer* renderer, Uint32 program_object, const char* uniform_name);
    
    /*! \see GPU_LoadShaderBlock() */
    GPU_ShaderBlock (*LoadShaderBlock)(GPU_Renderer* renderer, Uint32 program_object, const char* position_name, const char* texcoord_name, const char* color_name, const char* modelViewMatrix_name);
    
    /*! \see GPU_SetShaderBlock() */
    void (*SetShaderBlock)(GPU_Renderer* renderer, GPU_ShaderBlock block);
    
    /*! \see GPU_SetShaderImage() */
    void (*SetShaderImage)(GPU_Renderer* renderer, GPU_Image* image, int location, int image_unit);
    
    /*! \see GPU_GetUniformiv() */
    void (*GetUniformiv)(GPU_Renderer* renderer, Uint32 program_object, int location, int* values);

    /*! \see GPU_SetUniformi() */
    void (*SetUniformi)(GPU_Renderer* renderer, int location, int value);

    /*! \see GPU_SetUniformiv() */
    void (*SetUniformiv)(GPU_Renderer* renderer, int location, int num_elements_per_value, int num_values, int* values);

    /*! \see GPU_GetUniformuiv() */
    void (*GetUniformuiv)(GPU_Renderer* renderer, Uint32 program_object, int location, unsigned int* values);

    /*! \see GPU_SetUniformui() */
    void (*SetUniformui)(GPU_Renderer* renderer, int location, unsigned int value);

    /*! \see GPU_SetUniformuiv() */
    void (*SetUniformuiv)(GPU_Renderer* renderer, int location, int num_elements_per_value, int num_values, unsigned int* values);

    /*! \see GPU_GetUniformfv() */
    void (*GetUniformfv)(GPU_Renderer* renderer, Uint32 program_object, int location, float* values);

    /*! \see GPU_SetUniformf() */
    void (*SetUniformf)(GPU_Renderer* renderer, int location, float value);

    /*! \see GPU_SetUniformfv() */
    void (*SetUniformfv)(GPU_Renderer* renderer, int location, int num_elements_per_value, int num_values, float* values);

    /*! \see GPU_SetUniformMatrixfv() */
    void (*SetUniformMatrixfv)(GPU_Renderer* renderer, int location, int num_matrices, int num_rows, int num_columns, Uint8 transpose, float* values);
    
    /*! \see GPU_SetAttributef() */
    void (*SetAttributef)(GPU_Renderer* renderer, int location, float value);
    
    /*! \see GPU_SetAttributei() */
    void (*SetAttributei)(GPU_Renderer* renderer, int location, int value);
    
    /*! \see GPU_SetAttributeui() */
    void (*SetAttributeui)(GPU_Renderer* renderer, int location, unsigned int value);
    
    /*! \see GPU_SetAttributefv() */
    void (*SetAttributefv)(GPU_Renderer* renderer, int location, int num_elements, float* value);
    
    /*! \see GPU_SetAttributeiv() */
    void (*SetAttributeiv)(GPU_Renderer* renderer, int location, int num_elements, int* value);
    
    /*! \see GPU_SetAttributeuiv() */
    void (*SetAttributeuiv)(GPU_Renderer* renderer, int location, int num_elements, unsigned int* value);
    
    /*! \see GPU_SetAttributeSource() */
    void (*SetAttributeSource)(GPU_Renderer* renderer, int num_values, GPU_Attribute source);
    
    
    // Shapes
    
    /*! \see GPU_SetLineThickness() */
	float (*SetLineThickness)(GPU_Renderer* renderer, float thickness);
	
    /*! \see GPU_GetLineThickness() */
	float (*GetLineThickness)(GPU_Renderer* renderer);
	
    /*! \see GPU_Pixel() */
	void (*Pixel)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, SDL_Color color);

    /*! \see GPU_Line() */
	void (*Line)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

    /*! \see GPU_Arc() */
	void (*Arc)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, float start_angle, float end_angle, SDL_Color color);
	
    /*! \see GPU_ArcFilled() */
	void (*ArcFilled)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, float start_angle, float end_angle, SDL_Color color);

    /*! \see GPU_Circle() */
	void (*Circle)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, SDL_Color color);

    /*! \see GPU_CircleFilled() */
	void (*CircleFilled)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, SDL_Color color);

    /*! \see GPU_Sector() */
    void (*Sector)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float inner_radius, float outer_radius, float start_angle, float end_angle, SDL_Color color);

    /*! \see GPU_SectorFilled() */
    void (*SectorFilled)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float inner_radius, float outer_radius, float start_angle, float end_angle, SDL_Color color);
    
    /*! \see GPU_Tri() */
	void (*Tri)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color);

    /*! \see GPU_TriFilled() */
	void (*TriFilled)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color);

    /*! \see GPU_Rectangle() */
	void (*Rectangle)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

    /*! \see GPU_RectangleFilled() */
	void (*RectangleFilled)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

    /*! \see GPU_RectangleRound() */
	void (*RectangleRound)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color);

    /*! \see GPU_RectangleRoundFilled() */
	void (*RectangleRoundFilled)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color);

    /*! \see GPU_Polygon() */
	void (*Polygon)(GPU_Renderer* renderer, GPU_Target* target, unsigned int num_vertices, float* vertices, SDL_Color color);

    /*! \see GPU_PolygonFilled() */
	void (*PolygonFilled)(GPU_Renderer* renderer, GPU_Target* target, unsigned int num_vertices, float* vertices, SDL_Color color);
	
} GPU_RendererImpl;


#endif
