#include "TurretEnemy.h"

#include "base/TextureManager.h"
#include <algorithm>
#include <cmath>

using namespace KamataEngine;

// 砲台のモデル・射撃設定・HP・コライダーなど初期状態を設定する
void TurretEnemy::Initialize() {
	// --- 基底クラス初期化 ---
	CharacterBase::Initialize();

	// --- モデル生成 ---
	// 現在は球体モデルを使用（専用モデルがあれば差し替え可能）
	model_.reset(Model::CreateSphere());
	objectColor_.Initialize();
	objectColor_.SetColor({0.95f, 0.32f, 0.18f, 1.0f});
	flashColor_.Initialize();
	flashColor_.SetColor({1.0f, 0.95f, 0.28f, 0.85f});
	flashTextureHandle_ = TextureManager::Load("./Resources/white1x1.png");

	// --- HP設定 ---
	maxHp_ = initialHP_;
	hp_ = maxHp_;

	// --- 見た目スケール ---
	worldTransform_.scale_ = kTurretScale;
	worldTransform_.UpdateMatrix();

	// --- コライダー設定 ---
	if (collider_) {
		collider_->SetRadius(colliderRadius_);
		collider_->SetTranslate(GetWorldTranslation());
		collider_->Update();
	}

	// --- 初期状態 ---
	bullets_.clear();
	shootTimerFrames_ = 0;
	state_ = State::Active;
	isDead_ = false;
	flashTimer_ = 0.0f;
	shakeTimer_ = 0.0f;
	baseTranslation_ = worldTransform_.translation_;
}

// ターゲット追尾・射撃クールタイム・弾更新・死亡判定・コライダー同期を行う
void TurretEnemy::Update() {
	// 死亡中も弾は更新して自然消滅させる
	if (IsDead()) {
		UpdateBullets_();
		return;
	}

	const float dt = 1.0f / 60.0f;
	if (flashTimer_ > 0.0f) {
		flashTimer_ -= dt;
	}
	if (shakeTimer_ > 0.0f) {
		shakeTimer_ -= dt;
	}

	// 砲台は移動しないため translation_ は更新しない
	worldTransform_.translation_ = baseTranslation_;
	if (shakeTimer_ > 0.0f) {
		const float t = shakeTimer_ / kShakeDuration;
		worldTransform_.translation_.x += std::sin(t * 40.0f) * kShakePower * t;
		worldTransform_.translation_.y += std::cos(t * 52.0f) * kShakePower * 0.5f * t;
	}
	worldTransform_.UpdateMatrix();

	// ターゲット追尾
	AimToTarget_();

	// --- 射撃管理 ---
	shootTimerFrames_++;
	if (shootTimerFrames_ >= shootIntervalFrames_) {
		shootTimerFrames_ = 0;
		state_ = State::Shooting;
		Fire_();
		state_ = State::Active;
	}

	// --- コライダー同期 ---
	if (collider_) {
		collider_->SetTranslate(GetWorldTranslation());
		collider_->Update();
	}

	// --- 弾更新 ---
	UpdateBullets_();
}

// 砲台本体・発射済み弾・デバック用コライダーを描画する
void TurretEnemy::Draw(const Camera* camera) {
	if (!camera) {
		return;
	}

	// 砲台本体
	if (!IsDead() && model_) {
		model_->Draw(worldTransform_, *camera, textureHandle_, &objectColor_);
		if (flashTimer_ > 0.0f && flashTextureHandle_ != 0u) {
			Vector3 backupScale = worldTransform_.scale_;
			const float t = flashTimer_ / kFlashDuration;
			worldTransform_.scale_ = {backupScale.x * (1.0f + t * 0.3f), backupScale.y * (1.0f + t * 0.3f), backupScale.z * (1.0f + t * 0.3f)};
			worldTransform_.UpdateMatrix();
			model_->Draw(worldTransform_, *camera, flashTextureHandle_, &flashColor_);
			worldTransform_.scale_ = backupScale;
			worldTransform_.UpdateMatrix();
		}
	}

	// 弾描画
	for (auto& bullet : bullets_) {
		bullet->Draw(camera);
	}
}

// 砲台が被弾したときのダメージ処理と死亡判定を行う
void TurretEnemy::OnCollision(CharacterBase* /*other*/) {
	if (IsDead()) {
		return;
	}

	// ダメージ処理
	hp_--;
	flashTimer_ = kFlashDuration;
	shakeTimer_ = kShakeDuration;
	if (hp_ <= 0) {
		isDead_ = true;
	}
}

// ターゲット方向を向くように砲台のYaw回転を更新する
void TurretEnemy::AimToTarget_() {
	if (!hasTarget_) {
		return;
	}

	const Vector3 selfPos = GetWorldTranslation();
	Vector3 toTarget{targetPos_.x - selfPos.x, targetPos_.y - selfPos.y, targetPos_.z - selfPos.z};

	const float lenSq = toTarget.x * toTarget.x + toTarget.y * toTarget.y + toTarget.z * toTarget.z;

	if (lenSq < 0.000001f) {
		return;
	}

	// Y軸回転のみでターゲットを向く
	const float yaw = std::atan2(toTarget.x, toTarget.z);
	worldTransform_.rotation_.y = yaw;
}

// 現在位置と向きから新しい弾を生成し、弾リストへ登録する
void TurretEnemy::Fire_() {
	if (!hasTarget_) {
		return;
	}

	// --- 発射位置 ---
	Vector3 muzzlePos = GetWorldTranslation();
	const float yaw = worldTransform_.rotation_.y;
	muzzlePos.x += std::sin(yaw) * kMuzzleForward;
	muzzlePos.z += std::cos(yaw) * kMuzzleForward;

	// --- 発射方向 ---
	Vector3 dir{targetPos_.x - muzzlePos.x, targetPos_.y - muzzlePos.y, targetPos_.z - muzzlePos.z};

	const float len = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
	if (len < 0.000001f) {
		return;
	}
	dir.x /= len;
	dir.y /= len;
	dir.z /= len;

	// --- Bullet生成 ---
	auto bullet = std::make_unique<Bullet>();
	bullet->Initialize();
	bullet->SetSpeed(bulletSpeed_);
	bullet->SetLifeTime(bulletLifeTimeSec_);
	bullet->FireFrom(muzzlePos, dir);

	bullets_.push_back(std::move(bullet));
}

// 砲台が発射した弾を更新し、不要になった弾を削除する
void TurretEnemy::UpdateBullets_() {
	for (auto& bullet : bullets_) {
		bullet->Update();
	}

	bullets_.erase(std::remove_if(bullets_.begin(), bullets_.end(), [](const std::unique_ptr<Bullet>& b) { return b->IsDead(); }), bullets_.end());
}

// 敵の当たり判定サイズを外部から調整する
void TurretEnemy::SetColliderRadius(float radius) {
	colliderRadius_ = radius;
	if (collider_) {
		collider_->SetRadius(radius);
	}
}
