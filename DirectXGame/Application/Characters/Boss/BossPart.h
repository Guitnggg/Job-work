#pragma once

#include <memory>

#include "3d/Camera.h"
#include "3d/Model.h"
#include "3d/ObjectColor.h"

#include "Application/Characters/CharacterBase.h"

class Boss;

/// <summary>
/// ボスの弱点部位。左右コアと本体を別々の当たり判定として扱う。
/// </summary>
class BossPart : public CharacterBase {
public:
	enum class Type {
		LeftCore,
		RightCore,
		Body,
	};

	void Initialize(Boss* owner, Type type, const KamataEngine::Vector3& localOffset, int32_t hp, float radius, const KamataEngine::Vector4& color);
	void Update() override;
	void Draw(const KamataEngine::Camera* camera) override;
	void OnCollision(CharacterBase* other) override;

	bool IsDead() const override { return hp_ <= 0; }
	bool IsCollidable() const override;

	Type GetType() const { return type_; }
	const KamataEngine::Vector3& GetLocalOffset() const { return localOffset_; }
	void SetLocalOffset(const KamataEngine::Vector3& offset) { localOffset_ = offset; }
	void SetVisible(bool visible) { isVisible_ = visible; }

private:
	Boss* owner_ = nullptr;
	Type type_ = Type::Body;
	KamataEngine::Vector3 localOffset_{0.0f, 0.0f, 0.0f};
	std::unique_ptr<KamataEngine::Model> model_;
	KamataEngine::ObjectColor objectColor_;
	uint32_t textureHandle_ = 0u;
	bool isVisible_ = true;
};
