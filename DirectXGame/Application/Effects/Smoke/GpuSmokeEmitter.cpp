#include "GpuSmokeEmitter.h"

#include <d3dcompiler.h>
#include <d3dx12.h>

#include <algorithm>
#include <cstring>

#include "base/DirectXCommon.h"

using namespace KamataEngine;

namespace {
    // 2Dクアッドの頂点構造体
    struct Vertex2D {
        float x;
        float y;
    };

    // １枚のクアッド頂点データ
    constexpr float kQuadVertices[6][2] = {
        {-0.5f, -0.5f},
        {0.5f,  -0.5f},
        {0.5f,  0.5f },
        {-0.5f, -0.5f},
        {0.5f,  0.5f },
        {-0.5f, 0.5f },
    };

    // デフォルトの色
    // レンダーターゲット
    constexpr DXGI_FORMAT kRenderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

    // 深度フォーマット
    constexpr DXGI_FORMAT kDepthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    constexpr float kMinimumParticleLife = 0.01f;
    constexpr UINT kQuadVertexCount = 6;
} // namespace

void GpuSmokeEmitter::Initialize(uint32_t maxParticles) {
    // 最低１は確保する
    maxParticles_ = std::max<uint32_t>(maxParticles, 1);

    // CPU側パーティクル配列確保
    nextSpawnIndex_ = 0;
    elapsedTime_ = 0.0f;

    CreatePipeline_();       // パイプライン生成
    CreateVertexBuffer_();   // クアッド頂点生成
    CreateParticleBuffer_(); // StructuredBuffer生成
    CreateConstantBuffer_(); // 定数バッファ生成
}

void GpuSmokeEmitter::Emit(const Vector3& position, const Vector3& velocity, float life, float startScale, float endScale) {
    Emit(position, velocity, life, startScale, endScale, { 0.65f, 0.65f, 0.65f, 0.7f }, { 0.12f, 0.12f, 0.12f, 0.0f });
}

void GpuSmokeEmitter::Emit(const Vector3& position, const Vector3& velocity, float life, float startScale, float endScale, const Vector4& startColor, const Vector4& endColor) {
    if (!mappedParticles_ || maxParticles_ == 0) {
        return;
    }

    // 次の書き込み位置取得
    SmokeParticle& p = mappedParticles_[nextSpawnIndex_];

    // 初期値設定
    p.initialPosition = position;
    p.spawnTime = elapsedTime_;
    p.velocity = velocity;
    p.life = (std::max)(kMinimumParticleLife, life);
    p.startScale = startScale;
    p.endScale = endScale;
    p.active = 1.0f;
    p.startColor = startColor;
    p.endColor = endColor;

    // 次インデックス更新
    nextSpawnIndex_ = (nextSpawnIndex_ + 1) % maxParticles_;
}

void GpuSmokeEmitter::Update(float dt) {
    if (dt <= 0.0f) {
        return;
    }

    // 無効パーティクルはスキップ
#if 0
    for (auto& p : particles_) {
        if (p.active < 0.5f) {
            continue;
        }

        // 経過時間更新
        p.age += dt;

        // 寿命経過
        if (p.age >= p.life) {
            p.active = 0.0f;
            continue;
        }

        // 移動更新
        p.position.x += p.velocity.x * dt;
        p.position.y += p.velocity.y * dt;
        p.position.z += p.velocity.z * dt;

        // スケール補間
        const float t = std::clamp(p.age / p.life, 0.0f, 1.0f);
        p.scale = p.startScale + (p.endScale - p.startScale) * t;
        if (p.scale < 0.0f) {
            p.scale = 0.0f;
        }
    }

    // CPU→GPUに転送
    if (mappedParticles_) {
        std::memcpy(mappedParticles_, particles_.data(), sizeof(SmokeParticle) * particles_.size());
    }
#endif
    elapsedTime_ += dt;
}

void GpuSmokeEmitter::Draw(const Camera* camera) {
    if (!camera || !pipelineState_ || !rootSignature_) {
        return;
    }

    ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList();
    if (!commandList) {
        return;
    }

    // 定数バッファ更新
    if (mappedConstants_) {
        mappedConstants_->view = camera->matView;
        mappedConstants_->projection = camera->matProjection;
        mappedConstants_->elapsedTime = elapsedTime_;
    }

    // パイプライン設定
    commandList->SetGraphicsRootSignature(rootSignature_.Get());
    commandList->SetPipelineState(pipelineState_.Get());

    // 頂点バッファ設定
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

    // SRVヒープ設定
    ID3D12DescriptorHeap* heaps[] = { srvHeap_.Get() };
    commandList->SetDescriptorHeaps(1, heaps);

    // ルートパラメータ設定
    commandList->SetGraphicsRootConstantBufferView(0, constantBuffer_->GetGPUVirtualAddress());
    commandList->SetGraphicsRootDescriptorTable(1, srvHeap_->GetGPUDescriptorHandleForHeapStart());

    // 描画インスタンシング
    commandList->DrawInstanced(kQuadVertexCount, maxParticles_, 0, 0);
}

