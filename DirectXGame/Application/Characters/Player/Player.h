#pragma once

#include "3d/Camera.h"
#include "3d/Model.h"
#include "3d/WorldTransform.h"
#include "audio/Audio.h"
#include "input/Input.h"

#include "Application/Characters/CharacterBase.h"

/// <summary>
/// プレイヤーキャラクタークラス。
/// ・移動／傾き（バンク）
/// ・ロール回避
/// ・被弾演出・無敵時間
/// ・爆発演出と死亡管理
///
/// 入力・生存状態・演出制御を一括で管理する。
/// </summary>
class Player : public CharacterBase {
public:
	~Player() override = default;

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="camera">使用するカメラ</param>
	void Initialize(KamataEngine::Camera* camera);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update() override;

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw(const KamataEngine::Camera* camera) override;

	/// <summary>
	/// 当たり判定時の処理
	/// </summary>
	void OnCollision(CharacterBase* enemy) override;

public:
	/// <summary>ダメージを受ける</summary>
	void Damage(int32_t amount);

	/// <summary>死亡処理（爆発開始）</summary>
	void Kill();

public:
	/// <summary>死亡状態か</summary>
	bool IsDead() const override { return isDead_; }

	/// <summary>爆発中か</summary>
	bool IsExploding() const { return isExploding_; }

	/// <summary>爆発演出終了済みか</summary>
	bool IsExplosionFinished() const { return isExplosionFinished_; }

public:
	/// <summary>入力を受け付けるか</summary>
	void SetInputEnabled(bool enabled) { inputEnabled_ = enabled; }

	/// <summary>
	/// 無敵状態か
	/// （被弾無敵 or ロール回避中）
	/// </summary>
	bool IsInvincible() const { return invincibleFrames_ > 0 || isRolling_; }

	/// <summary>
	/// 被弾イベントを1回だけ取得
	/// </summary>
	bool ConsumeTookDamageEvent() {
		bool flag = tookDamageEvent_;
		tookDamageEvent_ = false;
		return flag;
	}

	/// <summary>
	/// 被弾フラッシュ進行度（0～1）
	/// </summary>
	float HitFlashT() const { return hitFlashFrames_ > 0 ? static_cast<float>(hitFlashFrames_) / static_cast<float>(kHitFlashDuration) : 0.0f; }

private:
	// ===== 内部処理 =====
	float EaseOutCubic(float t) const;
	void UpdateExplosion_();
	void UpdateMoveAndBank_(float dt);
	void StartRoll_(float dir);
	bool UpdateRoll_();

private:
	// ===== 定数 =====
	static constexpr float kFixedDeltaTime = 1.0f / 60.0f;

	// 初期姿勢
	static constexpr KamataEngine::Vector3 kInitialPos{0.0f, -2.0f, 20.0f};
	static constexpr KamataEngine::Vector3 kInitialRot{0.0f, 0.0f, 0.0f};
	static constexpr KamataEngine::Vector3 kInitialScale{1.0f, 1.0f, 1.0f};

	// 移動・傾き
	static constexpr float kMoveSpeedXY = 10.0f;
	static constexpr float kBankMaxRadZ = 0.35f;
	static constexpr float kPitchMaxRadX = 0.25f;
	static constexpr float kTiltLerp = 0.20f;

	// 画面内制限
	static constexpr float kClampXMin = -8.0f;
	static constexpr float kClampXMax = 8.0f;
	static constexpr float kClampYMin = -4.0f;
	static constexpr float kClampYMax = 3.0f;

	// ロール回避
	static constexpr float kRollDurationFrames = 24.0f;
	static constexpr float kRollMoveDistance = 7.0f;
	static constexpr int32_t kDoubleTapThreshold = 18;
	static constexpr float kTwoPi = 6.28318530717958647692f;

	// HP・当たり判定
	static constexpr int32_t kInitialHp = 100;
	static constexpr float kColliderRadius = 1.0f;
	static constexpr int32_t kCollisionDamage = 20;

	// 被弾・無敵
	static constexpr int32_t kHitFlashDuration = 18;
	static constexpr int32_t kInvincibleDuration = 30;
	static constexpr float kHitRollAmp = 0.18f;
	static constexpr float kHitRollFreq = 10.0f;
	static constexpr float kHitPulseAmp = 0.06f;
	static constexpr float kHitPulseFreq = 18.0f;

	// 爆発
	static constexpr int32_t kExplosionDurationFrames = 60;
	static constexpr float kExplodeFallSpeedY = 0.1f;
	static constexpr float kExplodeSpinSpeedY = 0.2f;

private:
	// ===== 参照 =====
	KamataEngine::Camera* camera_ = nullptr;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Input* input_ = nullptr;
	KamataEngine::Audio* audio_ = nullptr;

	// ===== 傾き =====
	float targetTiltZ_ = 0.0f;
	float targetTiltX_ = 0.0f;
	float currentTiltZ_ = 0.0f;
	float currentTiltX_ = 0.0f;

	// ===== ロール回避 =====
	bool isRolling_ = false;
	float rollFrame_ = 0.0f;
	float rollDir_ = 0.0f;
	float rollStartRotZ_ = 0.0f;
	KamataEngine::Vector3 rollStartPos_{};
	KamataEngine::Vector3 rollEndPos_{};

	// ダブルタップ
	int32_t doubleTapFrameA_ = 0;
	int32_t doubleTapFrameD_ = 0;

	// ===== 爆発・死亡 =====
	int32_t seExplosion_ = -1;
	bool isDead_ = false;
	bool isExploding_ = false;
	bool isExplosionFinished_ = false;
	int32_t explosionFrame_ = 0;
	KamataEngine::Vector3 initialScale_{};

	// ===== 被弾 =====
	bool inputEnabled_ = true;
	bool tookDamageEvent_ = false;
	int32_t hitFlashFrames_ = 0;
	int32_t invincibleFrames_ = 0;
	float lastHitRollOffset_ = 0.0f;
};
