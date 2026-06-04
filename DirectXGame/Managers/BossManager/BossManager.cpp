#include "BossManager.h"

#include <algorithm>
#include <cmath>

#include "Application/Characters/Player/Bullet.h"
#include "Application/Characters/Player/HomingMissile.h"
#include "Application/Characters/Player/Laser.h"

using namespace KamataEngine;

namespace {
CharacterBase::DamageSource GetProjectileDamageSource(CharacterBase* projectile) {
	if (dynamic_cast<Laser*>(projectile)) {
		return CharacterBase::DamageSource::Laser;
	}
	if (dynamic_cast<HomingMissile*>(projectile)) {
		return CharacterBase::DamageSource::HomingMissile;
	}
	return CharacterBase::DamageSource::Bullet;
}

bool IsHit(CharacterBase* a, CharacterBase* b) {
	if (!a || !b || !a->GetCollider() || !b->GetCollider()) {
		return false;
	}
	const Vector3 ap = a->GetCollider()->GetTranslate();
	const Vector3 bp = b->GetCollider()->GetTranslate();
	const float ar = a->GetCollider()->GetRadius();
	const float br = b->GetCollider()->GetRadius();
	const float dx = ap.x - bp.x;
	const float dy = ap.y - bp.y;
	const float dz = ap.z - bp.z;
	const float rr = ar + br;
	return dx * dx + dy * dy + dz * dz <= rr * rr;
}
} // namespace

void BossManager::Initialize() {
	state_ = State::Inactive;
	boss_.reset();
	introTimer_ = 0.0f;
	clearTimer_ = 0.0f;
	defeatHandled_ = false;
	uiManager_.Initialize();
}

void BossManager::StartBossBattle(const Vector3& playerPos) {
	boss_ = std::make_unique<Boss>();
	boss_->Initialize({playerPos.x, playerPos.y + 0.8f, playerPos.z + 55.0f});
	attackController_.Initialize(boss_.get());
	state_ = State::Intro;
	introTimer_ = 0.0f;
	clearTimer_ = 0.0f;
	defeatHandled_ = false;
}

void BossManager::Update(float dt) {
	if (state_ == State::Inactive) {
		uiManager_.Update(nullptr, 0.0f, false, false);
		return;
	}

	if (state_ == State::Intro) {
		introTimer_ += dt;
		if (boss_) {
			boss_->Update();
		}
		const float warningRate = std::fabs(std::sin(introTimer_ * 10.0f));
		uiManager_.Update(boss_.get(), warningRate, false, true);
		if (introTimer_ >= kIntroDuration) {
			state_ = State::Battle;
		}
		return;
	}

	if (state_ == State::Battle) {
		if (boss_) {
			boss_->Update();
			attackController_.Update(dt);
			if (boss_->IsDead() && !defeatHandled_) {
				state_ = State::Clear;
				clearTimer_ = 0.0f;
				defeatHandled_ = true;
			}
		}
		uiManager_.Update(boss_.get(), 0.0f, true, false);
		return;
	}

	if (state_ == State::Clear) {
		clearTimer_ += dt;
		uiManager_.Update(boss_.get(), 0.0f, false, false);
	}
}

void BossManager::Draw(const Camera* camera) {
	if (boss_ && state_ != State::Inactive && state_ != State::Clear) {
		boss_->Draw(camera);
	}
}

void BossManager::DrawUI() { uiManager_.Draw(); }

void BossManager::ResolvePlayerAttackCollisions(std::vector<std::unique_ptr<Bullet>>& bullets, std::vector<std::unique_ptr<Laser>>& lasers, std::vector<std::unique_ptr<HomingMissile>>& missiles) {
	if (state_ != State::Battle || !boss_) {
		return;
	}
	ResolveProjectileCollisions_(bullets);
	ResolveProjectileCollisions_(lasers);
	ResolveProjectileCollisions_(missiles);
}

template <class TProjectile>
void BossManager::ResolveProjectileCollisions_(std::vector<std::unique_ptr<TProjectile>>& projectiles) {
	if (!boss_) {
		return;
	}
	for (auto& projectile : projectiles) {
		if (!projectile || projectile->IsDead()) {
			continue;
		}
		for (BossPart* part : boss_->GetCollisionParts()) {
			if (!part || !part->IsCollidable()) {
				continue;
			}
			if (!IsHit(projectile.get(), part)) {
				continue;
			}
			part->SetLastDamageSource(GetProjectileDamageSource(projectile.get()));
			part->OnCollision(projectile.get());
			projectile->OnCollision(part);
			break;
		}
	}
}