void GpuSmokeEmitter::CreatePipeline_() {
    ID3D12Device* device = DirectXCommon::GetInstance()->GetDevice();
    if (!device) {
        return;
    }

    // シェーダーコンパイル
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    // 頂点シェーダー
    HRESULT hr = D3DCompileFromFile(L"./Resources/shaders/GpuSmokeVS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &vsBlob, &errorBlob);
    if (FAILED(hr)) {
        return;
    }

    // ピクセルシェーダー
    hr = D3DCompileFromFile(L"./Resources/shaders/GpuSmokePS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &psBlob, &errorBlob);
    if (FAILED(hr)) {
        return;
    }

    // ===== RootSignature構築 =====
    // t0 : StructuredBuffer（パーティクルデータ）
    CD3DX12_DESCRIPTOR_RANGE rangeSrv;
    rangeSrv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

    // b0 : 定数バッファ
    CD3DX12_ROOT_PARAMETER rootParameters[2];
    rootParameters[0].InitAsConstantBufferView(0);
    rootParameters[1].InitAsDescriptorTable(1, &rangeSrv, D3D12_SHADER_VISIBILITY_VERTEX);

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;

    hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

    if (FAILED(hr)) {
        return;
    }

    hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));

    if (FAILED(hr)) {
        return;
    }

    // ===== 入力レイアウト =====
    // クアッド頂点（2D座標のみ）
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    // ===== ブレンド設定（アルファ合成）=====
    D3D12_BLEND_DESC blendDesc{};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    // ===== ラスタライザ設定 =====
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE; // 両面描画
    rasterizerDesc.DepthClipEnable = TRUE;

    // ===== 深度設定 =====
    D3D12_DEPTH_STENCIL_DESC depthDesc{};
    depthDesc.DepthEnable = TRUE;
    depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // 書き込み無効
    depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    // ===== PSO構築 =====
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.pRootSignature = rootSignature_.Get();
    psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
    psoDesc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };
    psoDesc.BlendState = blendDesc;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.RasterizerState = rasterizerDesc;
    psoDesc.DepthStencilState = depthDesc;
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = kRenderTargetFormat;
    psoDesc.DSVFormat = kDepthFormat;
    psoDesc.SampleDesc.Count = 1;

    device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_));
}

void GpuSmokeEmitter::CreateVertexBuffer_() {
    ID3D12Device* device = DirectXCommon::GetInstance()->GetDevice();
    if (!device) {
        return;
    }

    // 頂点データサイズ
    const UINT bufferSize = sizeof(kQuadVertices);

    // Uploadヒープ作成
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    if (FAILED(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBuffer_)))) {
        return;
    }

    // CPUから書き込み
    void* mapped = nullptr;
    vertexBuffer_->Map(0, nullptr, &mapped);
    std::memcpy(mapped, kQuadVertices, bufferSize);
    vertexBuffer_->Unmap(0, nullptr);

    // VBView設定
    vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = bufferSize;
    vertexBufferView_.StrideInBytes = sizeof(Vertex2D);
}

void GpuSmokeEmitter::CreateParticleBuffer_() {
    ID3D12Device* device = DirectXCommon::GetInstance()->GetDevice();
    if (!device) {
        return;
    }

    // バッファサイズ
    const UINT bufferSize = static_cast<UINT>(sizeof(SmokeParticle) * maxParticles_);

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    if (FAILED(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&particleBuffer_)))) {
        return;
    }

    // 永続マップ
    particleBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedParticles_));

    if (mappedParticles_) {
        std::memset(mappedParticles_, 0, bufferSize);
    }

    // ===== SRVヒープ生成 =====
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
    heapDesc.NumDescriptors = 1;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&srvHeap_));

    // ===== StructuredBuffer SRV作成 =====
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = maxParticles_;
    srvDesc.Buffer.StructureByteStride = sizeof(SmokeParticle);
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;

    device->CreateShaderResourceView(particleBuffer_.Get(), &srvDesc, srvHeap_->GetCPUDescriptorHandleForHeapStart());
}

void GpuSmokeEmitter::CreateConstantBuffer_() {
    ID3D12Device* device = DirectXCommon::GetInstance()->GetDevice();
    if (!device) {
        return;
    }

    // 256byte境界に切り上げ
    constantBufferSize_ = (sizeof(Constants) + 0xFF) & ~0xFF;

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize_);

    if (FAILED(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constantBuffer_)))) {
        return;
    }

    // 永続マップ
    constantBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedConstants_));

    if (mappedConstants_) {
        std::memset(mappedConstants_, 0, constantBufferSize_);
    }
}
