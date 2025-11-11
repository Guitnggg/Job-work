#pragma once

#include "3d/WorldTransform.h"
#include "3d/Model.h"
#include "3d/Camera.h"
#include "input/Input.h"
#include "audio/Audio.h"

#include "Application/Charactor/CharactorBase.h"

class Player :public CharactorBase {
public:

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~Player()override = default;

    /// <summary>
    /// 初期化処理
    /// </summary>
    /// <param name="camera">描画で使用するカメラへのポインタ</param>
    void Initialize(KamataEngine::Camera* camera);

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update()override;

    /// <summary>
    /// 描画処理
    /// </summary>
    /// <param name="camera">描画に使用するカメラへのポインタ</param>
    void Draw(KamataEngine::Camera* camera)override;

    /// <summary>
    /// 当たり判定処理
    /// </summary>
    /// <param name="enemy">衝突相手キャラクターへのポインタ</param>
    void OnCollision(CharactorBase* enemy)override;

    /// <summary>
    /// ダメージ処理
    /// </summary>
    /// <param name="amount">減少させるHP量</param>
    void Damage(int amount);

    /// <summary>
    /// プレイヤー
    /// </summary>
    void Kill();

    /// <summary>
    /// 死亡しているがどうか
    /// </summary>
    bool IsDead() const { return isDead_; }

    /// <summary>
    /// 爆発のアニメーション
    /// </summary>
    bool IsExploding() const { return isExploding_; }

    /// <summary>
    /// 爆発のアニメーションが終了したかどうか
    /// </summary>
    bool IsExplosionFinished() const { return isExplosionFinished_; }

    /// <summary>
    /// 入力の有効/無効を設定する
    /// </summary>
    void SetInputEnabled(bool enabled) { inputEnabled_ = enabled; }

    /// <summary>
    /// 被弾したかどうかを判定して、内部フラグをリセットする
    /// </summary>
    bool ConsumeTookDamageEvent() { bool f = tookDamageEvent_; tookDamageEvent_ = false; return f; }

    /// <summary>
    /// 現在無敵状態かどうか返す
    /// </summary>
    bool IsInvincible() const { return invincibleFrames_ > 0; }

    /// <summary>
    /// 被弾演出の経過率を返す
    /// </summary>
    float HitFlashT() const { return hitFlashFrames_ > 0 ? (float)hitFlashFrames_ / (float)kHitFlashDuration_ : 0.0f; }

private:

    /// <summary>
    /// 左右ロール回避を開始する
    /// </summary>
    /// <param name="dir">ロール方向（-1: 左/ +1: 右）</param>
    void StartRoll(float dir);

    /// <summary>
    /// 減速する補間関数
    /// </summary>
    /// <param name="t">経過率（0.0～1.0）</param>
    /// <returns></returns>
    float EaseOutCubic(float t) const;

    /// <summary>
    /// 爆発のアニメーションの更新
    /// </summary>
    void UpdateExplosion();

private:
    // ===== 各種エンジン参照 =====
    KamataEngine::Camera* camera_ = nullptr;  // カメラ
    KamataEngine::Model* model_ = nullptr;    // モデル
    KamataEngine::Input* input_ = nullptr;    // 入力
    KamataEngine::Audio* audio_ = nullptr;    // サウンド

    // ===== 移動・ロール関連 =====
    const float kMoveSpeed = 10.0f;         // 移動速度
    const float kRotSpeed = 0.05f;          // 回転速度
    bool inputEnabled_ = true;              // 入力受付
    bool isRolling_ = false;                // ロール中フラグ
    float rollFrame_ = 0.0f;                // ロールの経過フレーム
    float rollDurationFrames_ = 24.0f;      // ロールにかけるフレーム数
    float rollDir_ = 0.0f;                  // ロール方向（-1 or 1）
    float rollStartRotZ_ = 0.0f;            // 開始時のz回転
    KamataEngine::Vector3 rollStartPos_{};  // 開始位置
    KamataEngine::Vector3 rollEndPos_{};    // 終了位置
    float rollMoveDistance_ = 5.0f;         // ロール中の移動距離

    // ===== 爆発・死亡関連 =====
    int seExplosion_ = -1;              // 爆発SE ID
    bool isDead_ = false;               // 死亡フラグ
    bool isExploding_ = false;          // 爆発のアニメーション中
    bool isExplosionFinished_ = false;  // 爆発のアニメーション完了
    int explosionFrame_ = 0;            // 爆発経過フレーム
    int explosionDurationFrames_ = 60;  // 爆発の継続フレーム
    KamataEngine::Vector3 initialScale_ = { 1.0f, 1.0f, 1.0f };  // 初期スケール保持

    // =====被弾・無敵関連 =====
    bool tookDamageEvent_ = false;          // このフレームで被弾したか（GameSceneのシェイク起動用）
    int  hitFlashFrames_ = 0;               // ヒット演出（モデル揺れ/スケール脈動）の残りフレーム
    int  invincibleFrames_ = 0;             // 無敵残りフレーム（点滅制御）
    const int kHitFlashDuration_ = 18;      // 0.3秒@60fps
    const int kInvincibleDuration_ = 30;    // 0.5秒@60fps
    float lastHitRollOffset_ = 0.0f;        // 全フレームのロールオフセット
};