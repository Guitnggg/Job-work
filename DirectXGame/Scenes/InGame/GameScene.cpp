#include "GameScene.h"

#include <cmath>

#include "Scenes/Clear/ClearScene.h"
#include "Scenes/Finish/FinishScene.h"
#include "Scenes/Title/TitleScene.h"

using namespace KamataEngine;

GameScene::GameScene() {}

GameScene::~GameScene() {
	delete railCamera_;
	delete skydome_;
	delete player_;

	delete worldTransform_;
	delete model_;

}

void GameScene::Initialize() {
	// ===== 基本 =====
	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();

	worldTransform_ = new WorldTransform();
	worldTransform_->Initialize();
	camera_.Initialize();

	model_ = Model::Create();

	// =====レールカメラ =====
	railCamera_ = new RailCamera();
	railCamera_->Initialize();

	// ===== 天球 =====
	skydome_ = new Skydome();
	skydome_->Initialize(&camera_);

	// ===== プレイヤー =====
	player_ = new Player();
	player_->Initialize(&camera_);
	player_->SetParent(&railCamera_->GetWorldTransform());

	previousPlayerPos_ = player_->GetWorldTranslation();  // プレイヤー位置情報

	// ===== スピードライン初期化 =====
	speedLine_.Initialize(&camera_, 10);

	// ===== ダメージ演出 =====
	damageParticleModel_ = Model::Create();
	damageParticles_.clear();

	// ===== エンジンスモーク初期化 =====
	engineSmokeEmitter_ = std::make_unique<GpuSmokeEmitter>();
	engineSmokeEmitter_->Initialize(256);
	smokeEmitTimer_ = 0.0f;

	// ===== 通常時スモーク =====
	normalSmokeParams_ = {
	    0.08f, // emitInterval
	    1.0f, // lifeTime
	    0.3f, // startScale
	    10,     // burstCount
	    -3.0f  // baseZSpeed
	};

	// ===== クリア演出（ブースト） =====
	boostSmokeParams_ = {
	    0.01f, // emitInterval
	    0.30f, // lifeTime
	    0.20f, // startScale
	    3,     // burstCount
	    -1.6f  // baseZSpeed
	};

	// ===== カウントダウン =====
	countDown_.InitializeFromPaths("./Resources/InGame/3.png", "./Resources/InGame/2.png", "./Resources/InGame/1.png", "./Resources/InGame/GO.png");
	countDown_.SetTimings(0.1f, 0.5f, 0.4f);
	countDown_.SetScaleRange(1.2f, 1.0f);
	countDown_.SetBackOvershoot(1.7f);

	countDown_.SetAudio(
		audio_->LoadWave("./Resources/SE/CountBeep.wav"),
		audio_->LoadWave("./Resources/SE/Start.wav")
	);

	// ===== UI（HPバー／スコア） =====
	uiManager_.Initialize(player_);

	// ===== 弾・敵 =====
	bulletManager_.Initialize();
	enemyManager_.Initialize();
	enemyManager_.LoadEnemyCsv("Resources/levels/stage1_more_enemies_turret_balanced.json");

	// ===== 開始 =====
	countDown_.Start();

	// 開始　３カウントから
	state_ = GameState::CountDown;

	// ===== 照準 =====
	reticleTexHandle_ = TextureManager::Load("./Resources/InGame/Reticle.png");
	reticleSprite_.reset(Sprite::Create(reticleTexHandle_, reticlePos_));
	if (reticleSprite_) {
		reticleSprite_->SetAnchorPoint({ 0.5f, 0.5f });
		reticleSprite_->SetSize({ 32.0f, 32.0f });
	}

	// ===== Pause =====
	pauseMenu_ = std::make_unique<PauseMenu>();
	pauseMenu_->Initialize();
	pauseTitleTexHandle_ = TextureManager::Load("./Resources/InGame/Pause.png");
	pauseTitleSprite_.reset(Sprite::Create(pauseTitleTexHandle_, { kPauseTitlePosX_, kPauseTitlePosY_ }));
	pauseTitleSprite_->SetAnchorPoint({ 0.0f, 0.0f });
	const Vector2 pauseTitleSize = pauseTitleSprite_->GetSize();
	pauseTitleSprite_->SetSize({ pauseTitleSize.x * 0.7f, pauseTitleSize.y * 0.7f });

	// ===== その他 =====
	isEnd_ = false;
	result_ = GameResult::None;
	clearScore_ = 0;

	isClearAnimating_ = false;
	clearAnimTimer_ = 0.0f;
}

