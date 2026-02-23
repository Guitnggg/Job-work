#pragma once

#include <KamataEngine.h>
#include <memory>
#include <vector>

#include "Application/Characters/Player/Bullet.h"
#include "Application/Characters/Player/Player.h"
#include "Application/Characters/Player/Laser.h"
#include "Application/Characters/Player/HomingMissile.h"
#include "UI/CountDown/CountDown.h"

class EnemyManager;

/// <summary>
/// 弾の生成・更新・破棄を管理するクラス。
/// ・プレイヤー入力を検知して弾を発射
/// ・飛行中の弾の更新
/// ・死亡（寿命/衝突/距離超過）した弾の後始末
/// を一括で担当する。
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
	/// 入力判定→弾の発射→弾の更新→死亡弾の削除
	/// </summary>
	/// <param name="input">入力処理を参照する為のポインタ</param>
	/// <param name="player">弾の発射位置取得に使うプレイヤー</param>
	/// <param name="countDown">カウントダウン中は発射操作を無効にする為の参照</param>
	void Update(KamataEngine::Input* input, Player* player, const CountDown& countDown, const KamataEngine::Vector3& shootDir, EnemyManager* enemyManager);

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="camera">描画に使用するカメラ</param>
	void Draw(const KamataEngine::Camera* camera);

	/// <summary>
	/// 現在飛行中の弾のコンテナを参照で返す。
	/// 外部で弾との当たり判定を行う用途で使用する。
	/// </summary>
	/// <returns>弾のコンテナ(std::vector&lt;unique_ptr&lt;Bullet&gt;&gt;)</returns>
	std::vector<std::unique_ptr<Bullet>>& GetBullets() { return bullets_; }

	/// <summary>
	/// 現在飛行中のレーザー(Laser)のコンテナを参照で返す。
	/// 外部でレーザーとの当たり判定を行う用途で使用する。
	/// </summary>
	std::vector<std::unique_ptr<Laser>>& GetLasers() { return lasers_; }

	/// <summary>
	/// 
	/// </summary>
	std::vector<std::unique_ptr<HomingMissile>>& GetHomingMissiles() { return homingMissiles_; }


	float GetHomingCooldownRate() const;
	int32_t GetCurrentLockCount() const { return static_cast<int32_t>(lockedTargets_.size()); }
	int32_t GetMaxLockCount() const { return kHomingMaxLockCount; }
	bool IsHomingLocking() const { return isHomingLocking_; }
	const std::vector<CharacterBase*>& GetLockedTargets() const { return lockedTargets_; }

private:
	/// <summary>
	/// プレイヤー入力に応じて弾発射処理を行う。
	/// クールダウン制御もここで行う。
	/// </summary>
	void HandleShooting_(KamataEngine::Input* input, Player* player, const CountDown& countDown, const KamataEngine::Vector3& shootDir, EnemyManager* enemyManager);

	/// <summary>
	/// 
	/// </summary>
	void HandleHomingMissile_(KamataEngine::Input* input, Player* player, const CountDown& countDown, EnemyManager* enemyManager);

	/// <summary>
	/// 全飛行弾の更新処理を行う。
	/// </summary>
	void UpdateBullets_();

	/// <summary>
	/// 死亡状態（寿命切れ／衝突／距離制限）の弾をコンテナから削除する。
	/// </summary>
	void RemoveDeadBullets_();

private:
	std::vector<std::unique_ptr<Bullet>> bullets_;
	std::vector<std::unique_ptr<Laser>> lasers_;
	std::vector<std::unique_ptr<HomingMissile>> homingMissiles_;

	// クールダウン
	int32_t fireCooldownFrames_ = 0;
	static constexpr int32_t kFireCooldownMax = 9;   // 通常弾：約0.15秒@60fps

	bool isHomingLocking_ = false;
	int32_t homingLockFrame_ = 0;
	int32_t homingCooldownFrames_ = 0;
	std::vector<CharacterBase*> lockedTargets_;
	bool wasHomingPressing_ = false;

	static constexpr int32_t kHomingLockStartFrame = 30; // 0.5s
	static constexpr int32_t kHomingLockMaxFrame = 60;   // 1.0s
	static constexpr int32_t kHomingMaxLockCount = 5;
	static constexpr int32_t kHomingCooldownMaxFrame = 600; // 10s
	
};
