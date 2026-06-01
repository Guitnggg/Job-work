#include "EnemyManager.h"

#include <cmath>

#include <fstream>
#include <random>

#include <json.hpp>
using json = nlohmann::json;

using namespace KamataEngine;

void EnemyManager::Initialize() {
	enemies_.clear();
	enemySpawnList_.clear();

	enemySpawnTimer_ = 0.0f;
}

void EnemyManager::LoadEnemyCsv(const std::string& path) {
	enemySpawnList_.clear();

	std::ifstream ifs(path);
	if (!ifs) {
		return;
	}

	json root;
	ifs >> root;

	if (!root.contains("randomAreas")) {
		return;
	}

	static std::mt19937_64 rng{123456789ull};

	for (auto& r : root["randomAreas"]) {
		const int32_t count = r.value("count", 0);
		const float timeMin = r.value("timeMin", 0.0f);
		const float timeMax = r.value("timeMax", timeMin);

		const auto centerNode = r["posCenter"];
		const Vector3 center{centerNode[0].get<float>(), centerNode[1].get<float>(), centerNode[2].get<float>()};

		const auto rangeNode = r["posRange"];
		const Vector3 range{rangeNode[0].get<float>(), rangeNode[1].get<float>(), rangeNode[2].get<float>()};

		const std::string type = r.value("type", std::string("seeker"));

		// Seeker
		const float baseSpeed = r.value("speed", kDefaultSeekerSpeed);
		const float speedRange = r.value("speedRange", 0.0f);
		const float turnRate = r.value("turnRate", kDefaultSeekerTurnRate);

		// 共通
		const int32_t hp = r.value("hp", kDefaultHp);
		const float radius = r.value("radius", kDefaultColliderRadius);
		const float lifeTime = r.value("lifeTime", kDefaultLifeTime);

		// Turret
		const int32_t shootIntervalFrames = r.value("shootIntervalFrames", kDefaultShootIntervalFrames);
		const float bulletSpeed = r.value("bulletSpeed", kDefaultBulletSpeed);
		const float bulletLifeTime = r.value("bulletLifeTime", kDefaultBulletLifeTime);

		std::uniform_real_distribution<float> timeDist(timeMin, timeMax);
		std::uniform_real_distribution<float> dx(-range.x, range.x);
		std::uniform_real_distribution<float> dy(-range.y, range.y);
		std::uniform_real_distribution<float> dz(-range.z, range.z);
		std::uniform_real_distribution<float> speedDist(baseSpeed - speedRange, baseSpeed + speedRange);

		for (int32_t i = 0; i < count; ++i) {
			EnemySpawnData d;
			d.time = timeDist(rng);
			d.pos = {center.x + dx(rng), center.y + dy(rng), center.z + dz(rng)};
			d.type = type;

			d.speed = speedDist(rng);
			d.turnRate = turnRate;

			d.hp = hp;
			d.radius = radius;
			d.lifeTime = lifeTime;

			d.shootIntervalFrames = shootIntervalFrames;
			d.bulletSpeed = bulletSpeed;
			d.bulletLifeTime = bulletLifeTime;

			enemySpawnList_.push_back(d);
		}
	}

	std::sort(enemySpawnList_.begin(), enemySpawnList_.end(), [](const EnemySpawnData& a, const EnemySpawnData& b) { return a.time < b.time; });
}

void EnemyManager::SpawnEnemiesByCsv(const Vector3& playerPos) {
	const float currentTime = enemySpawnTimer_;

	while (!enemySpawnList_.empty()) {
		const EnemySpawnData& d = enemySpawnList_.front();
		if (currentTime < d.time) {
			break;
		}

		const Vector3 spawnPos{playerPos.x + d.pos.x, playerPos.y + d.pos.y, playerPos.z + d.pos.z};

		auto enemy = CreateEnemy_(d, spawnPos);
		if (enemy) {
			enemies_.push_back(std::move(enemy));
		}

		enemySpawnList_.erase(enemySpawnList_.begin());
	}
}

std::unique_ptr<CharacterBase> EnemyManager::CreateEnemy_(const EnemySpawnData& spawnData, const Vector3& spawnPos) const {
	// Factory Method Pattern:
	// EnemyManager requests creation, and each enemy type setup is centralized here.
	if (spawnData.type == "turret") {
		auto turret = std::make_unique<TurretEnemy>();
		turret->GetWorldTransform().translation_ = spawnPos;

		turret->SetInitialHP(spawnData.hp);
		turret->SetColliderRadius(spawnData.radius);
		turret->SetShootIntervalFrames(spawnData.shootIntervalFrames);
		turret->SetBulletSpeed(spawnData.bulletSpeed);
		turret->SetBulletLifeTime(spawnData.bulletLifeTime);

		turret->Initialize();
		return turret;
	}

	auto seeker = std::make_unique<SeekerEnemy>();
	seeker->SetInitialPosition(spawnPos);
	seeker->SetSpeed(spawnData.speed);
	seeker->SetTurnRate(spawnData.turnRate);
	seeker->SetInitialHP(spawnData.hp);
	seeker->SetColliderRadius(spawnData.radius);
	seeker->SetLifeTime(spawnData.lifeTime);

	seeker->Initialize();
	return seeker;
}

void EnemyManager::Update(float dt, const Vector3& playerPos) {
	enemySpawnTimer_ += dt;
	SpawnEnemiesByCsv(playerPos);

	// 敵更新
	for (auto& enemy : enemies_) {
		// 敵タイプごとにターゲット設定
		// dynamic_cast を使用している理由：
		// EnemyManager は CharacterBase のみを保持し、
		// 各敵固有の機能は派生クラスに委譲する設計のため
		if (auto* seeker = dynamic_cast<SeekerEnemy*>(enemy.get())) {
			seeker->SetTarget(playerPos);
		} else if (auto* turret = dynamic_cast<TurretEnemy*>(enemy.get())) {
			turret->SetTarget(playerPos);
		}

		enemy->Update();
	}
}

void EnemyManager::Draw(const Camera* camera) {
	for (auto& enemy : enemies_) {
		enemy->Draw(camera);
	}
}

std::vector<CharacterBase*> EnemyManager::GetNearestEnemies(const KamataEngine::Vector3& from, int32_t maxCount) const {
	std::vector<std::pair<float, CharacterBase*>> distances;
	distances.reserve(enemies_.size());

	for (const auto& enemy : enemies_) {
		if (!enemy || enemy->IsDead()) {
			continue;
		}
		const KamataEngine::Vector3 pos = enemy->GetWorldTranslation();
		const float dx = pos.x - from.x;
		const float dy = pos.y - from.y;
		const float dz = pos.z - from.z;
		const float distSq = dx * dx + dy * dy + dz * dz;
		distances.emplace_back(distSq, enemy.get());
	}

	std::sort(distances.begin(), distances.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

	if (maxCount <= 0) {
		return {};
	}

	const size_t count = (std::min)(distances.size(), static_cast<size_t>(maxCount));
	std::vector<CharacterBase*> result;
	result.reserve(count);
	for (size_t i = 0; i < count; ++i) {
		result.push_back(distances[i].second);
	}
	return result;
}

void EnemyManager::RemoveDeadEnemies() {
	enemies_.erase(std::remove_if(enemies_.begin(), enemies_.end(), [](const std::unique_ptr<CharacterBase>& e) { return e->IsDead(); }), enemies_.end());
}
