#pragma once

#include "3d/WorldTransform.h"
#include "3d/Camera.h"
#include "math/Vector3.h"
#include <memory>

#include "Application/Characters/Collider.h"

/// <summary>
/// キャラクター共通の基底クラス
/// ワールド変換・移動量・コライダー・HP など、
/// プレイヤー／敵などのキャラで共通となる機能を提供する。
/// </summary>
class CharacterBase {
public:
    /// <summary>
    /// 仮想デストラクタ
    /// </summary>
    virtual ~CharacterBase() = default;

    /// <summary>
    /// 初期化処理
    /// </summary>
    virtual void Initialize();

    /// <summary>
    /// 更新処理
    /// </summary>
    virtual void Update();

    /// <summary>
    /// 描画処理
    /// </summary>
    /// <param name="camera">描画に使用するカメラ</param>
    virtual void Draw(const KamataEngine::Camera* camera) = 0;

    /// <summary>
    /// 当たり判定時の処理
    /// </summary>
    /// <param name="enemy">衝突相手となるキャラクター</param>
    virtual void OnCollision(CharacterBase* enemy) = 0;

    /// <summary>
    /// 死亡判定
    /// </summary>
    virtual bool IsDead()const { return false; }

    /// <summary>
    /// 当たり判定の有効（死亡中/無敵時間などで使用）
    /// </summary>
    virtual bool IsCollidable()const { return true; }

public:  /// === Getters === ///
    /// <summary>
    /// ワールド座標の取得
    /// </summary>
    KamataEngine::Vector3 GetWorldTranslation();

    /// <summary>
    /// WorldTransform の参照取得（編集や親子付けに使用）
    /// </summary>
    KamataEngine::WorldTransform& GetWorldTransform() { return worldTransform_; }

    /// <summary>
    /// 座標の取得
    /// </summary>
    KamataEngine::Vector3 GetTranslation() { return worldTransform_.translation_; }

    /// <summary>
    /// 角度の取得
    /// </summary>
    KamataEngine::Vector3 GetRotation() { return worldTransform_.rotation_; }

    /// <summary>
    /// 回転の取得
    /// </summary>
    KamataEngine::Vector3 GetRotate() { return worldTransform_.rotation_; }

    /// <summary>
    /// コライダーの取得
    /// </summary>
    std::unique_ptr<Collider>& GetCollider() { return collider_; }

    /// <summary>
    /// HPの取得
    /// </summary>
    int32_t GetHP() { return hp_; }

    /// <summary>
    /// 最大HPの取得
    /// </summary>
    int32_t GetMaxHP() { return maxHp_; }

public:  /// === Setters === ///
    /// <summary>
    /// 親の設定（RailCamera などの WT を渡す）
    /// </summary>
    /// <param name="parent">親となる WorldTransform（保持は非所有）</param>
    void SetParent(KamataEngine::WorldTransform* parent) { worldTransform_.parent_ = parent; }

    /// <summary>
    /// 座標の設定
    /// </summary>
    /// <param name="translate">modelの座標</param>
    void SetTranslate(KamataEngine::Vector3 translate) { worldTransform_.translation_ = translate; }

    /// <summary>
    /// 回転の設定
    /// </summary>
    /// <param name="rotate">modelの向き</param>
    void SetRotate(KamataEngine::Vector3 rotate) { worldTransform_.rotation_ = rotate; }

    /// <summary>
    /// 大きさの設定
    /// </summary>
    /// <param name="scale">modelの大きさ</param>
    void SetScale(KamataEngine::Vector3 scale) { worldTransform_.scale_ = scale; }

    /// <summary>
    /// HPの設定
    /// </summary>
    void SetHP(int32_t hp) { hp_ = hp; }

protected:
    // ワールド変換情報
    KamataEngine::WorldTransform worldTransform_;

    // 速度
    KamataEngine::Vector3 velocity_ = { 0.0f,0.0f,0.0f };

    // 当たり判定
    std::unique_ptr<Collider> collider_;

    // 体力
    int32_t hp_ = 0;

    // 最大体力
    int32_t maxHp_ = 0;
};
