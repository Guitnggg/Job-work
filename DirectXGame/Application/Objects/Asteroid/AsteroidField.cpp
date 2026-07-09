#include "AsteroidField.h"

#include "Application/Utility/GameTime.h"

using namespace KamataEngine;

void AsteroidField::Initialize(const AsteroidFieldConfig& config) {
    config_ = config;
    spawnTimer_ = 0.0f;

    asteroidModel_.reset(Model::CreateFromOBJ("Asteroid", true));

    asteroids_.clear();
    asteroids_.reserve(config_.count);
    for (int i = 0; i < config_.count; ++i) {
        SpawnAsteroid();
    }
}

void AsteroidField::Update() {
    spawnTimer_ += GameTime::kDeltaTime;

    for (auto& asteroid : asteroids_) {
        asteroid->Update();

        if (asteroid->GetZ() < config_.recycleZ && spawnTimer_ >= config_.spawnInterval) {
            spawnTimer_ = 0.0f;

            Vector3 pos = { Rand(-config_.rangeX, config_.rangeX), Rand(-config_.rangeY, config_.rangeY), Rand(config_.spawnZMin, config_.spawnZMax) };
            Vector3 velocity = { 0.0f, 0.0f, Rand(config_.speedMin, config_.speedMax) };
            Vector3 rotate = { Rand(config_.rotationMin, config_.rotationMax), Rand(config_.rotationMin, config_.rotationMax), Rand(config_.rotationMin, config_.rotationMax) };

            asteroid->Respawn(pos, velocity, rotate);
        }
    }
}

void AsteroidField::Draw(const Camera& camera) const {
    for (const auto& asteroid : asteroids_) {
        asteroid->Draw(camera);
    }
}

void AsteroidField::SpawnAsteroid() {
    Vector3 pos = { Rand(-config_.rangeX, config_.rangeX), Rand(-config_.rangeY, config_.rangeY), Rand(config_.spawnZMin, config_.spawnZMax) };
    Vector3 velocity = { 0.0f, 0.0f, Rand(config_.speedMin, config_.speedMax) };
    Vector3 rotate = { Rand(config_.rotationMin, config_.rotationMax), Rand(config_.rotationMin, config_.rotationMax), Rand(config_.rotationMin, config_.rotationMax) };

    auto asteroid = std::make_unique<Asteroid>();
    asteroid->Initialize(asteroidModel_.get(), pos, velocity, rotate);
    asteroids_.push_back(std::move(asteroid));
}

float AsteroidField::Rand(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(mt_);
}
