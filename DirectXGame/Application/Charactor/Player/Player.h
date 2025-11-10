#pragma once

#include "3d/WorldTransform.h"
#include "3d/Model.h"
#include "3d/Camera.h"
#include "input/Input.h"
#include "audio/Audio.h"

#include "Application/Charactor/CharactorBase.h"

class Player :public CharactorBase {
public:

    ~Player()override = default;

    void Initialize(KamataEngine::Camera* camera);

    void Update()override;

    void Draw(KamataEngine::Camera* camera)override;

    void OnCollision(CharactorBase* enemy)override;

    void Damage(int amount);
    void Kill();

    bool IsDead() const { return isDead_; }
    bool IsExploding() const { return isExploding_; }
    bool IsExplosionFinished() const { return isExplosionFinished_; }
    void SetInputEnabled(bool enabled) { inputEnabled_ = enabled; }

    bool ConsumeTookDamageEvent() { bool f = tookDamageEvent_; tookDamageEvent_ = false; return f; }
    bool IsInvincible() const { return invincibleFrames_ > 0; }
    float HitFlashT() const {
        return hitFlashFrames_ > 0 ? (float)hitFlashFrames_ / (float)kHitFlashDuration_ : 0.0f;
    }

private:
    void StartRoll(float dir);
    float EaseOutCubic(float t) const;
    void UpdateExplosion();

private:
    KamataEngine::Camera* camera_ = nullptr;
    KamataEngine::Model* model_ = nullptr;
    KamataEngine::Input* input_ = nullptr;
    KamataEngine::Audio* audio_ = nullptr;

    const float kMoveSpeed = 10.0f;
    const float kRotSpeed = 0.05f;

    bool inputEnabled_ = true;
    bool isRolling_ = false;
    float rollFrame_ = 0.0f;
    float rollDurationFrames_ = 24.0f;
    float rollDir_ = 0.0f;
    float rollStartRotZ_ = 0.0f;
    KamataEngine::Vector3 rollStartPos_{};
    KamataEngine::Vector3 rollEndPos_{};
    float rollMoveDistance_ = 5.0f;

    int seExplosion_ = -1;
    bool isDead_ = false;
    bool isExploding_ = false;
    bool isExplosionFinished_ = false;
    int explosionFrame_ = 0;
    int explosionDurationFrames_ = 60;
    KamataEngine::Vector3 initialScale_ = { 1.0f, 1.0f, 1.0f };

    bool tookDamageEvent_ = false;          // このフレームで被弾したか（GameSceneのシェイク起動用）
    int  hitFlashFrames_ = 0;               // ヒット演出（モデル揺れ/スケール脈動）の残りフレーム
    int  invincibleFrames_ = 0;             // 無敵残りフレーム（点滅制御）
    const int kHitFlashDuration_ = 18;      // 0.3秒@60fps
    const int kInvincibleDuration_ = 30;    // 0.5秒@60fps
};