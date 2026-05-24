#pragma once

#include <memory>
#include <vector>

#include "Application/Characters/Collider.h"
#include "Application/Characters/Enemy/SeekerEnemy.h"
#include "Application/Characters/Enemy/TurretEnemy.h"
#include "Application/Characters/Player/Bullet.h"
#include "Application/Characters/Player/HomingMissile.h"
#include "Application/Characters/Player/Laser.h"
#include "Application/Characters/Player/Player.h"

#include "UI/CountDown/CountDown.h"

/// <summary>
/// プレイヤー、敵、弾、レーザー、ミサイルの当たり判定をまとめる。
/// 各オブジェクトへの通知だけを行い、HPやスコアの処理は個別クラスへ任せる。
/// </summary>
class CollisionManager {
public:
	/// <summary>
	/// 繝励Ξ繧､繝､繝ｼ縺ｨ隍・焚縺ｮ謨ｵ縺ｮ陦晉ｪ∝愛螳壹ｒ陦後≧縲・
	/// 陦晉ｪ√′逋ｺ逕溘＠縺溷ｴ蜷医∽ｸ｡閠・・ OnCollision 繧貞他縺ｳ蜃ｺ縺吶・
	/// </summary>
	/// <param name="player">陦晉ｪ∝ｯｾ雎｡縺ｮ繝励Ξ繧､繝､繝ｼ</param>
	/// <param name="enemies">陦晉ｪ∝ｯｾ雎｡縺ｮ謨ｵ繝ｪ繧ｹ繝・/param>
	/// <param name="countDown">繧ｫ繧ｦ繝ｳ繝医ム繧ｦ繝ｳ荳ｭ縺ｯ陦晉ｪ√ｒ辟｡蜉ｹ縺ｫ縺吶ｋ縺溘ａ菴ｿ逕ｨ</param>
	static void ResolvePlayerEnemyCollisions(Player* player, std::vector<std::unique_ptr<CharacterBase>>& enemies, const CountDown& countDown);

	/// 蠑ｾ縺ｨ隍・焚縺ｮ謨ｵ縺ｨ縺ｮ陦晉ｪ∝愛螳壹ｒ陦後≧縲・
	/// 陦晉ｪ√′逋ｺ逕溘＠縺溷ｴ蜷医∵雰縺ｫ OnCollision縲∝ｼｾ縺ｫ OnCollision 繧帝夂衍縺励・
	/// 1逋ｺ縺ｮ蠑ｾ縺ｫ縺､縺・菴薙・謨ｵ縺ｸ蜻ｽ荳ｭ縺吶ｋ縺薙→繧呈Φ螳壹＠縺ｦ蜃ｦ逅・ｒ荳ｭ譁ｭ縺吶ｋ縲・
	/// </summary>
	/// <param name="bullets">陦晉ｪ∝ｯｾ雎｡縺ｮ蠑ｾ縺ｮ繝ｪ繧ｹ繝・/param>
	/// <param name="enemies">陦晉ｪ∝ｯｾ雎｡縺ｮ謨ｵ縺ｮ繝ｪ繧ｹ繝・/param>
	/// <param name="countDown">繧ｫ繧ｦ繝ｳ繝医ム繧ｦ繝ｳ荳ｭ縺ｯ陦晉ｪ√ｒ辟｡蜉ｹ縺ｫ縺吶ｋ縺溘ａ菴ｿ逕ｨ</param>
	static void ResolveBulletEnemyCollisions(std::vector<std::unique_ptr<Bullet>>& bullets, std::vector<std::unique_ptr<CharacterBase>>& enemies, const CountDown& countDown);

	/// <summary>
	/// 繝ｬ繝ｼ繧ｶ繝ｼ(Laser)縺ｨ隍・焚縺ｮ謨ｵ縺ｨ縺ｮ陦晉ｪ∝愛螳壹ｒ陦後≧縲・
	/// 陦晉ｪ√′逋ｺ逕溘＠縺溷ｴ蜷医∵雰縺ｫ OnCollision縲√Ξ繝ｼ繧ｶ繝ｼ縺ｫ OnCollision 繧帝夂衍縺吶ｋ縲・
	/// </summary>
	/// <param name="lasers">陦晉ｪ∝ｯｾ雎｡縺ｮ繝ｬ繝ｼ繧ｶ繝ｼ縺ｮ繝ｪ繧ｹ繝・/param>
	/// <param name="enemies">陦晉ｪ∝ｯｾ雎｡縺ｮ謨ｵ縺ｮ繝ｪ繧ｹ繝・/param>
	/// <param name="countDown">繧ｫ繧ｦ繝ｳ繝医ム繧ｦ繝ｳ荳ｭ縺ｯ陦晉ｪ√ｒ辟｡蜉ｹ縺ｫ縺吶ｋ縺溘ａ菴ｿ逕ｨ</param>
	static void ResolveLaserEnemyCollisions(std::vector<std::unique_ptr<Laser>>& lasers, std::vector<std::unique_ptr<CharacterBase>>& enemies, const CountDown& countDown);

	/// <summary>
	///
	/// </summary>
	/// <param name="missiles"></param>
	/// <param name="enemies"></param>
	/// <param name="countDown"></param>
	static void ResolveHomingMissileEnemyCollisions(std::vector<std::unique_ptr<HomingMissile>>& missiles, std::vector<std::unique_ptr<CharacterBase>>& enemies, const CountDown& countDown);

	/// <summary>
	/// 繝励Ξ繧､繝､繝ｼ縺ｨ繧ｿ繝ｼ繝ｬ繝・ヨ縺ｮ蠑ｾ縺ｮ陦晉ｪ∝愛螳壹ｒ陦後≧
	/// </summary>
	/// <param name="player"></param>
	/// <param name="enemies"></param>
	/// <param name="countDown"></param>
	static void ResolvePlayerTurretBulletCollisions(Player* player, std::vector<std::unique_ptr<CharacterBase>>& enemies, const CountDown& countDown);

	/// <summary>
	/// 繝励Ξ繧､繝､繝ｼ蠑ｾ縺ｨ繧ｿ繝ｼ繝ｬ繝・ヨ蠑ｾ縺ｮ陦晉ｪ∝愛螳壹ｒ陦後≧
	/// </summary>
	/// <param name="playerBullets"></param>
	/// <param name="enemies"></param>
	/// <param name="countDown"></param>
	static void ResolvePlayerBulletTurretBulletCollisions(std::vector<std::unique_ptr<Bullet>>& playerBullets, std::vector<std::unique_ptr<CharacterBase>>& enemies, const CountDown& countDown);
};
