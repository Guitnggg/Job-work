#pragma once

#include <memory>
#include <vector>

#include "3d/Camera.h"
#include "3d/Model.h"
#include "3d/ObjectColor.h"
#include "3d/WorldTransform.h"
#include "math/Vector3.h"

#include "Application/Characters/CharacterBase.h"
#include "Application/Characters/Player/Bullet.h"

/// <summary>
/// 固定砲台型の敵キャラクター。
/// ・自身は移動せず、その場でターゲットを追尾
/// ・一定間隔で弾を発射
/// ・発射した弾は砲台内部で管理する
/// </summary>
class TurretEnemy : public CharacterBase {
public:
    /// <summary>初期化処理</summary>
    void Initialize() override;

    /// <summary>更新処理</summary>
    void Update() override;

    /// <summary>描画処理</summary>
    void Draw(const KamataEngine::Camera* camera) override;

    /// <summary>当たり判定時の処理</summary>
    void OnCollision(CharacterBase* other) override;

    /// <summary>
    /// 死亡判定
    /// HP枯渇によって判定する。
    /// </summary>
    bool IsDead() const override { return isDead_ || hp_ <= 0; }

    /// <summary>
    /// 砲台が保持している弾コンテナを取得
    /// （CollisionManager で当たり判定を行う用途）
    /// </summary>
    std::vector<std::unique_ptr<Bullet>>& GetBullets() { return bullets_; }
    const std::vector<std::unique_ptr<Bullet>>& GetBullets() const { return bullets_; }


public: // ---- 外部設定 ----
    /// <summary>砲台が狙うターゲット位置</summary>
    void SetTarget(const KamataEngine::Vector3& worldTarget) {
        targetPos_ = worldTarget;
        hasTarget_ = true;
    }

    /// <summary>射撃間隔（フレーム）</summary>
    void SetShootIntervalFrames(int32_t frames) { shootIntervalFrames_ = frames; }

    /// <summary>弾速度</summary>
    void SetBulletSpeed(float speed) { bulletSpeed_ = speed; }

    /// <summary>弾寿命（秒）</summary>
    void SetBulletLifeTime(float sec) { bulletLifeTimeSec_ = sec; }

    /// <summary>初期HP</summary>
    void SetInitialHP(int32_t hp) { initialHP_ = hp; }

    /// <summary>コライダー半径</summary>
    void SetColliderRadius(float radius);

private:
    /// <summary>行動状態</summary>
    enum class State {
        Active,  // 通常待機
        Shooting // 射撃中（拡張用）
    };

private:
    // ===== 定数 =====
    static constexpr int32_t kDefaultHP = 3;
    static constexpr float kDefaultColliderRadius = 1.2f;

    static constexpr int32_t kDefaultShootIntervalFrames = 60; // 1秒@60fps
    static constexpr float kDefaultBulletSpeed = 2.8f;
    static constexpr float kDefaultBulletLifeTimeSec = 3.0f;

    // 見た目スケール
    static constexpr KamataEngine::Vector3 kTurretScale{ 1.2f, 1.2f, 1.2f };

    // 砲口オフセット（砲身表現用）
    static constexpr float kMuzzleForward = 1.0f;

private:
    // ---- 見た目 ----
    std::unique_ptr<KamataEngine::Model> model_;
    KamataEngine::ObjectColor objectColor_;
    KamataEngine::ObjectColor flashColor_;
    uint32_t textureHandle_ = 0u;
    uint32_t flashTextureHandle_ = 0u;

    // ---- 砲台パラメータ ----
    State state_ = State::Active;

    int32_t initialHP_ = kDefaultHP;
    float colliderRadius_ = kDefaultColliderRadius;

    // ターゲット
    KamataEngine::Vector3 targetPos_{};
    bool hasTarget_ = false;

    // 射撃タイマー
    int32_t shootTimerFrames_ = 0;
    int32_t shootIntervalFrames_ = kDefaultShootIntervalFrames;

    // 弾パラメータ
    float bulletSpeed_ = kDefaultBulletSpeed;
    float bulletLifeTimeSec_ = kDefaultBulletLifeTimeSec;

    // 弾コンテナ（砲台が所有）
    std::vector<std::unique_ptr<Bullet>> bullets_;

    // 被弾フィードバック
    float flashTimer_ = 0.0f;
    float shakeTimer_ = 0.0f;
    KamataEngine::Vector3 baseTranslation_{ 0.0f,0.0f,0.0f };

    static constexpr float kFlashDuration = 0.1f;
    static constexpr float kShakeDuration = 0.12f;
    static constexpr float kShakePower = 0.22f;

    // 死亡フラグ
    bool isDead_ = false;

private:
    /// <summary>ターゲット方向へ向く（Yawのみ）</summary>
    void AimToTarget_();

    /// <summary>弾の発射処理</summary>
    void Fire_();

    /// <summary>弾の更新と不要弾の削除</summary>
    void UpdateBullets_();
};
