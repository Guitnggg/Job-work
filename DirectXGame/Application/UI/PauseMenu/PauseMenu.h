#pragma once

#include <memory>
#include <vector>

#include "2d/Sprite.h"
#include "Application/Utility/MyMath/MyMath.h"
#include "input/Input.h"
#include "math/Vector2.h"

/// <summary>
/// ポーズ中に表示するメニューUIを管理するクラス
/// 背景・メニュー項目・カーソル表示、入力による選択変更
/// 開閉アニメーションと選択結果の通知をまとめて行う
/// </summary>
class PauseMenu {
public:
	/// <summary>
	/// ポーズメニューで選択された操作結果。
	/// ゲーム側はこの値を見て再開・リトライ・タイトル遷移を行う。
	/// </summary>
	enum class Result {
		None,    // 何もない
		Resume,  // ゲームへ戻る
		Retry,   // 現在のステージをやり直す
		ToTitle, // タイトル画面へ戻る
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

	/// <summary>
	/// 現在の選択結果を取得する
	/// </summary>
	Result GetResult() const { return result_; }

	bool IsHowToOpen() const { return isHowToOpen_; }

	/// <summary>
	/// 選択結果を見決定状態に戻す
	/// </summary>
	void ResetResult() { result_ = Result::None; }

	/// <summary>
	/// ポーズメニューを開くアニメーションを開始する
	/// </summary>
	void StartOpenAnimation();

	/// <summary>
	/// ポーズメニューの閉じるアニメーションを開始する
	/// </summary>
	void StartCloseAnimation();

private:
	/// <summary>
	/// 入力ン位応じてカーソル位置を変更する
	/// </summary>
	void MoveCursor();

private:
	// ===== テクスチャ =====
	enum class MenuTex { Resume, HowTo, Restart, ToTitle, HowToPlay, White, Count };

	static constexpr const char* kMenuTexPaths[static_cast<int>(MenuTex::Count)] = {
	    "./Resources/UI/HowTo.png", "./Resources/UI/HowToMenu.png", "./Resources/UI/Restart.png", "./Resources/UI/Totitle.png", "./Resources/UI/HowToPlay.png", "./Resources/White1x1.png"};

	// ===== 画面・UI定数 =====
	static constexpr float kScreenWidth = 1280.0f;
	static constexpr float kScreenHeight = 720.0f;

	static constexpr KamataEngine::Vector2 kMenuBaseSize = {300.0f, 40.0f};
	static constexpr KamataEngine::Vector2 kCursorBaseSize = {20.0f, 20.0f};
	static constexpr KamataEngine::Vector2 kHowToSize = {960.0f, 540.0f};

	static constexpr float kMenuSpacing = 72.0f;
	static constexpr float kCursorOffsetX = 40.0f;

	static constexpr float kBackgroundAlpha = 0.6f;

	static constexpr int kMenuCount = 4;

	// ===== アニメーション定数 =====
	static constexpr float kAnimDuration = 0.25f;
	static constexpr float kStartScale = 0.8f;

private:
	// ===== 状態 =====
	Result result_ = Result::None; // メニューで決定された操作
	int selectIndex_ = 0;          // 現在選択中の項目番号

	bool isOpening_ = false; // 開くアニメーション中か
	bool isClosing_ = false; // 閉じるアニメーション中か

	float animTimer_ = 0.0f; // アニメーションの経過時間
	float scale_ = 1.0f;     // メニュー全体の表示スケール

	bool isHowToOpen_ = false;

	KamataEngine::Vector2 basePos_ = {kScreenWidth * 0.5f, kScreenHeight * 0.45f};

	// ===== スプライト =====
	std::unique_ptr<KamataEngine::Sprite> bgSprite_;                 // 半透明の背景
	std::vector<std::unique_ptr<KamataEngine::Sprite>> menuSprites_; // メニュー項目画像
	std::unique_ptr<KamataEngine::Sprite> cursorSprite_;             // 選択位置を示すカーソル
	std::unique_ptr<KamataEngine::Sprite> howToSprite_;
};
