#include "GameScene.h"

#include <cmath>

#ifdef USE_IMGUI
#include <imgui.h>
#endif

#include "Scenes/Clear/ClearScene.h"
#include "Scenes/Finish/FinishScene.h"
#include "Scenes/Title/TitleScene.h"

using namespace KamataEngine;

GameScene::GameScene() {}

GameScene::~GameScene() {
	if (audio_ && bgmVoiceHandle_ != 0u) {
		audio_->StopWave(bgmVoiceHandle_);
	}
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

	previousPlayerPos_ = player_->GetWorldTranslation(); // プレイヤー位置情報

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
	    1.0f,  // lifeTime
	    0.3f,  // startScale
	    10,    // burstCount
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
	countDown_.InitializeFromPaths("./Resources/InGame/3.png", "./Resources/InGame/2.png", "./Resources/InGame/1.png", "./Resources/InGame/Go.png");
	countDown_.SetTimings(0.1f, 0.5f, 0.4f);
	countDown_.SetScaleRange(1.2f, 1.0f);
	countDown_.SetBackOvershoot(1.7f);

	countDown_.SetAudio(audio_->LoadWave("./Resources/SE/CountBeep.wav"), audio_->LoadWave("./Resources/SE/Start.wav"));

	stagePopupTexHandle_ = TextureManager::Load("./Resources/InGame/Rules.png");
	stagePopupSprite_.reset(Sprite::Create(stagePopupTexHandle_, {kScreenWidth_ * 0.5f, 120.0f}));
	if (stagePopupSprite_) {
		stagePopupSprite_->SetAnchorPoint({0.5f, 0.5f});
	}

	// ===== UI（HPバー／スコア） =====
	uiManager_.Initialize(player_);

	// ===== 弾・敵 =====
	bulletManager_.Initialize();
	enemyManager_.Initialize();
	enemyManager_.LoadEnemyCsv("Resources/levels/stage1_more_enemies_turret_balanced.json");

	// ===== 開始 =====
	startSequenceTimer_ = 0.0f;
	EnterStartSequencePhase(StartSequencePhase::StagePopup);

	// ===== 照準 =====
	reticleTexHandle_ = TextureManager::Load("./Resources/InGame/Reticle.png");
	lockOnTexHandle_ = TextureManager::Load("./Resources/InGame/Lockon.png"); // ロックオン演出
	reticleSprite_.reset(Sprite::Create(reticleTexHandle_, reticlePos_));
	if (reticleSprite_) {
		reticleSprite_->SetAnchorPoint({0.5f, 0.5f});
		reticleSprite_->SetSize({32.0f, 32.0f});
	}

	// ===== Pause =====
	pauseMenu_ = std::make_unique<PauseMenu>();
	pauseMenu_->Initialize();
	pauseTitleTexHandle_ = TextureManager::Load("./Resources/InGame/Pause.png");
	pauseTitleSprite_.reset(Sprite::Create(pauseTitleTexHandle_, {kPauseTitlePosX_, kPauseTitlePosY_}));
	pauseTitleSprite_->SetAnchorPoint({0.0f, 0.0f});
	const Vector2 pauseTitleSize = pauseTitleSprite_->GetSize();
	pauseTitleSprite_->SetSize({pauseTitleSize.x * 0.7f, pauseTitleSize.y * 0.7f});
	seExplosionHandle_ = audio_->LoadWave("./Resources/SE/Explosion.wav");
	bgmHandle_ = audio_->LoadWave("./Resources/mokugyo.wav");
	bgmVoiceHandle_ = audio_->PlayWave(bgmHandle_, true, 0.0f);
	bgmVolume_ = 0.0f;
	bgmTargetVolume_ = 0.6f;

	// ===== その他 =====
	isEnd_ = false;
	result_ = GameResult::None;
	clearScore_ = 0;

	isClearAnimating_ = false;
	clearAnimTimer_ = 0.0f;

	transitionPhase_ = SceneTransitionPhase::None;
	transitionTimer_ = 0.0f;
	timeScale_ = 1.0f;
	transitionScoreBonus_ = 0;
	failSecondExplosionDone_ = false;
}

