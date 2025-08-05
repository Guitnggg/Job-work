#include "IntroductionScene.h"

#include "GameScene.h"

using namespace KamataEngine;

IntroductionScene::IntroductionScene() {}

IntroductionScene::~IntroductionScene() {
}

void IntroductionScene::Initialize() {
}

void IntroductionScene::Update() {

    // 入力を受け付けるようにする
    input_ = Input::GetInstance();

    if (input_->PushKey(DIK_SPACE)) {  // シーン変遷の条件を書く
        isEnd_ = true;
    }
}

void IntroductionScene::Draw() {
}

IScene* IntroductionScene::NextScene() const {
    return new GameScene();
}
