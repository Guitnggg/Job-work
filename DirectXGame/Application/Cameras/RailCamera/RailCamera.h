#pragma once

#include <3d/WorldTransform.h>
#include <3d/Camera.h>

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
    /// 初期化
    /// </summary>
    void Initialize();

    /// <summary>
    /// 更新
    /// </summary>
    void Update();

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
    KamataEngine::WorldTransform worldTransform_;  // ワールド変換データ
    KamataEngine::Camera* camera_ = nullptr;       // カメラ（ビューポート）

    static constexpr float kInitialZ = -50.0f;   // 初期Z位置
    static constexpr float kMoveSpeedZ = 0.1f;   // Z方向移動速度（1フレーム）
};

