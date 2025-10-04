#pragma once
#include <math/Matrix4x4.h>
#include <math/Vector3.h>
#include <cmath>

struct Line {
	KamataEngine::Vector3 origin;
	KamataEngine::Vector3 diff;
};

struct Ray
{
	KamataEngine::Vector3 origin;
	KamataEngine::Vector3 diff;
};

struct Segment
{
	KamataEngine::Vector3 origin;
	KamataEngine::Vector3 diff;
};

struct  Plane {
	KamataEngine::Vector3 normal;
	float distance;
};

// アフィン変換行列の作成
KamataEngine::Matrix4x4 MakeAffineMatrix(const KamataEngine::Vector3& scale, const KamataEngine::Vector3& rot, const KamataEngine::Vector3& translate);
// 行列の掛け算
KamataEngine::Matrix4x4 MatrixMultiply(KamataEngine::Matrix4x4& m1, KamataEngine::Matrix4x4& m2);

float Dot(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);
float Length(const KamataEngine::Vector3& v);

class myMath {
public:
	static KamataEngine::Matrix4x4 Multiply(const KamataEngine::Matrix4x4& m1, const KamataEngine::Matrix4x4& m2);
	static KamataEngine::Vector3 Multiply(float scalar, const KamataEngine::Vector3& vector);
	static KamataEngine::Matrix4x4 MakeTranslateMatrix(const KamataEngine::Vector3& translate);
	static KamataEngine::Matrix4x4 MakeScaleMatrix(const KamataEngine::Vector3& scale);
	static KamataEngine::Matrix4x4 MakeRotateXMatrix(float radius);
	static KamataEngine::Matrix4x4 MakeRotateYMatrix(float radius);
	static KamataEngine::Matrix4x4 MakeRotateZMatrix(float radius);
	static KamataEngine::Matrix4x4 MakeAffineMatrix(const KamataEngine::Vector3& scale, const KamataEngine::Vector3& rotate, const KamataEngine::Vector3& translate);
	static KamataEngine::Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspecRatio, float nearClip, float farClip);
	static KamataEngine::Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);
	static KamataEngine::Matrix4x4 MakeViewportmatrix(float left, float top, float width, float height, float minDepth, float maxDepth);
	static KamataEngine::Matrix4x4 Inverse(const KamataEngine::Matrix4x4& m);
	static KamataEngine::Vector3 Transform(const KamataEngine::Vector3& vector, const KamataEngine::Matrix4x4& matrix);
	static KamataEngine::Vector3 TransformNormal(const KamataEngine::Vector3& vector, const KamataEngine::Matrix4x4& matrix);
	static KamataEngine::Vector3 Normalize(const KamataEngine::Vector3& v);
	// 加算
	static KamataEngine::Vector3 Add(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);
	//减算
	static KamataEngine::Vector3 Subtract(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);

	static float Length(const KamataEngine::Vector3& v);
};