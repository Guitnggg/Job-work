#include "Skydome.h"

using namespace KamataEngine;

void Skydome::Initialize(KamataEngine::Camera* camera) {
    model_ = Model::CreateFromOBJ("skydome");
    worldTransform_.Initialize();   
    camera_ = camera;
}

void Skydome::Update() {

}

void Skydome::Draw() {
    model_->Draw(worldTransform_, *camera_);
}