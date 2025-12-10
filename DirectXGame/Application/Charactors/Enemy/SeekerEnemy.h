#pragma once

#include <memory>

#include "3d/WorldTransform.h"
#include "3d/Model.h"
#include "3d/Camera.h"

#include "Application/Charactors/CharactorBase.h"

/// <summary>
/// プレイヤーやターゲットへ向かって旋回追尾する敵キャラクター。
/// ダメージフラッシュ・ヒットストップ・ノックバックなどの演出を備える。
/// </summary>
class SeekerEnemy : public CharactorBase {
public:
    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update() override;

    /// <summary>
    /// 描画処理
    /// </summary>
    /// <param name="camera">描画に使用するカメラ</param>
    void Draw(KamataEngine::Camera* camera) override;

    /// <summary>
    /// 当たり判定
    /// </summary>
    /// <param name="other">衝突相手となるキャラクター</param>
    void OnCollision(CharactorBase* other) override;

    /// <summary>
    /// 生存フラグ
    /// </summary>
    bool IsDead() const override { return isDead_; }

public:  // ==== 外部から設定するパラメータ ====
    /// <summary>
    /// ターゲットの設定（プレイヤー位置など）
    /// </summary>
    void SetTarget(const KamataEngine::Vector3& worldTarget) { targetPos_ = worldTarget; hasTarget_ = true; }

    /// <summary>
    /// 速度の設定
    /// </summary>
    void SetSpeed(float s) { speed_ = s; }

    /// <summary>
    /// 旋回率の設定
    /// </summary>
    void SetTurnRate(float r) { turnRate_ = r; }

    /// <summary>
    /// 当たり判定半径の設定
    /// </summary>
    void SetColliderRadius(float r) { colliderRadius_ = r; if (collider_) collider_->SetRadius(r); }

    /// <summary>
    /// HPの設定
    /// </summary>
    void SetInitialHP(int hp) { initialHP_ = hp; }

    /// <summary>
    /// 寿命の設定
    /// </summary>
    void SetLifeTime(float sec) { lifeTimeSec_ = sec; }

    /// <summary>
    /// 初期位置の設定
    /// </summary>
    void SetInitialPosition(const KamataEngine::Vector3& p) { initialPosition_ = p; }

private:
    // 状態
    enum class State {
        Active,   // 通常行動
        HitStop   // ダメージフラッシュ中＆その場ストップ
    };

private:
    // モデル
    std::unique_ptr<KamataEngine::Model> model_;
    uint32_t textureHandle_ = 0u; // 必要なら使う

    // 動作パラメータ
    KamataEngine::Vector3 initialPosition_ = { 0.0f, 0.0f, 80.0f };
    float speed_ = 0.2f;
    float turnRate_ = 0.15f;
    float colliderRadius_ = 1.0f;
    int   initialHP_ = 1;
    float lifeTimeSec_ = 30.0f;

    // 目標
    KamataEngine::Vector3 targetPos_ = { 0.0f, 0.0f, 0.0f };
    bool hasTarget_ = false;

    // 時間・死亡フラグ
    float timeSec_ = 0.0f;
    bool  isDead_ = false;

    // ==== 演出用 ====
    State state_ = State::Active;

    // ダメージフラッシュ
    float flashTimer_ = 0.0f;
    float flashDuration_ = 0.15f;

    // ヒットストップ
    float hitStopTimer_ = 0.0f;
    float hitStopDuration_ = 0.12f;

    // ヒットストップ終了後に消滅するか？
    bool pendingExplode_ = false;

    // 基本スケール（スケールパンチ用）
    KamataEngine::Vector3 baseScale_{ 1.0f, 1.0f, 1.0f };

    // ＝＝＝ 被弾モーション（ノックバック＋回転）用 ＝＝＝
    KamataEngine::Vector3 hitBasePos_{ 0.0f, 0.0f, 0.0f };  // 被弾時の基準位置
    KamataEngine::Vector3 hitDir_{ 0.0f, 0.0f, 0.0f };      // ノックバック方向
    float hitBaseRollZ_ = 0.0f;            // 被弾前のZ回転
    float hitMotionTimer_ = 0.0f;
    float hitMotionDuration_ = 0.28f;      // モーション時間
    float hitKnockback_ = 2.5f;       // ノックバック距離
    float hitRollRad_ = 1.0f;      // 最大ロール角（ラジアン）

private:
    /// <summary>範囲外や寿命による強制死亡チェック</summary>
    void ClampDeathByBounds_();
};
