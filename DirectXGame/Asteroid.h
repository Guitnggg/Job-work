#pragma once

#include <3d/WorldTransform.h>
#include <3d/Model.h>
#include <3d/Camera.h>

class Asteroid{
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
    /// 初期化
    /// </summary>
    void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3 pos, const KamataEngine::Vector3 velocity, KamataEngine::Vector3 rotate);

    /// <summary>
    /// 更新
    /// </summary>
    void Update();

    /// <summary>
    /// 描画
    /// </summary>
    void Draw(const KamataEngine::Camera& camera);

public:

    /// <summary>
    /// Z座標の取得
    /// </summary>
    float GetZ() const { return worldTransform_.translation_.z; }

    /// <summary>
    /// リスポーン
    /// </summary>
    void Respawn(const KamataEngine::Vector3 pos, const KamataEngine::Vector3 velocity, KamataEngine::Vector3 rotate);

private:
    // ワールド変換情報
    KamataEngine::WorldTransform worldTransform_;
    // 3Dモデル
    KamataEngine::Model* model_ = nullptr;
    // 速度
    KamataEngine::Vector3 velocity_ = {0,0,0};
    // 回転速度
    KamataEngine::Vector3 rotate_ = { 0,0,0 };

};

