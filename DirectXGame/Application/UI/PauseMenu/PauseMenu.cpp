#include "PauseMenu.h"

#include "base/TextureManager.h"

using namespace KamataEngine;

void PauseMenu::Initialize() {

	// 仮テクスチャ（白1pxなど）
	uint32_t whiteTex = TextureManager::Load("./Resources/White1x1.png");

	// ===== 背景 =====
	bgSprite_.reset(Sprite::Create(whiteTex, {0.0f, 0.0f}, {0, 0, 0, 0.6f}));
	bgSprite_->SetSize({1280.0f, 720.0f});

	// ===== メニュー =====
	menuSprites_.resize(kMenuCount_);
	for (int i = 0; i < kMenuCount_; ++i) {

		Vector2 pos = basePos_;
		pos.y += menuSpacing_ * i;

		menuSprites_[i].reset(Sprite::Create(whiteTex, pos, {1, 1, 1, 1}));
		menuSprites_[i]->SetSize({300.0f, 40.0f});
	}

	// ===== カーソル =====
	cursorSprite_.reset(Sprite::Create(whiteTex, basePos_, {1, 1, 0, 1}));
	cursorSprite_->SetSize({20.0f, 20.0f});
}
void PauseMenu::Update() {

	if (result_ != Result::None) {
		return;
	}

	MoveCursor();

	if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
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

void PauseMenu::MoveCursor() {

	if (Input::GetInstance()->TriggerKey(DIK_UP)) {
		selectIndex_ = (selectIndex_ + kMenuCount_ - 1) % kMenuCount_;
	}

	if (Input::GetInstance()->TriggerKey(DIK_DOWN)) {
		selectIndex_ = (selectIndex_ + 1) % kMenuCount_;
	}

	Vector2 pos = basePos_;
	pos.y += menuSpacing_ * selectIndex_;
	pos.x -= 40.0f;

	cursorSprite_->SetPosition(pos);
}

void PauseMenu::Draw() {

	bgSprite_->Draw();

	for (auto& s : menuSprites_) {
		s->Draw();
	}

	cursorSprite_->Draw();
}