void GameScene::Update() {
	const float dt = kFixedDeltaTime_;

	// ===== Pause 切り替え =====
	if (input_->TriggerKey(DIK_ESCAPE) && state_ == GameState::Playing) {
		isPaused_ = !isPaused_;

		if (isPaused_) {
			pauseMenu_->ResetResult();
			pauseMenu_->StartOpenAnimation();
		}
		else {
			pauseMenu_->StartCloseAnimation();
		}
	}

	// ===== Pause 中 =====
	if (isPaused_) {
		pauseMenu_->Update();

		switch (pauseMenu_->GetResult()) {
		case PauseMenu::Result::Resume:
			pauseMenu_->StartCloseAnimation();
			isPaused_ = false;
			break;

		case PauseMenu::Result::Retry:
			requestRetry_ = true;
			isEnd_ = true;
			break;

		case PauseMenu::Result::ToTitle:
			requestToTitle_ = true;
			isEnd_ = true;
			break;

		default:
			break;
		}

		return;
	}

	// ===== 通常更新 =====
	BackgroundUpdate();	

	switch (state_) {
	case GameState::CountDown:
		CountDownUpdate(dt);
		if (!countDown_.IsInputLocked()) {
			state_ = GameState::Playing;
		}
		break;

	case GameState::Playing:
		UpdateAimAndReticle();
		PlayerUpdate();
		SpawnDamageParticles();
		BattleUpdate(dt);
		UIUpdate();
		DamageParticleUpdate(dt);
		JudgeResultAndStartClear();
		ClearAnimationUpdate(dt);
		EngineSmokesUpdate(dt);
		SpeedLineUpdate(dt);
		break;
	}

	CameraUpdate();
}

void GameScene::Draw() {
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	const Camera* cam = railCamera_->GetCamera();

#pragma region 背景スプライト
	Sprite::PreDraw(commandList);
	// 背景スプライトの描画があればここ

	Sprite::PostDraw();
	dxCommon_->ClearDepthBuffer();
#pragma endregion

#pragma region 3Dオブジェクト描画
	Model::PreDraw();

	// 空/スカイドームは常に描く（雰囲気）
	skydome_->Draw();

	// （任意）CountDown中もプレイヤーだけ見せたいなら描く
	if (state_ == GameState::Playing) {
		player_->Draw(cam);
	}

	// ゲーム中だけ描きたいものは Playing に寄せる
	if (state_ == GameState::Playing && result_ == GameResult::None) {

		// スピードライン
		speedLine_.Draw();		

		// ダメージパーティクル
		if (damageParticleModel_) {
			for (auto& p : damageParticles_) {
				p->Draw(cam);
			}
		}

		// 敵・弾
		enemyManager_.Draw(cam);
		bulletManager_.Draw(cam);
	}

	// エンジンスモークは通常時とクリア演出中に描画
	const bool canDrawSmoke = (state_ == GameState::Playing) &&
		((result_ == GameResult::None) || (result_ == GameResult::Clear && isClearAnimating_));
	if (canDrawSmoke && engineSmokeEmitter_) {
		engineSmokeEmitter_->Draw(cam);
	}

	Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト
	Sprite::PreDraw(commandList);
	
	// 3カウントは CountDown 中だけ
	if (state_ == GameState::CountDown) {
		countDown_.Draw();
	}

	// UI（スコア/HP等）は Playing 中だけ（結果画面で出したいなら条件追加）
	if (state_ == GameState::Playing) {
		uiManager_.Draw();

		if (reticleSprite_ && result_ == GameResult::None) {
			reticleSprite_->Draw();
		}
	}

	// Pauseメニュー
	if (isPaused_) {		
		pauseMenu_->Draw();
	}

	// Pauseガイド
	if (!isPaused_ && state_ == GameState::Playing) {
		if (pauseTitleSprite_) {
			pauseTitleSprite_->Draw();
		}
	}

	Sprite::PostDraw();
#pragma endregion
}

