#pragma once

#include "3d/Camera.h"
#include "3d/Model.h"
#include "3d/WorldTransform.h"
#include "math/Vector3.h"

/// <summary>
/// ダメージ発生時の一瞬の破片・火花・光などを表現する軽量パーティクル。
/// ・速度ベースで移動
/// ・寿命（秒）で自動消滅
/// ・開始スケール → 終了スケールを線形補間
///
/// Enemy / Player の被弾演出や爆発演出など、
/// 「短時間で消える軽量エフェクト」を想定したクラス。
/// </summary>
class DamageParticle {
public:
	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="model">
	/// 描画に使用するモデル（非所有）。
	/// EnemyManager 等で共有される想定。
	/// </param>
	/// <param name="pos">パーティクル発生位置（ワールド座標）</param>
	/// <param name="vel">飛散方向＋速度（1秒あたりの移動量）</param>
	/// <param name="lifeTime">寿命（秒）</param>
	/// <param name="startScale">生成直後のスケール</param>
	/// <param name="endScale">寿命終了時のスケール</param>
	void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& pos, const KamataEngine::Vector3& vel, float lifeTime, float startScale, float endScale);

	/// <summary>
	/// 更新処理
	/// ・dt に基づいて移動
	/// ・寿命進行に応じてスケールを補間
	/// </summary>
	/// <param name="dt">前フレームからの経過時間（秒）</param>
	void Update(float dt);

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="camera">描画に使用するカメラ</param>
	void Draw(const KamataEngine::Camera* camera);

	/// <summary>
	/// 寿命が尽きたかどうか
	/// EnemyManager 等で削除判定に使用する。
	/// </summary>
	bool IsFinished() const { return currentTime_ >= lifeTime_; }

private:
	// ワールド変換（位置・回転・スケール）
	KamataEngine::WorldTransform worldTransform_;

	// 描画用モデル（非所有）
	KamataEngine::Model* model_ = nullptr;

	// 飛散速度
	KamataEngine::Vector3 velocity_{};

	// 寿命関連
	float lifeTime_ = 0.0f;    // 寿命（秒）
	float currentTime_ = 0.0f; // 経過時間（秒）

	// スケール補間用
	float startScale_ = 1.0f; // 初期スケール
	float endScale_ = 0.0f;   // 終了スケール
};
