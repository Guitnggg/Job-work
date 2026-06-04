#pragma once

#include <array>
#include <memory>
#include <vector>

#include "3d/Camera.h"
#include "3d/Model.h"
#include "3d/ObjectColor.h"

#include "Application/Characters/Boss/BossPart.h"
#include "Application/Characters/CharacterBase.h"

/// <summary>
/// Boss ステージ用の大型敵。左右コアを破壊した後、本体へダメージが通る。
/// 攻撃パターンは後続実装に備えて別クラスから呼び出せる形にしている。
/// </summary>
class Boss : public CharacterBase {
public:
	void Initialize(const KamataEngine::Vector3& position);
	void Update() override;
	void Draw(const KamataEngine::Camera* camera) override;
	void OnCollision(CharacterBase* other) override;

	bool IsDead() const override { return bodyPart_ && bodyPart_->IsDead(); }
	bool AreCoresDestroyed() const;
	bool CanDamageBody() const { return AreCoresDestroyed(); }

	void ApplyPartDamage(BossPart& part, CharacterBase::DamageSource source);
	int32_t GetBodyHP() const { return bodyPart_ ? bodyPart_->GetHP() : 0; }
	int32_t GetBodyMaxHP() const { return bodyPart_ ? bodyPart_->GetMaxHP() : 1; }
	float GetHpRate() const;

	std::vector<BossPart*> GetCollisionParts();

private:
	std::unique_ptr<KamataEngine::Model> model_;
	KamataEngine::ObjectColor objectColor_;
	uint32_t textureHandle_ = 0u;

	std::array<std::unique_ptr<BossPart>, 2> coreParts_;
	std::unique_ptr<BossPart> bodyPart_;

	float hoverTimer_ = 0.0f;
	KamataEngine::Vector3 basePosition_{0.0f, 0.0f, 0.0f};

	static constexpr int32_t kBodyHp = 100;
	static constexpr int32_t kCoreHp = 20;
};
