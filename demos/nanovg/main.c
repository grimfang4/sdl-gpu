#include <SDL2/SDL.h>
#include <SDL2/SDL_gpu.h>

#define GLEW_STATIC // needed for windows only(?)
#include <GL/glew.h>

#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg/nanovg.h"
#include "nanovg/nanovg_gl.h"
#include "nanovg/nanovg_gl_utils.h"

/* Create a GPU_Image from a NanoVG Framebuffer */
GPU_Image* generateFBO(NVGcontext* _vg, const float _w, const float _h, void (*draw)(NVGcontext*, const float, const float, const float, const float)) {
    // GPU_FlushBlitBuffer(); // call GPU_FlushBlitBuffer if you're doing this in the middle of SDL_gpu blitting
    NVGLUframebuffer* fb = nvgluCreateFramebuffer(_vg, _w, _h, NVG_IMAGE_NODELETE); // IMPORTANT: NVG_IMAGE_NODELETE allows us to run nvgluDeleteFramebuffer without freeing the GPU_Image data
    nvgluBindFramebuffer(fb);
    glViewport(0, 0, _w, _h);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    nvgBeginFrame(_vg, _w, _h, 1.0f);
    draw(_vg, 0, 0, _w, _h); // call the drawing function that was passed as parameter
    nvgEndFrame(_vg);
    /* nvgluBindFramebuffer(0); // official documentation says to unbind, but I haven't had issues not doing it */
    GPU_ResetRendererState(); // not calling GPU_ResetRendererState can cause problems with SDL_gpu depending on your order of operations
    // IMPORTANT: don't run nvgluDeleteFramebuffer, GPU_CreateImageUsingTexture takes the handle
    GPU_Image* return_image = GPU_CreateImageUsingTexture(fb->texture, false); // should take_ownership be true?
    nvgluDeleteFramebuffer(fb);
    return return_image;
}

/* Simple Drawing Example */
void drawNVG(NVGcontext* _vg, const float _x, const float _y, const float _w, const float _h) {
    const float square_r = 5.0f;
    nvgBeginPath(_vg);
    nvgRoundedRect(_vg, _x, _y, _w, _h, square_r);
    NVGpaint bg_paint = nvgLinearGradient(_vg, _x, _y, _x+_w, _y+_h, nvgRGBA(255, 255, 255, 255), nvgRGBA(255, 255, 255, 155));
    nvgFillPaint(_vg, bg_paint);
    nvgFill(_vg);
}

/* draw something that takes some awhile */
void drawComplexNVG(NVGcontext* _vg, const float _x, const float _y, const float _w, const float _h) {
    float x = _x;
    float y = _y;
    nvgBeginPath(_vg);
	nvgMoveTo(_vg, x, y);
	for (unsigned i = 1; i < 50000; i++) {
		nvgBezierTo(_vg, x-10.0f, y+10.0f, x+25, y+25, x,y);
        x += 10.0f;
        y += 5.0f;
        if (x > _w)
            x = 0.0f;
        if (y > _h)
            y = 0.0f;
    }
    NVGpaint stroke_paint = nvgLinearGradient(_vg, _x, _y, _w, _h, nvgRGBA(255, 255, 255, 20), nvgRGBA(0, 255, 255, 10));
    nvgStrokePaint(_vg, stroke_paint);
    nvgStroke(_vg);
}

/* Can help show STENCIL problems when _arc_radius because concave (convex?) */
void drawPie(NVGcontext* _vg, const float _x, const float _y, const float _arc_radius) {
    const float pie_radius = 100.0f;
    nvgBeginPath(_vg);
    nvgMoveTo(_vg, _x, _y);
    nvgArc(_vg, _x, _y, pie_radius, 0.0f, nvgDegToRad(_arc_radius), NVG_CW);
    nvgLineTo(_vg, _x, _y);
    nvgFillColor(_vg, nvgRGBA(0xFF,0xFF,0xFF,0xFF));
    nvgFill(_vg);
}

