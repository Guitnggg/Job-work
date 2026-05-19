#include "SeekerEnemy.h"
#include <cmath>

using namespace KamataEngine;

// 追尾敵のモデル・移動パラメータ・HP・コライダーなど初期状態を設定する
void SeekerEnemy::Initialize() {
	// --- 基底クラス初期化 ---
	CharacterBase::Initialize();

	// --- モデル生成 ---
	model_.reset(Model::CreateFromOBJ("Enemy", true));
	if (!model_) {
		model_.reset(Model::CreateSphere());
	}
	objectColor_.Initialize();
	objectColor_.SetColor({ 0.25f, 0.85f, 1.0f, 1.0f });
	flashColor_.Initialize();
	flashColor_.SetColor({ 1.0f, 1.0f, 0.30f, 1.0f });

	// --- 初期姿勢 ---
	worldTransform_.translation_ = initialPosition_;
	worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};
	worldTransform_.scale_ = baseScale_;
	worldTransform_.UpdateMatrix();

	// --- HP設定 ---
	maxHp_ = initialHP_;
	hp_ = maxHp_;

	// --- コライダー設定 ---
	if (collider_) {
		collider_->SetRadius(colliderRadius_);
		collider_->SetTranslate(GetWorldTranslation());
		collider_->Update();
	}

	// --- 状態初期化 ---
	timeSec_ = 0.0f;
	isDead_ = false;
	state_ = State::Active;

	flashTimer_ = 0.0f;
	hitStopTimer_ = 0.0f;
	hitMotionTimer_ = 0.0f;
	pendingExplode_ = false;

	// 初期進行方向（-Z）
	velocity_ = {0.0f, 0.0f, -1.0f};
}

// 被弾演出・ノックバック・追尾移動・死亡判定・コライダー同期をまとめて更新する
void SeekerEnemy::Update() {
	if (IsDead()) {
		return;
	}

	const float dt = kFixedDeltaTime;
	timeSec_ += dt;

	// --- 寿命・範囲チェック ---
	ClampDeathByBounds_();
	if (IsDead()) {
		return;
	}

	// --- フラッシュ更新 ---
	if (flashTimer_ > 0.0f) {
		flashTimer_ -= dt;
		if (flashTimer_ < 0.0f) {
			flashTimer_ = 0.0f;
		}
	}

	// --- 被弾モーション更新 ---
	if (hitMotionTimer_ > 0.0f) {
		hitMotionTimer_ -= dt;
		if (hitMotionTimer_ < 0.0f) {
			hitMotionTimer_ = 0.0f;
		}
	}

	switch (state_) {
	case State::Active: {
		// ===== 通常追尾処理 =====
		Vector3 pos = worldTransform_.translation_;

		if (hasTarget_) {
			Vector3 toTarget{targetPos_.x - pos.x, targetPos_.y - pos.y, targetPos_.z - pos.z};

			float len = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y + toTarget.z * toTarget.z);
			if (len > 0.00001f) {
				toTarget.x /= len;
				toTarget.y /= len;
				toTarget.z /= len;
			}

			// 旋回補間
			velocity_.x = (1.0f - turnRate_) * velocity_.x + turnRate_ * toTarget.x;
			velocity_.y = (1.0f - turnRate_) * velocity_.y + turnRate_ * toTarget.y;
			velocity_.z = (1.0f - turnRate_) * velocity_.z + turnRate_ * toTarget.z;

			float vlen = std::sqrt(velocity_.x * velocity_.x + velocity_.y * velocity_.y + velocity_.z * velocity_.z);
			if (vlen > 0.00001f) {
				velocity_.x /= vlen;
				velocity_.y /= vlen;
				velocity_.z /= vlen;
			}
		}

		// 前進
		pos.x += velocity_.x * speed_;
		pos.y += velocity_.y * speed_;
		pos.z += velocity_.z * speed_;

		// 被弾ノックバック
		if (hitMotionTimer_ > 0.0f) {
			float t = hitMotionTimer_ / kHitMotionDuration;
			pos.x += hitDir_.x * kHitKnockback * t;
			pos.y += hitDir_.y * kHitKnockback * t;
			pos.z += hitDir_.z * kHitKnockback * t;
			worldTransform_.rotation_.z = hitBaseRollZ_ + kHitRollRad * t;
		}

		worldTransform_.translation_ = pos;

		// 見た目用回転
		worldTransform_.rotation_.y += kYawRotateSpeed * dt;
		worldTransform_.UpdateMatrix();

		if (collider_) {
			collider_->SetTranslate(GetWorldTranslation());
			collider_->Update();
		}
		break;
	}

	case State::HitStop: {
		// ===== ヒットストップ中 =====
		hitStopTimer_ -= dt;

		Vector3 pos = hitBasePos_;
		if (hitMotionTimer_ > 0.0f) {
			float t = hitMotionTimer_ / kHitMotionDuration;
			pos.x += hitDir_.x * kHitKnockback * t;
			pos.y += hitDir_.y * kHitKnockback * t;
			pos.z += hitDir_.z * kHitKnockback * t;
			worldTransform_.rotation_.z = hitBaseRollZ_ + kHitRollRad * t;
		} else {
			worldTransform_.rotation_.z = hitBaseRollZ_;
		}

		worldTransform_.translation_ = pos;
		worldTransform_.UpdateMatrix();

		if (collider_) {
			collider_->SetTranslate(GetWorldTranslation());
			collider_->Update();
		}

		if (hitStopTimer_ <= 0.0f) {
			hitStopTimer_ = 0.0f;
			if (pendingExplode_) {
				isDead_ = true;
			} else {
				state_ = State::Active;
			}
		}
		break;
	}
	}
}

