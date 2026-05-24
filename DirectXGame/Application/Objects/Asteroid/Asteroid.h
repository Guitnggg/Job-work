#pragma once

#include <3d/Camera.h>
#include <3d/Model.h>
#include <3d/WorldTransform.h>

// Playerの前方宣言
class Player;

/// <summary>
/// 流れてくる隕石オブジェクト。
/// 初期位置・移動速度・回転速度を設定して空間を漂わせ、
/// Z方向の距離によってスケールを変えることで奥行き感を演出する。
/// 衝突判定を持たず、視覚的なオブジェクト専門の軽量クラス。
/// </summary>
class Asteroid {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	Asteroid();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Asteroid();

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="model">描画に使用するモデル（共有・非所有）</param>
	/// <param name="pos">初期位置</param>
	/// <param name="velocity">移動量（1フレームあたり）</param>
	/// <param name="rotate">回転量（1フレームあたり）</param>
	void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& pos, const KamataEngine::Vector3& velocity, const KamataEngine::Vector3& rotate);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="Camera">描画時に使用するカメラ</param>
	void Draw(const KamataEngine::Camera& camera);

public:
	/// <summary>
	/// Z座標の取得
	/// </summary>
	float GetZ() const { return worldTransform_.translation_.z; }

	/// <summary>
	/// リスポーン
	/// </summary>
	void Respawn(const KamataEngine::Vector3& pos, const KamataEngine::Vector3& velocity, const KamataEngine::Vector3& rotate);

	/// <summary>
	/// Playerのアドレスを取得
	/// </summary>
	void SetPlayer(Player* player) { player_ = player; }

private:
	KamataEngine::WorldTransform worldTransform_; // 位置・回転・スケール
	KamataEngine::Model* model_ = nullptr;        // 3Dモデル（共有・非所有）
	KamataEngine::Vector3 velocity_ = {0, 0, 0};  // 移動速度
	KamataEngine::Vector3 rotate_ = {0, 0, 0};    // 回転速度

	// Playerのアドレス
	Player* player_ = nullptr;
};
