#include "CountDown.h"


using namespace KamataEngine;

CountDown::~CountDown() {
    delete sp1_;
    delete sp2_;
    delete sp3_;
    delete spGo_;
}

void CountDown::InitializeFromPaths(const char* tex3, const char* tex2, const char* tex1, const char* texGo,
    const KamataEngine::Vector2& centerPos, const KamataEngine::Vector2& baseSizeCount, const KamataEngine::Vector2& baseGoSize) {
    // 
    InitializeFromHandles(
        TextureManager::Load(tex3),
        TextureManager::Load(tex2),
        TextureManager::Load(tex1),
        TextureManager::Load(texGo),
        centerPos, baseSizeCount, baseGoSize
    );
}

void CountDown::InitializeFromHandles(uint32_t tex3, uint32_t tex2, uint32_t tex1, uint32_t texGo,
    const KamataEngine::Vector2& centerPos, const KamataEngine::Vector2& baseSizeCount, const KamataEngine::Vector2& baseSizeGo) {
    center_ = centerPos;
    baseSizeCount_ = baseSizeCount;
    baseSizeGo_ = baseSizeGo;

    auto makeCentered = [&](uint32_t tex) {
        return Sprite::Create(tex, center_, { 1,1,1,0 }, { 0.5f,0.5f }, false, false);
        };
    delete sp3_; delete sp2_; delete sp1_; delete spGo_;
    sp3_ = makeCentered(tex3);
    sp2_ = makeCentered(tex2);
    sp1_ = makeCentered(tex1);
    spGo_ = makeCentered(texGo);

    phase_ = Phase::Inactive;
    phaseTimer_ = 0.0f;
    inputLocked_ = true;
    goPlayed_ = false;
}

void CountDown::SetTimings(float readyDeley, float countUnit, float goDuraction) {
    readyDelay_ = readyDeley;
    countUnit_ = countUnit;
    goDuration_ = goDuraction;
}

void CountDown::SetScaleRange(float startScale, float endScale) {
    scaleStart_ = startScale;
    scaleEnd_ = endScale;
}

void CountDown::SetBackOvershoot(float s) { backS_ = s; }

void CountDown::SetAudio(uint32_t seBeep, uint32_t seGo) {
    seBeep_ = seBeep;
    seGo_ = seGo;
}

void CountDown::Start() {
    phase_ = Phase::ReadyDelay;
    phaseTimer_ = 0.0f;
    inputLocked_ = true;
    goPlayed_ = false;
}

void CountDown::Reset() {
    phase_ = Phase::Inactive;
    phaseTimer_ = 0.0f;
    inputLocked_ = true;
    goPlayed_ = false;

}

void CountDown::Update(float dt) {
    if (phase_ == Phase::Inactive || phase_ == Phase::Done) { return; }

    phaseTimer_ += dt;
    float duraction = CurrentPhaseDuraction();

    // フェース開始のタイミングのSE
    if (phaseTimer_ == dt) {
        if (phase_ == Phase::Count3 || phase_ == Phase::Count2 || phase_ == Phase::Count1) {
            if (seBeep_) {
                Audio::GetInstance()->PlayWave(seBeep_);
            }
        }

        if (phase_ == Phase::Go) {
            if (!goPlayed_ && seGo_) {
                Audio::GetInstance()->PlayWave(seGo_);
                goPlayed_ = true;
            }

            inputLocked_ = false;
        }
    }

    if (phaseTimer_ >= duraction) {
        AdvancePhase();
    }
}

void CountDown::Draw() {
    if (phase_ == Phase::Inactive || phase_ == Phase::Done) { return; }

    float duraction = CurrentPhaseDuraction();
    float t01 = (duraction > 0.0f) ? std::clamp(phaseTimer_ / duraction, 0.0f, 1.0f) : 1.0f;

    if (auto* spr = CurrentPhaseSprite()) {
        float alpha = CurrentPhaseAlpha(t01);
        float scale = CurrentPhaseScale(t01);

        spr->SetColor({ 1.0f,1.0f,1.0f,alpha });

        // フェーズごとにサイズを切り替え
        Vector2 base = (phase_ == Phase::Go) ? baseSizeGo_ : baseSizeCount_;
        spr->SetSize({ base.x * scale,base.y * scale });
        spr->SetPosition(center_);
        spr->Draw();
    }
}

float CountDown::EaseoutBack(float t, float s)const {
    t -= 1.0f;
    return t * t * ((s + 1.0f) * t + s) + 1.0f;
}

float CountDown::CurrentPhaseDuraction() const{
    switch (phase_) {
    case Phase::ReadyDelay:
        return readyDelay_;

    case Phase::Count3:
        return countUnit_;

    case Phase::Count2:
        return countUnit_;

    case Phase::Count1:
        return countUnit_;

    case Phase::Go:
        return goDuration_;

    default:
        return 0.0f;
    }
}

Sprite* CountDown::CurrentPhaseSprite()const {
    switch (phase_) {
    case Phase::Count3:
        return sp3_;

    case Phase::Count2:
        return sp2_;

    case Phase::Count1:
        return sp1_;

    case Phase::Go:
        return spGo_;

    default:
        return nullptr;
    }
}

float CountDown::CurrentPhaseAlpha(float t01) const{
    // 0->1->0の三角波
    if (t01 > 0.5f) {
        return t01 / 0.5f;
    }

    return 1.0f - (t01 / 0.5f) / 0.5f;
}

float CountDown::CurrentPhaseScale(float t01)const {
    // easeOutBackで1.20->1.00
    float e = EaseoutBack(std::clamp(t01, 0.0f, 1.0f), backS_);

    return scaleStart_ + (scaleEnd_ - scaleStart_) * e;
}

void CountDown::AdvancePhase() {
    phaseTimer_ = 0.0f;

    switch (phase_) {
    case Phase::ReadyDelay:
        phase_ = Phase::Count3;
        break;

    case Phase::Count3:
        phase_ = Phase::Count2;
        break;

    case Phase::Count2:
        phase_ = Phase::Count1;
        break;

    case Phase::Count1:
        phase_ = Phase::Go;
        goPlayed_ = false;
        break;

    case Phase::Go:
        phase_ = Phase::Done;
        break;

    default:
        break;
    }
}