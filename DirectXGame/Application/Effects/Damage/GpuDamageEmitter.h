#pragma once

#include "Application/Effects/GpuParticleEmitter.h"

class GpuDamageEmitter : public GpuParticleEmitter {
public:
	void Initialize(uint32_t maxParticles) {
		GpuParticleEmitter::Initialize(maxParticles);
		SetColor(1.0f, 0.55f, 0.20f, 0.95f);
	}
};