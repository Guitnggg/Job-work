#include "TitleScene.h"

#include "Application/Utility/GameTime.h"
#include "Scenes/Introduction/IntroductionScene.h"

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
constexpr float kAsteroidRangeX = 25.0f;
constexpr float kAsteroidRangeY = 15.0f;
constexpr float kAsteroidSpeedMin = -0.3f;
constexpr float kAsteroidSpeedMax = -0.1f;
constexpr float kAsteroidRotMin = 0.01f;
constexpr float kAsteroidRotMax = 0.03f;

constexpr int kAsteroidCount = 10;
constexpr float kAsteroidSpawnZMin = 0.0f;
constexpr float kAsteroidSpawnZMax = 140.0f;
constexpr float kAsteroidRecycleZ = -50.0f;
constexpr float kAsteroidSpawnInterval = 1.0f;
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
	titleTextureHandle_ = TextureManager::Load("./Resources/title/GameTitle.png");
	titleSprite_.reset(Sprite::Create(titleTextureHandle_, titlePosition_));

	startTextureHandle_ = TextureManager::Load("./Resources/title/Start.png");
	startSprite_.reset(Sprite::Create(startTextureHandle_, {150.0f, 550.0f}));
	startSprite_->SetColor({1.0f, 1.0f, 1.0f, 0.0f}); // 最初は透明

	// 各種サウンド
	changeSEHandle_ = Audio::GetInstance()->LoadWave("./Resources/SE/SceneChange.wav");

	// 小惑星生成
	AsteroidFieldConfig asteroidConfig{};
	asteroidConfig.count = kAsteroidCount;
	asteroidConfig.spawnZMin = kAsteroidSpawnZMin;
	asteroidConfig.spawnZMax = kAsteroidSpawnZMax;
	asteroidConfig.recycleZ = kAsteroidRecycleZ;
	asteroidConfig.spawnInterval = kAsteroidSpawnInterval;
	asteroidConfig.rangeX = kAsteroidRangeX;
	asteroidConfig.rangeY = kAsteroidRangeY;
	asteroidConfig.speedMin = kAsteroidSpeedMin;
	asteroidConfig.speedMax = kAsteroidSpeedMax;
	asteroidConfig.rotationMin = kAsteroidRotMin;
	asteroidConfig.rotationMax = kAsteroidRotMax;
	asteroidField_.Initialize(asteroidConfig);

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

#pragma region 背景スプライト描画
	// 背景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに背景スプライトの描画処理を追加できる
	/// </summary>

	// スプライト描画後処理
	Sprite::PostDraw();

	// 深度バッファクリア
	dxCommon_->ClearDepthBuffer();
#pragma endregion

#pragma region 3Dオブジェクト描画
	// 3Dオブジェクト描画前処理
	Model::PreDraw();

	/// <summary>
	/// ここに3Dオブジェクトの描画処理を追加できる
	/// </summary>

	// 小惑星描画
	asteroidField_.Draw(camera_);

	// 天球描画
	skydome_->Draw();

	// 3Dオブジェクト描画後処理
	Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト描画
	// 前景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに前景スプライトの描画処理を追加できる
	/// </summary>

	titleSprite_->Draw();
	startSprite_->Draw();

	// スプライト描画後処理
	Sprite::PostDraw();
#pragma endregion
}

std::unique_ptr<IScene> TitleScene::NextScene() const { return std::make_unique<IntroductionScene>(); }
