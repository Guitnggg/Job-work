#include "UIManager.h"

#include <algorithm>

using namespace KamataEngine;

namespace {
    constexpr float kUiMaxHp = 100.0f;
    constexpr Vector2 kUiBasePos{ 0.0f, 0.0f };
    constexpr Vector2 kUiBaseSize{ 1280.0f, 720.0f };
    constexpr Vector2 kNormalPos{ 55.0f, 684.0f };
    constexpr Vector2 kNormalSize{ 280.0f, 14.0f };
    constexpr Vector2 kCoolPos{ 945.0f, 684.0f };
    constexpr Vector2 kCoolSize{ 280.0f, 14.0f };
    constexpr Vector2 kLockPos{ 945.0f, 673.0f };
    constexpr Vector2 kLockBaseSize{ 280.0f, 4.0f };
    constexpr Vector2 kSlotStartPos{ 1040.0f, 648.0f };
    constexpr Vector2 kSlotSize{ 14.0f, 14.0f };
    constexpr Vector2 kScoreGoalPos{ 1090.0f, 74.0f };
    constexpr Vector2 kScoreGoalSize{ 160.0f, 8.0f };
    constexpr float kSlotGap = 18.0f;
    constexpr Vector4 kHudGreen{ 0.10f, 1.0f, 0.38f, 0.95f };
    constexpr Vector4 kHudCyan{ 0.10f, 0.82f, 1.0f, 0.95f };
    constexpr Vector4 kHudYellow{ 1.0f, 0.82f, 0.12f, 0.98f };
    constexpr Vector4 kHudDim{ 0.02f, 0.10f, 0.13f, 0.90f };
} // namespace

// HPバー、スコア、ホーミング関連ゲージを生成して初期状態を整える。
void UIManager::Initialize(Player* player) {
    player_ = player;
    // HPバー（Graph）
    graph_ = std::make_unique<Graph>();
    graph_->Initialize();

    // スコア
    score_ = std::make_unique<Score>();
    score_->Initialize();
    score_->SetPosition(1090.0f, 8.0f);

    homingBarTexHandle_ = TextureManager::Load("./Resources/white1x1.png");
    uiBaseTexHandle_ = TextureManager::Load("./Resources/InGame/UIBase.png");
    uiBaseSprite_.reset(Sprite::Create(uiBaseTexHandle_, kUiBasePos));
    normalAttackBack_.reset(Sprite::Create(homingBarTexHandle_, kNormalPos, kHudDim));
    normalAttackFront_.reset(Sprite::Create(homingBarTexHandle_, kNormalPos, kHudCyan));
    homingCooldownBack_.reset(Sprite::Create(homingBarTexHandle_, kCoolPos, { 0.1f, 0.1f, 0.1f, 0.9f }));
    homingCooldownFront_.reset(Sprite::Create(homingBarTexHandle_, kCoolPos, kHudCyan));
    lockProgressBar_.reset(Sprite::Create(homingBarTexHandle_, kLockPos, kHudYellow));
    scoreGoalBack_.reset(Sprite::Create(homingBarTexHandle_, kScoreGoalPos, kHudDim));
    scoreGoalFront_.reset(Sprite::Create(homingBarTexHandle_, kScoreGoalPos, kHudYellow));

    uiBaseSprite_->SetSize(kUiBaseSize);
    normalAttackBack_->SetSize(kNormalSize);
    normalAttackFront_->SetSize(kNormalSize);
    homingCooldownBack_->SetSize(kCoolSize);
    homingCooldownFront_->SetSize(kCoolSize);
    lockProgressBar_->SetSize({ 0.0f, kLockBaseSize.y });
    scoreGoalBack_->SetSize(kScoreGoalSize);
    scoreGoalFront_->SetSize({ 0.0f, kScoreGoalSize.y });
    for (size_t i = 0; i < homingLockSlots_.size(); ++i) {
        homingLockSlots_[i].reset(Sprite::Create(homingBarTexHandle_, { kSlotStartPos.x + kSlotGap * static_cast<float>(i), kSlotStartPos.y }, kHudDim));
        homingLockSlots_[i]->SetSize(kSlotSize);
    }
}

