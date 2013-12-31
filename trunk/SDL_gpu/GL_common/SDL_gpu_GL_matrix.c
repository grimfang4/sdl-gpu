#include "SDL_gpu.h"

#define SDL_GPU_USE_INTERNAL_MATRICES
#include "SDL_gpu_GL_matrix.h"
#include <math.h>
#include <string.h>


#ifndef M_PI
#define M_PI 3.1415926f
#endif

static int _gpu_matrix_mode = GPU_MODELVIEW;

#ifndef GPU_MATRIX_STACK_MAX
#define GPU_MATRIX_STACK_MAX 5
#endif

// Can be used up to two times per line evaluation...
const char* GPU_GetMatrixString(float* A)
{
    static char buffer[512];
    static char buffer2[512];
    static char flip = 0;
    
    char* b = (flip? buffer : buffer2);
    flip = !flip;
    
    snprintf(b, 512, "%.1f %.1f %.1f %.1f\n"
                          "%.1f %.1f %.1f %.1f\n"
                          "%.1f %.1f %.1f %.1f\n"
                          "%.1f %.1f %.1f %.1f", 
                          A[0], A[1], A[2], A[3], 
                          A[4], A[5], A[6], A[7], 
                          A[8], A[9], A[10], A[11], 
                          A[12], A[13], A[14], A[15]);
    return b;
}

typedef struct GPU_MatrixStack
{
    unsigned int size;
    float matrix[GPU_MATRIX_STACK_MAX][16];
} GPU_MatrixStack;

static GPU_MatrixStack projection_matrix = {0};

static GPU_MatrixStack modelview_matrix = {0};

float* _GPU_GetModelView(void)
{
    if(modelview_matrix.size == 0)
        return NULL;
    return modelview_matrix.matrix[modelview_matrix.size-1];
}

float* _GPU_GetProjection(void)
{
    if(projection_matrix.size == 0)
        return NULL;
    return projection_matrix.matrix[projection_matrix.size-1];
}

float* _GPU_GetCurrentMatrix(void)
{
    if(_gpu_matrix_mode == GPU_MODELVIEW)
    {
        if(modelview_matrix.size == 0)
            return NULL;
        return modelview_matrix.matrix[modelview_matrix.size-1];
    }
    else
    {
        if(projection_matrix.size == 0)
            return NULL;
        return projection_matrix.matrix[projection_matrix.size-1];
    }
}

// Implementations based on Wayne Cochran's (wcochran) matrix.c

// Column-major
#define INDEX(row,col) ((col)*4 + (row))

void _GPU_MatrixCopy(float* result, const float* A)
{
    memcpy(result, A, 16*sizeof(float));
}

void _GPU_MatrixIdentity(float* result)
{
    memset(result, 0, 16*sizeof(float));
    result[0] = result[5] = result[10] = result[15] = 1;
}

// Matrix multiply: result = A * B
void _GPU_Multiply4x4(float* result, float* A, float* B)
{
    int r, c, i;
    float s;
    for(r = 0; r < 4; r++)
    {
        for(c = 0; c < 4; c++)
        {
            s = 0;
            for(i = 0; i < 4; i++)
                s += A[INDEX(r,i)]*B[INDEX(i,c)];

            result[INDEX(r,c)] = s;
        }
    }
}

void _GPU_MultiplyAndAssign(float* result, float* A)
{
    float temp[16];
    _GPU_Multiply4x4(temp, result, A);
    _GPU_MatrixCopy(result, temp);
}


// Public API

void _GPU_Dummy(void) {}

void _GPU_InitMatrix(void)
{
    projection_matrix.size = 1;
    _GPU_MatrixIdentity(projection_matrix.matrix[0]);
    
    modelview_matrix.size = 1;
    _GPU_MatrixIdentity(modelview_matrix.matrix[0]);
    
    _gpu_matrix_mode = GPU_MODELVIEW;
}

void _GPU_MatrixMode(int matrix_mode)
{
    _gpu_matrix_mode = matrix_mode;
}

void _GPU_PushMatrix()
{
    GPU_MatrixStack* stack = (_gpu_matrix_mode == GPU_MODELVIEW? &modelview_matrix : &projection_matrix);
    if(stack->size + 1 >= GPU_MATRIX_STACK_MAX)
    {
        GPU_LogError("GPU_PushMatrix() failed to push to a full stack!\n");
        return;
    }
    _GPU_MatrixCopy(stack->matrix[stack->size], stack->matrix[stack->size-1]);
    stack->size++;
}

void _GPU_PopMatrix()
{
    GPU_MatrixStack* stack = (_gpu_matrix_mode == GPU_MODELVIEW? &modelview_matrix : &projection_matrix);
    if(stack->size == 0)
    {
        GPU_LogError("GPU_PopMatrix() failed to pop an empty stack!\n");
        return;
    }
    stack->size--;
}

