#include "TitleScene.h"

#include "Application/Utility/GameTime.h"
#include "Scenes/Introduction/IntroductionScene.h"
#include "Scenes/SceneHelper.h"

using namespace KamataEngine;

// ===== TitleScene.cpp 内部定数（演出・時間）=====
namespace {
// フレーム
constexpr float kTwoPi = 6.28318530717958647692f;

// タイトル落下演出
constexpr float kTitleFallSpeed = 3.0f;
constexpr float kBlinkBaseAlpha = 0.5f;
constexpr float kBlinkAmpAlpha = 0.5f;

// 小惑星ランダム範囲
} // namespace

TitleScene::TitleScene() {}

TitleScene::~TitleScene() {}

void TitleScene::Initialize() {
	// 各初期化処理
	dxCommon_ = DirectXCommon::GetInstance();

	// カメラ
	camera_.Initialize();

	// 入力を受け付けるようにする
	input_ = Input::GetInstance();

	// 各種テクスチャ
	titleSprite_ = SceneHelper::CreateSprite("./Resources/title/GameTitle.png", titlePosition_, &titleTextureHandle_);
	startSprite_ = SceneHelper::CreateSprite("./Resources/title/Start.png", {150.0f, 550.0f}, &startTextureHandle_);
	startSprite_->SetColor({1.0f, 1.0f, 1.0f, 0.0f}); // 最初は透明

	// 各種サウンド
	changeSEHandle_ = Audio::GetInstance()->LoadWave("./Resources/SE/SceneChange.wav");

	// 小惑星生成
	asteroidField_.Initialize(SceneHelper::CreateMenuAsteroidFieldConfig());

	// 天球
	skydome_ = std::make_unique<Skydome>();
	skydome_->Initialize(&camera_);
}

void TitleScene::Update() {
	// 天球更新
	skydome_->Update();

	// 小惑星出現タイマー更新
	asteroidField_.Update();

	// Title落下用
	if (titlePosition_.y < titleTargetPosition_.y) {
		titlePosition_.y += titleFallSpeed_;
		if (titlePosition_.y > titleTargetPosition_.y) {
			titlePosition_.y = titleTargetPosition_.y; // 超えたら固定
			isTitleFallFinished_ = true;

			// 点滅の初期化
			blinkTimer_ = 0.0f;
		}
		titleSprite_->SetPosition(titlePosition_);
	}

	// Start点滅
	if (isTitleFallFinished_) {
		blinkTimer_ += GameTime::kDeltaTime;
		if (blinkTimer_ >= blinkInterval_) {
			blinkTimer_ -= blinkInterval_;
		}
		const float alpha = kBlinkBaseAlpha + kBlinkAmpAlpha * sinf(blinkTimer_ / blinkInterval_ * kTwoPi);
		startSprite_->SetColor({1.0f, 1.0f, 1.0f, alpha});
	} else {
		// 完了前は常に透明のまま
		startSprite_->SetColor({1.0f, 1.0f, 1.0f, 0.0f});
	}

	// シーン遷移（落下完了後のみ）
	if (isTitleFallFinished_ && input_->PushKey(DIK_SPACE)) {
		Audio::GetInstance()->PlayWave(changeSEHandle_);
		isEnd_ = true;
	}
}

void TitleScene::Draw() {
	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	SceneHelper::Begin3DDraw(dxCommon_, commandList);

#pragma region 3Dオブジェクト描画
	SceneHelper::DrawModelLayer([this]() {
		asteroidField_.Draw(camera_);
		skydome_->Draw();
	});
#pragma endregion

#pragma region 前景スプライト描画
	SceneHelper::DrawSpriteLayer(commandList, [this]() {
		titleSprite_->Draw();
		startSprite_->Draw();
	});
#pragma endregion
}

std::unique_ptr<IScene> TitleScene::NextScene() const { return std::make_unique<IntroductionScene>(); }
