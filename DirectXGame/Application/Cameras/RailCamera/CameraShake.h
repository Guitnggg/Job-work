#pragma once

#include "math/Vector3.h"
#include "Application/Utility/MyMath/MyMath.h"

class CameraShake {
public:
	/// <summary>
	/// カメラに揺れを追加する
	/// </summary>
	/// <param name="direction">揺れの方向</param>
	/// <param name="power">揺れの強さ</param>
	void AddShake(const KamataEngine::Vector3& direction, float power);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// リセット
	/// </summary>
	void Reset();

	/// <summary>
	/// オフセット
	/// </summary>
	/// <returns></returns>
	const KamataEngine::Vector3& GetOffset() const { return offset_; }

private:

	KamataEngine::Vector3 offset_{};  // カメラオフセット
	KamataEngine::Vector3 velocity_{};  // カメラ速度

	float decay_ = 0.85f;  // 減衰率
};