void _GPU_LoadIdentity(void)
{
    float* result = _GPU_GetCurrentMatrix();
    if(result == NULL)
        return;
    _GPU_MatrixIdentity(result);
}

void _GPU_Ortho(float left, float right, float bottom, float top, float near, float far)
{
    float* result = _GPU_GetCurrentMatrix();
    if(result == NULL)
        return;
    #ifdef ROW_MAJOR
    float A[16] = {2/(right - left), 0,  0, -(right + left)/(right - left),
                   0, 2/(top - bottom), 0, -(top + bottom)/(top - bottom),
                   0, 0, -2/(far - near), -(far + near)/(far - near),
                   0, 0, 0, 1};
    #else
    float A[16] = {2/(right - left), 0,  0, 0,
                   0, 2/(top - bottom), 0, 0,
                   0, 0, -2/(far - near), 0,
                   -(right + left)/(right - left), -(top + bottom)/(top - bottom), -(far + near)/(far - near), 1};
    #endif
    
    _GPU_MultiplyAndAssign(result, A);
}

void _GPU_Frustum(float right, float left, float bottom, float top, float near, float far)
{
    float* result = _GPU_GetCurrentMatrix();
    if(result == NULL)
        return;
    #ifdef ROW_MAJOR
    float A[16] = {2 * near / (right - left), 0, 0, 0,
                   0, 2 * near / (top - bottom), 0, 0,
                   (right + left) / (right - left), (top + bottom) / (top - bottom), -(far + near) / (far - near), -1,
                   0, 0, -(2 * far * near) / (far - near), 0};
    #else
    float A[16] = {2 * near / (right - left), 0, (right + left) / (right - left), 0,
                   0, 2 * near / (top - bottom), (top + bottom) / (top - bottom), 0,
                   0, 0, -(far + near) / (far - near), -(2 * far * near) / (far - near),
                   0, 0, -1, 0};
    #endif
                   
    _GPU_MultiplyAndAssign(result, A);
}

void _GPU_Translate(float x, float y, float z)
{
    float* result = _GPU_GetCurrentMatrix();
    if(result == NULL)
        return;
    #ifdef ROW_MAJOR
    float A[16] = {1, 0, 0, x,
                   0, 1, 0, y,
                   0, 0, 1, z,
                   0, 0, 0, 1};
    #else
    float A[16] = {1, 0, 0, 0,
                   0, 1, 0, 0,
                   0, 0, 1, 0,
                   x, y, z, 1};
    #endif
    
    _GPU_MultiplyAndAssign(result, A);
}

void _GPU_Scale(float sx, float sy, float sz)
{
    float* result = _GPU_GetCurrentMatrix();
    if(result == NULL)
        return;
    float A[16] = {sx, 0,  0,  0,
                    0,  sy, 0,  0,
                    0,  0,  sz, 0,
                    0,  0,  0,  1};
                  
    _GPU_MultiplyAndAssign(result, A);
}

void _GPU_Rotate(float degrees, float x, float y, float z)
{
    const float p = 1/sqrtf(x*x + y*y + z*z);
    x *= p; y *= p; z *= p;
    const float radians = degrees * (M_PI/180);
    const float c = cosf(radians);
    const float s = sinf(radians);
    const float c_ = 1 - c;
    const float zc_ = z*c_;
    const float yc_ = y*c_;
    const float xzc_ = x*zc_;
    const float xyc_ = x*y*c_;
    const float yzc_ = y*zc_;
    const float xs = x*s;
    const float ys = y*s;
    const float zs = z*s;

    float* result = _GPU_GetCurrentMatrix();
    if(result == NULL)
        return;
    #ifdef ROW_MAJOR
    float A[16] = {x*x*c_ + c,  xyc_ - zs,   xzc_ + ys, 0,
                    xyc_ + zs,   y*yc_ + c,   yzc_ - xs, 0,
                    xzc_ - ys,   yzc_ + xs,   z*zc_ + c, 0,
                    0,           0,           0,         1};
    #else
    float A[16] = {x*x*c_ + c,  xyc_ + zs,   xzc_ - ys, 0,
                    xyc_ - zs,   y*yc_ + c,   yzc_ + xs, 0,
                    xzc_ + ys,   yzc_ - xs,   z*zc_ + c, 0,
                    0,           0,           0,         1};
    #endif
                  
    _GPU_MultiplyAndAssign(result, A);
}

void _GPU_MultMatrix(float* A)
{
    float* result = _GPU_GetCurrentMatrix();
    if(result == NULL)
        return;
    _GPU_MultiplyAndAssign(result, A);
}

void _GPU_GetModelViewProjection(float* result)
{
    // MVP = P * MV
    _GPU_Multiply4x4(result, _GPU_GetProjection(), _GPU_GetModelView());
}
