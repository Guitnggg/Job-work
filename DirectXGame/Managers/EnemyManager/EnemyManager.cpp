#include "EnemyManager.h"

#include <fstream>
#include <random>

#include <json.hpp>
using json = nlohmann::json;

using namespace KamataEngine;

namespace {
constexpr float kDefaultSpeed = 0.2f;
constexpr float kDefaultSpeedVar = 0.0f;
constexpr float kDefaultTurnRate = 0.15f;
constexpr int kDefaultHp = 1;
constexpr float kDefaultRadius = 1.0f;
constexpr float kDefaultLifeTime = 30.0f;

constexpr uint64_t kSpawnSeed = 123456789ull;

constexpr int kExplosionParticleCount = 16;
constexpr float kExplosionSpeed = 3.0f;
constexpr float kExplosionLifeTime = 0.6f;
constexpr float kExplosionStartScale = 0.25f;
constexpr float kExplosionEndScale = 0.0f;
} // namespace

void EnemyManager::Initialize() {
	enemies_.clear();
	enemySpawnList_.clear();
	enemySpawnTimer_ = 0.0f;

	explosionModel_ = Model::Create();
	explosionParticles_.clear();
}

void EnemyManager::LoadEnemyScv(const std::string& path) {
	enemySpawnList_.clear();

	std::ifstream ifs(path);
	if (!ifs) {
		return;
	}

	json root;
	ifs >> root;

	if (root.contains("randomAreas")) {
		static std::mt19937_64 rng{kSpawnSeed};

		for (auto& r : root["randomAreas"]) {
			int count = r.value("count", 0);
			float timeMin = r.value("timeMin", 0.0f);
			float timeMax = r.value("timeMax", timeMin);

			auto centerNode = r["posCenter"];
			Vector3 center{centerNode[0].get<float>(), centerNode[1].get<float>(), centerNode[2].get<float>()};

			auto rangeNode = r["posRange"];
			Vector3 range{rangeNode[0].get<float>(), rangeNode[1].get<float>(), rangeNode[2].get<float>()};

			// ★追加：敵タイプ（未指定は seeker）
			std::string type = r.value("type", std::string("seeker"));

			// Seeker用
			float baseSpeed = r.value("speed", kDefaultSpeed);
			float speedRange = r.value("speedRange", kDefaultSpeedVar);
			float turnRate = r.value("turnRate", kDefaultTurnRate);

			// 共通
			int hp = r.value("hp", kDefaultHp);
			float radius = r.value("radius", kDefaultRadius);
			float lifeTime = r.value("lifeTime", kDefaultLifeTime);

			// ★追加：Turret用（未指定はデフォルト）
			int shootIntervalFrames = r.value("shootIntervalFrames", 60);
			float bulletSpeed = r.value("bulletSpeed", 2.8f);
			float bulletLifeTime = r.value("bulletLifeTime", 3.0f);

			std::uniform_real_distribution<float> timeDist(timeMin, timeMax);
			std::uniform_real_distribution<float> dx(-range.x, range.x);
			std::uniform_real_distribution<float> dy(-range.y, range.y);
			std::uniform_real_distribution<float> dz(-range.z, range.z);
			std::uniform_real_distribution<float> speedDist(baseSpeed - speedRange, baseSpeed + speedRange);

			for (int i = 0; i < count; ++i) {
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
	}

	std::sort(enemySpawnList_.begin(), enemySpawnList_.end(), [](const EnemySpawnData& a, const EnemySpawnData& b) { return a.time < b.time; });
}

void EnemyManager::SpawnEnemiesByScv(const Vector3& playerPos) {
	const float t = enemySpawnTimer_;

	while (!enemySpawnList_.empty()) {
		const auto& d = enemySpawnList_.front();
		if (t < d.time)
			break;

		Vector3 spawnPos{playerPos.x + d.pos.x, playerPos.y + d.pos.y, playerPos.z + d.pos.z};

		// ★ここが本題：typeで分岐
		if (d.type == "turret") {
			auto turret = std::make_unique<TurretEnemy>();
			turret->GetWorldTransform().translation_ = spawnPos;
			turret->Initialize();
			turret->SetInitialHP(d.hp);
			turret->SetColliderRadius(d.radius);

			// Turret専用パラメータ
			turret->SetShootIntervalFrames(d.shootIntervalFrames);
			turret->SetBulletSpeed(d.bulletSpeed);
			turret->SetBulletLifeTime(d.bulletLifeTime);

			turret->Initialize();
			enemies_.push_back(std::move(turret));
		} else { // "seeker" or unknown => seeker 扱い
			auto s = std::make_unique<SeekerEnemy>();
			s->SetInitialPosition(spawnPos);
			s->SetSpeed(d.speed);
			s->SetTurnRate(d.turnRate);
			s->SetInitialHP(d.hp);
			s->SetColliderRadius(d.radius);
			s->SetLifeTime(d.lifeTime);
			s->Initialize();

			enemies_.push_back(std::move(s));
		}

		enemySpawnList_.erase(enemySpawnList_.begin());
	}
}

void EnemyManager::Update(float dt, const Vector3& playerPos) {
	enemySpawnTimer_ += dt;
	SpawnEnemiesByScv(playerPos);

	// ★TurretもSeekerも「ターゲット設定」を行う
	for (auto& e : enemies_) {
		if (auto* s = dynamic_cast<SeekerEnemy*>(e.get())) {
			s->SetTarget(playerPos);
		}
		if (auto* t = dynamic_cast<TurretEnemy*>(e.get())) {
			t->SetTarget(playerPos);
		}
		e->Update();
	}

	for (auto& p : explosionParticles_) {
		p->Update(dt);
	}
	explosionParticles_.erase(
	    std::remove_if(explosionParticles_.begin(), explosionParticles_.end(), [](const std::unique_ptr<DamageParticle>& p) { return p->IsFinished(); }), explosionParticles_.end());
}

void EnemyManager::Draw(Camera* camera) {
	for (auto& e : enemies_) {
		e->Draw(camera);
	}
	if (explosionModel_) {
		for (auto& p : explosionParticles_) {
			p->Draw(camera);
		}
	}
}

void EnemyManager::SpawnExplosionAt(const KamataEngine::Vector3& pos) {
	if (!explosionModel_)
		return;

	static std::mt19937 rng{std::random_device{}()};
	std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

	for (int i = 0; i < kExplosionParticleCount; ++i) {
		Vector3 vel{dist(rng) * kExplosionSpeed, dist(rng) * kExplosionSpeed, dist(rng) * kExplosionSpeed};

		auto p = std::make_unique<DamageParticle>();
		p->Initialize(explosionModel_, pos, vel, kExplosionLifeTime, kExplosionStartScale, kExplosionEndScale);
		explosionParticles_.push_back(std::move(p));
	}
}

void EnemyManager::RemoveDeadEnemies() {
	// ★Seeker固定だったのを共通化（Turretも含めて削除できる）
	enemies_.erase(std::remove_if(enemies_.begin(), enemies_.end(), [](const std::unique_ptr<CharacterBase>& e) { return e->IsDead(); }), enemies_.end());
}
