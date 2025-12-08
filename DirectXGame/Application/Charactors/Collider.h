#pragma once

#include "3d/WorldTransform.h"
#include "3d/Model.h"
#include "3d/Camera.h"
#include "math/Vector3.h"
#include "math/Vector4.h"

/// <summary>
/// 球体コリジョン（当たり判定）を表すクラス。
/// 半径をもつ球を基準に当たり判定処理を行い、
/// 必要に応じて可視コライダーとして描画も可能。
/// </summary>
class Collider {
public:

    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize();

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// 描画処理
    /// </summary>
    /// <param name="camera">描画に使用するカメラ</param>
    void Draw(KamataEngine::Camera& camera);

public:  /// === Getters === ///

    /// <summary>
    /// 座標の取得
    /// </summary>
    KamataEngine::Vector3 GetTranslate() { return worldTransform_.translation_; }

    /// <summary>
    /// 半径の取得
    /// </summary>
    float GetRadius() { return radius_; }

public:  /// === Setters === ///

    /// <summary>
    /// 座標の設定
    /// </summary>
    void SetTranslate(KamataEngine::Vector3 translate) { worldTransform_.translation_ = translate; }

    /// <summary>
    /// 半径の設定
    /// </summary>
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

