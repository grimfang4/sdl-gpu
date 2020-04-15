#include "SDL_gpu.h"
#include <math.h>
#include <string.h>

#ifdef _MSC_VER
#define __func__ __FUNCTION__
// Disable warning: selection for inlining
#pragma warning(disable: 4514 4711)
// Disable warning: Spectre mitigation
#pragma warning(disable: 5045)
#endif


#ifndef PI
#define PI 3.1415926f
#endif

#define RAD_PER_DEG 0.017453293f
#define DEG_PER_RAD 57.2957795f


// Visual C does not support static inline
#ifndef static_inline
	#ifdef _MSC_VER
		#define static_inline static
	#else
		#define static_inline static inline
	#endif
#endif

// Old Visual C did not support C99 (which includes a safe snprintf)
#if defined(_MSC_VER) && (_MSC_VER < 1900)
    #define snprintf c99_snprintf
    // From Valentin Milea: http://stackoverflow.com/questions/2915672/snprintf-and-visual-studio-2010
    static_inline int c99_vsnprintf(char* str, size_t size, const char* format, va_list ap)
    {
        int count = -1;

        if (size != 0)
            count = _vsnprintf_s(str, size, _TRUNCATE, format, ap);
        if (count == -1)
            count = _vscprintf(format, ap);

        return count;
    }

    static_inline int c99_snprintf(char* str, size_t size, const char* format, ...)
    {
        int count;
        va_list ap;

        va_start(ap, format);
        count = c99_vsnprintf(str, size, format, ap);
        va_end(ap);

        return count;
    }
#endif



GPU_MatrixStack* GPU_CreateMatrixStack(void)
{
    GPU_MatrixStack* stack = (GPU_MatrixStack*)SDL_malloc(sizeof(GPU_MatrixStack));
    stack->matrix = NULL;
    stack->size = 0;
    stack->storage_size = 0;
    GPU_InitMatrixStack(stack);
    return stack;
}


void GPU_FreeMatrixStack(GPU_MatrixStack* stack)
{
    GPU_ClearMatrixStack(stack);
    SDL_free(stack);
}

void GPU_InitMatrixStack(GPU_MatrixStack* stack)
{
    if(stack == NULL)
        return;
    
	if (stack->storage_size != 0)
		GPU_ClearMatrixStack(stack);

    stack->storage_size = 1;
    stack->size = 1;
    
    stack->matrix = (float**)SDL_malloc(sizeof(float*) * stack->storage_size);
    stack->matrix[0] = (float*)SDL_malloc(sizeof(float) * 16);
    GPU_MatrixIdentity(stack->matrix[0]);
}

void GPU_CopyMatrixStack(const GPU_MatrixStack* source, GPU_MatrixStack* dest)
{
	unsigned int i;
	unsigned int matrix_size = sizeof(float) * 16;
	if (source == NULL || dest == NULL)
		return;

	GPU_ClearMatrixStack(dest);
	dest->matrix = (float**)SDL_malloc(sizeof(float*) * source->storage_size);
	for (i = 0; i < source->storage_size; ++i)
	{
		dest->matrix[i] = (float*)SDL_malloc(matrix_size);
		memcpy(dest->matrix[i], source->matrix[i], matrix_size);
	}
	dest->storage_size = source->storage_size;
}

void GPU_ClearMatrixStack(GPU_MatrixStack* stack)
{
	unsigned int i;
	for (i = 0; i < stack->storage_size; ++i)
	{
		SDL_free(stack->matrix[i]);
	}
	SDL_free(stack->matrix);

	stack->matrix = NULL;
	stack->storage_size = 0;
}


void GPU_ResetProjection(GPU_Target* target)
{
    if(target == NULL)
        return;
    
    GPU_bool invert = (target->image != NULL);
    
    // Set up default projection
    float* projection_matrix = GPU_GetTopMatrix(&target->projection_matrix);
    GPU_MatrixIdentity(projection_matrix);
    
    if(!invert ^ GPU_GetCoordinateMode())
        GPU_MatrixOrtho(projection_matrix, 0, target->w, target->h, 0, target->camera.z_near, target->camera.z_far);
    else
        GPU_MatrixOrtho(projection_matrix, 0, target->w, 0, target->h, target->camera.z_near, target->camera.z_far);  // Special inverted orthographic projection because tex coords are inverted already for render-to-texture
}

