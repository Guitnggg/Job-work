#pragma once

#include "3d/WorldTransform.h"
#include "3d/Model.h"
#include "3d/Camera.h"
#include "math/Vector3.h"
#include "math/Vector4.h"

class Collider {
public:

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize();

    /// <summary>
    /// 更新
    /// </summary>
    void Update();

    /// <summary>
    /// 描画
    /// </summary>
    /// <param name="camera"></param>
    void Draw(KamataEngine::Camera& camera);

public:  /// === Getters === ///

    /// <summary>
    /// 座標の取得
    /// </summary>
    /// <returns>座標</returns>
    KamataEngine::Vector3 GetTranslate() { return worldTransform_.translation_; }

    /// <summary>
    /// 半径の取得
    /// </summary>
    /// <returns></returns>
    float GetRadius() { return radius_; }

public:  /// === Setters === ///

    /// <summary>
    /// 座標の設定
    /// </summary>
    /// <param name="translate">座標</param>
    void SetTranslate(KamataEngine::Vector3 translate) { worldTransform_.translation_ = translate; }

    /// <summary>
    /// 半径の設定
    /// </summary>
    /// <param name="radius"></param>
    void SetRadius(float radius) { radius_ = radius; }

private:

    // ワールドトランスフォーム
    KamataEngine::WorldTransform worldTransform_;

    // 半径
    float radius_ = 1.0f;

    // モデル
    std::unique_ptr<KamataEngine::Model> model_;

    // テクスチャ
    uint32_t textureHandle_ = 0u;
};

