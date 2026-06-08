#pragma once

#include <memory>
#include <vector>

#include <KamataEngine.h>

#include "Application/Characters/Boss/Boss.h"
#include "Application/Characters/Boss/BossAttackController.h"
#include "Managers/BossManager/BossUIManager.h"

class Bullet;
class HomingMissile;
class Player;

/// <summary>
/// Boss ステージの出現演出、ボス本体、専用 UI、撃破演出を管理する。
/// </summary>
class BossManager {
public:
	enum class State {
		Inactive,
		Intro,
		Battle,
		Clear,
	};

	void Initialize();
	void StartBossBattle(const KamataEngine::Vector3& playerPos, KamataEngine::WorldTransform* parent);
	void Update(float dt);
	void Draw(const KamataEngine::Camera* camera);
	void DrawUI();

	void ResolvePlayerAttackCollisions(std::vector<std::unique_ptr<Bullet>>& bullets, std::vector<std::unique_ptr<HomingMissile>>& missiles);

	bool IsActive() const { return state_ != State::Inactive; }
	bool IsBattle() const { return state_ == State::Battle; }
	bool IsClearFinished() const { return state_ == State::Clear && clearTimer_ >= kClearDuration; }
	State GetState() const { return state_; }
	Boss* GetBoss() const { return boss_.get(); }

private:
	template <class TProjectile>
	void ResolveProjectileCollisions_(std::vector<std::unique_ptr<TProjectile>>& projectiles);

	State state_ = State::Inactive;
	std::unique_ptr<Boss> boss_;
	BossAttackController attackController_;
	BossUIManager uiManager_;
	float introTimer_ = 0.0f;
	float clearTimer_ = 0.0f;
	bool defeatHandled_ = false;

	static constexpr float kIntroDuration = 2.0f;
	static constexpr float kClearDuration = 2.0f;
};