// BulletManager から受け取ったロックオン状態を UI 表示用に保持する。
void UIManager::SetHomingLockInfo(int32_t currentLockCount, int32_t maxLockCount, bool isLocking, float lockProgressRate) {
    homingLockCount_ = (std::max)(currentLockCount, 0);
    homingMaxLockCount_ = (std::max)(maxLockCount, 1);
    isHomingLocking_ = isLocking;
    homingLockProgressRate_ = std::clamp(lockProgressRate, 0.0f, 1.0f);
}

void UIManager::SetScoreGoal(int32_t currentScore, int32_t requiredScore) {
    if (requiredScore <= 0) {
        scoreGoalRate_ = 1.0f;
        return;
    }
    scoreGoalRate_ = std::clamp(static_cast<float>(currentScore) / static_cast<float>(requiredScore), 0.0f, 1.0f);
}

// プレイヤーHP、スコア、ホーミングゲージの表示状態を更新する。
void UIManager::Update() {
    // HPバー更新
    if (player_ && graph_) {
        const int32_t currentHp = player_->GetHP();

        // HP(0〜100想定) を 0.0〜1.0 に変換してGraphに渡す
        float hpRate = static_cast<float>(currentHp) / kUiMaxHp;

        graph_->SetValue(hpRate);
        graph_->Update();
    }

    // スコア更新
    if (score_) {
        score_->Update();
    }

    if (normalAttackFront_) {
        normalAttackFront_->SetSize({ kNormalSize.x * normalAttackCooldownRate_, kNormalSize.y });
        normalAttackFront_->SetColor(normalAttackCooldownRate_ >= 0.999f ? kHudGreen : kHudCyan);
    }

    if (homingCooldownFront_) {
        // クールダウン率に合わせて前面バーの横幅だけを変える。
        homingCooldownFront_->SetSize({ kCoolSize.x * homingCooldownRate_, kCoolSize.y });
    }

    if (lockProgressBar_) {
        lockProgressBar_->SetSize({ kLockBaseSize.x * homingLockProgressRate_, kLockBaseSize.y });
    }

    if (scoreGoalFront_) {
        scoreGoalFront_->SetSize({ kScoreGoalSize.x * scoreGoalRate_, kScoreGoalSize.y });
        scoreGoalFront_->SetColor(scoreGoalRate_ >= 1.0f ? kHudGreen : kHudYellow);
    }

    for (size_t i = 0; i < homingLockSlots_.size(); ++i) {
        if (!homingLockSlots_[i]) {
            continue;
        }
        const bool locked = static_cast<int32_t>(i) < homingLockCount_;
        homingLockSlots_[i]->SetColor(locked ? kHudYellow : Vector4{ 0.08f, 0.25f, 0.25f, 0.85f });
    }

    if (homingCooldownFront_) {
        const Vector4 cooldownColor = homingCooldownRate_ >= 0.999f ? kHudGreen : kHudCyan;
        homingCooldownFront_->SetColor(cooldownColor);
    }
}

// 生成済みの UI 要素を順番に描画する。
// ロックオン進行バーはロック操作中だけ表示する。
void UIManager::Draw() {
    if (uiBaseSprite_) {
        uiBaseSprite_->Draw();
    }
    if (graph_) {
        graph_->Draw();
    }
    if (score_) {
        score_->Draw();
    }
    if (scoreGoalBack_) {
        scoreGoalBack_->Draw();
    }
    if (scoreGoalFront_) {
        scoreGoalFront_->Draw();
    }
    if (normalAttackBack_) {
        normalAttackBack_->Draw();
    }
    if (normalAttackFront_) {
        normalAttackFront_->Draw();
    }
    for (auto& slot : homingLockSlots_) {
        if (slot) {
            slot->Draw();
        }
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
