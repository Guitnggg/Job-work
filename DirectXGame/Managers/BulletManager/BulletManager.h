#pragma once

#include <KamataEngine.h>
#include <vector>
#include <memory>

#include "Application/Characters/Player/Bullet.h"
#include "Application/Characters/Player/Player.h"
#include "UI/CountDown/CountDown.h"

/// <summary>
/// 弾の生成・更新・破棄を管理するクラス。  
/// ・プレイヤー入力を検知して弾を発射  
/// ・飛行中の弾の更新  
/// ・死亡（寿命/衝突/距離超過）した弾の後始末  
/// を一括で担当する。
/// </summary>
class BulletManager {
public:
    BulletManager() = default;
    ~BulletManager() = default;

    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize();

    /// <summary>
    /// 更新処理
    /// 入力判定→弾の発射→弾の更新→死亡弾の削除
    /// </summary>
    /// <param name="input">入力処理を参照する為のポインタ</param>
    /// <param name="player">弾の発射位置取得に使うプレイヤー</param>
    /// <param name="countDown">カウントダウン中は発射操作を無効にする為の参照</param>
    void Update(KamataEngine::Input* input, Player* player, const CountDown& countDown);

    /// <summary>
    /// 描画処理
    /// </summary>
    /// <param name="camera">描画に使用するカメラ</param>
    void Draw(KamataEngine::Camera* camera);

    /// <summary>
    /// 現在飛行中の弾のコンテナを参照で返す。  
    /// 外部で弾との当たり判定を行う用途で使用する。
    /// </summary>
    /// <returns>弾のコンテナ(std::vector&lt;unique_ptr&lt;Bullet&gt;&gt;)</returns>
    std::vector<std::unique_ptr<Bullet>>& GetBullets() { return bullets_; }

private:

    /// <summary>
    /// プレイヤー入力に応じて弾発射処理を行う。  
    /// クールダウン制御もここで行う。
    /// </summary>
    /// <param name="input">入力処理</param>
    /// <param name="player">弾の発射位置参照元</param>
    /// <param name="countDown">カウントダウン中は射撃抑制に使用</param>
    void HandleShooting(KamataEngine::Input* input, Player* player, const CountDown& countDown);

    /// <summary>
    /// 全飛行弾の更新処理を行う。
    /// </summary>
    void UpdateBullets();

    /// <summary>
    /// 死亡状態（寿命切れ／衝突／距離制限）の弾をコンテナから削除する。
    /// </summary>
    void RemoveDeadBullets();

private:
    std::vector<std::unique_ptr<Bullet>> bullets_;
    int fireCooldownFrames_ = 0;
    static constexpr int kFireCooldownMax_ = 9;  // 約0.15秒@60fps
};
