# Introduction #

This page contains notes that explain or argue the implementation details of the library.  If you have anything to say about them, let us know!


# Notes #

Couldn't there be GPU\_BLEND\_NONE instead of GPU\_Image::use\_blending?
> Yeah, that is what SDL2 does.  However, I feel that the separation is a useful and intuitive convention.  It separates "how" blending is done (blend mode) from "whether" it is done (use\_blending).  To combine them would be to change the abstraction to "pixel transfer mode".

Clearing new blank textures.
> glClearTexImage() extension is very new, so not well supported.
> Clearing via upload works but is a little slower.
> > Allocate a zeroed array and grow it when more is needed.

Rendering to intermediate buffer that has an odd dimension size will blur the result a little.  I think the rule should be: If you want pixel-perfect, don't use odd sizes!

GPU\_Init() sets an empty window caption.  It's not a big deal to change after init by using target->context->window\_id, so adding stuff to SDL\_gpu to do it would just be unnecessary.  Maybe GPU\_GetWindowID().

Making every shape able to be textured would mean either a lot of assumptions about the texture coordinates or else a few extra function arguments to specify the texture coordinates and number of vertices to use (which would also lead to ambiguity).  The current functions are only untextured and their number of vertices is not specified (which makes shader attributes not work reliably with them).  The best way to texture shapes is to write your own textured shape functions using GPU\_TriangleBatch().

Attributes

> Don't try to use expanded (per-sprite) attributes for positions or texcoords...  You would have 4 vertices on top of each other for each sprite.

Shader blocks
> Used to pass default shader variables (attributes and a uniform) to the shader.
> Has 3 default attributes: position, color, texcoords.  Has 1 default uniform: gpu\_ModelViewProjectionMatrix.  These are handled by the blit buffer and flush.

State contained in images:
> Pixel descriptors
> > width, height
> > channels

> Modulation color (GPU\_SetRGBA())
> Blending on/off (GPU\_SetBlending())
> Blend mode (GPU\_SetBlendMode())

State contained in render targets:
> Camera transform (via matrix stack for GL 3+)
> Virtual width and height
> Framebuffer width and height (duplicated from image or window)
> Clipping rect
> Modulation color (GPU\_SetTargetRGBA())

State contained in context targets:
> Current shader
> Default shaders
> Blit buffer
> Window ID
> Shape blending
> Shape blend mode
> Line thickness (GPU\_SetThickness())
> GL Context (SDL\_GLContext is just a void`*`)
> Matrix stack

State contained in renderer:
> Renderer ID
> Renderer tier
> Renderer features
> Current context target

Windowless OpenGL context would be useful for triggering renderer init failure, but it gets messy and platform-specific.

FlushBlitBuffer() uses GL\_TRIANGLES instead of a fan or strip because a single glDrawArrays/glDrawElements call can't draw separate strips.  Thankfully, I can use an index buffer to reduce the vertex count back to 4, instead of the 6 that two triangles would normally use.

GLES does not support GL\_UNPACK\_ROW\_LENGTH, which is a shame.  It means that if image data is not tightly-packed, it has to be re-copied into a tightly-packed array first.  This makes the GL and GLES renderers do loading/updating images in slightly different ways.  I think there's a GLES2 extension for it though, so that might be worth looking into.

Multiple windows can be done in two ways:
  1. GPU\_CreateTargetFromWindow() creates a separate context for rendering, shaders, and textures.
> 2) GPU\_MakeCurrent() can be used to switch the window of a GPU\_Target in order to share a context between windows.
> What about SDL\_GL\_SHARE\_WITH\_CURRENT\_CONTEXT?