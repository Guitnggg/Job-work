#pragma once

#include "3d/Camera.h"
#include "3d/Model.h"
#include "3d/WorldTransform.h"
#include "math/Vector3.h"

/// <summary>
/// 被弾・爆発・推進エンジンなどの演出に使用する残留系パーティクル（煙）。
/// ・速度ベースで移動（dt対応）
/// ・寿命（秒）で消滅
/// ・開始スケール → 終了スケールを補間
///
/// DamageParticle よりも寿命が長く、
/// ふわっと残る演出（煙・蒸気・残像）を想定したクラス。
/// </summary>
class Smoke {
public:
	/// <summary>
	/// コンストラクタ / デストラクタ
	/// </summary>
	Smoke() = default;
	~Smoke() = default;

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="model">
	/// </param>
	/// <param name="position">生成位置（ワールド座標）</param>
	/// <param name="velocity">移動速度（1秒あたり）</param>
	/// <param name="lifeTime">寿命（秒）</param>
	/// <param name="startScale">初期スケール</param>
	/// <param name="endScale">終了スケール</param>
	void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& position, const KamataEngine::Vector3& velocity, float lifeTime, float startScale, float endScale);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="dt">前フレームからの経過時間（秒）</param>
	void Update(float dt);

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="camera">描画に使用するカメラ</param>
	void Draw(const KamataEngine::Camera* camera);

	/// <summary>
	/// 寿命が尽きているかどうか
	/// 管理クラス側で削除判定に使用する。
	/// </summary>
	bool IsFinished() const { return isFinished_; }

private:
	// ワールド変換情報
	KamataEngine::WorldTransform worldTransform_;

	// 描画用モデル（非所有）
	KamataEngine::Model* model_ = nullptr;

	// 移動速度（1秒あたり）
	KamataEngine::Vector3 velocity_{};

	// 時間管理
	float elapsedTimeSec_ = 0.0f; // 経過時間
	float lifeTimeSec_ = 1.0f;    // 寿命

	// スケール補間
	float startScale_ = 1.0f;
	float endScale_ = 0.0f;

	// 終了フラグ
	bool isFinished_ = false;
};