// Column-major
#define INDEX(row,col) ((col)*4 + (row))


float GPU_VectorLength(const float* vec3)
{
	return sqrtf(vec3[0] * vec3[0] + vec3[1] * vec3[1] + vec3[2] * vec3[2]);
}

void GPU_VectorNormalize(float* vec3)
{
	float mag = GPU_VectorLength(vec3);
	vec3[0] /= mag;
	vec3[1] /= mag;
	vec3[2] /= mag;
}

float GPU_VectorDot(const float* A, const float* B)
{
	return A[0] * B[0] + A[1] * B[1] + A[2] * B[2];
}

void GPU_VectorCross(float* result, const float* A, const float* B)
{
	result[0] = A[1] * B[2] - A[2] * B[1];
	result[1] = A[2] * B[0] - A[0] * B[2];
	result[2] = A[0] * B[1] - A[1] * B[0];
}

void GPU_VectorCopy(float* result, const float* A)
{
	result[0] = A[0];
	result[1] = A[1];
	result[2] = A[2];
}

void GPU_VectorApplyMatrix(float* vec3, const float* matrix_4x4)
{
	float x = matrix_4x4[0] * vec3[0] + matrix_4x4[4] * vec3[1] + matrix_4x4[8] * vec3[2] + matrix_4x4[12];
	float y = matrix_4x4[1] * vec3[0] + matrix_4x4[5] * vec3[1] + matrix_4x4[9] * vec3[2] + matrix_4x4[13];
	float z = matrix_4x4[2] * vec3[0] + matrix_4x4[6] * vec3[1] + matrix_4x4[10] * vec3[2] + matrix_4x4[14];
	float w = matrix_4x4[3] * vec3[0] + matrix_4x4[7] * vec3[1] + matrix_4x4[11] * vec3[2] + matrix_4x4[15];
	vec3[0] = x / w;
	vec3[1] = y / w;
	vec3[2] = z / w;
}

void GPU_Vector4ApplyMatrix(float* vec4, const float* matrix_4x4)
{
	float x = matrix_4x4[0] * vec4[0] + matrix_4x4[4] * vec4[1] + matrix_4x4[8] * vec4[2] + matrix_4x4[12] * vec4[3];
	float y = matrix_4x4[1] * vec4[0] + matrix_4x4[5] * vec4[1] + matrix_4x4[9] * vec4[2] + matrix_4x4[13] * vec4[3];
	float z = matrix_4x4[2] * vec4[0] + matrix_4x4[6] * vec4[1] + matrix_4x4[10] * vec4[2] + matrix_4x4[14] * vec4[3];
	float w = matrix_4x4[3] * vec4[0] + matrix_4x4[7] * vec4[1] + matrix_4x4[11] * vec4[2] + matrix_4x4[15] * vec4[3];
	
    vec4[0] = x;
    vec4[1] = y;
    vec4[2] = z;
    vec4[3] = w;
	if(w != 0.0f)
    {
        vec4[0] = x / w;
        vec4[1] = y / w;
        vec4[2] = z / w;
        vec4[3] = 1;
    }
}


// Matrix math implementations based on Wayne Cochran's (wcochran) matrix.c

#define FILL_MATRIX_4x4(A, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15) \
	A[0] = a0; \
	A[1] = a1; \
	A[2] = a2; \
	A[3] = a3; \
	A[4] = a4; \
	A[5] = a5; \
	A[6] = a6; \
	A[7] = a7; \
	A[8] = a8; \
	A[9] = a9; \
	A[10] = a10; \
	A[11] = a11; \
	A[12] = a12; \
	A[13] = a13; \
	A[14] = a14; \
	A[15] = a15;

void GPU_MatrixCopy(float* result, const float* A)
{
    memcpy(result, A, 16*sizeof(float));
}

void GPU_MatrixIdentity(float* result)
{
    memset(result, 0, 16*sizeof(float));
    result[0] = result[5] = result[10] = result[15] = 1;
}


