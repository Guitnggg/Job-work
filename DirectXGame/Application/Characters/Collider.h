#pragma once

#include "3d/Camera.h"
#include "3d/Model.h"
#include "3d/WorldTransform.h"
#include "math/Vector3.h"
#include <memory>

/// <summary>
/// 球体コリジョンを表すクラス。
/// 当たり判定は半径を持つ球として扱い、
/// デバッグ用途として可視化描画も行える。
/// </summary>
class Collider {
public:
    /// <summary>
    /// 初期化処理
    /// ワールド変換および可視コライダーモデルを生成する。
    /// </summary>
    void Initialize();

    /// <summary>
    /// 更新処理
    /// ワールド行列の更新を行う。
    /// </summary>
    void Update();

    /// <summary>
    /// デバッグ描画処理
    /// </summary>
    /// <param name="camera">描画に使用するカメラ</param>
    void Draw(const KamataEngine::Camera& camera);

public: /// === Getters === ///
    /// <summary>
    /// ローカル座標の取得
    /// </summary>
    KamataEngine::Vector3 GetTranslate() const { return worldTransform_.translation_; }

    /// <summary>
    /// 半径の取得
    /// </summary>
    float GetRadius() const { return radius_; }

public: /// === Setters === ///
    /// <summary>
    /// 座標の設定
    /// </summary>
    void SetTranslate(const KamataEngine::Vector3& translate) { worldTransform_.translation_ = translate; }

    /// <summary>
    /// 半径の設定
    /// 半径変更時は見た目（scale）にも反映する。
    /// </summary>
    void SetRadius(float radius);

private:
    // ===== 定数 =====
    static constexpr float kDefaultRadius = 1.0f; // デフォルト半径

private:
    // ワールド変換情報
    KamataEngine::WorldTransform worldTransform_;

    // 球コライダーの半径
    float radius_ = kDefaultRadius;

    // デバッグ表示用モデル
    std::unique_ptr<KamataEngine::Model> model_;

    // デバッグ描画用テクスチャ（未使用時は 0）
    uint32_t textureHandle_ = 0u;
};
