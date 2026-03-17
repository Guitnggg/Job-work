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
	constexpr float kDifficultyBaseX = 700.0f;
	constexpr float kDifficultyBaseY = 300.0f;
	constexpr float kDifficultyStepY = 80.0f;
	constexpr float kDifficultyNormalScale = 1.0f;
	constexpr float kDifficultySelectedScale = 1.12f;
	constexpr Vector4 kDifficultyNormalColor = { 0.8f, 0.8f, 0.8f, 1.0f };
	constexpr Vector4 kDifficultySelectedColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	constexpr Vector4 kDifficultyCursorColor = { 1.0f, 1.0f, 0.5f, 1.0f };
	constexpr float kDifficultyCursorOffsetX = -54.0f;
}

IntroductionScene::IntroductionScene() {}

IntroductionScene::~IntroductionScene() {}

void IntroductionScene::Initialize() {
	dxCommon_ = DirectXCommon::GetInstance();

	// シーン変遷の初期化
	nextScene_ = SceneName::None;
	isEnd_ = false;
	selectedIndex_ = 0;

	// 各種テクスチャ
	returnTitleTextureHandle_ = TextureManager::Load("./Resources/introduction/Esc-export.png");
	returnTitleSprite_.reset(Sprite::Create(returnTitleTextureHandle_, {20.0f, 20.0f}));

	introTextureHandle_ = TextureManager::Load("./Resources/Introduction/setumei.png");
	introSprite_.reset(Sprite::Create(introTextureHandle_, {0.0f, 0.0f}));

	const std::array<const char*, static_cast<size_t>(Difficulty::Count)> difficultyTexturePaths = {
		"./Resources/Introduction/DifficultyTutorial.png",
		"./Resources/Introduction/DifficultyEasy.png",
		"./Resources/Introduction/DifficultyNormal.png",
		"./Resources/Introduction/DifficultyHard.png",
	};

	for (size_t i = 0; i < difficultySprites_.size(); ++i) {
		difficultyTextureHandles_[i] = TextureManager::Load(difficultyTexturePaths[i]);
		difficultySprites_[i].reset(Sprite::Create(difficultyTextureHandles_[i], { kDifficultyBaseX, kDifficultyBaseY + kDifficultyStepY * static_cast<float>(i) }));
		difficultySprites_[i]->SetColor(kDifficultyNormalColor);
	}

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
		return;
	}

	const int difficultyCount = static_cast<int>(Difficulty::Count);
	if (input_->TriggerKey(DIK_UP) || input_->TriggerKey(DIK_W)) {
		selectedIndex_ = (selectedIndex_ - 1 + difficultyCount) % difficultyCount;
	}
	if (input_->TriggerKey(DIK_DOWN) || input_->TriggerKey(DIK_S)) {
		selectedIndex_ = (selectedIndex_ + 1) % difficultyCount;
	}

	if (input_->TriggerKey(DIK_RETURN) || input_->TriggerKey(DIK_SPACE)) {
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

	for (size_t i = 0; i < difficultySprites_.size(); ++i) {
		auto& sprite = difficultySprites_[i];
		if (!sprite) {
			continue;
		}
		sprite->SetPosition({ kDifficultyBaseX, kDifficultyBaseY + kDifficultyStepY * static_cast<float>(i) });
		if (static_cast<int>(i) == selectedIndex_) {
			sprite->SetColor(kDifficultySelectedColor);
			sprite->SetSize({ 512.0f * kDifficultySelectedScale, 128.0f * kDifficultySelectedScale });
		}
		else {
			sprite->SetColor(kDifficultyNormalColor);
			sprite->SetSize({ 512.0f * kDifficultyNormalScale, 128.0f * kDifficultyNormalScale });
		}
		sprite->Draw();
	}

	// スプライト描画後処理
	Sprite::PostDraw();
#pragma endregion
}

std::unique_ptr<IScene> IntroductionScene::NextScene() const {
	switch (nextScene_) {
	case SceneName::Title:
		return std::make_unique<TitleScene>();

	case SceneName::InGame: {
		constexpr std::array<const char*, static_cast<size_t>(Difficulty::Count)> kLevelJsonPaths = {
			"./Resources/Levels/Tutorial.json",
			"./Resources/Levels/Easy.json",
			"./Resources/Levels/Normal.json",
			"./Resources/Levels/Hard.json",
		};

		size_t index = static_cast<size_t>(selectedIndex_);
		if (index >= kLevelJsonPaths.size()) {
			index = 0;
		}

		return std::make_unique<GameScene>(kLevelJsonPaths[index]);
	}

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