void GPU_MatrixOrtho(float* result, float left, float right, float bottom, float top, float z_near, float z_far)
{
    if(result == NULL)
		return;

	{
#ifdef ROW_MAJOR
		float A[16];
		FILL_MATRIX_4x4(A,
				2/(right - left), 0,  0, -(right + left)/(right - left),
				0, 2/(top - bottom), 0, -(top + bottom)/(top - bottom),
				0, 0, -2/(z_far - z_near), -(z_far + z_near)/(z_far - z_near),
				0, 0, 0, 1
			);
#else
		float A[16];
		FILL_MATRIX_4x4(A,
				2 / (right - left), 0, 0, 0,
				0, 2 / (top - bottom), 0, 0,
				0, 0, -2 / (z_far - z_near), 0,
				-(right + left) / (right - left), -(top + bottom) / (top - bottom), -(z_far + z_near) / (z_far - z_near), 1
			);
#endif

		GPU_MultiplyAndAssign(result, A);
	}
}


void GPU_MatrixFrustum(float* result, float left, float right, float bottom, float top, float z_near, float z_far)
{
    if(result == NULL)
		return;

	{
		float A[16];
		FILL_MATRIX_4x4(A, 
				2 * z_near / (right - left), 0, 0, 0,
				0, 2 * z_near / (top - bottom), 0, 0,
				(right + left) / (right - left), (top + bottom) / (top - bottom), -(z_far + z_near) / (z_far - z_near), -1,
				0, 0, -(2 * z_far * z_near) / (z_far - z_near), 0
			);

		GPU_MultiplyAndAssign(result, A);
	}
}

void GPU_MatrixPerspective(float* result, float fovy, float aspect, float z_near, float z_far)
{
	float fW, fH;
    
    // Make it left-handed?
    fovy = -fovy;
    aspect = -aspect;
    
	fH = tanf((fovy / 360) * PI) * z_near;
	fW = fH * aspect;
	GPU_MatrixFrustum(result, -fW, fW, -fH, fH, z_near, z_far);
}

void GPU_MatrixLookAt(float* matrix, float eye_x, float eye_y, float eye_z, float target_x, float target_y, float target_z, float up_x, float up_y, float up_z)
{
	float forward[3] = {target_x - eye_x, target_y - eye_y, target_z - eye_z};
	float up[3] = {up_x, up_y, up_z};
	float side[3];
	float view[16];

	GPU_VectorNormalize(forward);
	GPU_VectorNormalize(up);

	// Calculate sideways vector
	GPU_VectorCross(side, forward, up);

	// Calculate new up vector
	GPU_VectorCross(up, side, forward);

	// Set up view matrix
	view[0] = side[0];
	view[4] = side[1];
	view[8] = side[2];
	view[12] = 0.0f;

	view[1] = up[0];
	view[5] = up[1];
	view[9] = up[2];
	view[13] = 0.0f;

	view[2] = -forward[0];
	view[6] = -forward[1];
	view[10] = -forward[2];
	view[14] = 0.0f;

	view[3] = view[7] = view[11] = 0.0f;
	view[15] = 1.0f;

	GPU_MultiplyAndAssign(matrix, view);
	GPU_MatrixTranslate(matrix, -eye_x, -eye_y, -eye_z);
}

void GPU_MatrixTranslate(float* result, float x, float y, float z)
{
    if(result == NULL)
		return;

	{
#ifdef ROW_MAJOR
		float A[16];
		FILL_MATRIX_4x4(A,
				1, 0, 0, x,
				0, 1, 0, y,
				0, 0, 1, z,
				0, 0, 0, 1
			);
#else
		float A[16];
		FILL_MATRIX_4x4(A,
				1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				x, y, z, 1
			);
#endif

		GPU_MultiplyAndAssign(result, A);
	}
}

void GPU_MatrixScale(float* result, float sx, float sy, float sz)
{
    if(result == NULL)
		return;

	{
		float A[16];
		FILL_MATRIX_4x4(A,
				sx, 0, 0, 0,
				0, sy, 0, 0,
				0, 0, sz, 0,
				0, 0, 0, 1
			);

		GPU_MultiplyAndAssign(result, A);
	}
}

