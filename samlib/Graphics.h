//
// Graphics.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include <SimpleMath.h>
#include "MSAAHelper.h"

using namespace DirectX::SimpleMath;

class Graphics final : public DX::IDeviceNotify
{
public:

    Graphics() noexcept(false);
    ~Graphics() = default;

    Graphics(Graphics&&) = default;
    Graphics& operator= (Graphics&&) = default;

    Graphics(Graphics const&) = delete;
    Graphics& operator= (Graphics const&) = delete;

    void Initialize(::IUnknown* window, int width, int height, DXGI_MODE_ROTATION rotation);

    void Tick();

    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnDisplayChange();
    void OnWindowSizeChanged(int width, int height, DXGI_MODE_ROTATION rotation);
    void ValidateDevice();

    void GetDefaultSize( int& width, int& height ) const noexcept;

    void BeginDrawing();
    void EndDrawing();

    void BeginMode2D();
    void EndMode2D();

    void DrawPixel(float x, float y, Color color);
    void DrawRectangle(float x, float y, float w, float h, Color color);
    void DrawText(std::wstring text, float x, float y, Color color);

    void DrawCube(float x, float y, float z, float width, float height, float length, Color color);

    void ClearBackground(Color color);
    void DrawGrid(size_t divisions, float spacing);

    DX::StepTimer m_timer;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    using VertexType = DirectX::VertexPositionColor;

    std::unique_ptr<DirectX::CommonStates> m_states;
    std::unique_ptr<DirectX::BasicEffect> m_effect;
    std::unique_ptr<DirectX::PrimitiveBatch<VertexType>> m_batch;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

    std::unique_ptr<DirectX::SpriteFont> m_font;

    DirectX::SimpleMath::Matrix m_world;
    DirectX::SimpleMath::Matrix m_view;
    DirectX::SimpleMath::Matrix m_proj;

    std::unique_ptr<DirectX::IEffectFactory> m_fxFactory;

    std::unique_ptr<DirectX::GeometricPrimitive> m_shape;

    std::unique_ptr<DX::MSAAHelper> m_msaaHelper;
};
