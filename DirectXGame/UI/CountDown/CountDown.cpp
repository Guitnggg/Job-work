#include "CountDown.h"

using namespace KamataEngine;

CountDown::~CountDown() {
    delete sp1_;
    delete sp2_;
    delete sp3_;
    delete spGo_;
}

void CountDown::InitializeFromPaths(const char* tex3, const char* tex2, const char* tex1, const char* texGo,
    const Vector2& centerPos, const Vector2& baseSizeCount, const Vector2& baseGoSize) {
    // 画像パスからテクスチャを読み込む
    InitializeFromHandles(
        TextureManager::Load(tex3),
        TextureManager::Load(tex2),
        TextureManager::Load(tex1),
        TextureManager::Load(texGo),
        centerPos, baseSizeCount, baseGoSize
    );
}

void CountDown::InitializeFromHandles(uint32_t tex3, uint32_t tex2, uint32_t tex1, uint32_t texGo,
    const Vector2& centerPos, const Vector2& baseSizeCount, const Vector2& baseSizeGo) {
    // 表示パラメータ設定
    center_ = centerPos;
    baseSizeCount_ = baseSizeCount;
    baseSizeGo_ = baseSizeGo;

    // 中心揃え・透明状態でスプライトを生成するラムダ
    auto makeCentered = [&](uint32_t tex) {
        return Sprite::Create(tex, center_, { 1,1,1,0 }, { 0.5f,0.5f }, false, false);
        };

    // 既存スプライトを破棄してから再生成
    delete sp3_; delete sp2_; delete sp1_; delete spGo_;
    sp3_ = makeCentered(tex3);
    sp2_ = makeCentered(tex2);
    sp1_ = makeCentered(tex1);
    spGo_ = makeCentered(texGo);

    // 状態を初期化
    phase_ = Phase::Inactive;
    phaseTimer_ = 0.0f;
    inputLocked_ = true;
    goPlayed_ = false;
}

void CountDown::SetTimings(float readyDeley, float countUnit, float goDuraction) {
    // 各フェーズの長さを設定
    readyDelay_ = readyDeley;
    countUnit_ = countUnit;
    goDuration_ = goDuraction;
}

void CountDown::SetScaleRange(float startScale, float endScale) {
    // 拡大アニメーションの開始・終了スケール設定
    scaleStart_ = startScale;
    scaleEnd_ = endScale;
}

void CountDown::SetBackOvershoot(float s) { backS_ = s; }

void CountDown::SetAudio(uint32_t seBeep, uint32_t seGo) {
    // それぞれで使用するSEを設定
    seBeep_ = seBeep;
    seGo_ = seGo;
}

void CountDown::Start() {
    // カウントダウン開始
    phase_ = Phase::ReadyDelay;
    phaseTimer_ = 0.0f;
    inputLocked_ = true;
    goPlayed_ = false;
}

void CountDown::Reset() {
    // 完全リセットして非アクティブ状態へ戻す
    phase_ = Phase::Inactive;
    phaseTimer_ = 0.0f;
    inputLocked_ = true;
    goPlayed_ = false;

}

void CountDown::Update(float dt) {
    // 非アクティブ or 完了状態なら何もしない
    if (phase_ == Phase::Inactive || phase_ == Phase::Done) { return; }

    // 経過時間を加算
    phaseTimer_ += dt;
    float duraction = CurrentPhaseDuraction();

    // フェース開始のタイミングのSE
    if (phaseTimer_ == dt) {
        // 3/2/1開始時のSE
        if (phase_ == Phase::Count3 || phase_ == Phase::Count2 || phase_ == Phase::Count1) {
            if (seBeep_) {
                Audio::GetInstance()->PlayWave(seBeep_);
            }
        }

        // Go開始時のSEと入力解放
        if (phase_ == Phase::Go) {
            if (!goPlayed_ && seGo_) {
                Audio::GetInstance()->PlayWave(seGo_);
                goPlayed_ = true;
            }

            // Go表示開始と同時に入力ロック解除
            inputLocked_ = false;
        }
    }

    // フェーズ持ち時間を超えたら次のフェーズへ
    if (phaseTimer_ >= duraction) {
        AdvancePhase();
    }
}

void CountDown::Draw() {
    // 非アクティブ or 完了状態h描画不要
    if (phase_ == Phase::Inactive || phase_ == Phase::Done) { return; }

    float duraction = CurrentPhaseDuraction();
    float t01 = (duraction > 0.0f) ? std::clamp(phaseTimer_ / duraction, 0.0f, 1.0f) : 1.0f;

    // 現在フェーズ用スプライト取得
    if (auto* spr = CurrentPhaseSprite()) {
        float alpha = CurrentPhaseAlpha(t01);
        float scale = CurrentPhaseScale(t01);

        // 色（アルファ）の設定
        spr->SetColor({ 1.0f,1.0f,1.0f,alpha });

        // フェーズごとにサイズを切り替え
        Vector2 base = (phase_ == Phase::Go) ? baseSizeGo_ : baseSizeCount_;
        spr->SetSize({ base.x * scale,base.y * scale });
        spr->SetPosition(center_);
        spr->Draw();
    }
}

float CountDown::EaseoutBack(float t, float s)const {
    // BackEaseのイージング関数本体
    t -= 1.0f;
    return t * t * ((s + 1.0f) * t + s) + 1.0f;
}

float CountDown::CurrentPhaseDuraction() const{
    // 現在フェーズに対応する表示時間を返す
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
    // 現在フェーズで表示すべきスプライトを返す
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
    if (t01 < 0.5f) {
        return t01 / 0.5f;                  // 0→1
    }
    else {
        return 1.0f - (t01 - 0.5f) / 0.5f;  // 1→0
    }
}

float CountDown::CurrentPhaseScale(float t01)const {
    // easeOutBackで1.20->1.00
    float e = EaseoutBack(std::clamp(t01, 0.0f, 1.0f), backS_);

    return scaleStart_ + (scaleEnd_ - scaleStart_) * e;
}

void CountDown::AdvancePhase() {
    // タイマーをリセットして次フェーズへ移行
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