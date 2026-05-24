#pragma once

#include <KamataEngine.h>
#include <algorithm>
#include <memory>

#include "Application/Characters/Player/Player.h"
#include "UI/HPBar/Graph.h"
#include "UI/Score/Score.h"

/// <summary>
/// HPバー、スコア、ホーミング関連ゲージなどの画面UIをまとめて管理する。
/// </summary>
class UIManager {
public:
	UIManager() = default;
	~UIManager() = default;

	/// <summary>
	/// 蛻晄悄蛹・
	/// </summary>
	/// <param name="player">HP蜿ら・逕ｨ縺ｮ繝励Ξ繧､繝､繝ｼ・域園譛峨＠縺ｪ縺・ｼ・/param>
	void Initialize(Player* player);

	/// <summary>
	/// 譖ｴ譁ｰ蜃ｦ逅・ｼ・P繝舌・縲√せ繧ｳ繧｢縺ｪ縺ｩ・・
	/// </summary>
	void Update();

	/// <summary>
	/// 謠冗判蜃ｦ逅・
	/// </summary>
	void Draw();

	/// <summary>
	/// 繧ｹ繧ｳ繧｢繧ｪ繝悶ず繧ｧ繧ｯ繝医∈縺ｮ繧｢繧ｯ繧ｻ繧ｹ・亥ｿ・ｦ√↑繧会ｼ・
	/// </summary>
	Score* GetScore() { return score_.get(); }

	/// <summary>
	///
	/// </summary>
	void SetHomingCooldownRate(float rate) { homingCooldownRate_ = std::clamp(rate, 0.0f, 1.0f); }

	/// <summary>
	///
	/// </summary>
	void SetHomingLockInfo(int32_t currentLockCount, int32_t maxLockCount, bool isLocking);

private:
	// HP蜿ら・逕ｨ・域園譛画ｨｩ縺ｪ縺暦ｼ・
	Player* player_ = nullptr;

	// HP繝舌・
	std::unique_ptr<Graph> graph_;

	// 繧ｹ繧ｳ繧｢
	std::unique_ptr<Score> score_;

	std::unique_ptr<KamataEngine::Sprite> homingCooldownBack_;
	std::unique_ptr<KamataEngine::Sprite> homingCooldownFront_;
	std::unique_ptr<KamataEngine::Sprite> lockProgressBar_;
	uint32_t homingBarTexHandle_ = 0;

	float homingCooldownRate_ = 1.0f;
	int32_t homingLockCount_ = 0;
	int32_t homingMaxLockCount_ = 5;
	bool isHomingLocking_ = false;
};
