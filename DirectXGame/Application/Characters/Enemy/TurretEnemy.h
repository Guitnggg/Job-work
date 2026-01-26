#pragma once

#include <memory>
#include <vector>

#include "3d/Camera.h"
#include "3d/Model.h"
#include "3d/WorldTransform.h"
#include "math/Vector3.h"

#include "Application/Characters/CharacterBase.h"
#include "Application/Characters/Player/Bullet.h"

/// <summary>
/// 固定砲台敵
/// ・移動しない（座標は基本固定）
/// ・ターゲット方向へ向き、一定間隔で弾を発射
/// ・弾は TurretEnemy 内部で管理（Update/Drawもここで実行）
/// </summary>
class TurretEnemy : public CharacterBase {
public:
	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize() override;

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update() override;

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="camera">描画に使用するカメラ</param>
	void Draw(const KamataEngine::Camera* camera) override;

	/// <summary>
	/// 当たり判定
	/// </summary>
	/// <param name="other">衝突相手となるキャラクター</param>
	void OnCollision(CharacterBase* other) override;

	/// <summary>
	/// 生存フラグ
	/// </summary>
	bool IsDead() const override { return isDead_; }

public: // ---- 外部から設定（必要なら使う）----
	/// <summary>
	/// 砲台が狙うターゲット位置（ワールド）
	/// </summary>
	void SetTarget(const KamataEngine::Vector3& worldTarget) {
		targetPos_ = worldTarget;
		hasTarget_ = true;
	}

	/// <summary>
	/// 発射間隔（フレーム）
	/// </summary>
	void SetShootIntervalFrames(int32_t frames) { shootIntervalFrames_ = frames; }

	/// <summary>
	/// 弾速度（Bullet::SetSpeed に渡す）
	/// </summary>
	void SetBulletSpeed(float s) { bulletSpeed_ = s; }

	/// <summary>
	/// 弾寿命（Bullet::SetLifeTime に渡す）
	/// </summary>
	void SetBulletLifeTime(float sec) { bulletLifeTimeSec_ = sec; }

	/// <summary>
	/// 初期HP
	/// </summary>
	void SetInitialHP(int32_t hp) { initialHP_ = hp; }

	/// <summary>
	/// コライダー半径
	/// </summary>
	void SetColliderRadius(float r) {
		colliderRadius_ = r;
		if (collider_)
			collider_->SetRadius(r);
	}

private:
	// 状態
	enum class State { Active, Shooting };

private:
	// ---- 見た目 ----
	std::unique_ptr<KamataEngine::Model> model_;
	uint32_t textureHandle_ = 0u;

	// ---- 砲台パラメータ ----
	State state_ = State::Active;

	int32_t initialHP_ = 3;
	float colliderRadius_ = 1.2f;

	// 狙い
	KamataEngine::Vector3 targetPos_{0.0f, 0.0f, 0.0f};
	bool hasTarget_ = false;

	// 射撃タイマー（フレーム管理）
	int32_t shootTimerFrames_ = 0;
	int32_t shootIntervalFrames_ = 60; // 1秒@60fps

	// 弾パラメータ
	float bulletSpeed_ = 2.8f; // Bullet標準に合わせる
	float bulletLifeTimeSec_ = 3.0f;

	// 弾コンテナ（砲台が内部管理）
	std::vector<std::unique_ptr<Bullet>> bullets_;

	// 死亡フラグ
	bool isDead_ = false;

private:
	/// <summary>ターゲットに向けて弾発射</summary>
	void Fire_();

	/// <summary>弾の更新と削除</summary>
	void UpdateBullets_();

	/// <summary>ターゲット方向へ向きを合わせる（簡易）</summary>
	void AimToTarget_();
};
