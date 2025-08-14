#pragma once

#include <3d/WorldTransform.h>
#include <3d/Camera.h>

class RailCamera{
public:

    /// <summary>
	/// 初期化
    /// </summary>
	void Initialize();

    /// <summary>
	/// 更新
    /// </summary>
	void Update();

public:

    /// <summary>
	/// カメラを取得
    /// </summary>
	const KamataEngine::Camera* GetCamera() { return camera_; }

    /// <summary>
	/// getter
    /// </summary>
	const KamataEngine::WorldTransform& GetWorldTransform() { return worldTransform_; }

private:

	KamataEngine::WorldTransform worldTransform_;  // ワールド変換データ	
	KamataEngine::Camera* camera_ = nullptr;       // カメラ
};