void main_loop(GPU_Target* _screen, NVGcontext* _vg, const Uint16 _screen_w, const Uint16 _screen_h) {
    const float px_ratio = (float)_screen_w / (float)_screen_w; // spoilers: it's 1.0f

    GPU_Rect fbo_simple_rect = { 65.0f, 10.0f, 50.0f, 50.0f}; // Blitting Destination
    GPU_Image* fbo_simple = generateFBO(_vg, fbo_simple_rect.w, fbo_simple_rect.h, drawNVG);

    GPU_Rect fbo_complex_rect = { 0.0f, 0.0f, _screen_w, _screen_h}; // Blitting Destination
    GPU_Image* fbo_complex = generateFBO(_vg, fbo_complex_rect.w, fbo_complex_rect.h, drawComplexNVG);

    float arc_radius = 0.0f;

    SDL_Event event;
    bool loop = true;
    do {
        /* SDL Event Handling */
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT)
                loop = false;
            else if(event.type == SDL_KEYDOWN) {
                if(event.key.keysym.sym == SDLK_ESCAPE)
                    loop = false;
            }
        }

        /* Animation Pass */
        arc_radius += 1.0f;
        
        /* SDL_gpu + NanoVG Rendering */
        GPU_ClearRGBA(_screen, 0x00, 0x00, 0x00, 0xFF); // GPU_ClearRGBA clears GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
        glClear(GL_STENCIL_BUFFER_BIT); // IMPORTANT: GPU_ClearRGBA does not clear GL_STENCIL_BUFFER_BIT

        /* SDL_gpu Blitting */
        GPU_BlitRectX(fbo_complex, NULL, _screen, &fbo_complex_rect, 0.0f, 0.0f, 0.0f, GPU_FLIP_VERTICAL); // IMPORTANT: GPU_BlitRectX is required to use GPU_FLIP_VERTICAL which is required for NVGLUframebuffer data (why???)
        GPU_BlitRectX(fbo_simple, NULL, _screen, &fbo_simple_rect, 0.0f, 0.0f, 0.0f, GPU_FLIP_VERTICAL); // IMPORTANT: GPU_BlitRectX is required to use GPU_FLIP_VERTICAL which is required for NVGLUframebuffer data (why???)

        /* NanoVG Section */
        GPU_FlushBlitBuffer(); // IMPORTANT: run GPU_FlushBlitBuffer before nvgBeginFrame
        nvgBeginFrame(_vg, _screen_w, _screen_h, px_ratio); // Do your normal NanoVG stuff
        drawNVG(_vg, 10.0f, 10.0f, fbo_simple_rect.w, fbo_simple_rect.h); // run our simple drawing code directly
        drawPie(_vg, _screen_w/2, _screen_h/2, arc_radius); // drawing the pie chart will break if you don't have a stencil buffer
        /* drawComplexNVG(_vg); */
        nvgEndFrame(_vg); // Finish our NanoVG pass
        GPU_ResetRendererState(); // IMPORTANT: run GPU_ResetRendererState after nvgEndFrame

        /* Finish */
        GPU_Flip(_screen); // Render to screen
    } while (loop);
    
    /* Loops over, Cleanup */
    GPU_FreeImage(fbo_simple);
    GPU_FreeImage(fbo_complex);
}

int main() {
    const Uint16 screen_w = 500;
    const Uint16 screen_h = 500;

    /* Init SDL and our renderer, no error handling! */
    SDL_Init(SDL_INIT_EVERYTHING); // Init SDL
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1); // NanoVG _REQUIRES_ a stencil buffer
    GPU_Target* target = GPU_InitRenderer(GPU_RENDERER_OPENGL_3, screen_w, screen_h, 0); // Init SDL_gpu
    NVGcontext* vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG); // Init NanoVG

    main_loop(target, vg, screen_w, screen_h);

    return 0;
}
