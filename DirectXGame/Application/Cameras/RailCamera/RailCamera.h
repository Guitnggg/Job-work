#pragma once

#include <memory>

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
	/// カメラシェイクを追加する
	/// </summary>
	/// <param name="dir">揺れの方向</param>
	/// <param name="power">揺れの強さ</param>
	void AddShake(const KamataEngine::Vector3& dir, float power);

	/// <summary>
	/// 移動入力によるロール角の目標値を設定
	/// </summary>
	/// <param name="inputX">-1.0（左）～ +1.0（右）</param>
	void SetMoveInput(float inputX);

public:
	/// <summary>
	/// カメラ取得（所有権は渡さない）
	/// </summary>
	const KamataEngine::Camera* GetCamera() { return camera_.get(); }

	/// <summary>
	/// ワールド変換取得
	/// </summary>
	KamataEngine::WorldTransform& GetWorldTransform() { return worldTransform_; }

private:
	// ===== 定数 =====
	static constexpr float kInitialZ = -50.0f; // 初期Z位置
	static constexpr float kMoveSpeedZ = 0.1f; // Z方向移動速度（1フレーム）

	static constexpr float kFollowRate = 0.1f;      // 遅延追従補間率
	static constexpr float kMaxRollAngle = 0.35f;   // 最大ロール角（ラジアン）
	static constexpr float kRollFollowRate = 0.15f; // ロール補間率

	static constexpr float kRollPosOffsetX = 5.0f;  // ロールによるX位置補正量
	static constexpr float kRollPosOffsetY = 0.5f;  // ロールによるY位置補正量

private:
	// ===== 基本 =====
	KamataEngine::WorldTransform worldTransform_; // ワールド変換データ
	std::unique_ptr<KamataEngine::Camera> camera_; // カメラ（ビューポート）

	// ===== 遅延追従 =====
	KamataEngine::Vector3 lagCameraPos_{}; // 遅延追従用の仮想カメラ位置

	// ===== カメラシェイク =====
	CameraShake cameraShake_;

	// ===== ロール補間 =====
	float rollAngle_ = 0.0f;       // 現在のロール角
	float targetRollAngle_ = 0.0f; // 目標ロール角
};