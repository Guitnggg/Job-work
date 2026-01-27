#pragma once

#include <memory>
#include <vector>

#include "2d/Sprite.h"
#include "input/Input.h"
#include "math/Vector2.h"
#include "Application/Utility/MyMath/MyMath.h"

class PauseMenu {
public:
	/// <summary>
	/// 
	/// </summary>
	enum class Result {
		None,
		Resume,
		Retry,
		ToTitle,
	};

public:
	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	// 状態
	Result GetResult() const { return result_; }
	void ResetResult() { result_ = Result::None; }

	// アニメ開始
	void StartOpenAnimation();
	void StartCloseAnimation();

private:
	void MoveCursor();

private:
	// ===== テクスチャ =====
	enum class MenuTex {
		HowTo,
		Restart,
		ToTitle,
		White,
		Count
	};

	static constexpr const char* kMenuTexPaths[static_cast<int>(MenuTex::Count)] = {
		"./Resources/UI/HowTo.png",
		"./Resources/UI/Restart.png",
		"./Resources/UI/ToTitle.png",
		"./Resources/White1x1.png"
	};

	// ===== 画面・UI定数 =====
	static constexpr float kScreenWidth = 1280.0f;
	static constexpr float kScreenHeight = 720.0f;

	static constexpr KamataEngine::Vector2 kMenuBaseSize = { 360.0f, 64.0f };
	static constexpr KamataEngine::Vector2 kCursorBaseSize = { 20.0f, 20.0f };

	static constexpr float kMenuSpacing = 72.0f;
	static constexpr float kCursorOffsetX = 40.0f;

	static constexpr float kBackgroundAlpha = 0.6f;

	static constexpr int kMenuCount = 3;

	// ===== アニメーション定数 =====
	static constexpr float kAnimDuration = 0.25f;
	static constexpr float kStartScale = 0.8f;
	static constexpr float kDeltaTime = 1.0f / 60.0f;

private:
	// ===== 状態 =====
	Result result_ = Result::None;
	int selectIndex_ = 0;

	bool isOpening_ = false;
	bool isClosing_ = false;

	float animTimer_ = 0.0f;
	float scale_ = 1.0f;

	KamataEngine::Vector2 basePos_ = {
		kScreenWidth * 0.5f,
		kScreenHeight * 0.45f
	};

	// ===== スプライト =====
	std::unique_ptr<KamataEngine::Sprite> bgSprite_;
	std::vector<std::unique_ptr<KamataEngine::Sprite>> menuSprites_;
	std::unique_ptr<KamataEngine::Sprite> cursorSprite_;
};