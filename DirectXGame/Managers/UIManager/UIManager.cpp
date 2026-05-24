#include "UIManager.h"

#include <algorithm>

using namespace KamataEngine;

namespace {
constexpr float kUiMaxHp = 100.0f;
constexpr Vector2 kCoolPos{24.0f, 84.0f};
constexpr Vector2 kCoolSize{200.0f, 10.0f};
constexpr Vector2 kLockPos{24.0f, 98.0f};
constexpr Vector2 kLockBaseSize{200.0f, 6.0f};
} // namespace

void UIManager::Initialize(Player* player) {
	player_ = player;

	// HPバー（Graph）
	graph_ = std::make_unique<Graph>();
	graph_->Initialize();

	// スコア
	score_ = std::make_unique<Score>();
	score_->Initialize();

	homingBarTexHandle_ = TextureManager::Load("./Resources/white1x1.png");
	homingCooldownBack_.reset(Sprite::Create(homingBarTexHandle_, kCoolPos, {0.1f, 0.1f, 0.1f, 0.9f}));
	homingCooldownFront_.reset(Sprite::Create(homingBarTexHandle_, kCoolPos, {0.2f, 0.8f, 1.0f, 0.9f}));
	lockProgressBar_.reset(Sprite::Create(homingBarTexHandle_, kLockPos, {1.0f, 0.85f, 0.2f, 0.9f}));

	homingCooldownBack_->SetSize(kCoolSize);
	homingCooldownFront_->SetSize(kCoolSize);
	lockProgressBar_->SetSize({0.0f, kLockBaseSize.y});
}

void UIManager::SetHomingLockInfo(int32_t currentLockCount, int32_t maxLockCount, bool isLocking) {
	homingLockCount_ = (std::max)(currentLockCount, 0);
	homingMaxLockCount_ = (std::max)(maxLockCount, 1);
	isHomingLocking_ = isLocking;
}

void UIManager::Update() {
	// HPバー更新
	if (player_ && graph_) {
		// HP(0〜100想定) を 0.0〜1.0 に変換してGraphに渡す
		float hpRate = static_cast<float>(player_->GetHP()) / kUiMaxHp;

		graph_->SetValue(hpRate);
		graph_->Update();
	}

	// スコア更新
	if (score_) {
		score_->Update();
	}

	if (homingCooldownFront_) {
		homingCooldownFront_->SetSize({kCoolSize.x * homingCooldownRate_, kCoolSize.y});
	}

	if (lockProgressBar_) {
		const float lockRate = static_cast<float>(homingLockCount_) / static_cast<float>(homingMaxLockCount_);
		lockProgressBar_->SetSize({kLockBaseSize.x * std::clamp(lockRate, 0.0f, 1.0f), kLockBaseSize.y});
	}
}

void UIManager::Draw() {
	if (graph_) {
		graph_->Draw();
	}
	if (score_) {
		score_->Draw();
	}
	if (homingCooldownBack_) {
		homingCooldownBack_->Draw();
	}
	if (homingCooldownFront_) {
		homingCooldownFront_->Draw();
	}
	if (isHomingLocking_ && lockProgressBar_) {
		lockProgressBar_->Draw();
	}
}