void GPU_MatrixRotate(float* result, float degrees, float x, float y, float z)
{
	float p, radians, c, s, c_, zc_, yc_, xzc_, xyc_, yzc_, xs, ys, zs;

    if(result == NULL)
		return;

    p = 1/sqrtf(x*x + y*y + z*z);
    x *= p; y *= p; z *= p;
    radians = degrees * RAD_PER_DEG;
    c = cosf(radians);
    s = sinf(radians);
    c_ = 1 - c;
    zc_ = z*c_;
    yc_ = y*c_;
    xzc_ = x*zc_;
    xyc_ = x*y*c_;
    yzc_ = y*zc_;
    xs = x*s;
    ys = y*s;
    zs = z*s;

	{
#ifdef ROW_MAJOR
		float A[16];
		FILL_MATRIX_4x4(A,
				x*x*c_ + c,  xyc_ - zs,   xzc_ + ys, 0,
				xyc_ + zs,   y*yc_ + c,   yzc_ - xs, 0,
				xzc_ - ys,   yzc_ + xs,   z*zc_ + c, 0,
				0,           0,           0,         1
			);
#else
		float A[16];
		FILL_MATRIX_4x4(A,
				x*x*c_ + c, xyc_ + zs, xzc_ - ys, 0,
				xyc_ - zs, y*yc_ + c, yzc_ + xs, 0,
				xzc_ + ys, yzc_ - xs, z*zc_ + c, 0,
				0, 0, 0, 1
			);
#endif

		GPU_MultiplyAndAssign(result, A);
	}
}

// Matrix multiply: result = A * B
void GPU_MatrixMultiply(float* result, const float* A, const float* B)
{
    float (*matR)[4] = (float(*)[4])result;
    float (*matA)[4] = (float(*)[4])A;
    float (*matB)[4] = (float(*)[4])B;
    matR[0][0] = matB[0][0] * matA[0][0] + matB[0][1] * matA[1][0] + matB[0][2] * matA[2][0] + matB[0][3] * matA[3][0]; 
    matR[0][1] = matB[0][0] * matA[0][1] + matB[0][1] * matA[1][1] + matB[0][2] * matA[2][1] + matB[0][3] * matA[3][1]; 
    matR[0][2] = matB[0][0] * matA[0][2] + matB[0][1] * matA[1][2] + matB[0][2] * matA[2][2] + matB[0][3] * matA[3][2]; 
    matR[0][3] = matB[0][0] * matA[0][3] + matB[0][1] * matA[1][3] + matB[0][2] * matA[2][3] + matB[0][3] * matA[3][3]; 
    matR[1][0] = matB[1][0] * matA[0][0] + matB[1][1] * matA[1][0] + matB[1][2] * matA[2][0] + matB[1][3] * matA[3][0]; 
    matR[1][1] = matB[1][0] * matA[0][1] + matB[1][1] * matA[1][1] + matB[1][2] * matA[2][1] + matB[1][3] * matA[3][1]; 
    matR[1][2] = matB[1][0] * matA[0][2] + matB[1][1] * matA[1][2] + matB[1][2] * matA[2][2] + matB[1][3] * matA[3][2]; 
    matR[1][3] = matB[1][0] * matA[0][3] + matB[1][1] * matA[1][3] + matB[1][2] * matA[2][3] + matB[1][3] * matA[3][3]; 
    matR[2][0] = matB[2][0] * matA[0][0] + matB[2][1] * matA[1][0] + matB[2][2] * matA[2][0] + matB[2][3] * matA[3][0]; 
    matR[2][1] = matB[2][0] * matA[0][1] + matB[2][1] * matA[1][1] + matB[2][2] * matA[2][1] + matB[2][3] * matA[3][1]; 
    matR[2][2] = matB[2][0] * matA[0][2] + matB[2][1] * matA[1][2] + matB[2][2] * matA[2][2] + matB[2][3] * matA[3][2]; 
    matR[2][3] = matB[2][0] * matA[0][3] + matB[2][1] * matA[1][3] + matB[2][2] * matA[2][3] + matB[2][3] * matA[3][3]; 
    matR[3][0] = matB[3][0] * matA[0][0] + matB[3][1] * matA[1][0] + matB[3][2] * matA[2][0] + matB[3][3] * matA[3][0]; 
    matR[3][1] = matB[3][0] * matA[0][1] + matB[3][1] * matA[1][1] + matB[3][2] * matA[2][1] + matB[3][3] * matA[3][1]; 
    matR[3][2] = matB[3][0] * matA[0][2] + matB[3][1] * matA[1][2] + matB[3][2] * matA[2][2] + matB[3][3] * matA[3][2]; 
    matR[3][3] = matB[3][0] * matA[0][3] + matB[3][1] * matA[1][3] + matB[3][2] * matA[2][3] + matB[3][3] * matA[3][3];
}

void GPU_MultiplyAndAssign(float* result, const float* B)
{
    float temp[16];
    GPU_MatrixMultiply(temp, result, B);
    GPU_MatrixCopy(result, temp);
}





