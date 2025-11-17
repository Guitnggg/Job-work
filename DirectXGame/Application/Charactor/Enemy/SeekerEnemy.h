#pragma once

#include <memory>

#include "3d/WorldTransform.h"
#include "3d/Model.h"
#include "3d/Camera.h"

#include "Application/Charactor/CharactorBase.h"

class SeekerEnemy : public CharactorBase {
public:

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 更新
    /// </summary>
    void Update() override;

    /// <summary>
    /// 描画
    /// </summary>
    /// <param name="camera"></param>
    void Draw(KamataEngine::Camera* camera) override;

    /// <summary>
    /// 当たり判定時の処理
    /// </summary>
    /// <param name="enemy"></param>
    void OnCollision(CharactorBase* enemy) override;

    /// <summary>
    /// 
    /// </summary>
    /// <returns></returns>
    bool IsDead()const override { return isDead_; }

public:

    /// <summary>
    /// ターゲットの設定
    /// </summary>
    /// <param name="worldTarget"></param>
    void SetTarget(const KamataEngine::Vector3& worldTarget) { targetPos_ = worldTarget; hasTarget_ = true; }

    /// <summary>
    /// 速度の設定
    /// </summary>
    /// <param name="s"></param>
    void SetSpeed(float s) { speed_ = s; }

    /// <summary>
    /// 旋回率の設定
    /// </summary>
    /// <param name="r"></param>
    void SetTurnRate(float r) { turnRate_ = r; }

    /// <summary>
    /// 当たり判定半径の設定
    /// </summary>
    /// <param name="r"></param>
    void SetColliderRadius(float r) { colliderRadius_ = r; if (collider_) collider_->SetRadius(r); }

    /// <summary>
    /// HPの設定
    /// </summary>
    /// <param name="hp"></param>
    void SetInitialHP(int hp) { initialHP_ = hp; }

    /// <summary>
    /// 寿命の設定
    /// </summary>
    /// <param name="sec"></param>
    void SetLifeTime(float sec) { lifeTimeSec_ = sec; }

    /// <summary>
    /// 初期位置の設定
    /// </summary>
    /// <param name="p"></param>
    void SetInitialPosition(const KamataEngine::Vector3& p) { initialPosition_ = p; }

private:

    // モデル
    std::unique_ptr<KamataEngine::Model> model_;
    uint32_t textureHandle_ = 0u;

    // 動作パラメータ（デフォルト値）
    KamataEngine::Vector3 initialPosition_ = { 0.0f, 0.0f, 80.0f };
    float speed_ = 0.2f;           // 前進速度（単位: units/sec）
    float turnRate_ = 0.15f;       // 旋回追従率（0〜1、小さいほど鈍い）
    float colliderRadius_ = 1.0f;  // 球コライダー半径
    int   initialHP_ = 1;          // 初期HP
    float lifeTimeSec_ = 30.0f;    // 寿命

    // 目標
    KamataEngine::Vector3 targetPos_ = { 0.0f, 0.0f, 0.0f };
    bool hasTarget_ = false;

    // タイマー/死亡
    float timeSec_ = 0.0f;
    bool isDead_ = false;

    // ====== ユーティリティ ======
    void ClampDeathByBounds_(); // 範囲外/寿命チェック
};

