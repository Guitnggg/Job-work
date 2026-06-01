#pragma once

class GameScene;

/// <summary>
/// GameScene の 1 フレーム更新を担当する実行クラス。
/// GameScene 本体はライフサイクル管理と描画に集中し、更新の詳細を本クラスに
/// </summary>
class GameSceneUpdateExecutor {
public:
	/// <summary>
	/// GameSceneの更新を1フレーム分実行する
	/// </summary>
	static void Update(GameScene& gameScene);

	/// <summary>
	/// Command Pattern support: pause menu command actions.
	/// </summary>
	static void ExecuteResumeCommand(GameScene& gameScene);
	static void ExecuteRetryCommand(GameScene& gameScene);
	static void ExecuteToTitleCommand(GameScene& gameScene);

private:
	/// <summary>
	/// カウントダウンの更新処理
	/// </summary>
	static void CountDownUpdate(GameScene& gameScene, float dt);

	/// <summary>
	/// 背景（天球）の更新処理
	/// </summary>
	static void BackgroundUpdate(GameScene& gameScene);

	/// <summary>
	/// プレイヤー本体の更新処理
	/// </summary>
	static void PlayerUpdate(GameScene& gameScene);

	/// <summary>
	/// 被弾時のダメージパーティクル発生処理
	/// </summary>
	static void SpawnDamageParticles(GameScene& gameScene);

	/// <summary>
	/// 弾・敵・当たり判定・スコア加算などの戦闘更新処理
	/// </summary>
	static void BattleUpdate(GameScene& gameScene, float dt);

	/// <summary>
	/// 照準位置と射撃方向の更新処理
	/// </summary>
	static void UpdateAimAndReticle(GameScene& gameScene);

	/// <summary>
	/// UIの更新処理
	/// </summary>
	static void UIUpdate(GameScene& gameScene);

	/// <summary>
	/// ロックオンマーカーの更新処理
	/// </summary>
	static void UpdateLockOnMarkers(GameScene& gameScene);

	/// <summary>
	/// ダメージパーティクルの更新処理
	/// </summary>
	static void DamageGpuParticlesUpdate(GameScene& gameScene, float dt);

	/// <summary>
	/// エンジンスモークの更新処理
	/// </summary>
	static void EngineSmokesUpdate(GameScene& gameScene, float dt);

	/// <summary>
	/// ミサイルのアフターバーナーの更新処理
	/// </summary>
	static void MissileAfterburnerUpdate(GameScene& gameScene, float dt);

	/// <summary>
	/// カメラの更新処理
	/// </summary>
	static void CameraUpdate(GameScene& gameScene);

	/// <summary>
	/// スピードライン演出の更新処理
	/// </summary>
	static void SpeedLineUpdate(GameScene& gameScene, float dt);

	/// <summary>
	/// クリア/失敗の判定処理と演出を開始するかの判断処理
	/// </summary>
	static void JudgeResultAndStartClear(GameScene& gameScene);

	/// <summary>
	/// クリア演出アニメーションの更新処理
	/// </summary>
	static void ClearAnimationUpdate(GameScene& gameScene, float dt);

	/// <summary>
	/// 各種シーン遷移演出の更新処理
	/// </summary>
	static void UpdateTransitionDirection(GameScene& gameScene, float dt);

	/// <summary>
	/// プレイヤーの爆発演出更新処理
	/// </summary>
	static void StartExplosionAtPlayer(GameScene& gameScene, float scale);
};