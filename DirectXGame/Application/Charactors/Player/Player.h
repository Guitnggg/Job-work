#pragma once

#include "3d/WorldTransform.h"
#include "3d/Model.h"
#include "3d/Camera.h"
#include "input/Input.h"
#include "audio/Audio.h"

#include "Application/Charactors/CharactorBase.h"

class Player : public CharactorBase {
public:
    /// <summary>
    /// デストラクタ
    /// </summary>
    ~Player() override = default;

    /// <summary>
    /// 初期化処理
    /// </summary>
    /// <param name="camera">使用するカメラへのポインタ</param>
    void Initialize(KamataEngine::Camera* camera);

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
    /// 当たり判定処理
    /// </summary>
    /// <param name="enemy">衝突した相手</param>
    void OnCollision(CharactorBase* enemy) override;

public:
    /// <summary>
    /// ダメージを受ける
    /// </summary>
    /// <param name="amount">受けるダメージ量</param>
    void Damage(int amount);

    /// <summary>
    /// 爆発処理を開始（HP0のとき）
    /// </summary>
    void Kill();

public:
    /// <summary>
    /// 死亡状態か判定
    /// </summary>
    bool IsDead() const { return isDead_; }

    /// <summary>
    /// 爆発中か判定
    /// </summary>
    bool IsExploding() const { return isExploding_; }

    /// <summary>
    /// 爆発演出が終わったか判定
    /// </summary>
    bool IsExplosionFinished() const { return isExplosionFinished_; }

public:
    /// <summary>
    /// 入力を受け付けるか？
    /// </summary>
    /// <param name="enabled">trueで入力可能にする</param>
    void SetInputEnabled(bool enabled) { inputEnabled_ = enabled; }

    /// <summary>
    /// 無敵状態か？
    /// ロール回避中も無敵
    /// </summary>
    bool IsInvincible() const { return invincibleFrames_ > 0 || isRolling_; }

    /// <summary>
    /// 被弾イベントを一度だけ取得
    /// </summary>
    bool ConsumeTookDamageEvent() { bool f = tookDamageEvent_; tookDamageEvent_ = false; return f; }

    /// <summary>
    /// 被弾エフェクトの進行具合（0～1）
    /// </summary>
    float HitFlashT() const { return hitFlashFrames_ > 0 ? (float)hitFlashFrames_ / (float)kHitFlashDuration_ : 0.0f; }

private:
    /// <summary>
    /// EaseOutCubic（緩やかに減速する補間）
    /// </summary>
    float EaseOutCubic(float t) const;

    /// <summary>
    /// 爆発アニメーションの更新
    /// </summary>
    void UpdateExplosion();

    /// <summary>
    /// 通常移動と傾き処理
    /// </summary>
    void UpdateMoveAndBank_(float dt);

    /// <summary>
    /// ロール回転を開始（dir = +1: 右回避, -1: 左回避）
    /// </summary>
    void StartRoll(float dir);

    /// <summary>
    /// ロール中の更新処理
    /// </summary>
    bool UpdateRoll_();

private:
    // ===== 参照 =====
    KamataEngine::Camera* camera_ = nullptr;  // カメラ
    KamataEngine::Model* model_ = nullptr;    // モデル
    KamataEngine::Input* input_ = nullptr;    // 入力
    KamataEngine::Audio* audio_ = nullptr;    // オーディオ

    // ===== 通常移動・傾き =====
    const float kMoveSpeedXY_ = 10.0f;   // 画面内のXY移動速度
    const float kBankMaxRadZ_ = 0.35f;   // 最大傾き角度
    const float kPitchMaxRadX_ = 0.25f;  // 最大ピッチ角
    const float kTiltLerp_ = 0.20f;      // 傾きの追従速度

    float targetTiltZ_ = 0.0f;   // 入力から求めた目標z傾き
    float targetTiltX_ = 0.0f;   // 入力から求めた目標x傾き
    float currentTiltZ_ = 0.0f;  // 実際のz傾き
    float currentTiltX_ = 0.0f;  // 実際のx傾き

    float clampXMin_ = -8.0f, clampXMax_ = 8.0f;  // ｘ軸の画面端の制限
    float clampYMin_ = -4.0f, clampYMax_ = 3.0f;  // ｙ軸の画面端の制限

    // ===== ロール回避 =====
    bool  isRolling_ = false;           // ロール中フラグ
    float rollFrame_ = 0.0f;            // ロール経過フレーム
    float rollDurationFrames_ = 24.0f;  // ロールにかかる時間（フレーム）
    float rollDir_ = 0.0f;              // 回転方向（+1 = 右, -1 = 左）
    float rollStartRotZ_ = 0.0f;        // 開始時のｚ角度

    KamataEngine::Vector3 rollStartPos_{};  // 開始位置
    KamataEngine::Vector3 rollEndPos_{};    // 終了位置
    float rollMoveDistance_ = 7.0f;         // ロールで横にスライドする距離

    // --- ダブルタップ検出（A/D）---
    int  doubleTapFrameA_ = 0;            // Aキーの２回押し検出カウンタ
    int  doubleTapFrameD_ = 0;            // Dキーの２回押し検出カウンタ
    const int kDoubleTapThreshold_ = 18;  // 約0.3秒@60fps

    // ===== 爆発・死亡 =====
    int   seExplosion_ = -1;              // 爆発SE
    bool  isDead_ = false;                // 死亡フラグ
    bool  isExploding_ = false;           // 爆発中フラグ
    bool  isExplosionFinished_ = false;   // 爆発終了フラグ
    int   explosionFrame_ = 0;            // 爆発アニメ進行
    int   explosionDurationFrames_ = 60;  // 爆発時間
    KamataEngine::Vector3 initialScale_ = { 1.0f, 1.0f, 1.0f };  // 通常スケール

    // ===== 被弾・無敵 =====
    bool  inputEnabled_ = true;           // 入力有効フラグ
    bool  tookDamageEvent_ = false;       // このフレームで被弾したか
    int   hitFlashFrames_ = 0;            // 被弾中の演出フレーム
    int   invincibleFrames_ = 0;          // 点滅用
    const int kHitFlashDuration_ = 18;    // 0.3秒@60fps
    const int kInvincibleDuration_ = 30;  // 無敵持続時間
    float lastHitRollOffset_ = 0.0f;      // 前フレームで加えた被弾ゆれZオフセット
};
