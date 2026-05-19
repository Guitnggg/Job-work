#include "Laser.h"

#include <cmath>

using namespace KamataEngine;

void Laser::Initialize() {
	// 基底クラス初期化
	CharacterBase::Initialize();

	// モデル生成（レーザー用）
	model_.reset(Model::CreateFromOBJ("Beam", true));
	objectColor_.Initialize();
	objectColor_.SetColor({ 0.50f, 0.95f, 1.0f, 0.95f });

	// 細長いスケール
	worldTransform_.scale_ = kLaserScale;
	worldTransform_.UpdateMatrix();

	// コライダー設定
	if (collider_) {
		collider_->SetRadius(kColliderRadius);
		collider_->SetTranslate(GetWorldTranslation());
		collider_->Update();
	}

	// 状態初期化
	isDead_ = false;
	elapsedTimeSec_ = 0.0f;
}

void Laser::FireFrom(const Vector3& worldPos, const Vector3& dir) {
	// 発射位置・方向設定
	worldTransform_.translation_ = worldPos;
	dir_ = dir;

	// 距離計測の基準点
	startPos_ = worldPos;

	// 姿勢初期化
	worldTransform_.rotation_ = kZeroRotation;
	worldTransform_.UpdateMatrix();

	// 状態リセット
	elapsedTimeSec_ = 0.0f;
	isDead_ = false;

	// コライダー同期
	if (collider_) {
		collider_->SetTranslate(GetWorldTranslation());
		collider_->Update();
	}
}

void Laser::Update() {
	if (isDead_) {
		return;
	}

	// 固定Δt
	elapsedTimeSec_ += kFixedDeltaTime;

	// 前進
	worldTransform_.translation_.x += dir_.x * speed_;
	worldTransform_.translation_.y += dir_.y * speed_;
	worldTransform_.translation_.z += dir_.z * speed_;

	worldTransform_.UpdateMatrix();

	// コライダー追従
	if (collider_) {
		collider_->SetTranslate(GetWorldTranslation());
		collider_->Update();
	}

	// 距離判定
	const Vector3 pos = GetWorldTranslation();
	const Vector3 diff{pos.x - startPos_.x, pos.y - startPos_.y, pos.z - startPos_.z};

	const float distanceSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

	if (distanceSq > kMaxDistance * kMaxDistance) {
		isDead_ = true;
	}

	// 寿命判定
	if (elapsedTimeSec_ >= lifeTimeSec_) {
		isDead_ = true;
	}
}

void Laser::Draw(const Camera* camera) {
	if (!camera || isDead_) {
		return;
	}

	if (model_) {
		model_->Draw(worldTransform_, *camera, textureHandle_, &objectColor_);
	}
}

void Laser::OnCollision(CharacterBase* /*other*/) {
	// デフォルトは貫通レーザー
	// 即消滅にしたい場合は下を有効化
	// isDead_ = true;
}
