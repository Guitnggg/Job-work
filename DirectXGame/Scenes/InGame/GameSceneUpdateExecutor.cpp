#include "GameSceneUpdateExecutor.h"
#include "GameScene.h"

#include <algorithm>
#include <cmath>
#include <random>

using namespace KamataEngine;

// 各種 GPU パーティクルで使う色定義。
// Emit 呼び出し側では「発生時の色」と「寿命切れ時の色」を渡すため、
// エンジン煙・ミサイル噴射・被弾火花・爆発煙ごとに用途が分かる名前にしている。
namespace {
constexpr Vector4 kEngineCoreColor{0.35f, 0.85f, 1.0f, 0.72f};
constexpr Vector4 kEngineFadeColor{0.02f, 0.10f, 0.28f, 0.0f};
constexpr Vector4 kBoostCoreColor{0.62f, 0.95f, 1.0f, 0.95f};
constexpr Vector4 kBoostOuterColor{1.0f, 0.46f, 0.08f, 0.0f};
constexpr Vector4 kAfterburnerCoreColor{0.55f, 0.92f, 1.0f, 0.95f};
constexpr Vector4 kAfterburnerOuterColor{1.0f, 0.35f, 0.06f, 0.0f};
constexpr Vector4 kSparkStartColor{1.0f, 0.92f, 0.28f, 0.95f};
constexpr Vector4 kSparkEndColor{1.0f, 0.18f, 0.04f, 0.0f};
constexpr Vector4 kExplosionHotColor{1.0f, 0.82f, 0.22f, 1.0f};
constexpr Vector4 kExplosionFireColor{1.0f, 0.24f, 0.04f, 0.0f};
constexpr Vector4 kSmokeStartColor{0.24f, 0.22f, 0.20f, 0.55f};
constexpr Vector4 kSmokeEndColor{0.03f, 0.03f, 0.04f, 0.0f};
} // namespace

void GameSceneUpdateExecutor::Update(GameScene& gameScene) {
	// ヒットストップは複数箇所から要求されるため、現在の残りフレームより長い要求だけを採用する
	if (gameScene.hitStopRequestFrames_ > gameScene.hitStopFrames_) {
		gameScene.hitStopFrames_ = gameScene.hitStopRequestFrames_;
		gameScene.hitStopRequestFrames_ = 0;
	}

	// timeScale_ はカウントダウン直後・クリア/失敗演出でスローを掛けるため、以降の更新処理にはスケール済みの固定デルタを渡す
	const float dt = gameScene.kFixedDeltaTime_ * gameScene.timeScale_;

	// ESC でポーズを開閉する。HowTo 表示中だけは ESC をポーズ解除に使わず、メニュー側へ処理を渡す
	if (gameScene.input_->TriggerKey(DIK_ESCAPE) && gameScene.state_ == GameState::Playing) {
		if (gameScene.isPaused_ && gameScene.pauseMenu_->IsHowToOpen()) {
			gameScene.pauseMenu_->Update();
			return;
		}

		gameScene.isPaused_ = !gameScene.isPaused_;

		if (gameScene.isPaused_) {
			gameScene.pauseMenu_->ResetResult();
			gameScene.pauseMenu_->StartOpenAnimation();
		} else {
			gameScene.pauseMenu_->StartCloseAnimation();
		}
	}

	// ポーズ中はゲーム本体を進めず、メニュー操作の結果だけを監視する
	if (gameScene.isPaused_) {
		gameScene.pauseMenu_->Update();

		switch (gameScene.pauseMenu_->GetResult()) {
		case PauseMenu::Result::Resume:
			gameScene.pauseMenu_->StartCloseAnimation();
			gameScene.isPaused_ = false;
			break;

		case PauseMenu::Result::Retry:
			gameScene.requestRetry_ = true;
			gameScene.isEnd_ = true;
			break;

		case PauseMenu::Result::ToTitle:
			gameScene.requestToTitle_ = true;
			gameScene.isEnd_ = true;
			break;

		default:
			break;
		}
		return;
	}

	if (gameScene.state_ == GameState::Playing) {
		// デバック停止中は演出も含めたゲーム更新を完全に止める
		if (gameScene.isDebugUpdatePaused_) {
			return;
		}

		// ヒットストップ中は戦闘・演出を止め、ロックオン/UI/カメらだけ最低限更新して表示崩れを防ぐ
		if (gameScene.result_ == GameResult::None && gameScene.hitStopFrames_ > 0) {
			--gameScene.hitStopFrames_;
			UpdateLockOnMarkers(gameScene);
			UIUpdate(gameScene);
			CameraUpdate(gameScene);
			return;
		}
	}

	BackgroundUpdate(gameScene);

	switch (gameScene.state_) {
	case GameState::CountDown:
		CountDownUpdate(gameScene, dt);
		if (!gameScene.countDown_.IsInputLocked()) {
			gameScene.state_ = GameState::Playing;
			gameScene.transitionPhase_ = SceneTransitionPhase::IntroCinematic;
			gameScene.transitionTimer_ = 0.0f;
			gameScene.timeScale_ = 0.55f;
		}
		break;

	case GameState::Playing:
		UpdateAimAndReticle(gameScene);
		PlayerUpdate(gameScene);
		SpawnDamageParticles(gameScene);
		BattleUpdate(gameScene, dt);
		UIUpdate(gameScene);
		UpdateLockOnMarkers(gameScene);
		DamageGpuParticlesUpdate(gameScene, dt);
		JudgeResultAndStartClear(gameScene);
		ClearAnimationUpdate(gameScene, dt);
		UpdateTransitionDirection(gameScene, dt);
		EngineSmokesUpdate(gameScene, dt);
		MissileAfterburnerUpdate(gameScene, dt);
		SpeedLineUpdate(gameScene, dt);

		break;
	}

	CameraUpdate(gameScene);
}

