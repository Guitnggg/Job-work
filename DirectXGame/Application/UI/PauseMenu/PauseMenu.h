#pragma once

#include <memory>
#include <vector>

#include "2d/Sprite.h"
#include "input/Input.h"
#include "math/Vector2.h"

class PauseMenu {
public:
	// ポーズメニューの選択結果
	enum class Result {
		None,
		Resume,
		Retry,
		ToTitle,
	};

public:
	void Initialize();
	void Update();
	void Draw();

	// 選択結果取得
	Result GetResult() const { return result_; }

	// ポーズ開始時に呼ぶ
	void ResetResult() { result_ = Result::None; }

private:
	void MoveCursor();

private:
	// 選択中インデックス
	int selectIndex_ = 0;
	static constexpr int kMenuCount_ = 3;

	Result result_ = Result::None;

	// UI
	std::unique_ptr<KamataEngine::Sprite> bgSprite_;
	std::unique_ptr<KamataEngine::Sprite> cursorSprite_;
	std::vector<std::unique_ptr<KamataEngine::Sprite>> menuSprites_;

	// レイアウト
	KamataEngine::Vector2 basePos_{640.0f, 360.0f};
	float menuSpacing_ = 60.0f;
};