// Can be used up to two times per line evaluation...
const char* GPU_GetMatrixString(const float* A)
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

void GPU_MatrixMode(GPU_Target* target, int matrix_mode)
{
    GPU_Target* context_target;
    if(target == NULL)
        return;
    
    GPU_FlushBlitBuffer();
    target->matrix_mode = matrix_mode;
    
    context_target = GPU_GetContextTarget();
    if(context_target != NULL && context_target == target->context_target)
        context_target->context->active_target = target;
}

float* GPU_GetModel(void)
{
    GPU_Target* target = GPU_GetActiveTarget();
    if(target == NULL)
        return NULL;
    return GPU_GetTopMatrix(&target->model_matrix);
}

float* GPU_GetView(void)
{
    GPU_Target* target = GPU_GetActiveTarget();
    if(target == NULL)
        return NULL;
    return GPU_GetTopMatrix(&target->view_matrix);
}

float* GPU_GetProjection(void)
{
    GPU_Target* target = GPU_GetActiveTarget();
    if(target == NULL)
        return NULL;
    return GPU_GetTopMatrix(&target->projection_matrix);
}

float* GPU_GetCurrentMatrix(void)
{
    GPU_MatrixStack* stack;
    GPU_Target* target = GPU_GetActiveTarget();
    if(target == NULL)
        return NULL;
    if(target->matrix_mode == GPU_MODEL)
        stack = &target->model_matrix;
    else if(target->matrix_mode == GPU_VIEW)
        stack = &target->view_matrix;
    else// if(target->matrix_mode == GPU_PROJECTION)
        stack = &target->projection_matrix;
    
    return GPU_GetTopMatrix(stack);
}

void GPU_PushMatrix(void)
{
	GPU_MatrixStack* stack;
    GPU_Target* target = GPU_GetActiveTarget();
    if(target == NULL)
        return;
    
    if(target->matrix_mode == GPU_MODEL)
        stack = &target->model_matrix;
    else if(target->matrix_mode == GPU_VIEW)
        stack = &target->view_matrix;
    else// if(target->matrix_mode == GPU_PROJECTION)
        stack = &target->projection_matrix;
    
    if(stack->size + 1 >= stack->storage_size)
    {
        // Grow matrix stack (1, 6, 16, 36, ...)
        
        // Alloc new one
        unsigned int new_storage_size = stack->storage_size*2 + 4;
        float** new_stack = (float**)SDL_malloc(sizeof(float*) * new_storage_size);
        unsigned int i;
        for(i = 0; i < new_storage_size; ++i)
        {
            new_stack[i] = (float*)SDL_malloc(sizeof(float) * 16);
        }
        // Copy old one
        for(i = 0; i < stack->size; ++i)
        {
            GPU_MatrixCopy(new_stack[i], stack->matrix[i]);
        }
        // Free old one
        for(i = 0; i < stack->storage_size; ++i)
        {
            SDL_free(stack->matrix[i]);
        }
        SDL_free(stack->matrix);
        
        // Switch to new one
        stack->storage_size = new_storage_size;
        stack->matrix = new_stack;
    }
    GPU_MatrixCopy(stack->matrix[stack->size], stack->matrix[stack->size-1]);
    stack->size++;
}

void GPU_PopMatrix(void)
{
	GPU_MatrixStack* stack;

    GPU_Target* target = GPU_GetActiveTarget();
    if(target == NULL)
        return;
    
	// FIXME: Flushing here is not always necessary if this isn't the last target
	GPU_FlushBlitBuffer();
	
    if(target->matrix_mode == GPU_MODEL)
        stack = &target->model_matrix;
    else if(target->matrix_mode == GPU_VIEW)
        stack = &target->view_matrix;
    else //if(target->matrix_mode == GPU_PROJECTION)
        stack = &target->projection_matrix;
        
    if(stack->size == 0)
    {
        GPU_PushErrorCode(__func__, GPU_ERROR_USER_ERROR, "Matrix stack is empty.");
    }
    else if(stack->size == 1)
    {
        GPU_PushErrorCode(__func__, GPU_ERROR_USER_ERROR, "Matrix stack would become empty!");
    }
    else
        stack->size--;
}

void GPU_SetProjection(const float* A)
{
    GPU_Target* target = GPU_GetActiveTarget();
    if(target == NULL || A == NULL)
        return;
    
	GPU_FlushBlitBuffer();
    GPU_MatrixCopy(GPU_GetProjection(), A);
}

