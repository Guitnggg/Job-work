#pragma once

#include <memory>

#include "3d/Camera.h"
#include "3d/Model.h"
#include "3d/ObjectColor.h"
#include "3d/WorldTransform.h"

#include "Application/Characters/CharacterBase.h"

/// <summary>
/// 単純な直進弾クラス。
/// ・発射位置と方向を指定して生成
/// ・一定距離、寿命、または衝突で消滅
/// ・移動はフレーム固定更新を前提とする
/// </summary>
class Bullet : public CharacterBase {
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

	/// <summary>
	/// Object Pool Pattern: marks this bullet as available for reuse.
	/// 弾を破棄せず、未使用状態としてプールに戻すための関数。
	/// </summary>
	void Deactivate() { isDead_ = true; }

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
	/// ※ 本クラスは固定フレーム前提
	/// </summary>
	void SetSpeed(float speed) { speed_ = speed; }

	/// <summary>
	/// 寿命設定（秒）
	/// </summary>
	void SetLifeTime(float sec) { lifeTimeSec_ = sec; }

private:
	// ===== 定数 =====
	// 見た目
	static constexpr KamataEngine::Vector3 kBulletScale{0.4f, 0.4f, 0.7f};
	static constexpr KamataEngine::Vector3 kZeroRotation{0.0f, 0.0f, 0.0f};

	// 当たり判定
	static constexpr float kColliderRadius = 0.4f;

	// 生存制限
	static constexpr float kMaxDistance = 200.0f;

private:
	// モデル
	std::unique_ptr<KamataEngine::Model> model_;
	KamataEngine::ObjectColor objectColor_;
	uint32_t textureHandle_ = 0u;

	// 移動
	KamataEngine::Vector3 dir_{0.0f, 0.0f, 1.0f};
	float speed_ = 2.8f;

	// 寿命管理
	float lifeTimeSec_ = 3.0f;
	float elapsedTimeSec_ = 0.0f;

	// 状態
	bool isDead_ = false;

	// 距離計測用
	KamataEngine::Vector3 startPos_{};
};
