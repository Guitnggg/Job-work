#include "CameraShake.h"

using namespace KamataEngine;

void CameraShake::AddShake(const KamataEngine::Vector3& direction, float power) {
	// 方向ベクトルの長さが極端に小さい場合は無視
	if (MyMath::Length(direction) < 0.0001f) {
		return;
	}

	// 正規化してから力を加える
	Vector3 dir = MyMath::Normalize(direction);
	velocity_ = MyMath::Add(velocity_, MyMath::Multiply(dir, power));
}

void CameraShake::Update() {
	// 速度を加算
	offset_ = MyMath::Add(offset_, velocity_);
	// 減衰
	velocity_ = MyMath::Multiply(velocity_, decay_);
	// オフセットも減衰
	offset_ = MyMath::Multiply(offset_, decay_);
}

void CameraShake::Reset() {
	// リセット
	offset_ = {0, 0, 0};
	velocity_ = {0, 0, 0};
}