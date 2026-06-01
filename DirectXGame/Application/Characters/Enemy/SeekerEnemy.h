#pragma once

#include <memory>

#include "3d/Camera.h"
#include "3d/Model.h"
#include "3d/ObjectColor.h"
#include "3d/WorldTransform.h"

#include "Application/Characters/CharacterBase.h"

/// <summary>
/// プレイヤーやターゲットへ向かって旋回追尾する敵キャラクター。
/// 通常追尾・被弾演出（フラッシュ／ヒットストップ／ノックバック）を備える。
/// </summary>

class SeekerEnemyStateBase;
class SeekerEnemyActiveState;
class SeekerEnemyHitStopState;
　
class SeekerEnemy : public CharacterBase {
public:
	/// <summary>
	/// 
	/// </summary>
	SeekerEnemy();
	~SeekerEnemy() override;

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
	/// 当たり判定時の処理
	/// </summary>
	/// <param name="other">衝突相手</param>
	void OnCollision(CharacterBase* other) override;

	/// <summary>
	/// 死亡判定
	/// HP枯渇、または演出完了後の消滅フラグで判定する。
	/// </summary>
	bool IsDead() const override { return isDead_ || hp_ <= 0; }

public: // ==== 外部設定パラメータ ====
	/// <summary>ターゲットの設定（プレイヤー位置など）</summary>
	void SetTarget(const KamataEngine::Vector3& worldTarget) {
		targetPos_ = worldTarget;
		hasTarget_ = true;
	}

	/// <summary>
	/// 移動速度の設定
	/// </summary>
	void SetSpeed(float speed) { speed_ = speed; }

	/// <summary>
	/// 旋回率の設定
	/// </summary>
	void SetTurnRate(float rate) { turnRate_ = rate; }

	/// <summary>
	/// 当たり判定半径の設定
	/// </summary>
	void SetColliderRadius(float radius);

	/// <summary>
	/// 初期HPの設定
	/// </summary>
	void SetInitialHP(int32_t hp) { initialHP_ = hp; }

	/// <summary>
	/// 寿命（秒）の設定
	/// </summary>
	void SetLifeTime(float sec) { lifeTimeSec_ = sec; }

	/// <summary>
	/// 初期位置の設定
	/// </summary>
	void SetInitialPosition(const KamataEngine::Vector3& pos) { initialPosition_ = pos; }

private:
	// ===== 定数（挙動・演出パラメータ）=====
	static constexpr float kDefaultSpeed = 0.2f;
	static constexpr float kDefaultTurnRate = 0.15f;
	static constexpr float kDefaultColliderRadius = 1.0f;
	static constexpr int32_t kDefaultHP = 1;
	static constexpr float kDefaultLifeTimeSec = 30.0f;

	static constexpr float kFlashDuration = 0.15f;
	static constexpr float kHitStopDuration = 0.12f;
	static constexpr float kHitMotionDuration = 0.28f;
	static constexpr float kHitKnockback = 2.5f;
	static constexpr float kHitRollRad = 1.0f;

	static constexpr float kYawRotateSpeed = 0.6f;

	// 固定Δtを使用する理由：
	// 敵AI挙動をフレームレート差の影響から切り離し、
	// 追尾挙動を安定させるため
	static constexpr float kFixedDeltaTime = 1.0f / 60.0f;

	// 強制消滅範囲
	static constexpr float kKillZ = -40.0f;
	static constexpr float kKillXY = 220.0f;

private:
	// モデル
	std::unique_ptr<KamataEngine::Model> model_;
	KamataEngine::ObjectColor objectColor_;
	KamataEngine::ObjectColor flashColor_;
	uint32_t textureHandle_ = 0u;

	// 動作パラメータ
	KamataEngine::Vector3 initialPosition_{0.0f, 0.0f, 80.0f};
	float speed_ = kDefaultSpeed;
	float turnRate_ = kDefaultTurnRate;
	float colliderRadius_ = kDefaultColliderRadius;
	int32_t initialHP_ = kDefaultHP;
	float lifeTimeSec_ = kDefaultLifeTimeSec;

	// 追尾対象
	KamataEngine::Vector3 targetPos_{};
	bool hasTarget_ = false;

	// 時間・死亡管理
	float timeSec_ = 0.0f;
	bool isDead_ = false;

	// 状態・演出
	std::unique_ptr<SeekerEnemyStateBase> state_;
	std::unique_ptr<SeekerEnemyStateBase> pendingState_;

	float flashTimer_ = 0.0f;
	float hitStopTimer_ = 0.0f;
	bool pendingExplode_ = false;

	KamataEngine::Vector3 baseScale_{1.0f, 1.0f, 1.0f};

	// 被弾モーション
	KamataEngine::Vector3 hitBasePos_{};
	KamataEngine::Vector3 hitDir_{};
	float hitBaseRollZ_ = 0.0f;
	float hitMotionTimer_ = 0.0f;

private:
	/// <summary>
	/// 範囲外・寿命による強制死亡判定
	/// </summary>
	void ClampDeathByBounds_();
	void UpdateFlashAndHitMotionTimer_(float dt);
	void UpdateActiveMotion_(float dt);
	void UpdateHitStopMotion_(float dt);
	void SyncCollider_();
	void ChangeState_(std::unique_ptr<SeekerEnemyStateBase> nextState);
	void RequestStateChange_(std::unique_ptr<SeekerEnemyStateBase> nextState);
	void ApplyPendingStateChange_();

	friend class SeekerEnemyActiveState;
	friend class SeekerEnemyHitStopState;
};
