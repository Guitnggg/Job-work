#include "TurretEnemy.h"
#include <algorithm>
#include <cmath>

using namespace KamataEngine;

namespace {
// 固定Δt（他クラスに合わせて 60fps 想定）
constexpr float kFixedDeltaTime = 1.0f / 60.0f;

// 見た目（砲台のサイズ）
constexpr Vector3 kTurretScale{1.2f, 1.2f, 1.2f};

// 砲台の発射位置オフセット（砲身っぽく前に出す）
constexpr float kMuzzleForward = 1.0f;
} // namespace

void TurretEnemy::Initialize() {
	// 親の初期化（WT/Colliderなど）
	CharacterBase::Initialize();

	// モデル（とりあえず球。専用モデルがあるなら差し替え可）
	model_.reset(Model::CreateSphere());

	// HP
	hp_ = initialHP_;
	maxHp_ = initialHP_;

	// 見た目スケール
	worldTransform_.scale_ = kTurretScale;
	worldTransform_.UpdateMatrix();

	// コライダー
	if (collider_) {
		collider_->SetRadius(colliderRadius_);
		collider_->SetTranslate(GetWorldTranslation());
		collider_->Update();
	}

	// 弾コンテナ初期化
	bullets_.clear();

	// タイマー初期化
	shootTimerFrames_ = 0;
	state_ = State::Active;
	isDead_ = false;
}

void TurretEnemy::Update() {
	if (isDead_) {
		// 死亡中も弾だけ更新して自然消滅させるならここ
		UpdateBullets_();
		return;
	}

	// 砲台は「移動しない」ので translation_ を変更しない
	worldTransform_.UpdateMatrix();

	// 狙い（ターゲットが設定されている時だけ向く）
	AimToTarget_();

	// 射撃タイマー
	shootTimerFrames_++;
	if (shootTimerFrames_ >= shootIntervalFrames_) {
		shootTimerFrames_ = 0;
		state_ = State::Shooting;
		Fire_();
		state_ = State::Active;
	}

	// コライダー追従
	if (collider_) {
		collider_->SetTranslate(GetWorldTranslation());
		collider_->Update();
	}

	// 弾更新
	UpdateBullets_();
}

void TurretEnemy::Draw(const Camera* camera) {
	if (!camera) {
		return;
	}

	// 砲台本体
	if (!isDead_ && model_) {
		model_->Draw(worldTransform_, *camera, textureHandle_);
	}

	// 弾
	for (auto& b : bullets_) {
		b->Draw(camera);
	}
}

void TurretEnemy::OnCollision(CharacterBase* /*other*/) {
	// 被弾：HP減少（弾側は CollisionManager 等で消える想定）
	if (isDead_) {
		return;
	}

	hp_--;
	if (hp_ <= 0) {
		isDead_ = true;
	}
}

void TurretEnemy::AimToTarget_() {
	if (!hasTarget_) {
		return;
	}

	const Vector3 selfPos = GetWorldTranslation();
	Vector3 to = {targetPos_.x - selfPos.x, targetPos_.y - selfPos.y, targetPos_.z - selfPos.z};

	// 長さ0対策
	const float lenSq = to.x * to.x + to.y * to.y + to.z * to.z;
	if (lenSq < 0.000001f) {
		return;
	}

	// yaw（Y軸回転）だけ合わせる簡易版：atan2(x, z)
	const float yaw = std::atan2(to.x, to.z);
	worldTransform_.rotation_.y = yaw;
}

void TurretEnemy::Fire_() {
	if (!hasTarget_) {
		return;
	}

	// 発射位置：砲台の前方に少し出す（見た目用）
	Vector3 muzzlePos = GetWorldTranslation();
	// 砲台の向き（yaw）に合わせて前方へ
	const float yaw = worldTransform_.rotation_.y;
	muzzlePos.x += std::sin(yaw) * kMuzzleForward;
	muzzlePos.z += std::cos(yaw) * kMuzzleForward;

	// 方向：ターゲットへ
	Vector3 dir = {targetPos_.x - muzzlePos.x, targetPos_.y - muzzlePos.y, targetPos_.z - muzzlePos.z};
	// Normalize
	const float len = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
	if (len < 0.000001f) {
		return;
	}
	dir.x /= len;
	dir.y /= len;
	dir.z /= len;

	// Bullet生成
	auto b = std::make_unique<Bullet>();
	b->Initialize();
	b->SetSpeed(bulletSpeed_);
	b->SetLifeTime(bulletLifeTimeSec_);
	b->FireFrom(muzzlePos, dir);

	bullets_.push_back(std::move(b));
}

void TurretEnemy::UpdateBullets_() {
	for (auto& b : bullets_) {
		b->Update();
	}
	bullets_.erase(std::remove_if(bullets_.begin(), bullets_.end(), [](const std::unique_ptr<Bullet>& b) { return b->IsDead(); }), bullets_.end());
}
