#include "DamageParticle.h"

using namespace KamataEngine;

void DamageParticle::Initialize(Model* model, const Vector3& pos, const Vector3& vel, float lifeTime, float startScale, float endScale) {
	// ===== パラメータ受け取り =====
	model_ = model;       // モデルは外部所有
	velocity_ = vel;      // 飛散速度
	lifeTime_ = lifeTime; // 寿命
	startScale_ = startScale;
	endScale_ = endScale;

	// ===== ワールド変換初期化 =====
	worldTransform_.Initialize();
	worldTransform_.translation_ = pos;

	// 初期スケールを明示的に設定（初回フレームの安全性向上）
	worldTransform_.scale_ = {startScale_, startScale_, startScale_};

	// ===== タイマー初期化 =====
	currentTime_ = 0.0f;

	worldTransform_.UpdateMatrix();
}

void DamageParticle::Update(float dt) {
	// 既に寿命が尽きている場合は何もしない
	if (IsFinished()) {
		return;
	}

	// ===== 時間進行 =====
	currentTime_ += dt;

	// ===== 速度による移動 =====
	worldTransform_.translation_.x += velocity_.x * dt;
	worldTransform_.translation_.y += velocity_.y * dt;
	worldTransform_.translation_.z += velocity_.z * dt;

	// ===== スケール補間 =====
	// 寿命0対策（念のため）
	float t = 1.0f;
	if (lifeTime_ > 0.0f) {
		// 0.0 ～ 1.0 にクランプ
		t = (std::min)(currentTime_ / lifeTime_, 1.0f);
	}

	const float scale = startScale_ + (endScale_ - startScale_) * t;
	worldTransform_.scale_ = {scale, scale, scale};

	// ===== 行列更新 =====
	worldTransform_.UpdateMatrix();
}

void DamageParticle::Draw(const Camera* camera) {
	// 無効な状態では描画しない
	if (!camera || !model_ || IsFinished()) {
		return;
	}

	// モデル描画
	model_->Draw(worldTransform_, *camera);
}
