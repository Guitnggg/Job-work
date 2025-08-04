#include "SceneManager.h"

using namespace KamataEngine;

SceneManager::SceneManager() {
    fadeSprite_ = Sprite::Create(
        0,                      // テクスチャハンドル（不要なら0でOK）
        { 0.0f, 0.0f },           // 位置
        { 0.0f, 0.0f, 0.0f, 0.0f }, // 色（透明）
        { 0.0f, 0.0f },           // アンカーポイント
        false,                  // FlipX
        false                   // FlipY
    );
    fadeSprite_->SetSize({ 1280, 720 });
}

SceneManager::~SceneManager() {
    delete currentScene_;
    delete fadeSprite_;
}

void SceneManager::ChangeScene(IScene* newScene) {
    nextScene_ = newScene;
    transitionState_ = SceneTransitionState::FadeOut;
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
    case SceneTransitionState::FadeOut:
        transitionAlpha_ += transitionSpeed_;
        fadeSprite_->SetColor({ 0, 0, 0, std::clamp(transitionAlpha_, 0.0f, 1.0f) });
        if (transitionAlpha_ >= 1.0f) {
            delete currentScene_;
            currentScene_ = nextScene_;
            nextScene_ = nullptr;
            currentScene_->Initialize();
            transitionState_ = SceneTransitionState::FadeIn;
        }
        break;
    case SceneTransitionState::FadeIn:
        transitionAlpha_ -= transitionSpeed_;
        fadeSprite_->SetColor({ 0, 0, 0, std::clamp(transitionAlpha_, 0.0f, 1.0f) });
        if (transitionAlpha_ <= 0.0f) {
            transitionState_ = SceneTransitionState::None;
        }
        break;
    }
}

void SceneManager::Draw() {
    if (currentScene_) {
        currentScene_->Draw();
    }

    if (transitionState_ != SceneTransitionState::None) {
        auto commandList = DirectXCommon::GetInstance()->GetCommandList();
        Sprite::PreDraw(commandList);
        fadeSprite_->Draw();
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
