#include "FinishScene.h"

#include "Application/Utility/GameTime.h"
#include "Scenes/SceneHelper.h"
#include "Scenes/Title/TitleScene.h"

using namespace KamataEngine;

namespace {
constexpr Vector2 kFinishTextPosition{150.0f, 200.0f};
constexpr Vector2 kReturnTextPosition{120.0f, 550.0f};
}

namespace {
constexpr float kTwoPi = 6.28318530717958647692f;

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
	finishSprite_ = SceneHelper::CreateSprite("./Resources/finish/End.png", kFinishTextPosition, &finishTextureHandle_);

	returnSprite_ = SceneHelper::CreateSprite("./Resources/finish/Return.png", kReturnTextPosition, &returnTextureHandle_);

	// 各種サウンド
	changeSEHandle_ = Audio::GetInstance()->LoadWave("./Resources/SE/SceneChange.wav");

	// 小惑星生成
	asteroidField_.Initialize(SceneHelper::CreateMenuAsteroidFieldConfig());

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

	SceneHelper::Begin3DDraw(dxCommon_, commandList);

#pragma region 3Dオブジェクト描画
	SceneHelper::DrawModelLayer([this]() {
		asteroidField_.Draw(camera_);
		skydome_->Draw();
	});

#pragma endregion

#pragma region 前景スプライト描画
	SceneHelper::DrawSpriteLayer(commandList, [this]() {
		finishSprite_->Draw();
		returnSprite_->Draw();
	});
#pragma endregion
}

std::unique_ptr<IScene> FinishScene::NextScene() const { return std::make_unique<TitleScene>(); }
