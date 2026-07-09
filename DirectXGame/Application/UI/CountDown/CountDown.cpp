#include "CountDown.h"

void CountDown::InitializeFromPaths(
    const char* tex3, const char* tex2, const char* tex1, const char* texGo, const KamataEngine::Vector2& centerPos, const KamataEngine::Vector2& baseSizeCount,
    const KamataEngine::Vector2& baseGoSize) {
	// 画像パスからテクスチャを読み込む
	InitializeFromHandles(
	    KamataEngine::TextureManager::Load(tex3), KamataEngine::TextureManager::Load(tex2), KamataEngine::TextureManager::Load(tex1), KamataEngine::TextureManager::Load(texGo), centerPos,
	    baseSizeCount, baseGoSize);
}

void CountDown::InitializeFromHandles(
    uint32_t tex3, uint32_t tex2, uint32_t tex1, uint32_t texGo, const KamataEngine::Vector2& centerPos, const KamataEngine::Vector2& baseSizeCount, const KamataEngine::Vector2& baseSizeGo) {
	// 表示パラメータ設定
	center_ = centerPos;
	baseSizeCount_ = baseSizeCount;
	baseSizeGo_ = baseSizeGo;

	// 既存スプライトを破棄してから再生成
	count3Sprite_.reset(KamataEngine::Sprite::Create(tex3, center_, {1, 1, 1, 0}, {0.5f, 0.5f}, false, false));
	count2Sprite_.reset(KamataEngine::Sprite::Create(tex2, center_, {1, 1, 1, 0}, {0.5f, 0.5f}, false, false));
	count1Sprite_.reset(KamataEngine::Sprite::Create(tex1, center_, {1, 1, 1, 0}, {0.5f, 0.5f}, false, false));
	goSprite_.reset(KamataEngine::Sprite::Create(texGo, center_, {1, 1, 1, 0}, {0.5f, 0.5f}, false, false));

	// 状態を初期化
	phase_ = Phase::Inactive;
	phaseTimer_ = 0.0f;
	inputLocked_ = true;
	goPlayed_ = false;
}

void CountDown::SetTimings(float readyDelay, float countUnit, float goDuration) {
	// 各フェーズの長さを設定
	readyDelay_ = readyDelay;
	countUnit_ = countUnit;
	goDuration_ = goDuration;
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
	if (phase_ == Phase::Inactive || phase_ == Phase::Done) {
		return;
	}

	// 経過時間を加算
	phaseTimer_ += dt;
	float duration = CurrentPhaseDuration();

	// フェース開始のタイミングのSE
	if (phaseTimer_ == dt) {
		const int phaseIndex = static_cast<int>(phase_);
		static constexpr bool kPlaysBeep[kPhaseCount] = { false, false, true, true, true, false, false };

		if (kPlaysBeep[phaseIndex] && seBeep_) {
			KamataEngine::Audio::GetInstance()->PlayWave(seBeep_);

			inputLocked_ = false;
		}
	}

	// フェーズ持ち時間を超えたら次のフェーズへ
	if (phaseTimer_ >= duration) {
		AdvancePhase();
	}
}

void CountDown::Draw() {
	// 非アクティブ or 完了状態h描画不要
	if (phase_ == Phase::Inactive || phase_ == Phase::Done) {
		return;
	}

	float duration = CurrentPhaseDuration();
	float t01 = (duration > 0.0f) ? std::clamp(phaseTimer_ / duration, 0.0f, 1.0f) : 1.0f;

	// 現在フェーズ用スプライト取得
	if (auto* spr = CurrentPhaseSprite()) {
		float alpha = CurrentPhaseAlpha(t01);
		float scale = CurrentPhaseScale(t01);

		// 色（アルファ）の設定
		spr->SetColor({1.0f, 1.0f, 1.0f, alpha});

		// フェーズごとにサイズを切り替え
		KamataEngine::Vector2 base = (phase_ == Phase::Go) ? baseSizeGo_ : baseSizeCount_;
		spr->SetSize({base.x * scale, base.y * scale});
		spr->SetPosition(center_);
		spr->Draw();
	}
}

float CountDown::EaseOutBack(float t, float s) const {
	// BackEaseのイージング関数本体
	t -= 1.0f;
	return t * t * ((s + 1.0f) * t + s) + 1.0f;
}

float CountDown::CurrentPhaseDuration() const {
	// フェーズごとの表示時間をテーブル化し、同じ処理で値だけを切り替える
	const float durations[kPhaseCount] = { 0.0f, readyDelay_, countUnit_, countUnit_, countUnit_, goDuration_, 0.0f };
	return durations[static_cast<int>(phase_)];
}

KamataEngine::Sprite* CountDown::CurrentPhaseSprite() const {
	// フェーズごとの表示スプライトをテーブル化し、switch を不要にする
	KamataEngine::Sprite* sprites[kPhaseCount] = { nullptr, nullptr, count3Sprite_.get(), count2Sprite_.get(), count1Sprite_.get(), goSprite_.get(), nullptr };
	return sprites[static_cast<int>(phase_)];
}

float CountDown::CurrentPhaseAlpha(float t01) const {
	// 0->1->0の三角波
	if (t01 < 0.5f) {
		return t01 / 0.5f; // 0→1
	} else {
		return 1.0f - (t01 - 0.5f) / 0.5f; // 1→0
	}
}

float CountDown::CurrentPhaseScale(float t01) const {
	// easeOutBackで1.20->1.00
	float e = EaseOutBack(std::clamp(t01, 0.0f, 1.0f), backS_);

	return scaleStart_ + (scaleEnd_ - scaleStart_) * e;
}

void CountDown::AdvancePhase() {
	// タイマーをリセットして次フェーズへ移行
	phaseTimer_ = 0.0f;

	static constexpr Phase kNextPhases[kPhaseCount] = { Phase::Inactive, Phase::Count3, Phase::Count2, Phase::Count1, Phase::Go, Phase::Done, Phase::Done };
	const Phase previousPhase = phase_;
	phase_ = kNextPhases[static_cast<int>(phase_)];

	if (previousPhase == Phase::Count1) {
		phase_ = Phase::Done;
	}
}
