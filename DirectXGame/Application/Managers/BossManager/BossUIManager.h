#pragma once

#include <memory>

#include <KamataEngine.h>

class Boss;

/// <summary>
/// Boss 専用 UI。画面上部の HP バーと WARNING 演出を管理する。
/// </summary>
class BossUIManager {
public:
	void Initialize();
	void Update(const Boss* boss, float warningRate, bool showHpBar, bool showWarning);
	void Draw();

private:
	std::unique_ptr<KamataEngine::Sprite> hpBackBar_;
	std::unique_ptr<KamataEngine::Sprite> hpFrontBar_;
	std::unique_ptr<KamataEngine::Sprite> hpFrameBar_;
	std::unique_ptr<KamataEngine::Sprite> warningBar_;
	uint32_t textureHandle_ = 0u;
	bool showHpBar_ = false;
	bool showWarning_ = false;
};
