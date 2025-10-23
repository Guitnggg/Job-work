#pragma once

#include <algorithm>

#include "3d/WorldTransform.h"
#include "3d/Model.h"
#include "3d/Camera.h"
#include "2d/Sprite.h"
#include "base/TextureManager.h"
#include "audio/Audio.h"
#include "math/Vector2.h"

class CountDown{
public:

    enum class Phase { Inactive, ReadyDelay, Count3, Count2, Count1, Go, Done };

    CountDown() = default;
    ~CountDown();


    void InitializeFromPaths(const char* tex3, const char* tex2, const char* tex1, const char* texGo,
        const KamataEngine::Vector2& centerPos = { 640.0f,360.0f },
        const KamataEngine::Vector2& baseSizeCount = { 256.0f,256.0f },
        const KamataEngine::Vector2& baseGoSize = { 320.0f,160.0f }
    );


    void InitializeFromHandles(uint32_t tex3, uint32_t tex2, uint32_t tex1, uint32_t texGo,
        const KamataEngine::Vector2& centerPos = { 640.0f, 360.0f },
        const KamataEngine::Vector2& baseSizeCount = { 256.0f, 256.0f },
        const KamataEngine::Vector2& baseSizeGo = { 320.0f, 160.0f }
    );

    // 
    void SetTimings(float readyDeley, float countUnit, float goDuraction);
    void SetScaleRange(float startScale, float endScale);
    void SetBackOvershoot(float s);
    void SetAudio(uint32_t seBeep, uint32_t seGo);

    // 制御
    void Start();
    void Reset();
    void Update(float dt); 
    void Draw();

    // 状態問い合わせ
    bool IsActive() const { return phase_ != Phase::Inactive; }
    bool IsDone()   const { return phase_ == Phase::Done; }
    bool IsInputLocked() const { return inputLocked_; }
    Phase GetPhase() const { return phase_; }

private:

    float EaseoutBack(float t, float s)const;
    float CurrentPhaseDuraction()const;
    KamataEngine::Sprite* CurrentPhaseSprite()const;
    float CurrentPhaseAlpha(float t01)const;
    float CurrentPhaseScale(float t01)const;
    void AdvancePhase();

private:

    // 
    KamataEngine::Sprite* sp3_ = nullptr;
    KamataEngine::Sprite* sp2_ = nullptr;
    KamataEngine::Sprite* sp1_ = nullptr;
    KamataEngine::Sprite* spGo_ = nullptr;

    // 見た目
    KamataEngine::Vector2 center_ = { 640.0f, 360.0f };
    KamataEngine::Vector2 baseSizeCount_ = { 256.0f, 256.0f };
    KamataEngine::Vector2 baseSizeGo_ = { 320.0f, 160.0f };
    float scaleStart_ = 1.20f;
    float scaleEnd_ = 1.00f;
    float backS_ = 1.70f;

    // タイミング
    float readyDelay_ = 0.10f;
    float countUnit_ = 0.50f;
    float goDuration_ = 0.40f;

    // SE
    uint32_t seBeep_ = 0;
    uint32_t seGo_ = 0;
    bool goPlayed_ = false;

    // 進行
    Phase phase_ = Phase::Inactive;
    float phaseTimer_ = 0.0f;
    bool inputLocked_ = true;
};

