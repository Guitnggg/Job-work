#pragma once

class GameScene;

/// <summary>
/// 
/// </summary>
class GameSceneUpdateExecutor{
public:
    /// <summary>
    /// 
    /// </summary>
    /// <param name="gameScene"></param>
    static void Update(GameScene& gameScene);

private:
	static void CountDownUpdate(GameScene& gameScene, float dt);
	static void BackgroundUpdate(GameScene& gameScene);
	static void PlayerUpdate(GameScene& gameScene);
	static void SpawnDamageParticles(GameScene& gameScene);
	static void BattleUpdate(GameScene& gameScene, float dt);
	static void UpdateAimAndReticle(GameScene& gameScene);
	static void UIUpdate(GameScene& gameScene);
	static void UpdateLockOnMakers(GameScene& gameScene);
	static void DamageGpuParticlesUpdate(GameScene& gameScene, float dt);
	static void EngineSmokesUpdate(GameScene& gameScene, float dt);
	static void MissileAfterburnerUpdate(GameScene& gameScene, float dt);
	static void CameraUpdate(GameScene& gameScene);
	static void SpeedLineUpdate(GameScene& gameScene, float dt);
	static void JudgeResultAndStartClear(GameScene& gameScene);
	static void ClearAnimationUpdate(GameScene& gameScene, float dt);
	static void UpdateTransitionDirection(GameScene& gameScene, float dt);
	static void StartExplosionAtPlayer(GameScene& gameScene, float scale);
};

