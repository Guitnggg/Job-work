#include "CharacterBase.h"

using namespace KamataEngine;

void CharacterBase::Initialize() {
	// --- ワールド変換初期化 ---
	worldTransform_.Initialize();

	// --- コライダー生成・初期化 ---
	collider_ = std::make_unique<Collider>();
	collider_->Initialize();

	// 初期位置を同期
	collider_->SetTranslate(GetWorldTranslation());
}

void CharacterBase::Update() {
	// --- ワールド行列更新 ---
	worldTransform_.UpdateMatrix();

	// --- コライダー同期 ---
	// 行列計算後のワールド座標を使用することで、
	// 親子付けされた移動も正しく反映させる
	collider_->SetTranslate(GetWorldTranslation());

	// --- コライダー更新 ---
	collider_->Update();
}

Vector3 CharacterBase::GetWorldTranslation() const {
	Vector3 result{};

	// ワールド行列から平行移動成分を抽出
	result.x = worldTransform_.matWorld_.m[3][0];
	result.y = worldTransform_.matWorld_.m[3][1];
	result.z = worldTransform_.matWorld_.m[3][2];

	return result;
}
