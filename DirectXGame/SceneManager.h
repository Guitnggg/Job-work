#pragma once

#include <2d/Sprite.h>
#include <base/DirectXCommon.h>

#include <algorithm>

#include "IScene.h"
#include "SceneName.h"

enum class SceneTransitionState {
    None,
    FadeOut,
    LoadNextScene,
    FadeIn
};

class SceneManager {
public:

    /// <summary>
    /// コンストラクタ：初期化処理
    /// </summary>
    SceneManager();

    /// <summary>
    /// デストラクタ：現在のシーンを解放
    /// </summary>
    ~SceneManager();

    /// <summary>
    /// シーンを新しいものに切り替える
    /// </summary>
    /// <param name="newScene">次に実行するシーン</param>
    void ChangeScene(IScene* newScene);

    /// <summary>
    /// 現在のシーンの更新処理
    /// シーンの終了フラグが立っていたら次のシーンに遷移する
    /// </summary>
    void Update();

    /// <summary>
    /// 現在のシーンの描画処理
    /// </summary>
    void Draw();

    /// <summary>
    /// 現在のシーンが存在しない＝全て終了したかどうか
    /// </summary>
    bool IsEnd() const;

    /// <summary>
    /// 現在のシーンを取得（ポインタ）
    /// </summary>
    IScene* GetCurrentScene() const;

    /// <summary>
    ///  最初のsceneを設定する
    /// </summary>
    /// <param name="scene"></param>
    void SetInitialScene(IScene* scene);

private:

    IScene* currentScene_ = nullptr;  // 現在実行中のシーン
    IScene* nextScene_ = nullptr;

    SceneTransitionState transitionState_ = SceneTransitionState::None;

    float transitionAlpha_ = 0.0f;
    float transitionSpeed_ = 0.02f;

    KamataEngine::Sprite* fadeSprite_ = nullptr;
};
