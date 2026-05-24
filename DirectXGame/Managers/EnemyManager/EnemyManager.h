#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include <KamataEngine.h>

#include "Application/Characters/Enemy/SeekerEnemy.h"
#include "Application/Characters/Enemy/TurretEnemy.h"

/// <summary>
/// JSON の出現データを読み込み、敵の生成、更新、描画、破棄を管理する。
/// </summary>
class EnemyManager {
public:
	/// <summary>
	/// 謨ｵ蜃ｺ迴ｾ繝・・繧ｿ・・SON 1莉ｶ蛻・ｼ・
	/// </summary>
	struct EnemySpawnData {
		float time = 0.0f;
		KamataEngine::Vector3 pos{0.0f, 0.0f, 0.0f};

		// 謨ｵ繧ｿ繧､繝暦ｼ・seeker" / "turret"・・
		std::string type = "seeker";

		// ===== Seeker 逕ｨ =====
		float speed = kDefaultSeekerSpeed;
		float turnRate = kDefaultSeekerTurnRate;

		// ===== 蜈ｱ騾・=====
		int32_t hp = kDefaultHp;
		float radius = kDefaultColliderRadius;
		float lifeTime = kDefaultLifeTime;

		// ===== Turret 逕ｨ =====
		int32_t shootIntervalFrames = kDefaultShootIntervalFrames;
		float bulletSpeed = kDefaultBulletSpeed;
		float bulletLifeTime = kDefaultBulletLifeTime;
	};

public:
	/// <summary>蛻晄悄蛹門・逅・/summary>
	void Initialize();

	/// <summary>
	/// JSON 蠖｢蠑上・謨ｵ蜃ｺ迴ｾ繝・・繧ｿ隱ｭ縺ｿ霎ｼ縺ｿ
	/// </summary>
	void LoadEnemyCsv(const std::string& path);

	/// <summary>
	/// 譖ｴ譁ｰ蜃ｦ逅・
	/// </summary>
	/// <param name="dt">繝・Ν繧ｿ繧ｿ繧､繝・育ｧ抵ｼ・/param>
	/// <param name="playerPos">繝励Ξ繧､繝､繝ｼ縺ｮ繝ｯ繝ｼ繝ｫ繝牙ｺｧ讓・/param>
	void Update(float dt, const KamataEngine::Vector3& playerPos);

	/// <summary>
	/// 謠冗判蜃ｦ逅・
	/// </summary>
	void Draw(const KamataEngine::Camera* camera);

	/// <summary>
	/// 逕溷ｭ倅ｸｭ縺ｮ謨ｵ繝ｪ繧ｹ繝亥叙蠕暦ｼ・ollisionManager 逕ｨ・・
	/// </summary>
	std::vector<std::unique_ptr<CharacterBase>>& GetEnemies() { return enemies_; }

	/// <summary>
	/// 繝励Ξ繧､繝､繝ｼ縺九ｉ霑代＞鬆・・謨ｵ繝昴う繝ｳ繧ｿ繧呈怙螟ｧ謨ｰ縺ｾ縺ｧ蜿門ｾ励☆繧・
	/// </summary>
	std::vector<CharacterBase*> GetNearestEnemies(const KamataEngine::Vector3& from, int32_t maxCount) const;

	/// <summary>
	/// 豁ｻ莠｡縺励◆謨ｵ繧貞炎髯､縺吶ｋ
	/// </summary>
	void RemoveDeadEnemies();

private:
	/// <summary>
	/// 蜃ｺ迴ｾ譎る俣縺ｫ驕斐＠縺滓雰繧堤函謌舌☆繧・
	/// </summary>
	void SpawnEnemiesByCsv(const KamataEngine::Vector3& playerPos);

private:
	// ===== 螳壽焚・医ョ繝輔か繝ｫ繝亥､・・====
	static constexpr float kDefaultSeekerSpeed = 0.2f;
	static constexpr float kDefaultSeekerTurnRate = 0.15f;
	static constexpr int32_t kDefaultHp = 1;
	static constexpr float kDefaultColliderRadius = 1.0f;
	static constexpr float kDefaultLifeTime = 30.0f;

	static constexpr int32_t kDefaultShootIntervalFrames = 60;
	static constexpr float kDefaultBulletSpeed = 2.8f;
	static constexpr float kDefaultBulletLifeTime = 3.0f;

private:
	// 逕溷ｭ倅ｸｭ縺ｮ謨ｵ
	std::vector<std::unique_ptr<CharacterBase>> enemies_;

	// 蜃ｺ迴ｾ蠕・■謨ｵ繝・・繧ｿ
	std::vector<EnemySpawnData> enemySpawnList_;

	// 邨碁℃譎る俣
	float enemySpawnTimer_ = 0.0f;
};
