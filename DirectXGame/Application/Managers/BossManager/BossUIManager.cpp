#include "BossUIManager.h"

#include <algorithm>

#include "Application/Characters/Boss/Boss.h"

using namespace KamataEngine;

namespace {
constexpr Vector2 kHpPos{330.0f, 22.0f};
constexpr Vector2 kHpSize{620.0f, 18.0f};
constexpr Vector2 kWarningPos{270.0f, 300.0f};
constexpr Vector2 kWarningSize{740.0f, 88.0f};
} // namespace

void BossUIManager::Initialize() {
	textureHandle_ = TextureManager::Load("./Resources/white1x1.png");
	hpFrameBar_.reset(Sprite::Create(textureHandle_, {kHpPos.x - 4.0f, kHpPos.y - 4.0f}, {1.0f, 1.0f, 1.0f, 0.9f}));
	hpBackBar_.reset(Sprite::Create(textureHandle_, kHpPos, {0.08f, 0.01f, 0.01f, 0.92f}));
	hpFrontBar_.reset(Sprite::Create(textureHandle_, kHpPos, {1.0f, 0.16f, 0.05f, 0.95f}));
	warningBar_.reset(Sprite::Create(textureHandle_, kWarningPos, {1.0f, 0.04f, 0.02f, 0.0f}));

	if (hpFrameBar_) {
		hpFrameBar_->SetSize({kHpSize.x + 8.0f, kHpSize.y + 8.0f});
	}
	if (hpBackBar_) {
		hpBackBar_->SetSize(kHpSize);
	}
	if (hpFrontBar_) {
		hpFrontBar_->SetSize(kHpSize);
	}
	if (warningBar_) {
		warningBar_->SetSize(kWarningSize);
	}
}

void BossUIManager::Update(const Boss* boss, float warningRate, bool showHpBar, bool showWarning) {
	showHpBar_ = showHpBar;
	showWarning_ = showWarning;

	if (hpFrontBar_) {
		const float rate = boss ? boss->GetHpRate() : 0.0f;
		hpFrontBar_->SetSize({kHpSize.x * std::clamp(rate, 0.0f, 1.0f), kHpSize.y});
	}

	if (warningBar_) {
		const float alpha = std::clamp(0.35f + warningRate * 0.55f, 0.0f, 0.9f);
		warningBar_->SetColor({1.0f, 0.02f, 0.02f, alpha});
	}
}

void BossUIManager::Draw() {
	if (showWarning_ && warningBar_) {
		warningBar_->Draw();
	}

	if (!showHpBar_) {
		return;
	}
	if (hpFrameBar_) {
		hpFrameBar_->Draw();
	}
	if (hpBackBar_) {
		hpBackBar_->Draw();
	}
	if (hpFrontBar_) {
		hpFrontBar_->Draw();
	}
}
