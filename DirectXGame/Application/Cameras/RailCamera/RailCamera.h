#pragma once

#include <3d/Camera.h>
#include <3d/WorldTransform.h>

#include "CameraShake.h"

/// <summary>
/// RailCameraクラス
/// レール上を進むように移動して、始点として利用するクラス
/// </summary>
class RailCamera {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	RailCamera();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~RailCamera();

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

public:
	/// <summary>
	/// カメラシェイク追加
	/// </summary>
	/// <param name="dir"></param>
	/// <param name="power"></param>
	void AddShake(const KamataEngine::Vector3& dir, float power);

	/// <summary>
	/// 移動入力による傾き設定
	/// </summary>
	/// <param name="inputX">-1.0(左)～+1.0(右)</param>
	void SetMoveInput(float inputX);

public:
	/// <summary>
	/// カメラの取得
	/// </summary>
	const KamataEngine::Camera* GetCamera() { return camera_; }

	/// <summary>
	/// Getter
	/// </summary>
	KamataEngine::WorldTransform& GetWorldTransform() { return worldTransform_; }

private:
	KamataEngine::WorldTransform worldTransform_; // ワールド変換データ
	KamataEngine::Camera* camera_ = nullptr;      // カメラ（ビューポート）

	static constexpr float kInitialZ = -50.0f; // 初期Z位置
	static constexpr float kMoveSpeedZ = 0.1f; // Z方向移動速度（1フレーム）

	KamataEngine::Vector3 lagCameraPos_{};
	float followRate_ = 0.1f;

	// ===== カメラシェイク =====
	CameraShake cameraShake_;

	// ===== カメラの傾き =====
	float rollAngle_ = 0.0f;       // 現在の傾き角度
	float targetRollAngle_ = 0.0f; // 目標傾き角度

	float maxRollAngle_ = 0.35f;   // 最大傾き角度（ラジアン）
	float rollFollowRate_ = 0.15f; // 補完率
};
