#pragma once

#include "SceneName.h"

/// <summary>
/// シーン遷移時の演出スタイル
/// ・Auto       : SceneManager が自動判定
/// ・BlackFade  : 黒フェードアウト/フェードイン
/// ・WhiteFlash : 白フラッシュ
/// ・None       : 無演出（即時切替）
/// </summary>
enum class TransitionStyle {
    Auto,
    BlackFade,
    WhiteFlash,
    None
};

/// <summary>
/// 遷移演出のパラメータ指定  
/// style      : 遷移表現の種類  
/// fadeSpeed  : フェードに使用する速度（-1 ならデフォルト）  
/// frashTime  : WhiteFlash の発光時間（-1 ならデフォルト）
/// </summary>
struct TransitionHint {
    TransitionStyle style = TransitionStyle::Auto;
    float fadeSpeed = -1.0f;
    float frashTime = -1.0f;
};

/// <summary>
/// すべてのシーンが継承すべきインターフェース。  
/// Title → Introduction → Game → Clear / Finish など、  
/// 全シーンの基本的な振る舞い（初期化・更新・描画・遷移）を統一する役割を持つ。
/// </summary>
class IScene {
public:
    virtual ~IScene() = default;

    /// <summary>
    /// 初期化処理
    /// </summary>
    virtual void Initialize() = 0;

    /// <summary>
    /// 更新処理（毎フレーム呼ばれる）
    /// </summary>
    virtual void Update() = 0;

    /// <summary>
    /// 描画処理（毎フレーム呼ばれる）
    /// </summary>
    virtual void Draw() = 0;

    /// <summary>
    /// シーンが終了したかどうか
    /// </summary>
    virtual bool IsEnd() const = 0;

    /// <summary>
    /// 次のシーンを返す（nullptr を返すとゲーム終了）
    /// </summary>
    virtual IScene* NextScene() const = 0;

    /// <summary>
    /// 現在のシーン名を返す（デバッグや切替の判定に使える）
    /// </summary>
    virtual SceneName GetSceneName() const = 0;

    /// <summary>
    /// このシーンからtoへ遷移するときに使ってほしい演出ヒント
    /// 規定は　Auto
    /// </summary>
    virtual TransitionHint GetTransitionHint(SceneName /*to*/)const { return {}; }
};