void GameSceneUpdateExecutor::CountDownUpdate(GameScene& gameScene, float dt) { gameScene.countDown_.Update(dt); }

void GameSceneUpdateExecutor::BackgroundUpdate(GameScene& gameScene) { gameScene.skydome_->Update(); }

void GameSceneUpdateExecutor::PlayerUpdate(GameScene& gameScene) {
	if (!gameScene.countDown_.IsInputLocked()) {
		if (!(gameScene.result_ == GameResult::Clear && gameScene.isClearAnimating_)) {
			gameScene.player_->Update();
		}
	}
}

void GameSceneUpdateExecutor::SpawnDamageParticles(GameScene& gameScene) {
	if (!gameScene.player_->ConsumeTookDamageEvent()) {
		return;
	}

	float sx = (rand() % 200 - 100) / 100.0f;
	float sy = (rand() % 200 - 100) / 100.0f;
	gameScene.railCamera_->AddShake({sx, sy, 0.0f}, 1.0f);

	Vector3 pos = gameScene.player_->GetWorldTranslation();
	static std::mt19937 rng{(std::random_device{}())};
	std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

	if (gameScene.damageSmokeEmitter_) {
		for (int i = 0; i < gameScene.kDamageGpuBurst_; ++i) {
			Vector3 v = {dist(rng) * gameScene.kDamageGpuSpeed_, dist(rng) * gameScene.kDamageGpuSpeed_, dist(rng) * gameScene.kDamageGpuSpeed_ * 0.6f};
			gameScene.damageSmokeEmitter_->Emit(pos, v, gameScene.kDamageGpuLife_, gameScene.kDamageGpuStartScale_, gameScene.kDamageGpuEndScale_, kSparkStartColor, kSparkEndColor);
		}
	}
}