void GPU_SetModel(const float* A)
{
    GPU_Target* target = GPU_GetActiveTarget();
    if(target == NULL || A == NULL)
        return;
    
	GPU_FlushBlitBuffer();
    GPU_MatrixCopy(GPU_GetModel(), A);
}

void GPU_SetView(const float* A)
{
    GPU_Target* target = GPU_GetActiveTarget();
    if(target == NULL || A == NULL)
        return;
    
	GPU_FlushBlitBuffer();
    GPU_MatrixCopy(GPU_GetView(), A);
}

void GPU_SetProjectionFromStack(GPU_MatrixStack* stack)
{
    GPU_Target* target = GPU_GetActiveTarget();
    if(target == NULL || stack == NULL)
        return;
    
    GPU_SetProjection(GPU_GetTopMatrix(stack));
}

void GPU_SetModelFromStack(GPU_MatrixStack* stack)
{
    GPU_Target* target = GPU_GetActiveTarget();
    if(target == NULL || stack == NULL)
        return;
    
    GPU_SetModel(GPU_GetTopMatrix(stack));
}

void GPU_SetViewFromStack(GPU_MatrixStack* stack)
{
    GPU_Target* target = GPU_GetActiveTarget();
    if(target == NULL || stack == NULL)
        return;
    
    GPU_SetView(GPU_GetTopMatrix(stack));
}

float* GPU_GetTopMatrix(GPU_MatrixStack* stack)
{
    if(stack == NULL || stack->size == 0)
        return NULL;
    return stack->matrix[stack->size-1];
}

void GPU_LoadIdentity(void)
{
    float* result = GPU_GetCurrentMatrix();
    if(result == NULL)
		return;
    
	GPU_FlushBlitBuffer();
    GPU_MatrixIdentity(result);
}

void GPU_LoadMatrix(const float* A)
{
    float* result = GPU_GetCurrentMatrix();
    if(result == NULL)
        return;
	GPU_FlushBlitBuffer();
    GPU_MatrixCopy(result, A);
}

void GPU_Ortho(float left, float right, float bottom, float top, float z_near, float z_far)
{
	GPU_FlushBlitBuffer();
    GPU_MatrixOrtho(GPU_GetCurrentMatrix(), left, right, bottom, top, z_near, z_far);
}

void GPU_Frustum(float left, float right, float bottom, float top, float z_near, float z_far)
{
	GPU_FlushBlitBuffer();
    GPU_MatrixFrustum(GPU_GetCurrentMatrix(), left, right, bottom, top, z_near, z_far);
}

void GPU_Perspective(float fovy, float aspect, float z_near, float z_far)
{
	GPU_FlushBlitBuffer();
    GPU_MatrixPerspective(GPU_GetCurrentMatrix(), fovy, aspect, z_near, z_far);
}

void GPU_LookAt(float eye_x, float eye_y, float eye_z, float target_x, float target_y, float target_z, float up_x, float up_y, float up_z)
{
	GPU_FlushBlitBuffer();
    GPU_MatrixLookAt(GPU_GetCurrentMatrix(), eye_x, eye_y, eye_z, target_x, target_y, target_z, up_x, up_y, up_z);
}


void GPU_Translate(float x, float y, float z)
{
	GPU_FlushBlitBuffer();
    GPU_MatrixTranslate(GPU_GetCurrentMatrix(), x, y, z);
}

void GPU_Scale(float sx, float sy, float sz)
{
	GPU_FlushBlitBuffer();
    GPU_MatrixScale(GPU_GetCurrentMatrix(), sx, sy, sz);
}

void GPU_Rotate(float degrees, float x, float y, float z)
{
	GPU_FlushBlitBuffer();
    GPU_MatrixRotate(GPU_GetCurrentMatrix(), degrees, x, y, z);
}

void GPU_MultMatrix(const float* A)
{
    float* result = GPU_GetCurrentMatrix();
    if(result == NULL)
        return;
	GPU_FlushBlitBuffer();
    GPU_MultiplyAndAssign(result, A);
}

void GPU_GetModelViewProjection(float* result)
{
    // MVP = P * V * M
    GPU_MatrixMultiply(result, GPU_GetProjection(), GPU_GetView());
    GPU_MultiplyAndAssign(result, GPU_GetModel());
}
