#pragma once

#include "math/Vector3.h"
#include "Application/Utility/MyMath/MyMath.h"

class CameraShake {
public:
	/// <summary>
	///
	/// </summary>
	/// <param name="direction"></param>
	/// <param name="power"></param>
	void AddShake(const KamataEngine::Vector3& direction, float power);

	/// <summary>
	///
	/// </summary>
	void Update();

	/// <summary>
	///
	/// </summary>
	void Reset();

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	const KamataEngine::Vector3& GetOffset() const { return offset_; }

private:

	KamataEngine::Vector3 offset_{};  // カメラオフセット
	KamataEngine::Vector3 velocity_{};  // カメラ速度

	float decay_ = 0.85f;  // 減衰率
};