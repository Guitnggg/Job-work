#pragma once

#include <3d/WorldTransform.h>
#include <3d/Model.h>
#include <3d/Camera.h>

/// <summary>
/// 天球描画クラス
/// ゲームシーンの背景として常時表示される天球モデルを描画する
/// </summary>
class Skydome {
public:
    /// <summary>
    /// 初期化処理
    /// </summary>
    /// <param name="camera">描画に使用するカメラ</param>
    void Initialize(KamataEngine::Camera* camera);

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw();

private:

    KamataEngine::WorldTransform worldTransform_;  // ワールド変換
    KamataEngine::Model* model_ = nullptr;         // モデルデータ
    KamataEngine::Camera* camera_ = nullptr;       // カメラ

};