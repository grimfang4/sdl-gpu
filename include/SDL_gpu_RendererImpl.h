#ifndef _GPU_RENDERERIMPL_H__
#define _GPU_RENDERERIMPL_H__

#include "SDL_gpu.h"

#ifdef __cplusplus
extern "C" {
#endif

// Internal API for managing window mappings
DECLSPEC void SDLCALL GPU_AddWindowMapping(GPU_Target* target);
DECLSPEC void SDLCALL GPU_RemoveWindowMapping(Uint32 windowID);
DECLSPEC void SDLCALL GPU_RemoveWindowMappingByTarget(GPU_Target* target);

/*! Private implementation of renderer members. */
typedef struct GPU_RendererImpl
{
	/*! \see GPU_Init()
	 *  \see GPU_InitRenderer()
	 *  \see GPU_InitRendererByID()
	 */
	GPU_Target* (SDLCALL *Init)(GPU_Renderer* renderer, GPU_RendererID renderer_request, Uint16 w, Uint16 h, GPU_WindowFlagEnum SDL_flags);
	
    /*! \see GPU_CreateTargetFromWindow
     * The extra parameter is used internally to reuse/reinit a target. */
	GPU_Target* (SDLCALL *CreateTargetFromWindow)(GPU_Renderer* renderer, Uint32 windowID, GPU_Target* target);
    
    /*! \see GPU_SetActiveTarget() */
    GPU_bool (SDLCALL *SetActiveTarget)(GPU_Renderer* renderer, GPU_Target* target);
    
    /*! \see GPU_CreateAliasTarget() */
	GPU_Target* (SDLCALL *CreateAliasTarget)(GPU_Renderer* renderer, GPU_Target* target);

    /*! \see GPU_MakeCurrent */
	void (SDLCALL *MakeCurrent)(GPU_Renderer* renderer, GPU_Target* target, Uint32 windowID);
	
	/*! Sets up this renderer to act as the current renderer.  Called automatically by GPU_SetCurrentRenderer(). */
	void (SDLCALL *SetAsCurrent)(GPU_Renderer* renderer);
	
	/*! \see GPU_ResetRendererState() */
	void (SDLCALL *ResetRendererState)(GPU_Renderer* renderer);
	
	/*! \see GPU_AddDepthBuffer() */
	GPU_bool (SDLCALL *AddDepthBuffer)(GPU_Renderer* renderer, GPU_Target* target);
	
	/*! \see GPU_SetWindowResolution() */
	GPU_bool (SDLCALL *SetWindowResolution)(GPU_Renderer* renderer, Uint16 w, Uint16 h);
	
	/*! \see GPU_SetVirtualResolution() */
	void (SDLCALL *SetVirtualResolution)(GPU_Renderer* renderer, GPU_Target* target, Uint16 w, Uint16 h);

	/*! \see GPU_UnsetVirtualResolution() */
	void (SDLCALL *UnsetVirtualResolution)(GPU_Renderer* renderer, GPU_Target* target);
	
	/*! Clean up the renderer state. */
	void (SDLCALL *Quit)(GPU_Renderer* renderer);
	
	/*! \see GPU_SetFullscreen() */
	GPU_bool (SDLCALL *SetFullscreen)(GPU_Renderer* renderer, GPU_bool enable_fullscreen, GPU_bool use_desktop_resolution);

	/*! \see GPU_SetCamera() */
	GPU_Camera (SDLCALL *SetCamera)(GPU_Renderer* renderer, GPU_Target* target, GPU_Camera* cam);
	
    /*! \see GPU_CreateImage() */
	GPU_Image* (SDLCALL *CreateImage)(GPU_Renderer* renderer, Uint16 w, Uint16 h, GPU_FormatEnum format);
	
    /*! \see GPU_CreateImageUsingTexture() */
	GPU_Image* (SDLCALL *CreateImageUsingTexture)(GPU_Renderer* renderer, GPU_TextureHandle handle, GPU_bool take_ownership);
	
    /*! \see GPU_CreateAliasImage() */
	GPU_Image* (SDLCALL *CreateAliasImage)(GPU_Renderer* renderer, GPU_Image* image);
	
	/*! \see GPU_SaveImage() */
	GPU_bool (SDLCALL *SaveImage)(GPU_Renderer* renderer, GPU_Image* image, const char* filename, GPU_FileFormatEnum format);
	
	/*! \see GPU_CopyImage() */
	GPU_Image* (SDLCALL *CopyImage)(GPU_Renderer* renderer, GPU_Image* image);
	
	/*! \see GPU_UpdateImage */
	void (SDLCALL *UpdateImage)(GPU_Renderer* renderer, GPU_Image* image, const GPU_Rect* image_rect, SDL_Surface* surface, const GPU_Rect* surface_rect);
	
	/*! \see GPU_UpdateImageBytes */
	void (SDLCALL *UpdateImageBytes)(GPU_Renderer* renderer, GPU_Image* image, const GPU_Rect* image_rect, const unsigned char* bytes, int bytes_per_row);
	
	/*! \see GPU_ReplaceImage */
	GPU_bool (SDLCALL *ReplaceImage)(GPU_Renderer* renderer, GPU_Image* image, SDL_Surface* surface, const GPU_Rect* surface_rect);
	
	/*! \see GPU_CopyImageFromSurface() */
	GPU_Image* (SDLCALL *CopyImageFromSurface)(GPU_Renderer* renderer, SDL_Surface* surface);
	
	/*! \see GPU_CopyImageFromTarget() */
	GPU_Image* (SDLCALL *CopyImageFromTarget)(GPU_Renderer* renderer, GPU_Target* target);
	
	/*! \see GPU_CopySurfaceFromTarget() */
	SDL_Surface* (SDLCALL *CopySurfaceFromTarget)(GPU_Renderer* renderer, GPU_Target* target);
	
	/*! \see GPU_CopySurfaceFromImage() */
	SDL_Surface* (SDLCALL *CopySurfaceFromImage)(GPU_Renderer* renderer, GPU_Image* image);
	
	/*! \see GPU_FreeImage() */
	void (SDLCALL *FreeImage)(GPU_Renderer* renderer, GPU_Image* image);
	
	/*! \see GPU_GetTarget() */
	GPU_Target* (SDLCALL *GetTarget)(GPU_Renderer* renderer, GPU_Image* image);
	
	/*! \see GPU_FreeTarget() */
	void (SDLCALL *FreeTarget)(GPU_Renderer* renderer, GPU_Target* target);

	/*! \see GPU_Blit() */
	void (SDLCALL *Blit)(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y);
	
	/*! \see GPU_BlitRotate() */
	void (SDLCALL *BlitRotate)(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float degrees);
	
	/*! \see GPU_BlitScale() */
	void (SDLCALL *BlitScale)(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float scaleX, float scaleY);
	
	/*! \see GPU_BlitTransform */
	void (SDLCALL *BlitTransform)(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float degrees, float scaleX, float scaleY);
	
	/*! \see GPU_BlitTransformX() */
	void (SDLCALL *BlitTransformX)(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float pivot_x, float pivot_y, float degrees, float scaleX, float scaleY);
	
	/*! \see GPU_PrimitiveBatchV() */
	void (SDLCALL *PrimitiveBatchV)(GPU_Renderer* renderer, GPU_Image* image, GPU_Target* target, GPU_PrimitiveEnum primitive_type, unsigned short num_vertices, void* values, unsigned int num_indices, unsigned short* indices, GPU_BatchFlagEnum flags);
	
	/*! \see GPU_GenerateMipmaps() */
	void (SDLCALL *GenerateMipmaps)(GPU_Renderer* renderer, GPU_Image* image);

	/*! \see GPU_SetClip() */
	GPU_Rect (SDLCALL *SetClip)(GPU_Renderer* renderer, GPU_Target* target, Sint16 x, Sint16 y, Uint16 w, Uint16 h);

	/*! \see GPU_UnsetClip() */
	void (SDLCALL *UnsetClip)(GPU_Renderer* renderer, GPU_Target* target);
	
	/*! \see GPU_GetPixel() */
	SDL_Color (SDLCALL *GetPixel)(GPU_Renderer* renderer, GPU_Target* target, Sint16 x, Sint16 y);
	
	/*! \see GPU_SetImageFilter() */
	void (SDLCALL *SetImageFilter)(GPU_Renderer* renderer, GPU_Image* image, GPU_FilterEnum filter);
	
	/*! \see GPU_SetWrapMode() */
	void (SDLCALL *SetWrapMode)(GPU_Renderer* renderer, GPU_Image* image, GPU_WrapEnum wrap_mode_x, GPU_WrapEnum wrap_mode_y);
    
    /*! \see GPU_GetTextureHandle() */
    GPU_TextureHandle (SDLCALL *GetTextureHandle)(GPU_Renderer* renderer, GPU_Image* image);
    
	/*! \see GPU_ClearRGBA() */
	void (SDLCALL *ClearRGBA)(GPU_Renderer* renderer, GPU_Target* target, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
	/*! \see GPU_FlushBlitBuffer() */
	void (SDLCALL *FlushBlitBuffer)(GPU_Renderer* renderer);
	/*! \see GPU_Flip() */
	void (SDLCALL *Flip)(GPU_Renderer* renderer, GPU_Target* target);
	
	
    /*! \see GPU_CreateShaderProgram() */
	Uint32 (SDLCALL *CreateShaderProgram)(GPU_Renderer* renderer);

    /*! \see GPU_FreeShaderProgram() */
	void (SDLCALL *FreeShaderProgram)(GPU_Renderer* renderer, Uint32 program_object);
	
    /*! \see GPU_CompileShader_RW() */
	Uint32 (SDLCALL *CompileShader_RW)(GPU_Renderer* renderer, GPU_ShaderEnum shader_type, SDL_RWops* shader_source, GPU_bool free_rwops);
	
    /*! \see GPU_CompileShader() */
	Uint32 (SDLCALL *CompileShader)(GPU_Renderer* renderer, GPU_ShaderEnum shader_type, const char* shader_source);

    /*! \see GPU_FreeShader() */
	void (SDLCALL *FreeShader)(GPU_Renderer* renderer, Uint32 shader_object);

    /*! \see GPU_AttachShader() */
	void (SDLCALL *AttachShader)(GPU_Renderer* renderer, Uint32 program_object, Uint32 shader_object);

    /*! \see GPU_DetachShader() */
	void (SDLCALL *DetachShader)(GPU_Renderer* renderer, Uint32 program_object, Uint32 shader_object);

    /*! \see GPU_LinkShaderProgram() */
	GPU_bool (SDLCALL *LinkShaderProgram)(GPU_Renderer* renderer, Uint32 program_object);

    /*! \see GPU_ActivateShaderProgram() */
	void (SDLCALL *ActivateShaderProgram)(GPU_Renderer* renderer, Uint32 program_object, GPU_ShaderBlock* block);

    /*! \see GPU_DeactivateShaderProgram() */
	void (SDLCALL *DeactivateShaderProgram)(GPU_Renderer* renderer);

    /*! \see GPU_GetShaderMessage() */
	const char* (SDLCALL *GetShaderMessage)(GPU_Renderer* renderer);

    /*! \see GPU_GetAttribLocation() */
	int (SDLCALL *GetAttributeLocation)(GPU_Renderer* renderer, Uint32 program_object, const char* attrib_name);

    /*! \see GPU_GetUniformLocation() */
	int (SDLCALL *GetUniformLocation)(GPU_Renderer* renderer, Uint32 program_object, const char* uniform_name);
    
    /*! \see GPU_LoadShaderBlock() */
	GPU_ShaderBlock (SDLCALL *LoadShaderBlock)(GPU_Renderer* renderer, Uint32 program_object, const char* position_name, const char* texcoord_name, const char* color_name, const char* modelViewMatrix_name);
    
    /*! \see GPU_SetShaderBlock() */
	void (SDLCALL *SetShaderBlock)(GPU_Renderer* renderer, GPU_ShaderBlock block);
    
    /*! \see GPU_SetShaderImage() */
	void (SDLCALL *SetShaderImage)(GPU_Renderer* renderer, GPU_Image* image, int location, int image_unit);
    
    /*! \see GPU_GetUniformiv() */
	void (SDLCALL *GetUniformiv)(GPU_Renderer* renderer, Uint32 program_object, int location, int* values);

    /*! \see GPU_SetUniformi() */
	void (SDLCALL *SetUniformi)(GPU_Renderer* renderer, int location, int value);

    /*! \see GPU_SetUniformiv() */
	void (SDLCALL *SetUniformiv)(GPU_Renderer* renderer, int location, int num_elements_per_value, int num_values, int* values);

    /*! \see GPU_GetUniformuiv() */
	void (SDLCALL *GetUniformuiv)(GPU_Renderer* renderer, Uint32 program_object, int location, unsigned int* values);

    /*! \see GPU_SetUniformui() */
	void (SDLCALL *SetUniformui)(GPU_Renderer* renderer, int location, unsigned int value);

    /*! \see GPU_SetUniformuiv() */
	void (SDLCALL *SetUniformuiv)(GPU_Renderer* renderer, int location, int num_elements_per_value, int num_values, unsigned int* values);

    /*! \see GPU_GetUniformfv() */
	void (SDLCALL *GetUniformfv)(GPU_Renderer* renderer, Uint32 program_object, int location, float* values);

    /*! \see GPU_SetUniformf() */
	void (SDLCALL *SetUniformf)(GPU_Renderer* renderer, int location, float value);

    /*! \see GPU_SetUniformfv() */
	void (SDLCALL *SetUniformfv)(GPU_Renderer* renderer, int location, int num_elements_per_value, int num_values, float* values);

    /*! \see GPU_SetUniformMatrixfv() */
	void (SDLCALL *SetUniformMatrixfv)(GPU_Renderer* renderer, int location, int num_matrices, int num_rows, int num_columns, GPU_bool transpose, float* values);
    
    /*! \see GPU_SetAttributef() */
	void (SDLCALL *SetAttributef)(GPU_Renderer* renderer, int location, float value);
    
    /*! \see GPU_SetAttributei() */
	void (SDLCALL *SetAttributei)(GPU_Renderer* renderer, int location, int value);
    
    /*! \see GPU_SetAttributeui() */
	void (SDLCALL *SetAttributeui)(GPU_Renderer* renderer, int location, unsigned int value);
    
    /*! \see GPU_SetAttributefv() */
	void (SDLCALL *SetAttributefv)(GPU_Renderer* renderer, int location, int num_elements, float* value);
    
    /*! \see GPU_SetAttributeiv() */
	void (SDLCALL *SetAttributeiv)(GPU_Renderer* renderer, int location, int num_elements, int* value);
    
    /*! \see GPU_SetAttributeuiv() */
	void (SDLCALL *SetAttributeuiv)(GPU_Renderer* renderer, int location, int num_elements, unsigned int* value);
    
    /*! \see GPU_SetAttributeSource() */
	void (SDLCALL *SetAttributeSource)(GPU_Renderer* renderer, int num_values, GPU_Attribute source);
    
    
    // Shapes
    
    /*! \see GPU_SetLineThickness() */
	float (SDLCALL *SetLineThickness)(GPU_Renderer* renderer, float thickness);
	
    /*! \see GPU_GetLineThickness() */
	float (SDLCALL *GetLineThickness)(GPU_Renderer* renderer);
	
    /*! \see GPU_Pixel() */
	void (SDLCALL *Pixel)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, SDL_Color color);

    /*! \see GPU_Line() */
	void (SDLCALL *Line)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

    /*! \see GPU_Arc() */
	void (SDLCALL *Arc)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, float start_angle, float end_angle, SDL_Color color);
	
    /*! \see GPU_ArcFilled() */
	void (SDLCALL *ArcFilled)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, float start_angle, float end_angle, SDL_Color color);

    /*! \see GPU_Circle() */
	void (SDLCALL *Circle)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, SDL_Color color);

    /*! \see GPU_CircleFilled() */
	void (SDLCALL *CircleFilled)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, SDL_Color color);
	
	/*! \see GPU_Ellipse() */
	void (SDLCALL *Ellipse)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float rx, float ry, float degrees, SDL_Color color);
	
	/*! \see GPU_EllipseFilled() */
	void (SDLCALL *EllipseFilled)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float rx, float ry, float degrees, SDL_Color color);

    /*! \see GPU_Sector() */
	void (SDLCALL *Sector)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float inner_radius, float outer_radius, float start_angle, float end_angle, SDL_Color color);

    /*! \see GPU_SectorFilled() */
	void (SDLCALL *SectorFilled)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float inner_radius, float outer_radius, float start_angle, float end_angle, SDL_Color color);
    
    /*! \see GPU_Tri() */
	void (SDLCALL *Tri)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color);

    /*! \see GPU_TriFilled() */
	void (SDLCALL *TriFilled)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color);

    /*! \see GPU_Rectangle() */
	void (SDLCALL *Rectangle)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

    /*! \see GPU_RectangleFilled() */
	void (SDLCALL *RectangleFilled)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

    /*! \see GPU_RectangleRound() */
	void (SDLCALL *RectangleRound)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color);

    /*! \see GPU_RectangleRoundFilled() */
	void (SDLCALL *RectangleRoundFilled)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color);

    /*! \see GPU_Polygon() */
	void (SDLCALL *Polygon)(GPU_Renderer* renderer, GPU_Target* target, unsigned int num_vertices, float* vertices, SDL_Color color);

	/*! \see GPU_Polyline() */
	void (SDLCALL *Polyline)(GPU_Renderer* renderer, GPU_Target* target, unsigned int num_vertices, float* vertices, SDL_Color color, GPU_bool close_loop);
	
    /*! \see GPU_PolygonFilled() */
	void (SDLCALL *PolygonFilled)(GPU_Renderer* renderer, GPU_Target* target, unsigned int num_vertices, float* vertices, SDL_Color color);
	
} GPU_RendererImpl;

#ifdef __cplusplus
}
#endif

#endif