void GameScene::CountDownUpdate(float dt) { countDown_.Update(dt); }

void GameScene::BackgroundUpdate() { skydome_->Update(); }

void GameScene::PlayerUpdate() {
	// クリア演出中は更新しない
	if (!countDown_.IsInputLocked()) {
		if (!(result_ == GameResult::Clear && isClearAnimating_)) {
			player_->Update();
		}
	}
}

void GameScene::SpawnDamageParticles() {
	// プレイヤーが被弾していないフレームでは生成しない
	if (!player_->ConsumeTookDamageEvent()) {
		return;
	}

	// カメラシェイク追加
	float sx = (rand() % 200 - 100) / 100.0f;
	float sy = (rand() % 200 - 100) / 100.0f;
	railCamera_->AddShake({ sx, sy, 0.0f }, 1.0f);

	Vector3 pos = player_->GetWorldTranslation();
	static std::mt19937 rng{(std::random_device{}())};
	std::uniform_real_distribution<float> dist(-1, 1);

	// 被弾時に複数のダメージパーティクルを一気に生成
	for (int i = 0; i < kDamageParticleCount_; i++) {
		Vector3 vel = {dist(rng) * kDamageParticleSpeedXY_, dist(rng) * kDamageParticleSpeedXY_, dist(rng) * kDamageParticleSpeedZ_};
		auto p = std::make_unique<DamageParticle>();

		p->Initialize(damageParticleModel_, pos, vel, kDamageParticleLife_, kDamageParticleStartScale_, kDamageParticleEndScale_);
		damageParticles_.push_back(std::move(p));
	}
}

void GameScene::BattleUpdate(float dt) {
	// 弾（戦闘中のみ）
	if (result_ == GameResult::None) {
		bulletManager_.Update(input_, player_, countDown_,shootDirection_);
	}

	// 敵・衝突・スコア（入力ロック解除＆戦闘中のみ）
	if (!countDown_.IsInputLocked() && result_ == GameResult::None) {
		// 敵更新（プレイヤー位置を基準に行動）
		enemyManager_.Update(dt, player_->GetWorldTranslation());

		// 弾→敵 を先に処理（＝スコア対象）
		CollisionManager::ResolveBulletEnemyCollisions(bulletManager_.GetBullets(), enemyManager_.GetEnemies(), countDown_);
		// チャージレーザー（Razer）
		CollisionManager::ResolveLaserEnemyCollisions(bulletManager_.GetLasers(), enemyManager_.GetEnemies(), countDown_);

		// 弾で倒された敵の数をカウントしてスコアを加算
		int deadCount = 0;
		for (auto& e : enemyManager_.GetEnemies()) {
			if (auto* s = dynamic_cast<SeekerEnemy*>(e.get())) {
				if (s->IsDead()) {
					deadCount++;
				}
			}
		}
		if (deadCount > 0) {
			uiManager_.GetScore()->Add(deadCount * kScorePerEnemy_);
		}

		// スコア加算対象（弾で倒した敵）を先に削除
		enemyManager_.RemoveDeadEnemies();

		// プレイヤー→敵（体当たりなど）は後で処理（＝スコア対象外）
		CollisionManager::ResolvePlayerEnemyCollisions(player_, enemyManager_.GetEnemies(), countDown_);

		// 体当たり等で死亡した敵があれば削除（※スコアは加算しない）
		enemyManager_.RemoveDeadEnemies();
	}
}

