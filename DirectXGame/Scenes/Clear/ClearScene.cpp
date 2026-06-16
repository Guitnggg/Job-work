#include "ClearScene.h"

#include "Application/Utility/GameTime.h"
#include "Scenes/Title/TitleScene.h"

using namespace KamataEngine;

namespace {
constexpr float kScreenWidth = 1280.0f;
constexpr float kScreenHeight = 720.0f;
constexpr float kScoreDigitWidth = 32.0f;
constexpr int kScoreDigitCount = 5;
constexpr float kScoreCenterRate = 0.6f;
constexpr float kClearTextPosY = 200.0f;
constexpr float kReturnTextPosY = 620.0f;
constexpr float kClearStartWaitTime = 1.5f;
constexpr float kClearPopDuration = 0.8f;
constexpr float kClearPopOverScale = 1.2f;
constexpr float kClearPopFirstRate = 0.6f;
constexpr float kResultCountSpeed = 500.0f;
constexpr float kResultEndWaitTime = 0.5f;
constexpr float kReturnBlinkSpeed = 4.0f;

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

ClearScene::ClearScene(int finalScore) : finalScore_(finalScore) {}

ClearScene::~ClearScene() {}

void ClearScene::Initialize() {
	// 各種初期化処理
	dxCommon_ = DirectXCommon::GetInstance();
	camera_.Initialize();
	input_ = Input::GetInstance();

	// SE 読み込み
	changeSEHandle_ = Audio::GetInstance()->LoadWave("./Resources/SE/SceneChange.wav");
	pointSEHandle_ = Audio::GetInstance()->LoadWave("./Resources/SE/point.wav");

	// 小惑星を一定量生成
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

	// スコアUIセットアップ（0からfinalScoreまでカウントアップ）
	scoreUI_.Initialize();
	displayedScore_ = 0;
	float centerX = kScreenWidth * 0.5f - (kScoreDigitWidth * kScoreDigitCount * 0.5f);
	float centerY = kScreenHeight * kScoreCenterRate;
	scoreUI_.SetPosition(centerX, centerY);

	// GAME CLEAR 表示
	uint32_t clearTex = TextureManager::Load("./Resources/Clear/GameClear.png");
	clearTextSprite_.reset(Sprite::Create(clearTex, {kScreenWidth * 0.5f, kClearTextPosY}));
	clearTextSprite_->SetAnchorPoint({0.5f, 0.5f});
	clearTextSprite_->SetSize({0, 0});
	clearTextSprite_->SetColor({1, 1, 1, 0});

	// RETURN テキスト（最初は透明）
	uint32_t returnTex = TextureManager::Load("./Resources/Clear/Return.png");
	returnTextSprite_.reset(Sprite::Create(returnTex, {kScreenWidth * 0.5f, kReturnTextPosY}));
	returnTextSprite_->SetAnchorPoint({0.5f, 0.5f});
	returnTextSprite_->SetSize(returnTextBaseSize_);
	returnTextSprite_->SetColor({1, 1, 1, 0});

	// 最初のフェーズ
	phase_ = ClearPhase::CameraMove;
	phaseTimer_ = 0.0f;
}

void ClearScene::Update() {
	// 天球更新
	skydome_->Update();

	// 小惑星生成間隔の更新
	const float dt = GameTime::kDeltaTime;
	asteroidField_.Update();

	// ===== クリア演出フェーズ制御 =====
	phaseTimer_ += dt;

	switch (phase_) {
	case ClearPhase::CameraMove:
		// 1.5秒ほど背景だけ流して“余韻”
		if (phaseTimer_ >= kClearStartWaitTime) {
			phase_ = ClearPhase::TitlePop;
			phaseTimer_ = 0.0f;
		}
		break;

	case ClearPhase::TitlePop: {
		// GAME CLEAR!! がバウンスしながら出てくる
		float duration = kClearPopFirstRate;
		float t = (std::min)(phaseTimer_ / duration, 1.0f);
		float over = kClearPopOverScale;
		float scale = (t < kClearPopFirstRate) ? over * (t / kClearPopFirstRate) : over + (1.0f - over) * ((t - kClearPopFirstRate) / (1.0f - kClearPopFirstRate));

		clearTextSprite_->SetSize({clearTextBaseSize_.x * scale, clearTextBaseSize_.y * scale});
		clearTextSprite_->SetColor({1.0f, 1.0f, 1.0f, t});

		if (t >= 1.0f) {
			phase_ = ClearPhase::ResultCount;
			phaseTimer_ = 0.0f;
		}
	} break;

	case ClearPhase::ResultCount: {
		//  スコアを0→finalScore_までカウントアップ
		const float speed = kResultCountSpeed; // 1秒で約500点増える感じ
		int target = static_cast<int>(displayedScore_ + speed * dt);
		if (target > finalScore_) {
			target = finalScore_;
		}
		int add = target - displayedScore_;
		if (add > 0) {
			scoreUI_.Add(add);
			displayedScore_ = target;
		}

		// カウントアップが終わったら 0.5秒待って入力待ちへ
		if (displayedScore_ >= finalScore_ && phaseTimer_ >= kResultEndWaitTime) {
			phase_ = ClearPhase::WaitInput;
			phaseTimer_ = 0.0f;

			if (returnTextSprite_) {
				returnTextSprite_->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
			}
		}
	} break;

	case ClearPhase::WaitInput:
		// 点滅用タイマー
		float blinkSpeed = kReturnBlinkSpeed;                         // 数字を変えると点滅の速さが変わる
		float a = 0.5f * (std::sin(phaseTimer_ * blinkSpeed) + 1.0f); // 0～1

		if (returnTextSprite_) {
			returnTextSprite_->SetColor({1.0f, 1.0f, 1.0f, a});
		}

		// 入力待ち
		break;
	}

	// スコアスプライト更新
	scoreUI_.Update();

	/// シーン変遷 ///
	if (phase_ == ClearPhase::WaitInput && input_->PushKey(DIK_SPACE)) {
		Audio::GetInstance()->PlayWave(changeSEHandle_);
		isEnd_ = true;
	}
}

void ClearScene::Draw() {
	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

#pragma region 背景スプライト描画
	Sprite::PreDraw(commandList);

	Sprite::PostDraw();
	dxCommon_->ClearDepthBuffer();
#pragma endregion

#pragma region 3Dオブジェクト描画
	Model::PreDraw();

	// 小惑星描画
	asteroidField_.Draw(camera_);

	// 天球描画
	skydome_->Draw();

	Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト描画
	Sprite::PreDraw(commandList);

	// GAME CLEAR!! テキスト
	if (clearTextSprite_) {
		clearTextSprite_->Draw();
	}

	// スコア
	scoreUI_.Draw();

	if (returnTextSprite_) {
		returnTextSprite_->Draw();
	}

	Sprite::PostDraw();
#pragma endregion
}

std::unique_ptr<IScene> ClearScene::NextScene() const { return std::make_unique<TitleScene>(); }