void GameSceneUpdateExecutor::BattleUpdate(GameScene& gameScene, float dt) {
	if (gameScene.result_ == GameResult::None) {
		gameScene.bulletManager_.Update(gameScene.input_, gameScene.player_.get(), gameScene.countDown_, gameScene.shootDirection_, &gameScene.enemyManager_);
	}

	if (!gameScene.countDown_.IsInputLocked() && gameScene.result_ == GameResult::None) {
		gameScene.enemyManager_.Update(dt, gameScene.player_->GetWorldTranslation());

		CollisionManager::ResolveBulletEnemyCollisions(gameScene.bulletManager_.GetBullets(), gameScene.enemyManager_.GetEnemies(), gameScene.countDown_);
		CollisionManager::ResolveLaserEnemyCollisions(gameScene.bulletManager_.GetLasers(), gameScene.enemyManager_.GetEnemies(), gameScene.countDown_);
		CollisionManager::ResolveHomingMissileEnemyCollisions(gameScene.bulletManager_.GetHomingMissiles(), gameScene.enemyManager_.GetEnemies(), gameScene.countDown_);
		CollisionManager::ResolvePlayerBulletTurretBulletCollisions(gameScene.bulletManager_.GetBullets(), gameScene.enemyManager_.GetEnemies(), gameScene.countDown_);
		CollisionManager::ResolvePlayerTurretBulletCollisions(gameScene.player_.get(), gameScene.enemyManager_.GetEnemies(), gameScene.countDown_);

		int deadCount = 0;
		for (auto& e : gameScene.enemyManager_.GetEnemies()) {
			if (e && e->IsDead()) {
				deadCount++;
			}
		}

		if (gameScene.damageSmokeEmitter_) {
			static std::mt19937 enemyHitRng{(std::random_device{}())};
			std::uniform_real_distribution<float> randDist(-1.0f, 1.0f);
			auto& enemies = gameScene.enemyManager_.GetEnemies();
			for (auto& e : enemies) {
				if (!e) {
					continue;
				}
				CharacterBase* key = e.get();
				const int32_t nowHp = e->GetHP();
				auto it = gameScene.prevEnemyHpMap_.find(key);
				const int32_t prevHp = (it != gameScene.prevEnemyHpMap_.end()) ? it->second : nowHp;
				if (nowHp < prevHp && !e->IsDead()) {
					const Vector3 hitPos = e->GetWorldTranslation();
					for (int i = 0; i < gameScene.kDamageGpuBurst_ * gameScene.kEnemyHitBurstScale_; ++i) {
						Vector3 v = {
						    randDist(enemyHitRng) * (gameScene.kDamageGpuSpeed_ * 0.85f), randDist(enemyHitRng) * (gameScene.kDamageGpuSpeed_ * 0.85f),
						    randDist(enemyHitRng) * (gameScene.kDamageGpuSpeed_ * 0.45f)};
						gameScene.damageSmokeEmitter_->Emit(
						    hitPos, v, gameScene.kDamageGpuLife_ * 0.6f, gameScene.kDamageGpuStartScale_ * 0.5f, gameScene.kDamageGpuEndScale_ * 0.5f, kSparkStartColor, kSparkEndColor);
					}
					if (gameScene.audio_ && gameScene.seEnemyHitHandle_ != 0u) {
						gameScene.audio_->PlayWave(gameScene.seEnemyHitHandle_, false, 0.25f);
					}
				}
				if (nowHp < prevHp && e->IsDead()) {
					const Vector3 deadPos = e->GetWorldTranslation();
					const bool killedByMissile = e->GetLastDamageSource() == CharacterBase::DamageSource::HomingMissile;
					const float killEffectScale = killedByMissile ? 2.0f : 1.0f;
					const int fireBurstCount = static_cast<int>(gameScene.kDamageGpuBurst_ * gameScene.kEnemyKillBurstScale_ * killEffectScale);
					const int smokeBurstCount = static_cast<int>(gameScene.kDamageGpuBurst_ * killEffectScale);
					for (int i = 0; i < fireBurstCount; ++i) {
						Vector3 v = {
							  randDist(enemyHitRng)* (gameScene.kDamageGpuSpeed_ * 1.6f * killEffectScale), randDist(enemyHitRng)* (gameScene.kDamageGpuSpeed_ * 1.6f * killEffectScale),
							randDist(enemyHitRng)* (gameScene.kDamageGpuSpeed_ * 1.3f * killEffectScale) };
						gameScene.damageSmokeEmitter_->Emit(
							deadPos, v, gameScene.kDamageGpuLife_ * 1.0f, gameScene.kDamageGpuStartScale_ * 1.2f * killEffectScale, gameScene.kDamageGpuEndScale_ * 1.8f * killEffectScale,
							kExplosionHotColor, kExplosionFireColor);
					}
					for (int i = 0; i < smokeBurstCount; ++i) {
						Vector3 v = {
						   randDist(enemyHitRng)* (gameScene.kDamageGpuSpeed_ * 0.75f * killEffectScale), randDist(enemyHitRng)* (gameScene.kDamageGpuSpeed_ * 0.75f * killEffectScale),
							randDist(enemyHitRng)* (gameScene.kDamageGpuSpeed_ * 0.55f * killEffectScale) };
						gameScene.damageSmokeEmitter_->Emit(
							deadPos, v, gameScene.kDamageGpuLife_ * 2.5f, gameScene.kDamageGpuStartScale_ * 1.6f * killEffectScale, gameScene.kDamageGpuEndScale_ * 3.0f * killEffectScale,
							kSmokeStartColor, kSmokeEndColor);
					}
					if (gameScene.railCamera_) {
						gameScene.railCamera_->AddShake({ randDist(enemyHitRng), randDist(enemyHitRng), 0.0f }, 0.7f * killEffectScale);
					}
					gameScene.hitStopRequestFrames_ = (std::max)(gameScene.hitStopRequestFrames_, gameScene.kEnemyHitStopFrames_);

					if (gameScene.audio_ && gameScene.seEnemyKillHandle_ != 0u) {
						gameScene.audio_->PlayWave(gameScene.seEnemyKillHandle_, false, 0.55f);
					}

					gameScene.scorePopups_.push_back({deadPos, gameScene.kScorePerEnemy_, 0.6f, 0.6f, 1.8f});
				}
				gameScene.prevEnemyHpMap_[key] = nowHp;
			}
		}

		if (deadCount > 0) {
			gameScene.uiManager_.GetScore()->Add(deadCount * gameScene.kScorePerEnemy_);
		}

		gameScene.enemyManager_.RemoveDeadEnemies();
		CollisionManager::ResolvePlayerEnemyCollisions(gameScene.player_.get(), gameScene.enemyManager_.GetEnemies(), gameScene.countDown_);
		gameScene.enemyManager_.RemoveDeadEnemies();
	}
}

