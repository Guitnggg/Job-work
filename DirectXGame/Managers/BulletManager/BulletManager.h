#pragma once

#include <KamataEngine.h>
#include <memory>
#include <vector>

#include "Application/Characters/Player/Bullet.h"
#include "Application/Characters/Player/HomingMissile.h"
#include "Application/Characters/Player/Player.h"
#include "UI/CountDown/CountDown.h"

class EnemyManager;

/// <summary>
/// プレイヤーの通常弾、ホーミングミサイル、ロックオン状態を管理する。
/// </summary>
class BulletManager {
public:
	BulletManager() = default;
	~BulletManager() = default;

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="input">入力情報</param>
	/// <param name="player">プレイヤー情報</param>
	/// <param name="countDown">カウントダウン情報</param>
	void Update(KamataEngine::Input* input, Player* player, const CountDown& countDown, const KamataEngine::Vector3& shootDir, EnemyManager* enemyManager);

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="camera">描画に使用するカメラ</param>
	void Draw(const KamataEngine::Camera* camera);

	/// <summary>
	/// 現在管理している通常団のコンテナを参照で返す
	/// CollisionManagerなど、外部で当たり判定を行う用途で使用する
	/// </summary>
	/// <returns>通常弾のコンテナ</returns>
	std::vector<std::unique_ptr<Bullet>>& GetBullets() { return bullets_; }

	/// <summary>
	/// 現在管理しているホーミングミサイルのコンテナを参照で返す
	/// CollisionManagerなど、外部で当たり判定を行う用途で使用する
	/// </summary>
	/// <returns>ホーミングミサイルのコンテナ</returns>
	std::vector<std::unique_ptr<HomingMissile>>& GetHomingMissiles() { return homingMissiles_; }

	/// <summary>
	/// ホーミングロックのクールダウン率（0.0～1.0）を返す
	/// </summary>
	float GetHomingCooldownRate() const;

	/// <summary>
	/// 現在ロックしている敵の数を返す
	/// </summary>
	/// <returns>現在ロックしている敵の数</returns>
	int32_t GetCurrentLockCount() const { return static_cast<int32_t>(lockedTargets_.size()); }

	/// <summary>
	/// 最大ロック可能な敵の数を返す
	/// </summary>
	/// <returns>最大ロック可能な敵の数</returns>
	int32_t GetMaxLockCount() const { return kHomingMaxLockCount; }

	/// <summary>
	/// ホーミングロック中かどうかを返す
	/// </summary>
	/// <returns>ホーミングロック中かどうか</returns>
	bool IsHomingLocking() const { return isHomingLocking_; }

	/// <summary>
	/// 現在ロックしている敵のリストを返す
	/// </summary>
	/// <returns>現在ロックしている敵のリスト</returns>
	const std::vector<CharacterBase*>& GetLockedTargets() const { return lockedTargets_; }

private:
	/// <summary>
	/// プレイヤーの入力に応じて通常弾を発射する処理 
	/// </summary>
	void HandleShooting_(KamataEngine::Input* input, Player* player, const CountDown& countDown, const KamataEngine::Vector3& shootDir, EnemyManager* enemyManager);

	/// <summary>
	/// プレイヤーの入力に応じてホーミングミサイルを発射する処理
	/// </summary>
	void HandleHomingMissile_(KamataEngine::Input* input, Player* player, const CountDown& countDown, EnemyManager* enemyManager);

	/// <summary>
	/// 通常弾の位置更新と寿命管理
	/// </summary>
	void UpdateBullets_(EnemyManager* enemyManager);

	/// <summary>
	/// ホーミングミサイルの位置更新と寿命管理
	/// </summary>
	void ValidateHomingTargets_(EnemyManager* enemyManager);

	/// <summary>
	/// 寿命が尽きた通常弾をリストから削除する処理
	/// </summary>
	void RemoveDeadBullets_();

	/// <summary>
	/// 寿命が尽きたホーミングミサイルをリストから削除する処理
	/// </summary>
	Bullet* AcquireBullet_();

private:
	std::vector<std::unique_ptr<Bullet>> bullets_;
	std::vector<std::unique_ptr<HomingMissile>> homingMissiles_;

	// 通常弾の発射管理
	int32_t fireCooldownFrames_ = 0;
	int32_t burstShotsRemaining_ = 0;
	static constexpr int32_t kBurstShotCount = 3;
	static constexpr int32_t kBurstIntervalFrames = 3;  // バースト内の連射間隔
	static constexpr int32_t kBurstCooldownFrames = 18; // バースト終了後のクールダウン
	static constexpr int32_t kInitialBulletPoolSize = 64;

	bool isHomingLocking_ = false;
	int32_t homingLockFrame_ = 0;
	int32_t homingCooldownFrames_ = 0;
	std::vector<CharacterBase*> lockedTargets_;
	bool wasHomingPressing_ = false;

	static constexpr int32_t kHomingLockStartFrame = 30; // 0.5s
	static constexpr int32_t kHomingLockMaxFrame = 60;   // 1.0s
	static constexpr int32_t kHomingMaxLockCount = 5;
	static constexpr int32_t kHomingCooldownMaxFrame = 600; // 10s

	KamataEngine::Audio* audio_ = nullptr;
	uint32_t shotSeHandle_ = 0;
	uint32_t missileSeHandle_ = 0;
};
