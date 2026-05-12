#include "CharacterBase.h"

using namespace KamataEngine;

// キャラクター共通のワールド座標と球体コライダーを初期化する
void CharacterBase::Initialize() {
	// --- ワールド変換初期化 ---
	worldTransform_.Initialize();

	// --- コライダー生成・初期化 ---
	collider_ = std::make_unique<Collider>();
	collider_->Initialize();

	// 初期位置を同期
	collider_->SetTranslate(GetWorldTranslation());
}

// ワールド行列を更新し、現在位置をコライダーへ同期する
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

// ワールド行列から平行移動成分を取り出し、実際のワールド座標として返す
Vector3 CharacterBase::GetWorldTranslation() const {
	Vector3 result{};

	// ワールド行列から平行移動成分を抽出
	result.x = worldTransform_.matWorld_.m[3][0];
	result.y = worldTransform_.matWorld_.m[3][1];
	result.z = worldTransform_.matWorld_.m[3][2];

	return result;
}