void GameSceneUpdateExecutor::UpdateAimAndReticle(GameScene& gameScene) {
	if (!gameScene.input_ || !gameScene.player_) {
		return;
	}

	const Vector2 mouse = gameScene.input_->GetMousePosition();
	gameScene.reticlePos_.x = std::clamp(mouse.x, 0.0f, gameScene.kScreenWidth_);
	gameScene.reticlePos_.y = std::clamp(mouse.y, 0.0f, gameScene.kScreenHeight_);
	if (gameScene.reticleSprite_) {
		gameScene.reticleSprite_->SetPosition(gameScene.reticlePos_);
	}

	const Camera* cam = gameScene.railCamera_ ? gameScene.railCamera_->GetCamera() : &gameScene.camera_;
	if (!cam) {
		return;
	}

	const float ndcX = (gameScene.reticlePos_.x / gameScene.kScreenWidth_) * 2.0f - 1.0f;
	const float ndcY = 1.0f - (gameScene.reticlePos_.y / gameScene.kScreenHeight_) * 2.0f;

	const Matrix4x4 viewProj = MyMath::Multiply(cam->matView, cam->matProjection);
	const Matrix4x4 invViewProj = MyMath::Inverse(viewProj);

	const Vector3 nearPoint = MyMath::Transform({ndcX, ndcY, 0.0f}, invViewProj);
	const Vector3 farPoint = MyMath::Transform({ndcX, ndcY, 1.0f}, invViewProj);

	Vector3 rayDir = MyMath::Normalize(MyMath::Subtract(farPoint, nearPoint));
	if (std::fabs(rayDir.z) <= 0.0001f) {
		rayDir = {0.0f, 0.0f, 1.0f};
	}

	const Vector3 playerPos = gameScene.player_->GetWorldTranslation();
	const float targetZ = playerPos.z + 120.0f;
	const float t = (targetZ - nearPoint.z) / rayDir.z;
	const Vector3 hit = MyMath::Add(nearPoint, MyMath::Multiply(rayDir, t));

	gameScene.shootDirection_ = MyMath::Normalize(MyMath::Subtract(hit, playerPos));
	if (gameScene.shootDirection_.z < 0.05f) {
		gameScene.shootDirection_.z = 0.05f;
		gameScene.shootDirection_ = MyMath::Normalize(gameScene.shootDirection_);
	}

	gameScene.player_->SetAimDirection(gameScene.shootDirection_);
}

