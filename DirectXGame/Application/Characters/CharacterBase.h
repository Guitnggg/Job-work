#pragma once

#include "3d/Camera.h"
#include "3d/WorldTransform.h"
#include "math/Vector3.h"
#include <memory>

#include "Application/Characters/Collider.h"

/// <summary>
/// キャラクター共通の基底クラス。
/// ワールド変換・当たり判定・HP など、
/// プレイヤーや敵に共通する最低限の振る舞いを提供する。
/// </summary>
class CharacterBase {
public:
    /// <summary>
    /// 最後に受けたダメージソース
    /// </summary>
    enum class DamageSource {
        Unknown,
        Bullet,
        HomingMissile,
        Collision
    };

public:
    /// <summary>
    /// 仮想デストラクタ
    /// </summary>
    virtual ~CharacterBase() = default;

    /// <summary>
    /// 初期化処理
    /// ワールド変換とコライダーの初期化を行う。
    /// </summary>
    virtual void Initialize();

    /// <summary>
    /// 更新処理
    /// ワールド行列更新とコライダー同期を行う。
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
    /// <param name="enemy">衝突相手</param>
    virtual void OnCollision(CharacterBase* enemy) = 0;

    /// <summary>
    /// 死亡判定
    /// </summary>
    virtual bool IsDead() const { return hp_ <= 0; }

    /// <summary>
    /// 当たり判定の有効判定
    /// </summary>
    virtual bool IsCollidable() const { return !IsDead(); }

public: /// === Getters === ///
    /// <summary>
    /// ワールド座標の取得
    /// </summary>
    KamataEngine::Vector3 GetWorldTranslation() const;

    /// <summary>
    /// WorldTransform の参照取得（親子付けなどに使用）
    /// </summary>
    KamataEngine::WorldTransform& GetWorldTransform() { return worldTransform_; }

    /// <summary>
    /// ローカル座標の取得
    /// </summary>
    KamataEngine::Vector3 GetTranslation() const { return worldTransform_.translation_; }

    /// <summary>
    /// 回転の取得
    /// </summary>
    KamataEngine::Vector3 GetRotation() const { return worldTransform_.rotation_; }

    /// <summary>
    /// コライダーの参照取得（所有権は保持しない）
    /// </summary>
    Collider* GetCollider() const { return collider_.get(); }

    /// <summary>
    /// HP の取得
    /// </summary>
    int32_t GetHP() const { return hp_; }

    /// <summary>
    /// 最大 HP の取得
    /// </summary>
    int32_t GetMaxHP() const { return maxHp_; }

    /// <summary>
    /// 最後に受けたダメージソースを取得する
    /// </summary>
    DamageSource GetLastDamageSource() const { return lastDamageSource_; }

public: /// === Setters === ///
    /// <summary>
    /// 親の設定（RailCamera など）
    /// </summary>
    void SetParent(KamataEngine::WorldTransform* parent) { worldTransform_.parent_ = parent; }

    /// <summary>
    /// 座標の設定
    /// </summary>
    void SetTranslate(const KamataEngine::Vector3& translate) { worldTransform_.translation_ = translate; }

    /// <summary>
    /// 回転の設定
    /// </summary>
    void SetRotate(const KamataEngine::Vector3& rotate) { worldTransform_.rotation_ = rotate; }

    /// <summary>
    /// スケールの設定
    /// </summary>
    void SetScale(const KamataEngine::Vector3& scale) { worldTransform_.scale_ = scale; }

    /// <summary>
    /// HP の設定
    /// </summary>
    void SetHP(int32_t hp) { hp_ = hp; }

    /// <summary>
    /// 最後に受けたダメージソースを記録する
    /// </summary>
    void SetLastDamageSource(DamageSource source) { lastDamageSource_ = source; }

protected:
    // ワールド変換情報
    KamataEngine::WorldTransform worldTransform_;

    // 移動速度（派生クラスで使用）
    KamataEngine::Vector3 velocity_{ 0.0f, 0.0f, 0.0f };

    // 当たり判定（CharacterBase が所有）
    std::unique_ptr<Collider> collider_;

    // 体力
    int32_t hp_ = 0;

    // 最大体力
    int32_t maxHp_ = 0;

    // 
    DamageSource lastDamageSource_ = DamageSource::Unknown;
};
