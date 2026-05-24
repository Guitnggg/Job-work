#include "PauseMenu.h"

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

	const uint32_t resumeTex = texHandles[static_cast<int>(MenuTex::HowTo)];
	const uint32_t restartTex = texHandles[static_cast<int>(MenuTex::Restart)];
	const uint32_t titleTex = texHandles[static_cast<int>(MenuTex::ToTitle)];
	const uint32_t whiteTex = texHandles[static_cast<int>(MenuTex::White)];

	// --- 背景 ---
	bgSprite_.reset(Sprite::Create(whiteTex, {0.0f, 0.0f}, {0, 0, 0, kBackgroundAlpha}));
	bgSprite_->SetSize({kScreenWidth, kScreenHeight});

	// --- メニュー ---
	menuSprites_.resize(kMenuCount);

	const uint32_t menuTex[kMenuCount] = {resumeTex, restartTex, titleTex};

	for (int i = 0; i < kMenuCount; ++i) {
		Vector2 pos = basePos_;
		pos.y += kMenuSpacing * i;

		menuSprites_[i].reset(Sprite::Create(menuTex[i], pos, {1, 1, 1, 1}));
		menuSprites_[i]->SetSize(kMenuBaseSize);
	}

	// --- カーソル ---
	cursorSprite_.reset(Sprite::Create(whiteTex, basePos_, {1, 1, 0, 1}));
	cursorSprite_->SetSize(kCursorBaseSize);
}

// =========================
// アニメーション開始
// =========================
void PauseMenu::StartOpenAnimation() {
	animTimer_ = 0.0f;
	scale_ = kStartScale;
	isOpening_ = true;
	isClosing_ = false;
}

void PauseMenu::StartCloseAnimation() {
	animTimer_ = 0.0f;
	isClosing_ = true;
	isOpening_ = false;
}

// =========================
// 更新
// =========================
void PauseMenu::Update() {

	// --- 開く ---
	if (isOpening_) {
		animTimer_ += kDeltaTime;
		float t = (std::min)(animTimer_ / kAnimDuration, 1.0f);
		scale_ = kStartScale + (1.0f - kStartScale) * EaseOutBack(t);

		if (t >= 1.0f) {
			scale_ = 1.0f;
			isOpening_ = false;
		}
	}

	// --- 閉じる ---
	if (isClosing_) {
		animTimer_ += kDeltaTime;
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

	MoveCursor();

	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		switch (selectIndex_) {
		case 0:
			result_ = Result::Resume;
			break;
		case 1:
			result_ = Result::Retry;
			break;
		case 2:
			result_ = Result::ToTitle;
			break;
		}
	}
}

// =========================
// カーソル移動
// =========================
void PauseMenu::MoveCursor() {

	if (Input::GetInstance()->TriggerKey(DIK_UP)) {
		selectIndex_ = (selectIndex_ + kMenuCount - 1) % kMenuCount;
	}

	if (Input::GetInstance()->TriggerKey(DIK_DOWN)) {
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
		s->SetSize({kMenuBaseSize.x * scale_, kMenuBaseSize.y * scale_});
		s->Draw();
	}

	cursorSprite_->SetSize({kCursorBaseSize.x * scale_, kCursorBaseSize.y * scale_});
	cursorSprite_->Draw();
}
