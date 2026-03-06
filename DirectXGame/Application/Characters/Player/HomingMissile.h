#pragma once

#include <memory>

#include "3d/Camera.h"
#include "3d/Model.h"

#include "Application/Characters/CharacterBase.h"

/// <summary>
/// ホーミングミサイルクラス
/// マウス右クリック長押しでロックオン
/// 離すと発射
/// インターバル有
/// </summary>
class HomingMissile : public CharacterBase {
public:
	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize() override;

	/// <summary>
	/// 
	/// </summary>
	void FireFrom(const KamataEngine::Vector3& worldPos, CharacterBase* target);

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
	/// 現在の追尾対象を取得（非所有）
	/// </summary>
	CharacterBase* GetTarget() const { return target_; }

	/// <summary>
	/// 追尾対象をクリアする
	/// </summary>
	void ClearTarget() { target_ = nullptr; }

private:
	// ===== 定数 =====
	static constexpr float kFixedDeltaTime = 1.0f / 60.0f;
	static constexpr KamataEngine::Vector3 kMissileScale{ 0.16f,0.16f,0.28f };
	static constexpr float kColliderRadius = 0.65f;
	static constexpr float kSpeed = 1.4f;
	static constexpr float kTurnRate = 0.14f;
	static constexpr float kLifeTimeSec = 6.0f;

	// ===== メンバ変数 =====
	std::unique_ptr<KamataEngine::Model> model_;
	uint32_t textureHandle_ = 0u;

	KamataEngine::Vector3 velocity_{0.0f, 0.0f, 1.0f};
	CharacterBase* target_ = nullptr; // 非所有
	float elapsedTimeSec_ = 0.0f;
	bool isDead_ = false;
};