void GameSceneUpdateExecutor::UIUpdate(GameScene& gameScene) {
	gameScene.uiManager_.SetHomingCooldownRate(gameScene.bulletManager_.GetHomingCooldownRate());
	gameScene.uiManager_.SetHomingLockInfo(gameScene.bulletManager_.GetCurrentLockCount(), gameScene.bulletManager_.GetMaxLockCount(), gameScene.bulletManager_.IsHomingLocking());
	gameScene.uiManager_.Update();
}

void GameSceneUpdateExecutor::UpdateLockOnMarkers(GameScene& gameScene) {
	gameScene.lockOnMarkers_.clear();

	if (gameScene.result_ != GameResult::None || gameScene.state_ != GameState::Playing) {
		return;
	}

	const Camera* cam = gameScene.railCamera_ ? gameScene.railCamera_->GetCamera() : &gameScene.camera_;
	if (!cam || gameScene.lockOnTexHandle_ == 0u) {
		return;
	}

	const Matrix4x4 viewProj = MyMath::Multiply(cam->matView, cam->matProjection);
	const auto& targets = gameScene.bulletManager_.GetLockedTargets();
	for (CharacterBase* target : targets) {
		if (!target || target->IsDead()) {
			continue;
		}

		const Vector3 clip = MyMath::Transform(target->GetWorldTranslation(), viewProj);
		if (clip.z < 0.0f || clip.z > 1.0f) {
			continue;
		}

		const Vector2 screenPos = {(clip.x * 0.5f + 0.5f) * gameScene.kScreenWidth_, (-clip.y * 0.5f + 0.5f) * gameScene.kScreenHeight_};
		if (screenPos.x < 0.0f || screenPos.x > gameScene.kScreenWidth_ || screenPos.y < 0.0f || screenPos.y > gameScene.kScreenHeight_) {
			continue;
		}

		auto sprite = std::unique_ptr<Sprite>(Sprite::Create(gameScene.lockOnTexHandle_, screenPos));
		if (!sprite) {
			continue;
		}

		sprite->SetAnchorPoint({0.5f, 0.5f});
		const float pulse = std::sin(gameScene.smokeEmitTimer_ * 18.0f) * 0.15f + 1.0f;
		sprite->SetSize({64.0f * pulse, 64.0f * pulse});
		sprite->SetColor({1.0f, 0.25f, 0.25f, 0.95f});

		gameScene.lockOnMarkers_.push_back({std::move(sprite), target, 0.0f});
	}
}

void GameSceneUpdateExecutor::DamageGpuParticlesUpdate(GameScene& gameScene, float dt) {
	if (gameScene.damageSmokeEmitter_) {
		gameScene.damageSmokeEmitter_->Update(dt);
	}

	const int32_t currentHp = gameScene.player_ ? gameScene.player_->GetHP() : gameScene.prevPlayerHp_;
	gameScene.prevPlayerHp_ = currentHp;

	auto& enemies = gameScene.enemyManager_.GetEnemies();
	for (auto it = gameScene.prevEnemyHpMap_.begin(); it != gameScene.prevEnemyHpMap_.end();) {
		const bool exists = std::any_of(enemies.begin(), enemies.end(), [it](const std::unique_ptr<CharacterBase>& e) { return e && e.get() == it->first; });
		if (!exists) {
			it = gameScene.prevEnemyHpMap_.erase(it);
		} else {
			++it;
		}
	}

	for (auto& popup : gameScene.scorePopups_) {
		popup.life -= dt;
		popup.worldPos.y += popup.riseSpeed * dt;
	}
	gameScene.scorePopups_.erase(
	    std::remove_if(gameScene.scorePopups_.begin(), gameScene.scorePopups_.end(), [](const GameScene::ScorePopup& p) { return p.life <= 0.0f; }), gameScene.scorePopups_.end());
}

