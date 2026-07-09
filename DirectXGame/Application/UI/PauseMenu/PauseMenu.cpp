#include "PauseMenu.h"

#include "Application/Utility/GameTime.h"
#include "base/TextureManager.h"

using namespace KamataEngine;

// =========================
// イージング関数
// =========================　
float EaseOutBack(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return 1.0f + c3 * powf(t - 1.0f, 3) + c1 * powf(t - 1.0f, 2);
}

float EaseInBack(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return c3 * t * t * t - c1 * t * t;
}

// =========================
// 初期化
// =========================
void PauseMenu::Initialize() {

    // --- テクスチャロード ---
    uint32_t texHandles[static_cast<int>(MenuTex::Count)]{};
    for (int i = 0; i < static_cast<int>(MenuTex::Count); ++i) {
        texHandles[i] = TextureManager::Load(kMenuTexPaths[i]);
    }

    const uint32_t howToPlayTex = texHandles[static_cast<int>(MenuTex::HowToPlay)];
    const uint32_t whiteTex = texHandles[static_cast<int>(MenuTex::White)];

    // --- 背景 ---
    bgSprite_.reset(Sprite::Create(whiteTex, { 0.0f, 0.0f }, { 0, 0, 0, kBackgroundAlpha }));
    bgSprite_->SetSize({ kScreenWidth, kScreenHeight });

    // --- メニュー ---
    menuSprites_.resize(kMenuCount);

    for (int i = 0; i < kMenuCount; ++i) {
        Vector2 pos = basePos_;
        pos.y += kMenuSpacing * i;

        const uint32_t menuTex = texHandles[static_cast<int>(kMenuTexOrder[i])];
        menuSprites_[i].reset(Sprite::Create(menuTex, pos, { 1, 1, 1, 1 }));
        menuSprites_[i]->SetSize(kMenuBaseSize);
    }

    // --- カーソル ---
    cursorSprite_.reset(Sprite::Create(whiteTex, basePos_, { 1, 1, 0, 1 }));
    cursorSprite_->SetSize(kCursorBaseSize);

    howToSprite_.reset(Sprite::Create(howToPlayTex, { kScreenWidth * 0.5f, kScreenHeight * 0.5f }, { 1, 1, 1, 1 }));
    howToSprite_->SetAnchorPoint({ 0.5f, 0.5f });
    howToSprite_->SetSize(kHowToSize);
}

// =========================
// アニメーション開始
// =========================
void PauseMenu::StartOpenAnimation() {
    animTimer_ = 0.0f;
    scale_ = kStartScale;
    isOpening_ = true;
    isClosing_ = false;
    isHowToOpen_ = false;
}

void PauseMenu::StartCloseAnimation() {
    animTimer_ = 0.0f;
    isClosing_ = true;
    isOpening_ = false;
    isHowToOpen_ = false;
}

// =========================
// 更新
// =========================
void PauseMenu::Update() {

    // --- 開く ---
    if (isOpening_) {
        animTimer_ += GameTime::kDeltaTime;
        float t = (std::min)(animTimer_ / kAnimDuration, 1.0f);
        scale_ = kStartScale + (1.0f - kStartScale) * EaseOutBack(t);

        if (t >= 1.0f) {
            scale_ = 1.0f;
            isOpening_ = false;
        }
    }

    // --- 閉じる ---
    if (isClosing_) {
        animTimer_ += GameTime::kDeltaTime;
        float t = (std::min)(animTimer_ / kAnimDuration, 1.0f);
        scale_ = 1.0f - (1.0f - kStartScale) * EaseInBack(t);

        if (t >= 1.0f) {
            scale_ = kStartScale;
            isClosing_ = false;
        }
        return;
    }

    if (result_ != Result::None) {
        return;
    }

    if (isHowToOpen_) {
        if (Input::GetInstance()->TriggerKey(DIK_SPACE) || Input::GetInstance()->TriggerKey(DIK_ESCAPE)) {
            isHowToOpen_ = false;
        }
        return;
    }

    MoveCursor();

    if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
        const MenuAction& action = kMenuActions[selectIndex_];
        if (action.opensHowTo) {
            isHowToOpen_ = true;
        }
        result_ = action.result;
    }
}

// =========================
// カーソル移動
// =========================
void PauseMenu::MoveCursor() {

    if (Input::GetInstance()->TriggerKey(DIK_W)) {
        selectIndex_ = (selectIndex_ + kMenuCount - 1) % kMenuCount;
    }

    if (Input::GetInstance()->TriggerKey(DIK_S)) {
        selectIndex_ = (selectIndex_ + 1) % kMenuCount;
    }

    Vector2 pos = basePos_;
    pos.y += kMenuSpacing * selectIndex_;
    pos.x -= kCursorOffsetX;

    cursorSprite_->SetPosition(pos);
}

// =========================
// 描画
// =========================
void PauseMenu::Draw() {

    bgSprite_->Draw();

    for (auto& s : menuSprites_) {
        s->SetSize({ kMenuBaseSize.x * scale_, kMenuBaseSize.y * scale_ });
        s->Draw();
    }

    cursorSprite_->SetSize({ kCursorBaseSize.x * scale_, kCursorBaseSize.y * scale_ });
    cursorSprite_->Draw();

    if (isHowToOpen_ && howToSprite_) {
        howToSprite_->Draw();
    }
}
