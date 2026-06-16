#include "FinishScene.h"

#include "Application/Utility/GameTime.h"
#include "TitleScene.h"

using namespace KamataEngine;

namespace {
constexpr float kTwoPi = 6.28318530717958647692f;

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
} // namespace

FinishScene::FinishScene() {}

FinishScene::~FinishScene() {}

void FinishScene::Initialize() {
	// 各種初期化処理
	dxCommon_ = DirectXCommon::GetInstance();

	// カメラ
	camera_.Initialize();

	// 入力を受け付けるようにする
	input_ = Input::GetInstance();

	// 各種テクスチャ
	finishTextureHandle_ = TextureManager::Load("./Resources/finish/End.png");
	finishSprite_.reset(Sprite::Create(finishTextureHandle_, {150.0f, 200.0f}));

	returnTextureHandle_ = TextureManager::Load("./Resources/finish/Return.png");
	returnSprite_.reset(Sprite::Create(returnTextureHandle_, {120.0f, 550.0f}));

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

void FinishScene::Update() {
	// 天球更新
	skydome_->Update();

	// 小惑星出現タイマー更新
	asteroidField_.Update();

	// 点滅演出更新
	blinkTimer_ += GameTime::kDeltaTime;

	if (blinkTimer_ >= blinkInterval_) {
		blinkTimer_ -= blinkInterval_;
	}

	// アルファをサイン波で変化させる
	float alpha = 0.5f + 0.5f * sinf(blinkTimer_ / blinkInterval_ * kTwoPi);
	returnSprite_->SetColor({1.0f, 1.0f, 1.0f, alpha});

	/// シーン変遷 ///
	if (input_->PushKey(DIK_SPACE)) { // シーン変遷の条件を書く
		Audio::GetInstance()->PlayWave(changeSEHandle_);
		isEnd_ = true;
	}
}

void FinishScene::Draw() {
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
	// 背景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに背景スプライトの描画処理を追加できる
	/// </summary>

	finishSprite_->Draw();
	returnSprite_->Draw();

	// スプライト描画後処理
	Sprite::PostDraw();
#pragma endregion
}

std::unique_ptr<IScene> FinishScene::NextScene() const { return std::make_unique<TitleScene>(); }