void GameSceneUpdateExecutor::EngineSmokesUpdate(GameScene& gameScene, float dt) {
	if (gameScene.result_ == GameResult::None && gameScene.countDown_.IsInputLocked()) {
		return;
	}

	const bool canEmit = (gameScene.result_ == GameResult::None) || (gameScene.result_ == GameResult::Clear && gameScene.isClearAnimating_);
	if (!canEmit) {
		return;
	}

	gameScene.smokeEmitTimer_ += dt;
	const GameScene::SmokeParams& params = (gameScene.result_ == GameResult::Clear && gameScene.isClearAnimating_) ? gameScene.boostSmokeParams_ : gameScene.normalSmokeParams_;

	static std::mt19937 rng{std::random_device{}()};
	std::uniform_real_distribution<float> xyDist(-gameScene.kSmokeRandXY_, gameScene.kSmokeRandXY_);
	std::uniform_real_distribution<float> zDist(-gameScene.kSmokeRandZ_, gameScene.kSmokeRandZ_);

	while (gameScene.smokeEmitTimer_ >= params.emitInterval) {
		gameScene.smokeEmitTimer_ -= params.emitInterval;

		const Vector3 playerPos = gameScene.player_->GetWorldTranslation();
		const auto& playerWt = gameScene.player_->GetWorldTransform();
		const Matrix4x4 playerRotateMatrix = MyMath::MakeAffineMatrix({1.0f, 1.0f, 1.0f}, playerWt.rotation_, {0.0f, 0.0f, 0.0f});

		const Vector3 localOffset = {0.0f, gameScene.kSmokeOffsetY_, gameScene.kSmokeOffsetZ_};
		const Vector3 offset = MyMath::TransformNormal(localOffset, playerRotateMatrix);
		const Vector3 spawnPos = MyMath::Add(playerPos, offset);

		for (int i = 0; i < params.burstCount; ++i) {
			const Vector3 localVel = {xyDist(rng), xyDist(rng), params.baseZSpeed + zDist(rng)};
			const Vector3 vel = MyMath::TransformNormal(localVel, playerRotateMatrix);

			if (gameScene.engineSmokeEmitter_) {
				const bool isBoosting = gameScene.result_ == GameResult::Clear && gameScene.isClearAnimating_;
				const Vector4 startColor = isBoosting ? kBoostCoreColor : kEngineCoreColor;
				const Vector4 endColor = isBoosting ? kBoostOuterColor : kEngineFadeColor;
				const float endScale = isBoosting ? params.startScale * 0.25f : params.startScale * 0.10f;
				gameScene.engineSmokeEmitter_->Emit(spawnPos, vel, params.lifeTime, params.startScale, endScale, startColor, endColor);
			}
		}
	}

	if (gameScene.engineSmokeEmitter_) {
		gameScene.engineSmokeEmitter_->Update(dt);
	}
}

void GameSceneUpdateExecutor::MissileAfterburnerUpdate(GameScene& gameScene, float dt) {
	if (!gameScene.missileAfterburnerEmitter_) {
		return;
	}

	static std::mt19937 rng{std::random_device{}()};
	std::uniform_real_distribution<float> randDist(-gameScene.kMissileAfterburnerRand_, gameScene.kMissileAfterburnerRand_);

	for (const auto& missile : gameScene.bulletManager_.GetHomingMissiles()) {
		if (!missile || missile->IsDead()) {
			continue;
		}

		Vector3 vel = missile->GetVelocity();
		const float lenSq = vel.x * vel.x + vel.y * vel.y + vel.z * vel.z;
		if (lenSq < 0.000001f) {
			vel = {0.0f, 0.0f, 1.0f};
		} else {
			vel = MyMath::Normalize(vel);
		}

		const Vector3 pos = missile->GetWorldTranslation();
		const Vector3 spawnPos = MyMath::Add(pos, MyMath::Multiply(vel, gameScene.kMissileAfterburnerOffsetZ_));
		Vector3 emitVel = MyMath::Multiply(vel, gameScene.kMissileAfterburnerSpeed_);
		emitVel.x += randDist(rng);
		emitVel.y += randDist(rng);
		emitVel.z += randDist(rng);
		gameScene.missileAfterburnerEmitter_->Emit(
		    spawnPos, emitVel, gameScene.kMissileAfterburnerLife_, gameScene.kMissileAfterburnerStartScale_, gameScene.kMissileAfterburnerEndScale_, kAfterburnerCoreColor, kAfterburnerOuterColor);

		Vector3 emberVel = emitVel;
		emberVel.x += randDist(rng) * 0.8f;
		emberVel.y += randDist(rng) * 0.8f;
		emberVel.z += randDist(rng) * 0.8f;
		gameScene.missileAfterburnerEmitter_->Emit(
		    spawnPos, emberVel, gameScene.kMissileAfterburnerLife_ * 0.65f, gameScene.kMissileAfterburnerStartScale_ * 0.6f, 0.0f, kBoostOuterColor, kExplosionFireColor);
	}

	gameScene.missileAfterburnerEmitter_->Update(dt);
}

