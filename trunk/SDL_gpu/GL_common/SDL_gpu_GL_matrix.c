#include "SDL_gpu.h"
#include "SDL_gpu_GL_matrix.h"
#include <math.h>
#include <string.h>


#ifndef M_PI
#define M_PI 3.1415926f
#endif



// Implementations based on Wayne Cochran's (wcochran) matrix.c

// Column-major
#define INDEX(row,col) ((col)*4 + (row))

void GPU_MatrixCopy(float* result, const float* A)
{
    memcpy(result, A, 16*sizeof(float));
}

void GPU_MatrixIdentity(float* result)
{
    memset(result, 0, 16*sizeof(float));
    result[0] = result[5] = result[10] = result[15] = 1;
}

// Matrix multiply: result = A * B
void GPU_Multiply4x4(float* result, float* A, float* B)
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

void GPU_MultiplyAndAssign(float* result, float* A)
{
    float temp[16];
    GPU_Multiply4x4(temp, result, A);
    GPU_MatrixCopy(result, temp);
}





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

void GPU_MatrixMode(int matrix_mode)
{
    GPU_Target* target = GPU_GetContextTarget();
    if(target == NULL || target->context == NULL)
        return;
    
    target->context->matrix_mode = matrix_mode;
}

float* GPU_GetModelView(void)
{
    GPU_Target* target = GPU_GetContextTarget();
    if(target == NULL || target->context == NULL)
        return NULL;
    GPU_MatrixStack* stack = &target->context->modelview_matrix;
    if(stack->size == 0)
        return NULL;
    return stack->matrix[stack->size-1];
}

float* GPU_GetProjection(void)
{
    GPU_Target* target = GPU_GetContextTarget();
    if(target == NULL || target->context == NULL)
        return NULL;
    GPU_MatrixStack* stack = &target->context->projection_matrix;
    if(stack->size == 0)
        return NULL;
    return stack->matrix[stack->size-1];
}

float* GPU_GetCurrentMatrix(void)
{
    GPU_Target* target = GPU_GetContextTarget();
    if(target == NULL || target->context == NULL)
        return NULL;
    GPU_MatrixStack* stack;
    if(target->context->matrix_mode == GPU_MODELVIEW)
        stack = &target->context->modelview_matrix;
    else
        stack = &target->context->projection_matrix;
    
    if(stack->size == 0)
        return NULL;
    return stack->matrix[stack->size-1];
}

void GPU_PushMatrix()
{
    GPU_Target* target = GPU_GetContextTarget();
    if(target == NULL || target->context == NULL)
        return;
    
    GPU_MatrixStack* stack = (target->context->matrix_mode == GPU_MODELVIEW? &target->context->modelview_matrix : &target->context->projection_matrix);
    if(stack->size + 1 >= GPU_MATRIX_STACK_MAX)
    {
        GPU_PushErrorCode(__func__, GPU_ERROR_USER_ERROR, "Matrix stack is full (%d)", GPU_MATRIX_STACK_MAX);
        return;
    }
    GPU_MatrixCopy(stack->matrix[stack->size], stack->matrix[stack->size-1]);
    stack->size++;
}

void GPU_PopMatrix()
{
    GPU_Target* target = GPU_GetContextTarget();
    if(target == NULL || target->context == NULL)
        return;
    
    GPU_MatrixStack* stack = (target->context->matrix_mode == GPU_MODELVIEW? &target->context->modelview_matrix : &target->context->projection_matrix);
    if(stack->size == 0)
    {
        GPU_PushErrorCode(__func__, GPU_ERROR_USER_ERROR, "Matrix stack is empty.");
        return;
    }
    stack->size--;
}

void GPU_LoadIdentity(void)
{
    float* result = GPU_GetCurrentMatrix();
    if(result == NULL)
        return;
    GPU_MatrixIdentity(result);
}

void GPU_Ortho(float left, float right, float bottom, float top, float near, float far)
{
    float* result = GPU_GetCurrentMatrix();
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
    
    GPU_MultiplyAndAssign(result, A);
}

void GPU_Frustum(float right, float left, float bottom, float top, float near, float far)
{
    float* result = GPU_GetCurrentMatrix();
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
                   
    GPU_MultiplyAndAssign(result, A);
}

void GPU_Translate(float x, float y, float z)
{
    float* result = GPU_GetCurrentMatrix();
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
    
    GPU_MultiplyAndAssign(result, A);
}

void GPU_Scale(float sx, float sy, float sz)
{
    float* result = GPU_GetCurrentMatrix();
    if(result == NULL)
        return;
    float A[16] = {sx, 0,  0,  0,
                    0,  sy, 0,  0,
                    0,  0,  sz, 0,
                    0,  0,  0,  1};
                  
    GPU_MultiplyAndAssign(result, A);
}

void GPU_Rotate(float degrees, float x, float y, float z)
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

    float* result = GPU_GetCurrentMatrix();
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
                  
    GPU_MultiplyAndAssign(result, A);
}

void GPU_MultMatrix(float* A)
{
    float* result = GPU_GetCurrentMatrix();
    if(result == NULL)
        return;
    GPU_MultiplyAndAssign(result, A);
}

void GPU_GetModelViewProjection(float* result)
{
    // MVP = P * MV
    GPU_Multiply4x4(result, GPU_GetProjection(), GPU_GetModelView());
}
