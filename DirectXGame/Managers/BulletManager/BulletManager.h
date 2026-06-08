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
	/// 蛻晄悄蛹門・逅・
	/// </summary>
	void Initialize();

	/// <summary>
	/// 譖ｴ譁ｰ蜃ｦ逅・
	/// 蜈･蜉帛愛螳壺・蠑ｾ縺ｮ逋ｺ蟆・・蠑ｾ縺ｮ譖ｴ譁ｰ竊呈ｭｻ莠｡蠑ｾ縺ｮ蜑企勁
	/// </summary>
	/// <param name="input">蜈･蜉帛・逅・ｒ蜿ら・縺吶ｋ轤ｺ縺ｮ繝昴う繝ｳ繧ｿ</param>
	/// <param name="player">蠑ｾ縺ｮ逋ｺ蟆・ｽ咲ｽｮ蜿門ｾ励↓菴ｿ縺・・繝ｬ繧､繝､繝ｼ</param>
	/// <param name="countDown">繧ｫ繧ｦ繝ｳ繝医ム繧ｦ繝ｳ荳ｭ縺ｯ逋ｺ蟆・桃菴懊ｒ辟｡蜉ｹ縺ｫ縺吶ｋ轤ｺ縺ｮ蜿ら・</param>
	void Update(KamataEngine::Input* input, Player* player, const CountDown& countDown, const KamataEngine::Vector3& shootDir, EnemyManager* enemyManager);

	/// <summary>
	/// 謠冗判蜃ｦ逅・
	/// </summary>
	/// <param name="camera">謠冗判縺ｫ菴ｿ逕ｨ縺吶ｋ繧ｫ繝｡繝ｩ</param>
	void Draw(const KamataEngine::Camera* camera);

	/// <summary>
	/// 迴ｾ蝨ｨ鬟幄｡御ｸｭ縺ｮ蠑ｾ縺ｮ繧ｳ繝ｳ繝・リ繧貞盾辣ｧ縺ｧ霑斐☆縲・
	/// 螟夜Κ縺ｧ蠑ｾ縺ｨ縺ｮ蠖薙◆繧雁愛螳壹ｒ陦後≧逕ｨ騾斐〒菴ｿ逕ｨ縺吶ｋ縲・
	/// </summary>
	/// <returns>蠑ｾ縺ｮ繧ｳ繝ｳ繝・リ(std::vector&lt;unique_ptr&lt;Bullet&gt;&gt;)</returns>
	std::vector<std::unique_ptr<Bullet>>& GetBullets() { return bullets_; }

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
	/// 繝励Ξ繧､繝､繝ｼ蜈･蜉帙↓蠢懊§縺ｦ蠑ｾ逋ｺ蟆・・逅・ｒ陦後≧縲・
	/// 繧ｯ繝ｼ繝ｫ繝繧ｦ繝ｳ蛻ｶ蠕｡繧ゅ％縺薙〒陦後≧縲・
	/// </summary>
	void HandleShooting_(KamataEngine::Input* input, Player* player, const CountDown& countDown, const KamataEngine::Vector3& shootDir, EnemyManager* enemyManager);

	/// <summary>
	///
	/// </summary>
	void HandleHomingMissile_(KamataEngine::Input* input, Player* player, const CountDown& countDown, EnemyManager* enemyManager);

	/// <summary>
	/// 蜈ｨ鬟幄｡悟ｼｾ縺ｮ譖ｴ譁ｰ蜃ｦ逅・ｒ陦後≧縲・
	/// </summary>
	void UpdateBullets_(EnemyManager* enemyManager);

	/// <summary>
	/// 繝帙・繝溘Φ繧ｰ蟇ｾ雎｡縺梧里縺ｫ蜑企勁貂医∩縺ｮ繝溘し繧､繝ｫ縺九ｉ繧ｿ繝ｼ繧ｲ繝・ヨ蜿ら・繧貞､悶☆
	/// </summary>
	void ValidateHomingTargets_(EnemyManager* enemyManager);

	/// <summary>
	/// 豁ｻ莠｡迥ｶ諷具ｼ亥ｯｿ蜻ｽ蛻・ｌ・剰｡晉ｪ・ｼ剰ｷ晞屬蛻ｶ髯撰ｼ峨・蠑ｾ繧偵さ繝ｳ繝・リ縺九ｉ蜑企勁縺吶ｋ縲・
	/// </summary>
	void RemoveDeadBullets_();

	/// <summary>
	/// Object Pool Pattern: returns an inactive bullet from the pool, or grows the pool.
	/// </summary>
	Bullet* AcquireBullet_();

private:
	std::vector<std::unique_ptr<Bullet>> bullets_;
	std::vector<std::unique_ptr<HomingMissile>> homingMissiles_;

	// 繧ｯ繝ｼ繝ｫ繝繧ｦ繝ｳ
	int32_t fireCooldownFrames_ = 0;
	int32_t burstShotsRemaining_ = 0;
	static constexpr int32_t kBurstShotCount = 3;
	static constexpr int32_t kBurstIntervalFrames = 3;  // 繝舌・繧ｹ繝亥・縺ｮ逋ｺ蟆・俣髫・邏・.05遘叩60fps)
	static constexpr int32_t kBurstCooldownFrames = 18; // 繝舌・繧ｹ繝亥ｾ後・蠕・ｩ滓凾髢・邏・.3遘叩60fps)
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