void GameSceneUpdateExecutor::CameraUpdate(GameScene& gameScene) {
	if (gameScene.isRailCameraActive_) {
		Vector3 now = gameScene.player_->GetWorldTranslation();
		float deltaX = now.x - gameScene.previousPlayerPos_.x;
		float inputX = MyMath::Clamp(deltaX * 40.0f, -1.0f, 1.0f);

		gameScene.railCamera_->SetMoveInput(inputX);
		gameScene.railCamera_->Update();

		gameScene.camera_.matView = gameScene.railCamera_->GetCamera()->matView;
		gameScene.camera_.matProjection = gameScene.railCamera_->GetCamera()->matProjection;
		gameScene.camera_.TransferMatrix();

		gameScene.previousPlayerPos_ = now;
	} else {
		gameScene.camera_.UpdateMatrix();
	}
}

void GameSceneUpdateExecutor::SpeedLineUpdate(GameScene& gameScene, float dt) {
	if (gameScene.isRailCameraActive_) {
		gameScene.speedLine_.Update(dt, gameScene.railCamera_->GetWorldTransform().translation_);
	}
}

void GameSceneUpdateExecutor::JudgeResultAndStartClear(GameScene& gameScene) {
	if (gameScene.result_ == GameResult::None) {
		if (gameScene.player_->IsExplosionFinished()) {
			gameScene.result_ = GameResult::Fail;
			gameScene.transitionPhase_ = SceneTransitionPhase::FailCinematic;
			gameScene.transitionTimer_ = 0.0f;
			gameScene.timeScale_ = 0.35f;
			StartExplosionAtPlayer(gameScene, 2.3f);
		} else if (gameScene.uiManager_.GetScore()->GetScore() >= gameScene.kClearScore_) {
			gameScene.result_ = GameResult::Clear;
			gameScene.clearScore_ = gameScene.uiManager_.GetScore()->GetScore();
			gameScene.isClearAnimating_ = true;
			gameScene.clearAnimTimer_ = 0.0f;
			gameScene.transitionPhase_ = SceneTransitionPhase::ClearCinematic;
			gameScene.transitionTimer_ = 0.0f;
			gameScene.timeScale_ = 0.45f;
			gameScene.transitionScoreBonus_ = 300;
			gameScene.uiManager_.GetScore()->Add(gameScene.transitionScoreBonus_);
		}
	}
}

void GameSceneUpdateExecutor::ClearAnimationUpdate(GameScene& gameScene, float dt) {
	if (gameScene.result_ == GameResult::Clear && gameScene.isClearAnimating_) {
		gameScene.clearAnimTimer_ += dt;
		auto& wt = gameScene.player_->GetWorldTransform();

		wt.translation_.z += gameScene.kClearBoostSpeedZ_ * dt;
		wt.translation_.y += gameScene.kClearBoostSpeedY_ * dt;
		wt.rotation_.x -= gameScene.kClearRotateSpeedX_ * dt;

		if (gameScene.clearAnimTimer_ > gameScene.kClearShrinkStart_) {
			float shrink = gameScene.clearAnimTimer_ - gameScene.kClearShrinkStart_;
			float scale = (std::max)(0.0f, 1.0f - shrink * gameScene.kClearShrinkSpeed_);
			wt.scale_ = {scale, scale, scale};
		}

		wt.UpdateMatrix();

		if (gameScene.clearAnimTimer_ >= gameScene.kClearAnimEndTime_) {
			gameScene.isEnd_ = true;
			gameScene.isClearAnimating_ = false;
		}
	}
}