void GameScene::UpdateAimAndReticle(){
	if (!input_ || !player_) {
		return;
	}

	const Vector2 mouse = input_->GetMousePosition();
	reticlePos_.x = std::clamp(mouse.x, 0.0f, kScreenWidth_);
	reticlePos_.y = std::clamp(mouse.y, 0.0f, kScreenHeight_);
	if (reticleSprite_) {
		reticleSprite_->SetPosition(reticlePos_);
	}

	const Camera* cam = railCamera_ ? railCamera_->GetCamera() : &camera_;
	if (!cam) {
		return;
	}

	const float ndcX = (reticlePos_.x / kScreenWidth_) * 2.0f - 1.0f;
	const float ndcY = 1.0f - (reticlePos_.y / kScreenHeight_) * 2.0f;

	const Matrix4x4 viewProj = MyMath::Multiply(cam->matView, cam->matProjection);
	const Matrix4x4 invViewProj = MyMath::Inverse(viewProj);

	const Vector3 nearPoint = MyMath::Transform({ ndcX, ndcY, 0.0f }, invViewProj);
	const Vector3 farPoint = MyMath::Transform({ ndcX, ndcY, 1.0f }, invViewProj);

	Vector3 rayDir = MyMath::Normalize(MyMath::Subtract(farPoint, nearPoint));
	if (std::fabs(rayDir.z) <= 0.0001f) {
		rayDir = { 0.0f, 0.0f, 1.0f };
	}

	const Vector3 playerPos = player_->GetWorldTranslation();
	const float targetZ = playerPos.z + 120.0f;
	const float t = (targetZ - nearPoint.z) / rayDir.z;
	const Vector3 hit = MyMath::Add(nearPoint, MyMath::Multiply(rayDir, t));

	shootDirection_ = MyMath::Normalize(MyMath::Subtract(hit, playerPos));
	if (shootDirection_.z < 0.05f) {
		shootDirection_.z = 0.05f;
		shootDirection_ = MyMath::Normalize(shootDirection_);
	}

	player_->SetAimDirection(shootDirection_);
}

void GameScene::UIUpdate() { uiManager_.Update(); }

void GameScene::DamageParticleUpdate(float dt) {
	//
	for (auto& p : damageParticles_) {
		p->Update(dt);
	}

	//
	damageParticles_.erase(std::remove_if(damageParticles_.begin(), damageParticles_.end(), [](const std::unique_ptr<DamageParticle>& p) { return p->IsFinished(); }), damageParticles_.end());
}

void GameScene::EngineSmokesUpdate(float dt) {
	// 戦闘開始カウントダウン中はスモークを出さない（クリア演出は例外）
	if (result_ == GameResult::None && countDown_.IsInputLocked()) {
		return;
	}

	// 戦闘中、またはクリア演出中のみスモークを生成
	const bool canEmit = (result_ == GameResult::None) || (result_ == GameResult::Clear && isClearAnimating_);

	if (!canEmit) {
		return;
	}

	smokeEmitTimer_ += dt;

	// 使用するパラメータを選択
	const SmokeParams& params = (result_ == GameResult::Clear && isClearAnimating_) ? boostSmokeParams_ : normalSmokeParams_;

	static std::mt19937 rng{std::random_device{}()};
	std::uniform_real_distribution<float> xyDist(-kSmokeRandXY_, kSmokeRandXY_);
	std::uniform_real_distribution<float> zDist(-kSmokeRandZ_, kSmokeRandZ_);

	//
	while (smokeEmitTimer_ >= params.emitInterval) {
		smokeEmitTimer_ -= params.emitInterval;

		const Vector3 playerPos = player_->GetWorldTranslation();
		const auto& playerWt = player_->GetWorldTransform();
		const Matrix4x4 playerRotateMatrix = MyMath::MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, playerWt.rotation_, { 0.0f, 0.0f, 0.0f });

		const Vector3 localOffset = { 0.0f, kSmokeOffsetY_, kSmokeOffsetZ_ };
		const Vector3 offset = MyMath::TransformNormal(localOffset, playerRotateMatrix);
		const Vector3 spawnPos = MyMath::Add(playerPos, offset);

		//
		for (int i = 0; i < params.burstCount; ++i) {
			const Vector3 localVel = { xyDist(rng), xyDist(rng), params.baseZSpeed + zDist(rng) };
			const Vector3 vel = MyMath::TransformNormal(localVel, playerRotateMatrix);

			if (engineSmokeEmitter_) {
				engineSmokeEmitter_->Emit(spawnPos, vel, params.lifeTime, params.startScale, 0.0f);
			}
		}
	}

	if (engineSmokeEmitter_) {
		engineSmokeEmitter_->Update(dt);
	}
}

