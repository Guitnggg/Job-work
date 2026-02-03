#include "IntroductionScene.h"

#include "GameScene.h"
#include "TitleScene.h"

using namespace KamataEngine;

namespace {
	constexpr int kAsteroidCount = 10;
	constexpr float kAsteroidSpawnZMin = 0.0f;
	constexpr float kAsteroidSpawnZMax = 140.0f;
	constexpr float kAsteroidRecycleZ = -50.0f;
	constexpr float kAsteroidSpawnInterval = 1.0f;
	constexpr float kAsteroidRangeX = 25.0f;
	constexpr float kAsteroidRangeY = 15.0f;
	constexpr float kAsteroidSpeedMin = -0.3f;
	constexpr float kAsteroidSpeedMax = -0.1f;
	constexpr float kAsteroidRotMin = 0.01f;
	constexpr float kAsteroidRotMax = 0.03f;
}

IntroductionScene::IntroductionScene() {}

IntroductionScene::~IntroductionScene() {}

void IntroductionScene::Initialize() {
	dxCommon_ = DirectXCommon::GetInstance();

	// シーン変遷の初期化
	nextScene_ = SceneName::None;

	// 各種テクスチャ
	returnTitleTextureHandle_ = TextureManager::Load("./Resources/introduction/Esc-export.png");
	returnTitleSprite_.reset(Sprite::Create(returnTitleTextureHandle_, {20.0f, 20.0f}));

	introTextureHandle_ = TextureManager::Load("./Resources/Introduction/setumei.png");
	introSprite_.reset(Sprite::Create(introTextureHandle_, {0.0f, 0.0f}));

	// 各種サウンド
	changeSEHandle_ = Audio::GetInstance()->LoadWave("./Resources/SE/SceneChange.wav");

	// カメラ
	camera_.Initialize();

	// 天球
	skydome_ = std::make_unique<Skydome>();
	skydome_->Initialize(&camera_);

	// 小惑星
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
}

void IntroductionScene::Update() {
	// 入力を受け付けるようにする
	input_ = Input::GetInstance();

	// 天球更新
	skydome_->Update();

	// 小惑星更新
	asteroidField_.Update();

	// １つ前のシーンへ
	if (input_->PushKey(DIK_ESCAPE)) {
		Audio::GetInstance()->PlayWave(changeSEHandle_);
		nextScene_ = SceneName::Title;
		isEnd_ = true;
	}

	// 次のシーンへ
	if (input_->PushKey(DIK_SPACE)) {
		Audio::GetInstance()->PlayWave(changeSEHandle_);
		nextScene_ = SceneName::InGame;
		isEnd_ = true;
	}
}

void IntroductionScene::Draw() {
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

	// 天球描画
	skydome_->Draw();

	// 小惑星描画
	asteroidField_.Draw(camera_);

	// 3Dオブジェクト描画後処理
	Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト描画
	// 前景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに前景スプライトの描画処理を追加できる
	/// </summary>

	returnTitleSprite_->Draw();

	introSprite_->Draw();

	// スプライト描画後処理
	Sprite::PostDraw();
#pragma endregion
}

std::unique_ptr<IScene> IntroductionScene::NextScene() const {
	switch (nextScene_) {
	case SceneName::Title:
		return std::make_unique<TitleScene>();

	case SceneName::InGame:
		return std::make_unique<GameScene>();

	default:
		return nullptr;
	}
}

TransitionHint IntroductionScene::GetTransitionHint(SceneName to) const {
	TransitionHint h{};
	if (to == SceneName::InGame) {
		h.style = TransitionStyle::WhiteFlash;
		h.flashTime = 0.35f; // 既定0.16fより長め（好みに合わせて調整）
		// h.fadeSpeed = 0.03f; // 必要なら黒フェード速度も上書き可能
	}
	return h;
}