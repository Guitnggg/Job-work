#pragma once

#include "3d/WorldTransform.h"
#include "3d/Model.h"
#include "3d/Camera.h"
#include "input/Input.h"
#include "audio/Audio.h"

class Player {
public:

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~Player();

    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize(KamataEngine::Camera* camera);

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw();

public:

    /// <summary>
    /// 親となるワールドトランスフォーム
    /// </summary>
    void SetParent(const KamataEngine::WorldTransform* parent);

    /// <summary>
    /// ワールド座標の取得
    /// </summary>
    /// <returns></returns>
    KamataEngine::Vector3 GetWorldTranslation()const;

    // ===== HP / ライフ制御 =====
    void SetHP(int hp) { hp_ = hp; }
    int  GetHP() const { return hp_; }
    void Damage(int amount);     // HPを減らし、0以下ならKill
    void Kill();                 // 即爆発開始

    // 状態取得
    bool IsDead() const { return isDead_; }
    bool IsExploding() const { return isExploding_; }
    bool IsExplosionFinished() const { return isExplosionFinished_; }

private:

    /// <summary>
    /// ロール制御
    /// </summary>
    /// <param name="dir">ロール方向</param>
    void StartRoll(float dir);
    float EaseOutCubic(float t)const;


    void UpdateExplosion_();

private:

    KamataEngine::WorldTransform* worldTransform_;
    KamataEngine::Camera* camera_;
    KamataEngine::Model* model_ = nullptr;
    KamataEngine::Input* input_ = nullptr;
    KamataEngine::Audio* audio_ = nullptr;

    const float kMoveSpeed = 10.0f;  // 移動速度
    const float kRotSpeed = 0.05f;  // 回転速度

    // ロール制御パラメータ
    bool isRolling_ = false;
    float rollFrame_ = 0.0f;
    float rollDurationFrames_ = 24.0f;
    float rollDir_ = 0.0f;                   // +1 or -1
    float rollStartRotZ_ = 0.0f;
    KamataEngine::Vector3 rollStartPos_{};
    KamataEngine::Vector3 rollEndPos_{};
    float rollMoveDistance_ = 5.0f;          // 横にどれだけ移動するか

    // 爆発制御パラメータ
    int seExplosion_ = -1;
    int hp_ = 100;
    bool isDead_ = false;
    bool isExploding_ = false;
    bool isExplosionFinished_ = false;
    int  explosionFrame_ = 0;
    int  explosionDurationFrames_ = 60; // 1秒想定(60fps)
    KamataEngine::Vector3 initialScale_ = { 1.0f, 1.0f, 1.0f };
};