void GameScene::CameraUpdate() {
	if (isRailCameraActive_) {

		// ===== プレイヤー移動量から入力を作る =====
		Vector3 now = player_->GetWorldTranslation();
		float deltaX = now.x - previousPlayerPos_.x;

		float inputX = MyMath::Clamp(deltaX * 40.0f, -1.0f, 1.0f);

		// 先に入力を渡す
		railCamera_->SetMoveInput(inputX);

		// その後でカメラ更新
		railCamera_->Update();

		// 描画用 Camera に反映
		camera_.matView = railCamera_->GetCamera()->matView;
		camera_.matProjection = railCamera_->GetCamera()->matProjection;
		camera_.TransferMatrix();

		previousPlayerPos_ = now;
	} else {
		camera_.UpdateMatrix();
	}
}


void GameScene::SpeedLineUpdate(float dt) {
	//
	if (isRailCameraActive_) {
		//
		speedLine_.Update(dt, railCamera_->GetWorldTransform().translation_);
	}
}

void GameScene::JudgeResultAndStartClear() {
	if (result_ == GameResult::None) {
		if (player_->IsExplosionFinished()) {
			result_ = GameResult::Fail;
			isEnd_ = true;
		} else if (uiManager_.GetScore()->GetScore() >= kClearScore_) {
			result_ = GameResult::Clear;
			clearScore_ = uiManager_.GetScore()->GetScore();
			isClearAnimating_ = true;
			clearAnimTimer_ = 0.0f;
		}
	}
}

void GameScene::ClearAnimationUpdate(float dt) {
	//
	if (result_ == GameResult::Clear && isClearAnimating_) {
		clearAnimTimer_ += dt;
		auto& wt = player_->GetWorldTransform();

		//
		wt.translation_.z += kClearBoostSpeedZ_ * dt;
		wt.translation_.y += kClearBoostSpeedY_ * dt;
		wt.rotation_.x -= kClearRotateSpeedX_ * dt;

		//
		if (clearAnimTimer_ > kClearShrinkStart_) {
			float shrink = clearAnimTimer_ - kClearShrinkStart_;
			float scale = (std::max)(0.0f, 1.0f - shrink * kClearShrinkSpeed_);
			wt.scale_ = {scale, scale, scale};
		}

		//
		wt.UpdateMatrix();

		//
		if (clearAnimTimer_ >= kClearAnimEndTime_) {
			isEnd_ = true;
			isClearAnimating_ = false;
		}
	}
}

std::unique_ptr<IScene> GameScene::NextScene() const {
	// Pause 由来の遷移を最優先
	if (requestRetry_) {
		return std::make_unique<GameScene>();
	}

	if (requestToTitle_) {
		return std::make_unique<TitleScene>();
	}

	// 結果に応じて遷移先を切り替える
	if (result_ == GameResult::Clear) {
		return std::make_unique<ClearScene>(clearScore_); // クリア時
	} else {
		return std::make_unique<FinishScene>(); // 失敗時（Fail or None）
	}
}