// 生存中の敵本体とデバック用コライダーを描画する
void SeekerEnemy::Draw(const Camera* camera) {
	if (!camera || IsDead() || !model_) {
		return;
	}

	Vector3 originalScale = worldTransform_.scale_;

	if (flashTimer_ > 0.0f) {
		float t = flashTimer_ / kFlashDuration;
		float scaleMul = 1.0f + 0.5f * t;

		worldTransform_.scale_.x = baseScale_.x * scaleMul;
		worldTransform_.scale_.y = baseScale_.y * scaleMul;
		worldTransform_.scale_.z = baseScale_.z * scaleMul;
		worldTransform_.UpdateMatrix();

		bool visible = true;
		if (t > 0.5f) {
			int32_t blink = static_cast<int32_t>(t * 10.0f);
			visible = (blink % 2) == 0;
		}

		if (visible) {
			model_->Draw(worldTransform_, *camera, textureHandle_, &flashColor_);
		}
	} else {
		model_->Draw(worldTransform_, *camera, textureHandle_, &objectColor_);
	}

	worldTransform_.scale_ = originalScale;
	worldTransform_.UpdateMatrix();
}

// 衝突時にダメージ・ヒットストップ・点滅・ノックバックを発生させる
void SeekerEnemy::OnCollision(CharacterBase* /*other*/) {
	if (IsDead()) {
		return;
	}

	// ダメージ処理
	hp_ -= 1;
	if (hp_ < 0) {
		hp_ = 0;
	}

	// フラッシュ・ヒットストップ
	flashTimer_ = kFlashDuration;
	hitStopTimer_ = kHitStopDuration;
	state_ = State::HitStop;

	// 被弾基準
	hitBasePos_ = worldTransform_.translation_;
	hitBaseRollZ_ = worldTransform_.rotation_.z;
	hitMotionTimer_ = kHitMotionDuration;

	// ノックバック方向（進行方向の逆）
	hitDir_ = {-velocity_.x, -velocity_.y, -velocity_.z};
	float len = std::sqrt(hitDir_.x * hitDir_.x + hitDir_.y * hitDir_.y + hitDir_.z * hitDir_.z);
	if (len > 0.00001f) {
		hitDir_.x /= len;
		hitDir_.y /= len;
		hitDir_.z /= len;
	} else {
		hitDir_ = {0.0f, 0.0f, -1.0f};
	}

	if (hp_ <= 0) {
		pendingExplode_ = true;
	}
}

// 敵の当たり判定サイズを外部から調整する
void SeekerEnemy::SetColliderRadius(float radius) {
	colliderRadius_ = radius;
	if (collider_) {
		collider_->SetRadius(radius);
	}
}

// プレイ範囲外へ出た敵を死亡扱いにして管理対象から外させるようにする
void SeekerEnemy::ClampDeathByBounds_() {
	const Vector3 p = GetWorldTranslation();

	if (p.z < kKillZ || std::abs(p.x) > kKillXY || std::abs(p.y) > kKillXY) {
		isDead_ = true;
	}

	if (timeSec_ >= lifeTimeSec_) {
		isDead_ = true;
	}
}
