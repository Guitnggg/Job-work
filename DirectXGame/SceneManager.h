#pragma once

#include "IScene.h"

class SceneManager {
public:

    /// <summary>
    /// デストラクタ：現在のシーンを解放
    /// </summary>
    ~SceneManager() {
        delete currentScene_;
    }

    /// <summary>
    /// シーンを新しいものに切り替える
    /// </summary>
    /// <param name="newScene">次に実行するシーン</param>
    void ChangeScene(IScene* newScene) {
        delete currentScene_;          // 前のシーンを削除
        currentScene_ = newScene;      // 新しいシーンをセット
        if (currentScene_) {
            currentScene_->Initialize(); // 初期化処理
        }
    }

    /// <summary>
    /// 現在のシーンの更新処理
    /// シーンの終了フラグが立っていたら次のシーンに遷移する
    /// </summary>
    void Update() {
        if (currentScene_) {
            currentScene_->Update();
            if (currentScene_->IsEnd()) {
                IScene* next = currentScene_->NextScene();
                ChangeScene(next);
            }
        }
    }

    /// <summary>
    /// 現在のシーンの描画処理
    /// </summary>
    void Draw() {
        if (currentScene_) {
            currentScene_->Draw();
        }
    }

    /// <summary>
    /// 現在のシーンが存在しない＝全て終了したかどうか
    /// </summary>
    bool IsEnd() const {
        return currentScene_ == nullptr;
    }

    /// <summary>
    /// 現在のシーンを取得（ポインタ）
    /// </summary>
    IScene* GetCurrentScene() const {
        return currentScene_;
    }

private:
    IScene* currentScene_ = nullptr;  // 現在実行中のシーン
};
