#include "Smoke.h"

using namespace KamataEngine;

void Smoke::Initialize(Model* model, const Vector3& position, const Vector3& velocity, float lifeTime, float startScale, float endScale) {
	// ===== パラメータ設定 =====
	model_ = model;          // 非所有
	velocity_ = velocity;    // 移動速度
	lifeTimeSec_ = lifeTime; // 寿命
	startScale_ = startScale;
	endScale_ = endScale;

	elapsedTimeSec_ = 0.0f;
	isFinished_ = false;

	// ===== ワールド変換初期化 =====
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};
	worldTransform_.scale_ = {startScale_, startScale_, startScale_};

	worldTransform_.UpdateMatrix();
}

void Smoke::Update(float dt) {
	// 終了している場合は更新しない
	if (isFinished_) {
		return;
	}

	// ===== 時間進行 =====
	elapsedTimeSec_ += dt;
	if (elapsedTimeSec_ >= lifeTimeSec_) {
		isFinished_ = true;
		return;
	}

	// ===== 移動（dt対応）=====
	worldTransform_.translation_.x += velocity_.x * dt;
	worldTransform_.translation_.y += velocity_.y * dt;
	worldTransform_.translation_.z += velocity_.z * dt;

	// ===== スケール補間 =====
	float t = 1.0f;
	if (lifeTimeSec_ > 0.0f) {
		t = elapsedTimeSec_ / lifeTimeSec_;
		if (t < 0.0f)
			t = 0.0f;
		if (t > 1.0f)
			t = 1.0f;
	}

	const float scale = startScale_ + (endScale_ - startScale_) * t;
	worldTransform_.scale_ = {(scale < 0.0f) ? 0.0f : scale, (scale < 0.0f) ? 0.0f : scale, (scale < 0.0f) ? 0.0f : scale};

	// ===== 行列更新 =====
	worldTransform_.UpdateMatrix();
}

void Smoke::Draw(const Camera* camera) {
	// 無効な状態では描画しない
	if (isFinished_ || !camera || !model_) {
		return;
	}

	model_->Draw(worldTransform_, *camera);
}
