#include "SceneManager.h"

using namespace KamataEngine;

SceneManager::SceneManager() {
    // 黒フェード
    fadeSprite_ = Sprite::Create(
        0,                           // テクスチャハンドル（不要なら0でOK）
        { 0.0f, 0.0f },              // 位置
        { 0.0f, 0.0f, 0.0f, 0.0f },  // 色（透明）
        { 0.0f, 0.0f },              // アンカーポイント
        false,                       // FlipX
        false                        // FlipY
    );
    fadeSprite_->SetSize({ 1280, 720 });

    // 白フラッシュ
    flashSprite_ = Sprite::Create(
        0,
        { 0.0f, 0.0f },
        { 1.0f, 1.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f },
        false,
        false
    );
    flashSprite_->SetSize({ 1280,720 });
}

SceneManager::~SceneManager() {
    delete currentScene_;
    delete fadeSprite_;
    delete flashSprite_;
}

bool SceneManager::ShouldUseWhiteFlash(SceneName from, SceneName to) const {
    // Title→Introduction は使わない（現状維持）
    // Introduction→InGame のときだけ true
    return (from == SceneName::Introduction && to == SceneName::InGame);
}

void SceneManager::ChangeScene(IScene* newScene) {
    nextScene_ = newScene;

    // 今いるシーン名（起動直後など、currentScene_がなければNone）
    SceneName from = currentScene_ ? currentScene_->GetSceneName() : SceneName::None;
    SceneName to = nextScene_ ? nextScene_->GetSceneName() : SceneName::None;

    pendingWhiteFlash_ = ShouldUseWhiteFlash(from, to);

    if (pendingWhiteFlash_) {
        // 白フラッシュから開始
        flashTimer_ = 0.0f;
        flashSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
        transitionState_ = SceneTransitionState::FlashOut;
    }
    else {
        transitionState_ = SceneTransitionState::FadeOut;
    }   
}

void SceneManager::Update() {
    switch (transitionState_) {
    case SceneTransitionState::None:
        if (currentScene_) {
            currentScene_->Update();
            if (currentScene_->IsEnd()) {
                ChangeScene(currentScene_->NextScene());
            }
        }
        break;

    // 白フラッシュ
    case SceneTransitionState::FlashOut: {
        flashTimer_ += 1.0f / 60.0f;
        float t = std::clamp(flashTimer_/flashTimer_, 0.0f, 1.0f);
        float alpha = (t < 0.5f) ? (t * 2.0f) : (2.0f - t * 2.0f);
        flashSprite_->SetColor({ 1.0f, 1.0f ,1.0f, alpha });

        if (flashTimer_ >= flashTime_) {
            // 白が消えたら黒フェードアウトへバトンタッチ
            transitionAlpha_ = 0.0f;
            transitionState_ = SceneTransitionState::FadeOut;
        }
    } break;

    // 黒フェードアウト
    case SceneTransitionState::FadeOut: {
        transitionAlpha_ += transitionSpeed_;
        fadeSprite_->SetColor({ 0, 0, 0, std::clamp(transitionAlpha_, 0.0f, 1.0f) });

        if (transitionAlpha_ >= 1.0f) {
            delete currentScene_;
            currentScene_ = nextScene_;
            nextScene_ = nullptr;
            currentScene_->Initialize();
            transitionState_ = SceneTransitionState::FadeIn;
        }
    } break;

    // 黒フェードイン
    case SceneTransitionState::FadeIn:{
        transitionAlpha_ -= transitionSpeed_;
        fadeSprite_->SetColor({ 0, 0, 0, std::clamp(transitionAlpha_, 0.0f, 1.0f) });

        if (transitionAlpha_ <= 0.0f) {
            transitionState_ = SceneTransitionState::None;
        }
    }break;
    }
}

void SceneManager::Draw() {
    if (currentScene_) {
        currentScene_->Draw();
    }

    if (transitionState_ != SceneTransitionState::None) {
        auto commandList = DirectXCommon::GetInstance()->GetCommandList();
        Sprite::PreDraw(commandList);
        
        // 白フラッシュ
        if (transitionState_ == SceneTransitionState::FlashOut) {
            flashSprite_->Draw();
        }

        // 黒フェード
        if (transitionState_ == SceneTransitionState::FadeOut || 
            transitionState_ == SceneTransitionState::FadeIn) {
            fadeSprite_->Draw();
        }

        Sprite::PostDraw();
    }
}

bool SceneManager::IsEnd() const {
    return currentScene_ == nullptr && nextScene_ == nullptr;
}

IScene* SceneManager::GetCurrentScene() const {
    return currentScene_;
}

void SceneManager::SetInitialScene(IScene* scene) {
    currentScene_ = scene;
    if (currentScene_) {
        currentScene_->Initialize();
    }

    transitionState_ = SceneTransitionState::None;

    transitionAlpha_ = 0.0f;

    fadeSprite_->SetColor({ 0, 0, 0, 0 });
}
