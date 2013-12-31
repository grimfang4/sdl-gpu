#ifndef _SDL_GPU_GL_MATRIX_H__
#define _SDL_GPU_GL_MATRIX_H__

#ifdef SDL_GPU_USE_GL_TIER3
#define SDL_GPU_USE_INTERNAL_MATRICES
#endif


#ifdef SDL_GPU_USE_INTERNAL_MATRICES
void _GPU_Dummy(void);
void _GPU_InitMatrix(void);
void _GPU_MatrixMode(int matrix_mode);
void _GPU_PushMatrix(void);
void _GPU_PopMatrix(void);
void _GPU_LoadIdentity(void);
void _GPU_Ortho(float left, float right, float bottom, float top, float near, float far);
void _GPU_Frustum(float right, float left, float bottom, float top, float near, float far);
void _GPU_Translate(float x, float y, float z);
void _GPU_Scale(float sx, float sy, float sz);
void _GPU_Rotate(float degrees, float x, float y, float z);
void _GPU_MultMatrix(float* matrix4x4);
void _GPU_GetModelViewProjection(float* result);

#define GPU_MODELVIEW 0
#define GPU_PROJECTION 1
#define GPU_InitMatrix _GPU_InitMatrix
#define GPU_MatrixMode _GPU_MatrixMode
#define GPU_PushMatrix _GPU_PushMatrix
#define GPU_PopMatrix _GPU_PopMatrix
#define GPU_LoadIdentity _GPU_LoadIdentity
#define GPU_Ortho _GPU_Ortho
#define GPU_Frustum _GPU_Frustum
#define GPU_Translate _GPU_Translate
#define GPU_Scale _GPU_Scale
#define GPU_Rotate _GPU_Rotate
#define GPU_MultMatrix _GPU_MultMatrix
#define GPU_GetModelViewProjection _GPU_GetModelViewProjection
#else
void _GPU_Dummy(void);

#define GPU_MODELVIEW GL_MODELVIEW
#define GPU_PROJECTION GL_PROJECTION
#define GPU_InitMatrix _GPU_Dummy
#define GPU_MatrixMode glMatrixMode
#define GPU_PushMatrix glPushMatrix
#define GPU_PopMatrix glPopMatrix
#define GPU_LoadIdentity glLoadIdentity
#define GPU_Ortho glOrtho
#define GPU_Frustum glFrustum
#define GPU_Translate glTranslatef
#define GPU_Scale glScalef
#define GPU_Rotate glRotatef
#define GPU_MultMatrix glMultMatrixf
#endif

#endif
