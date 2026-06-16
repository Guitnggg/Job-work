#pragma once

#include <memory>

#include "3d/Camera.h"
#include "3d/Model.h"
#include "3d/ObjectColor.h"
#include "3d/WorldTransform.h"

#include "Application/Characters/CharacterBase.h"

/// <summary>
/// 直進型レーザー弾クラス。
/// ・細長いモデルを用いた高威力弾
/// ・一定距離、寿命、または衝突で消滅
/// ・移動は固定フレーム前提
/// </summary>
class Laser : public CharacterBase {
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
	void Draw(const KamataEngine::Camera* camera) override;

	/// <summary>
	/// 当たり判定
	/// </summary>
	void OnCollision(CharacterBase* other) override;

	/// <summary>
	/// 死亡判定
	/// </summary>
	bool IsDead() const override { return isDead_; }

public:
	/// <summary>
	/// 発射処理
	/// </summary>
	/// <param name="worldPos">発射位置（ワールド座標）</param>
	/// <param name="dir">進行方向（正規化済み）</param>
	void FireFrom(const KamataEngine::Vector3& worldPos, const KamataEngine::Vector3& dir);

public:
	/// <summary>
	/// 移動速度設定（1フレーム当たり）
	/// </summary>
	void SetSpeed(float speed) { speed_ = speed; }

	/// <summary>
	/// 寿命設定（秒）
	/// </summary>
	void SetLifeTime(float sec) { lifeTimeSec_ = sec; }

private:
	// ===== 定数 =====
	// 見た目
	static constexpr KamataEngine::Vector3 kLaserScale{1.0f, 1.0f, 10.0f};
	static constexpr KamataEngine::Vector3 kZeroRotation{0.0f, 0.0f, 0.0f};

	// 当たり判定
	static constexpr float kColliderRadius = 1.0f;

	// 生存制限
	static constexpr float kMaxDistance = 500.0f;

private:
	// モデル
	std::unique_ptr<KamataEngine::Model> model_;
	KamataEngine::ObjectColor objectColor_;
	uint32_t textureHandle_ = 0u;

	// 移動
	KamataEngine::Vector3 dir_{0.0f, 0.0f, 1.0f};
	float speed_ = 3.5f;

	// 寿命管理
	float lifeTimeSec_ = 3.0f;
	float elapsedTimeSec_ = 0.0f;

	// 状態
	bool isDead_ = false;

	// 距離計測用
	KamataEngine::Vector3 startPos_{};
};
