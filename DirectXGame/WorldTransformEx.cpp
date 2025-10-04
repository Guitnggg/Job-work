#include <3d/WorldTransform.h>
#include "MyMath.h"

using namespace KamataEngine;

void WorldTransform::UpdateMatrix() {
    matWorld_ = MakeAffineMatrix(scale_, rotation_, translation_);

    TransferMatrix();
}