#include "CameraShake.h"

#include "Application/Utility/MyMath/MyMath.h"

using namespace KamataEngine;

void CameraShake::AddShake(const Vector3& direction, float power) {
	// 極端に小さい方向ベクトルはノイズになるため無視する
	if (MyMath::Length(direction) < kMinDirectionLength) {
		return;
	}

	// 方向のみを利用するため正規化してから力を加える
	const Vector3 normalizedDir = MyMath::Normalize(direction);
	velocity_ = MyMath::Add(velocity_, MyMath::Multiply(normalizedDir, power));
}

void CameraShake::Update() {
	// 速度を位置に反映
	offset_ = MyMath::Add(offset_, velocity_);

	// 速度を減衰させ、揺れが徐々に収束するようにする
	velocity_ = MyMath::Multiply(velocity_, kDecayRate);

	// オフセット自体も減衰させ、残り揺れを抑える
	offset_ = MyMath::Multiply(offset_, kDecayRate);
}

void CameraShake::Reset() {
	// 揺れ状態を完全に初期化する
	offset_ = kZeroVector;
	velocity_ = kZeroVector;
}