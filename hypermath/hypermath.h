/*
Copyright (c) 2014, Alexander Charlton
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef HYPERMATH
#define HYPERMATH 1

typedef struct {
    float _11, _21, _31, _41,
          _12, _22, _32, _42,
	  _13, _23, _33, _43,
	  _14, _24, _34, _44;
} HPMmat4;

float hpmDegreesToRadians(float deg);

float hpmRadiansToDegrees(float rad);

void hpmMultMat4(const float *matA, const float *matB, float *result);

void hpmPrintMat4(const float *m);

void hpmIdentityMat4(float *m);

void hpmTranslation(float x, float y, float z, float *mat);

void hpmTranslate(float x, float y, float z, float *mat);

void hpmXRotation(float rotation, float *mat);

void hpmRotateX(float rotation, float *mat);

void hpmYRotation(float rotation, float *mat);

void hpmRotateY(float rotation, float *mat);

void hpmZRotation(float rotation, float *mat);

void hpmRotateZ(float rotation, float *mat);

void hpmRotation(float x, float y, float z, float angle, float *mat);

void hpmRotate(float x, float y, float z, float angle, float *mat);

void hpmQuaternionRotation(float x, float y, float z, float w, float *mat);

void hpmRotateQuaternion(float x, float y, float z, float w, float *mat);

void hpmYPRRotation(float yaw, float pitch, float roll, float *mat);

void hpmRotateYPR(float yaw, float pitch, float roll, float *mat);

void hpm2DScaling(float scaleX, float scaleY, float *mat);

void hpmScale2D(float scaleX, float scaleY, float *mat);

void hpm3DScaling(float scaleX, float scaleY, float scaleZ, float *mat);

void hpmScale3D(float scaleX, float scaleY, float scaleZ, float *mat);

void hpmScaling(float factor, float *mat);

void hpmScale(float factor, float *mat);

void hpmFlipX(float *mat);

void hpmFlipY(float *mat);

void hpmFlipZ(float *mat);

void hpmTranslateRotateScale2D(float x, float y, float z, float angle, float scale,
                               float *mat);

void hpmTranspose(const float *mat, float *result);

void hpmInverse(const float *mat, float *result);

// Vector operations
void hpmCross(float ax, float ay, float az, float bx, float by, float bz, float *rx, float *ry, float *rz);

void hpmNormalize(float x, float y, float z, float *rx, float *ry, float *rz);

float hpmDot(float ax, float ay, float az, float bx, float by, float bz);

void hpmMat4VecMult(const float *mat, float *vec);

void hpmMat4VecArrayMult(const float *mat, float *vec, size_t length, size_t stride);

// Projection
void hpmOrtho(int width, int height, float near, float far, float *mat);

void hpmOrthoFloat(float width, float height, float near, float far, float *mat);

void hpmFrustum(float left, float right, float bottom, float top,
		float near, float far, float *mat);

void hpmPerspective(int width, int height, float near, float far, float angle,
		    float *mat);

// Camera
void hpmLookAt(float eyeX, float eyeY, float eyeZ, float x, float y, float z, float upX, float upY, float upZ, float *mat);

void hpmCameraInverse(const float *camera, float *inverse);

#endif
