#pragma once

#include <KamataEngine.h>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "Application/Characters/Enemy/SeekerEnemy.h"
#include "Application/Characters/Enemy/TurretEnemy.h" // ★追加
#include "Application/Effects/Damage/DamageParticle.h"

class EnemyManager {
public:
	struct EnemySpawnData {
		float time = 0.0f;
		KamataEngine::Vector3 pos{0, 0, 0};

		// ★追加：敵タイプ（"seeker" / "turret"）
		std::string type = "seeker";

		// Seeker用
		float speed = 0.2f;
		float turnRate = 0.15f;

		// 共通
		int hp = 1;
		float radius = 1.0f;
		float lifeTime = 30.0f;

		// Turret用（TurretEnemy.h にある setter に合わせる）
		int shootIntervalFrames = 60; // 1秒@60fps
		float bulletSpeed = 2.8f;
		float bulletLifeTime = 3.0f;
	};

public:
	EnemyManager() = default;
	~EnemyManager() = default;

	void Initialize();
	void LoadEnemyScv(const std::string& path);
	void Update(float dt, const KamataEngine::Vector3& playerPos);
	void Draw(KamataEngine::Camera* camera);

	std::vector<std::unique_ptr<CharacterBase>>& GetEnemies() { return enemies_; }
	void RemoveDeadEnemies();

private:
	void SpawnEnemiesByScv(const KamataEngine::Vector3& playerPos);
	void SpawnExplosionAt(const KamataEngine::Vector3& pos);

private:
	std::vector<std::unique_ptr<CharacterBase>> enemies_;
	std::vector<EnemySpawnData> enemySpawnList_;
	float enemySpawnTimer_ = 0.0f;

	KamataEngine::Model* explosionModel_ = nullptr;
	std::vector<std::unique_ptr<DamageParticle>> explosionParticles_;
};