void GameScene::Update() {
	const float dt = kFixedDeltaTime_ * timeScale_;
	UpdateBgmFade(kFixedDeltaTime_);

	// ===== Pause 切り替え =====
	if (input_->TriggerKey(DIK_ESCAPE) && state_ == StartSequencePhase::Playing) {
		isPaused_ = !isPaused_;

		if (isPaused_) {
			pauseMenu_->ResetResult();
			pauseMenu_->StartOpenAnimation();
		} else {
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

		DrawImGui();
		return;
	}

	if (state_ == StartSequencePhase::Playing) {
		DrawImGui();
		if (isDebugUpdatePaused_) {
			return;
		}
	}

	// ===== 通常更新 =====
	BackgroundUpdate();
	StartSequenceUpdate(dt);

	if (state_ == StartSequencePhase::Playing) {
		UpdateAimAndReticle();
		PlayerUpdate();
		SpawnDamageParticles();
		BattleUpdate(dt);
		UIUpdate();
		UpdateLockOnMakers();
		DamageParticleUpdate(dt);
		JudgeResultAndStartClear();
		ClearAnimationUpdate(dt);
		UpdateTransitionDirection(dt);
		EngineSmokesUpdate(dt);
		SpeedLineUpdate(dt);

#ifdef USE_IMGUI
		DrawImGui();
#endif // DEBUG
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
	if (state_ == StartSequencePhase::StagePopup) {
		player_->Draw(cam);
	}

	// ゲーム中だけ描きたいものは Playing に寄せる
	if (state_ == StartSequencePhase::Playing && (result_ == GameResult::None || transitionPhase_ == SceneTransitionPhase::FailCinematic)) {

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
	const bool canDrawSmoke = (state_ == StartSequencePhase::Playing) && ((result_ == GameResult::None) || (result_ == GameResult::Clear && isClearAnimating_));
	if (canDrawSmoke && engineSmokeEmitter_) {
		engineSmokeEmitter_->Draw(cam);
	}

	Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト
	Sprite::PreDraw(commandList);

	if (state_ == StartSequencePhase::StagePopup && stagePopupSprite_) {
		const float t = std::clamp(startSequenceTimer_ / stagePopupDuration_, 0.0f, 1.0f);
		const float alpha = (t < 0.2f) ? (t / 0.2f) : ((t > 0.8f) ? ((1.0f - t) / 0.2f) : 1.0f);
		stagePopupSprite_->SetColor({1.0f, 1.0f, 1.0f, std::clamp(alpha, 0.0f, 1.0f)});
		stagePopupSprite_->Draw();
	}

	if (state_ == StartSequencePhase::CountDown || state_ == StartSequencePhase::Launch) {
		countDown_.Draw();
	}

	// UI（スコア/HP等）は Playing 中だけ（結果画面で出したいなら条件追加）
	if (state_ == StartSequencePhase::Playing) {
		uiManager_.Draw();

		if (reticleSprite_ && result_ == GameResult::None) {
			reticleSprite_->Draw();
		}

		// ロックオン演出
		for (auto& marker : lockOnMarkers_) {
			if (marker.sprite) {
				marker.sprite->Draw();
			}
		}
	}

	// Pauseメニュー
	if (isPaused_) {
		pauseMenu_->Draw();
	}

	// Pauseガイド
	if (!isPaused_ && state_ == StartSequencePhase::Playing) {
		if (pauseTitleSprite_) {
			pauseTitleSprite_->Draw();
		}
	}

	Sprite::PostDraw();
#pragma endregion
}

void GameScene::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("GameScene InGame");

	ImGui::Checkbox("Pause Game Progress", &isDebugUpdatePaused_);
	ImGui::Separator();

	if (ImGui::BeginTabBar("GameSceneTabs")) {
		if (ImGui::BeginTabItem("State")) {
			const char* phaseText = "Playing";
			switch (state_) {
			case StartSequencePhase::StagePopup:
				phaseText = "StagePopup";
				break;
			case StartSequencePhase::PlayerEntry:
				phaseText = "PlayerEntry";
				break;
			case StartSequencePhase::CameraMove:
				phaseText = "CameraMove";
				break;
			case StartSequencePhase::CountDown:
				phaseText = "CountDown";
				break;
			case StartSequencePhase::Launch:
				phaseText = "Launch";
				break;
			case StartSequencePhase::Playing:
				phaseText = "Playing";
				break;
			}
			ImGui::Text("StartSequence: %s", phaseText);
			ImGui::Text("Result: %s", result_ == GameResult::Clear ? "Clear" : result_ == GameResult::Fail ? "Fail" : "None");
			ImGui::Text("PauseMenu: %s", isPaused_ ? "Open" : "Closed");
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Score")) {
			ImGui::DragInt("Score Per Enemy", &kScorePerEnemy_, 1.0f, 0, 10000);
			ImGui::DragInt("Clear Score", &kClearScore_, 10.0f, 0, 999999);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Damage")) {
			ImGui::DragInt("Particle Count", &kDamageParticleCount_, 1.0f, 1, 200);
			ImGui::DragFloat("Particle Speed XY", &kDamageParticleSpeedXY_, 0.01f, 0.0f, 20.0f);
			ImGui::DragFloat("Particle Speed Z", &kDamageParticleSpeedZ_, 0.01f, -20.0f, 20.0f);
			ImGui::DragFloat("Particle Life", &kDamageParticleLife_, 0.01f, 0.01f, 10.0f);
			ImGui::DragFloat("Particle Start Scale", &kDamageParticleStartScale_, 0.01f, 0.0f, 5.0f);
			ImGui::DragFloat("Particle End Scale", &kDamageParticleEndScale_, 0.01f, 0.0f, 5.0f);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Smoke")) {
			ImGui::Text("Normal");
			ImGui::DragFloat("Normal Emit Interval", &normalSmokeParams_.emitInterval, 0.001f, 0.001f, 1.0f);
			ImGui::DragFloat("Normal Life", &normalSmokeParams_.lifeTime, 0.01f, 0.01f, 10.0f);
			ImGui::DragFloat("Normal Start Scale", &normalSmokeParams_.startScale, 0.01f, 0.0f, 5.0f);
			ImGui::DragInt("Normal Burst", &normalSmokeParams_.burstCount, 1.0f, 1, 100);
			ImGui::DragFloat("Normal Base Z Speed", &normalSmokeParams_.baseZSpeed, 0.01f, -20.0f, 20.0f);

			ImGui::Separator();
			ImGui::Text("Boost");
			ImGui::DragFloat("Boost Emit Interval", &boostSmokeParams_.emitInterval, 0.001f, 0.001f, 1.0f);
			ImGui::DragFloat("Boost Life", &boostSmokeParams_.lifeTime, 0.01f, 0.01f, 10.0f);
			ImGui::DragFloat("Boost Start Scale", &boostSmokeParams_.startScale, 0.01f, 0.0f, 5.0f);
			ImGui::DragInt("Boost Burst", &boostSmokeParams_.burstCount, 1.0f, 1, 100);
			ImGui::DragFloat("Boost Base Z Speed", &boostSmokeParams_.baseZSpeed, 0.01f, -20.0f, 20.0f);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Clear")) {
			ImGui::DragFloat("Clear Boost Z", &kClearBoostSpeedZ_, 0.01f, -20.0f, 20.0f);
			ImGui::DragFloat("Clear Boost Y", &kClearBoostSpeedY_, 0.01f, -20.0f, 20.0f);
			ImGui::DragFloat("Clear Rotate Speed X", &kClearRotateSpeedX_, 0.01f, -20.0f, 20.0f);
			ImGui::DragFloat("Clear Shrink Start", &kClearShrinkStart_, 0.01f, 0.0f, 10.0f);
			ImGui::DragFloat("Clear Shrink Speed", &kClearShrinkSpeed_, 0.01f, 0.0f, 20.0f);
			ImGui::DragFloat("Clear End Time", &kClearAnimEndTime_, 0.01f, 0.1f, 20.0f);
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
#endif
}

void GameScene::CountDownUpdate(float dt) { countDown_.Update(dt); }

void GameScene::StartSequenceUpdate(float dt) {
	startSequenceTimer_ += dt;
	
	switch (state_) {
	case StartSequencePhase::StagePopup:
		player_->SetInputEnabled(false);
		if (startSequenceTimer_ >= stagePopupDuration_) {
			EnterStartSequencePhase(StartSequencePhase::PlayerEntry);
		}
		break;

	case StartSequencePhase::PlayerEntry:
		const float t = std::clamp(startSequenceTimer_ / playerEntryDuration_, 0.0f, 1.0f);
		const float ease = 1.0f - (1.0f - t) * (1.0f - t);
		const Vector3 pos = {
		    playerEntryStartPos_.x + (playerEntryEndPos_.x - playerEntryStartPos_.x) * ease,
		    playerEntryStartPos_.y + (playerEntryEndPos_.y - playerEntryStartPos_.y) * ease,
		    playerEntryStartPos_.z + (playerEntryEndPos_.z - playerEntryStartPos_.z) * ease,
		};
		player_->SetTranslate(pos);
		player_->GetWorldTransform().UpdateMatrix();

		if (startSequenceTimer_ >= playerEntryDuration_) {
			EnterStartSequencePhase(StartSequencePhase::CameraMove);
		}
		break;

	case StartSequencePhase::CameraMove:
		const float t = std::clamp(startSequenceTimer_ / cameraMoveDuration_, 0.0f, 1.0f);
		railCamera_->SetCinematicZoom(-10.0f * t);
		if (startSequenceTimer_ >= cameraMoveDuration_) {
			railCamera_->SetCinematicZoom(-10.0f);
			EnterStartSequencePhase(StartSequencePhase::CountDown);
		}
		break;

	case StartSequencePhase::CountDown:
		CountDownUpdate(dt);
		if (!countDown_.IsInputLocked()) {
			EnterStartSequencePhase(StartSequencePhase::Launch);
		}
		break;

	case StartSequencePhase::Launch:
		if (startSequenceTimer_ >= launchDuration_) {
			railCamera_->SetCinematicZoom(0.0f);
			EnterStartSequencePhase(StartSequencePhase::Playing);
		}
		break;

	case StartSequencePhase::Playing:
		break;
	}
}


void GameScene::EnterStartSequencePhase(StartSequencePhase nextPhase) {
	state_ = nextPhase;
	startSequenceTimer_ = 0.0f;

	if (nextPhase == StartSequencePhase::PlayerEntry) {
		player_->SetTranslate(playerEntryStartPos_);
		player_->GetWorldTransform().UpdateMatrix();
		player_->SetInputEnabled(false);
	} else if (nextPhase == StartSequencePhase::CountDown) {
		countDown_.Start();
		bgmTargetVolume_ = 0.55f;
	} else if (nextPhase == StartSequencePhase::Launch) {
		transitionPhase_ = SceneTransitionPhase::IntroCinematic;
		transitionTimer_ = 0.0f;
		timeScale_ = 0.55f;
		bgmTargetVolume_ = 0.85f;
	} else if (nextPhase == StartSequencePhase::Playing) {
		player_->SetInputEnabled(true);
		timeScale_ = 1.0f;
	}
}

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
	railCamera_->AddShake({sx, sy, 0.0f}, 1.0f);

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
		bulletManager_.Update(input_, player_, countDown_, shootDirection_, &enemyManager_);
	}

	// 敵・衝突・スコア（入力ロック解除＆戦闘中のみ）
	if (!countDown_.IsInputLocked() && result_ == GameResult::None) {
		// 敵更新（プレイヤー位置を基準に行動）
		enemyManager_.Update(dt, player_->GetWorldTranslation());

		// 弾→敵 を先に処理（＝スコア対象）
		CollisionManager::ResolveBulletEnemyCollisions(bulletManager_.GetBullets(), enemyManager_.GetEnemies(), countDown_);
		// チャージレーザー（Laser）
		CollisionManager::ResolveLaserEnemyCollisions(bulletManager_.GetLasers(), enemyManager_.GetEnemies(), countDown_);
		// ホーミングミサイル
		CollisionManager::ResolveHomingMissileEnemyCollisions(bulletManager_.GetHomingMissiles(), enemyManager_.GetEnemies(), countDown_);
		// プレイヤー弾 ↔ ターレット弾
		CollisionManager::ResolvePlayerBulletTurretBulletCollisions(bulletManager_.GetBullets(), enemyManager_.GetEnemies(), countDown_);
		// プレイヤー ↔ ターレット弾
		CollisionManager::ResolvePlayerTurretBulletCollisions(player_, enemyManager_.GetEnemies(), countDown_);

		// 弾で倒された敵の数をカウントしてスコアを加算
		int deadCount = 0;
		for (auto& e : enemyManager_.GetEnemies()) {
			if (e && e->IsDead()) {
				deadCount++;
			}
		}

		// スコア加算（弾で倒した敵のみ）
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

void GameScene::UpdateAimAndReticle() {
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

	const Vector3 nearPoint = MyMath::Transform({ndcX, ndcY, 0.0f}, invViewProj);
	const Vector3 farPoint = MyMath::Transform({ndcX, ndcY, 1.0f}, invViewProj);

	Vector3 rayDir = MyMath::Normalize(MyMath::Subtract(farPoint, nearPoint));
	if (std::fabs(rayDir.z) <= 0.0001f) {
		rayDir = {0.0f, 0.0f, 1.0f};
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

void GameScene::UIUpdate() {
	uiManager_.SetHomingCooldownRate(bulletManager_.GetHomingCooldownRate());
	uiManager_.SetHomingLockInfo(bulletManager_.GetCurrentLockCount(), bulletManager_.GetMaxLockCount(), bulletManager_.IsHomingLocking());
	uiManager_.Update();
}

void GameScene::UpdateLockOnMakers() {
	lockOnMarkers_.clear();

	//
	if (result_ != GameResult::None || state_ != StartSequencePhase::Playing) {
		return;
	}

	const Camera* cam = railCamera_ ? railCamera_->GetCamera() : &camera_; //

	//
	if (!cam || lockOnTexHandle_ == 0u) {
		return;
	}

	const Matrix4x4 viewProj = MyMath::Multiply(cam->matView, cam->matProjection);
	const auto& targets = bulletManager_.GetLockedTargets();
	for (CharacterBase* target : targets) {
		if (!target || target->IsDead()) {
			continue;
		}

		const Vector3 clip = MyMath::Transform(target->GetWorldTranslation(), viewProj);
		if (clip.z < 0.0f || clip.z > 1.0f) {
			continue;
		}

		const Vector2 screenPos = {(clip.x * 0.5f + 0.5f) * kScreenWidth_, (-clip.y * 0.5f + 0.5f) * kScreenHeight_};
		if (screenPos.x < 0.0f || screenPos.x > kScreenWidth_ || screenPos.y < 0.0f || screenPos.y > kScreenHeight_) {
			continue;
		}

		auto sprite = std::unique_ptr<Sprite>(Sprite::Create(lockOnTexHandle_, screenPos));
		if (!sprite) {
			continue;
		}

		sprite->SetAnchorPoint({0.5f, 0.5f});
		const float pulse = std::sin(smokeEmitTimer_ * 18.0f) * 0.15f + 1.0f;
		sprite->SetSize({64.0f * pulse, 64.0f * pulse});
		sprite->SetColor({1.0f, 0.25f, 0.25f, 0.95f});

		lockOnMarkers_.push_back({std::move(sprite), target, 0.0f});
	}
}

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
		const Matrix4x4 playerRotateMatrix = MyMath::MakeAffineMatrix({1.0f, 1.0f, 1.0f}, playerWt.rotation_, {0.0f, 0.0f, 0.0f});

		const Vector3 localOffset = {0.0f, kSmokeOffsetY_, kSmokeOffsetZ_};
		const Vector3 offset = MyMath::TransformNormal(localOffset, playerRotateMatrix);
		const Vector3 spawnPos = MyMath::Add(playerPos, offset);

		//
		for (int i = 0; i < params.burstCount; ++i) {
			const Vector3 localVel = {xyDist(rng), xyDist(rng), params.baseZSpeed + zDist(rng)};
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
			transitionPhase_ = SceneTransitionPhase::FailCinematic;
			transitionTimer_ = 0.0f;
			timeScale_ = 0.35f;
			bgmTargetVolume_ = 0.0f;
			StartExplosionAtPlayer(2.3f);
		} else if (uiManager_.GetScore()->GetScore() >= kClearScore_) {
			result_ = GameResult::Clear;
			clearScore_ = uiManager_.GetScore()->GetScore();
			isClearAnimating_ = true;
			clearAnimTimer_ = 0.0f;
			transitionPhase_ = SceneTransitionPhase::ClearCinematic;
			transitionTimer_ = 0.0f;
			timeScale_ = 0.45f;
			bgmTargetVolume_ = 0.15f;
			transitionScoreBonus_ = 300;
			uiManager_.GetScore()->Add(transitionScoreBonus_);
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
			bgmTargetVolume_ = 0.0f;
		}
	}
}

void GameScene::UpdateTransitionDirection(float dt) {
	if (transitionPhase_ == SceneTransitionPhase::None) {
		return;
	}

	transitionTimer_ += dt;

	if (transitionPhase_ == SceneTransitionPhase::IntroCinematic) {
		railCamera_->SetCinematicZoom(-10.0f);
		if (transitionTimer_ >= 0.8f) {
			transitionPhase_ = SceneTransitionPhase::None;
			timeScale_ = 1.0f;
			railCamera_->SetCinematicZoom(0.0f);
		}
	} else if (transitionPhase_ == SceneTransitionPhase::ClearCinematic) {
		railCamera_->SetCinematicZoom(-18.0f);
		if (transitionTimer_ >= 0.55f) {
			timeScale_ = 1.0f;
		}
	} else if (transitionPhase_ == SceneTransitionPhase::FailCinematic) {
		railCamera_->SetCinematicZoom(-24.0f);
		if (transitionTimer_ >= 0.40f && !failSecondExplosionDone_) {
			StartExplosionAtPlayer(1.4f);
			failSecondExplosionDone_ = true;
		}
		if (transitionTimer_ >= 1.2f) {
			isEnd_ = true;
			timeScale_ = 1.0f;
			bgmTargetVolume_ = 0.0f;
		}
	}
}

void GameScene::StartExplosionAtPlayer(float scale) {
	if (audio_ && seExplosionHandle_ != 0u) {
		audio_->PlayWave(seExplosionHandle_, false, 0.7f);
	}

	Vector3 pos = player_->GetWorldTranslation();
	static std::mt19937 rng{(std::random_device{}())};
	std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

	const int count = static_cast<int>(kDamageParticleCount_ * scale);
	for (int i = 0; i < count; i++) {
		Vector3 vel = {dist(rng) * (kDamageParticleSpeedXY_ * scale), dist(rng) * (kDamageParticleSpeedXY_ * scale), dist(rng) * (kDamageParticleSpeedZ_ * scale)};
		auto p = std::make_unique<DamageParticle>();
		p->Initialize(damageParticleModel_, pos, vel, kDamageParticleLife_ * 1.5f, kDamageParticleStartScale_ * scale, 0.0f);
		damageParticles_.push_back(std::move(p));
	}
	if (railCamera_) {
		railCamera_->AddShake({0.5f, 0.3f, 0.0f}, scale * 2.0f);
	}
}

void GameScene::UpdateBgmFade(float dt) {
	if (!audio_ || bgmVoiceHandle_ == 0u) {
		return;
	}

	const float fadeSpeed = 1.6f;
	bgmVolume_ += (bgmTargetVolume_ - bgmVolume_) * (std::min)(1.0f, dt * fadeSpeed * 4.0f);
	bgmVolume_ = std::clamp(bgmVolume_, 0.0f, 1.0f);
	audio_->SetVolume(bgmVoiceHandle_, bgmVolume_);
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
