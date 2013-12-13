#include "SDL_gpu_OpenGL_1.h"


#if defined(SDL_GPU_DISABLE_OPENGL) || defined(SDL_GPU_DISABLE_OPENGL_1)

// Dummy implementations
GPU_Renderer* GPU_CreateRenderer_OpenGL_1(GPU_RendererID request) {return NULL;}
void GPU_FreeRenderer_OpenGL_1(GPU_Renderer* renderer) {}

#else

// Most of the code pulled in from here...
#define SDL_GPU_USE_OPENGL
#define SDL_GPU_USE_GL_TIER1
#define RENDERER_DATA RendererData_OpenGL_1
#define IMAGE_DATA ImageData_OpenGL_1
#define TARGET_DATA TargetData_OpenGL_1
#include "../GL_common/SDL_gpu_GL_common.inl"
#include "../GL_common/SDL_gpuShapes_GL_common.inl"


GPU_Renderer* GPU_CreateRenderer_OpenGL_1(GPU_RendererID request)
{
    GPU_Renderer* renderer = (GPU_Renderer*)malloc(sizeof(GPU_Renderer));
    if(renderer == NULL)
        return NULL;

    memset(renderer, 0, sizeof(GPU_Renderer));

    renderer->id = request;
	renderer->id.id = GPU_RENDERER_OPENGL_1;
    
    renderer->display = NULL;
    renderer->camera = GPU_GetDefaultCamera();

    renderer->data = (RENDERER_DATA*)malloc(sizeof(RENDERER_DATA));
    memset(renderer->data, 0, sizeof(RENDERER_DATA));

    renderer->Init = &Init;
    renderer->IsFeatureEnabled = &IsFeatureEnabled;
    renderer->SetAsCurrent = &SetAsCurrent;
    renderer->SetDisplayResolution = &SetDisplayResolution;
    renderer->SetVirtualResolution = &SetVirtualResolution;
    renderer->Quit = &Quit;

    renderer->ToggleFullscreen = &ToggleFullscreen;
    renderer->SetCamera = &SetCamera;
    renderer->GetWindow = &GetWindow;

    renderer->CreateImage = &CreateImage;
    renderer->LoadImage = &LoadImage;
    renderer->SaveImage = &SaveImage;
    renderer->CopyImage = &CopyImage;
    renderer->UpdateImage = &UpdateImage;
    renderer->CopyImageFromSurface = &CopyImageFromSurface;
    renderer->CopyImageFromTarget = &CopyImageFromTarget;
    renderer->CopySurfaceFromTarget = &CopySurfaceFromTarget;
    renderer->CopySurfaceFromImage = &CopySurfaceFromImage;
    renderer->SubSurfaceCopy = &SubSurfaceCopy;
    renderer->FreeImage = &FreeImage;

    renderer->GetDisplayTarget = &GetDisplayTarget;
    renderer->LoadTarget = &LoadTarget;
    renderer->FreeTarget = &FreeTarget;

    renderer->Blit = &Blit;
    renderer->BlitRotate = &BlitRotate;
    renderer->BlitScale = &BlitScale;
    renderer->BlitTransform = &BlitTransform;
    renderer->BlitTransformX = &BlitTransformX;
    renderer->BlitTransformMatrix = &BlitTransformMatrix;

    renderer->SetZ = &SetZ;
    renderer->GetZ = &GetZ;
    renderer->GenerateMipmaps = &GenerateMipmaps;

    renderer->SetClip = &SetClip;
    renderer->ClearClip = &ClearClip;
    renderer->GetBlending = &GetBlending;
    renderer->SetBlending = &SetBlending;
    renderer->SetRGBA = &SetRGBA;

    renderer->ReplaceRGB = &ReplaceRGB;
    renderer->MakeRGBTransparent = &MakeRGBTransparent;
    renderer->ShiftHSV = &ShiftHSV;
    renderer->ShiftHSVExcept = &ShiftHSVExcept;
    renderer->GetPixel = &GetPixel;
    renderer->SetImageFilter = &SetImageFilter;
    renderer->SetBlendMode = &SetBlendMode;

    renderer->Clear = &Clear;
    renderer->ClearRGBA = &ClearRGBA;
    renderer->FlushBlitBuffer = &FlushBlitBuffer;
    renderer->Flip = &Flip;
    
    renderer->CompileShader_RW = &CompileShader_RW;
    renderer->CompileShader = &CompileShader;
    renderer->LinkShaderProgram = &LinkShaderProgram;
    renderer->LinkShaders = &LinkShaders;
    renderer->FreeShader = &FreeShader;
    renderer->FreeShaderProgram = &FreeShaderProgram;
    renderer->AttachShader = &AttachShader;
    renderer->DetachShader = &DetachShader;
    renderer->ActivateShaderProgram = &ActivateShaderProgram;
    renderer->DeactivateShaderProgram = &DeactivateShaderProgram;
    renderer->GetShaderMessage = &GetShaderMessage;
    renderer->GetUniformLocation = &GetUniformLocation;
    renderer->GetUniformiv = &GetUniformiv;
    renderer->SetUniformi = &SetUniformi;
    renderer->SetUniformiv = &SetUniformiv;
    renderer->GetUniformuiv = &GetUniformuiv;
    renderer->SetUniformui = &SetUniformui;
    renderer->SetUniformuiv = &SetUniformuiv;
    renderer->GetUniformfv = &GetUniformfv;
    renderer->SetUniformf = &SetUniformf;
    renderer->SetUniformfv = &SetUniformfv;
	
	
	// Shape rendering
	
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
    renderer->Rectangle = &Rectangle;
    renderer->RectangleFilled = &RectangleFilled;
    renderer->RectangleRound = &RectangleRound;
    renderer->RectangleRoundFilled = &RectangleRoundFilled;
    renderer->Polygon = &Polygon;
    renderer->PolygonFilled = &PolygonFilled;
    renderer->PolygonBlit = &PolygonBlit;

    return renderer;
}

void GPU_FreeRenderer_OpenGL_1(GPU_Renderer* renderer)
{
    if(renderer == NULL)
        return;

    free(renderer->data);
    free(renderer);
}

#endif