void GameSceneUpdateExecutor::UpdateTransitionDirection(GameScene& gameScene, float dt) {
	if (gameScene.transitionPhase_ == SceneTransitionPhase::None) {
		return;
	}

	gameScene.transitionTimer_ += dt;

	if (gameScene.transitionPhase_ == SceneTransitionPhase::IntroCinematic) {
		gameScene.railCamera_->SetCinematicZoom(-10.0f);
		if (gameScene.transitionTimer_ >= 0.8f) {
			gameScene.transitionPhase_ = SceneTransitionPhase::None;
			gameScene.timeScale_ = 1.0f;
			gameScene.railCamera_->SetCinematicZoom(0.0f);
		}
	} else if (gameScene.transitionPhase_ == SceneTransitionPhase::ClearCinematic) {
		gameScene.railCamera_->SetCinematicZoom(-18.0f);
		if (gameScene.transitionTimer_ >= 0.55f) {
			gameScene.timeScale_ = 1.0f;
		}
	} else if (gameScene.transitionPhase_ == SceneTransitionPhase::FailCinematic) {
		gameScene.railCamera_->SetCinematicZoom(-24.0f);
		if (gameScene.transitionTimer_ >= 0.40f && !gameScene.failSecondExplosionDone_) {
			StartExplosionAtPlayer(gameScene, 1.4f);
			gameScene.failSecondExplosionDone_ = true;
		}
		if (gameScene.transitionTimer_ >= 1.2f) {
			gameScene.isEnd_ = true;
			gameScene.timeScale_ = 1.0f;
		}
	}
}

void GameSceneUpdateExecutor::StartExplosionAtPlayer(GameScene& gameScene, float scale) {
	if (gameScene.audio_ && gameScene.seExplosionHandle_ != 0u) {
		gameScene.audio_->PlayWave(gameScene.seExplosionHandle_, false, 0.7f);
	}

	Vector3 pos = gameScene.player_->GetWorldTranslation();
	static std::mt19937 rng{(std::random_device{}())};
	std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

	const int count = static_cast<int>(gameScene.kDamageGpuBurst_ * scale);
	for (int i = 0; i < count; i++) {
		if (gameScene.damageSmokeEmitter_) {
			Vector3 vel = {dist(rng) * (gameScene.kDamageGpuSpeed_ * scale), dist(rng) * (gameScene.kDamageGpuSpeed_ * scale), dist(rng) * (gameScene.kDamageGpuSpeed_ * scale)};
			gameScene.damageSmokeEmitter_->Emit(
			    pos, vel, gameScene.kDamageGpuLife_ * 1.3f, gameScene.kDamageGpuStartScale_ * scale, gameScene.kDamageGpuEndScale_ * scale, kExplosionHotColor, kExplosionFireColor);
		}
	}
	for (int i = 0; i < count / 2; i++) {
		if (gameScene.damageSmokeEmitter_) {
			Vector3 vel = {
			    dist(rng) * (gameScene.kDamageGpuSpeed_ * scale * 0.45f), dist(rng) * (gameScene.kDamageGpuSpeed_ * scale * 0.45f), dist(rng) * (gameScene.kDamageGpuSpeed_ * scale * 0.45f)};
			gameScene.damageSmokeEmitter_->Emit(
			    pos, vel, gameScene.kDamageGpuLife_ * 3.2f, gameScene.kDamageGpuStartScale_ * scale * 1.2f, gameScene.kDamageGpuEndScale_ * scale * 2.4f, kSmokeStartColor, kSmokeEndColor);
		}
	}
	if (gameScene.railCamera_) {
		gameScene.railCamera_->AddShake({0.5f, 0.3f, 0.0f}, scale * 2.0f);
	}
}
