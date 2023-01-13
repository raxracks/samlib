//
// Game.cpp
//

#include "pch.h"
#include "Graphics.h"
#include "Game.h"
#include "MSAAHelper.h"

extern void ExitGame() noexcept;

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Game game;

Graphics::Graphics() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>(
        DXGI_FORMAT_B8G8R8A8_UNORM,
        DXGI_FORMAT_UNKNOWN);
    m_msaaHelper = std::make_unique<DX::MSAAHelper>(
        m_deviceResources->GetBackBufferFormat(),
        DXGI_FORMAT_D32_FLOAT,
        4);
    m_deviceResources->RegisterDeviceNotify(this);
}

void Graphics::Initialize(::IUnknown* window, int width, int height, DXGI_MODE_ROTATION rotation)
{
    m_deviceResources->SetWindow(window, width, height, rotation);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    game = Game(this);
}

#pragma region Frame Update
void Graphics::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

void Graphics::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

    game.Update(elapsedTime);
}
#pragma endregion

#pragma region Frame Render
void Graphics::Render()
{
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    game.Render();
}

void Graphics::BeginDrawing() {
}

void Graphics::EndDrawing() {
    auto context = m_deviceResources->GetD3DDeviceContext();

    m_msaaHelper->Resolve(context, m_deviceResources->GetRenderTarget());
    m_deviceResources->Present();
}

void Graphics::BeginMode2D() {
    auto context = m_deviceResources->GetD3DDeviceContext();

    context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_states->DepthNone(), 0);
    context->RSSetState(m_states->CullNone());

    m_effect->SetWorld(m_world);

    m_effect->Apply(context);

    context->IASetInputLayout(m_inputLayout.Get());

    m_batch->Begin();
    m_spriteBatch->Begin();
}

void Graphics::EndMode2D() {
    m_batch->End();
    m_spriteBatch->End();
}

void Graphics::DrawPixel(float x, float y, Color color) {
    DrawRectangle(x, y, 1, 1, color);
}

void Graphics::DrawGrid(size_t divisions, float spacing) {
    Vector3 xaxis(spacing, 0.f, 0.f);
    Vector3 yaxis(0.f, 0.f, spacing);
    Vector3 origin = Vector3::Zero;

    for (size_t i = 0; i <= divisions; ++i)
    {
        float fPercent = float(i) / float(divisions);
        fPercent = (fPercent * 2.0f) - 1.0f;

        Vector3 scale = xaxis * fPercent + origin;

        VertexPositionColor v1(scale - yaxis, Colors::Gray);
        VertexPositionColor v2(scale + yaxis, Colors::Gray);
        m_batch->DrawLine(v1, v2);
    }

    for (size_t i = 0; i <= divisions; i++)
    {
        float fPercent = float(i) / float(divisions);
        fPercent = (fPercent * 2.0f) - 1.0f;

        Vector3 scale = yaxis * fPercent + origin;

        VertexPositionColor v1(scale - xaxis, Colors::Gray);
        VertexPositionColor v2(scale + xaxis, Colors::Gray);
        m_batch->DrawLine(v1, v2);
    }
}

void Graphics::DrawRectangle(float x, float y, float w, float h, Color color) {
    VertexPositionColor v1(Vector2(x,       y    ), color);
    VertexPositionColor v2(Vector2(x,       y + h), color);
    VertexPositionColor v3(Vector2(x + w,   y + h), color);
    VertexPositionColor v4(Vector2(x + w,   y    ), color);

    m_batch->DrawQuad(v1, v2, v3, v4);
}

void Graphics::DrawCube(float x, float y, float z, float width, float height, float length, Color color) {
    XMMATRIX local = m_world * Matrix::CreateTranslation(x, y, z) * Matrix::CreateScale(width, height, length);
    m_shape->Draw(local, m_view, m_proj, color);
}

void Graphics::DrawText(std::wstring text, float x, float y, Color color) {
    m_font->DrawString(m_spriteBatch.get(), text.c_str(),
        Vector2(x, y), color, 0.f, Vector2::Zero);
}

void Graphics::ClearBackground(Color color)
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    auto renderTarget = m_msaaHelper->GetMSAARenderTargetView();
    auto depthStencil = m_msaaHelper->GetMSAADepthStencilView();

    context->ClearRenderTargetView(renderTarget, color);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    auto const viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);
}
#pragma endregion

#pragma region Message Handlers
void Graphics::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Graphics::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Graphics::OnSuspending()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    context->ClearState();

    m_deviceResources->Trim();

    // TODO: Game is being power-suspended.
}

void Graphics::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed.
}

void Graphics::OnDisplayChange()
{
    m_deviceResources->UpdateColorSpace();
}

void Graphics::OnWindowSizeChanged(int width, int height, DXGI_MODE_ROTATION rotation)
{
    if (!m_deviceResources->WindowSizeChanged(width, height, rotation))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

void Graphics::ValidateDevice()
{
    m_deviceResources->ValidateDevice();
}

// Properties
void Graphics::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Graphics::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    m_states = std::make_unique<CommonStates>(device);

    m_effect = std::make_unique<BasicEffect>(device);
    m_effect->SetVertexColorEnabled(true);

    DX::ThrowIfFailed(
        CreateInputLayoutFromEffect<VertexType>(device, m_effect.get(),
            m_inputLayout.ReleaseAndGetAddressOf())
    );

    auto context = m_deviceResources->GetD3DDeviceContext();
    m_batch = std::make_unique<PrimitiveBatch<VertexType>>(context);
    m_spriteBatch = std::make_unique<SpriteBatch>(context);

    m_font = std::make_unique<SpriteFont>(device, L"Assets/font.spritefont");

    m_fxFactory = std::make_unique<EffectFactory>(device);

    m_world = Matrix::Identity;

    m_shape = GeometricPrimitive::CreateBox(context, Vector3(1, 1, 1));

    m_msaaHelper->SetDevice(device);
}

void Graphics::CreateWindowSizeDependentResources()
{
    auto size = m_deviceResources->GetOutputSize();

    Matrix proj = Matrix::CreateScale(2.f / float(size.right),
        -2.f / float(size.bottom), 1.f)
        * Matrix::CreateTranslation(-1.f, 1.f, 0.f);
    m_effect->SetProjection(proj);

    m_view = Matrix::CreateLookAt(Vector3(0.f, 10.f, 10.f),
        Vector3::Zero, Vector3::UnitY);
    m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f,
        float(size.right) / float(size.bottom), 0.1f, 1000.f);

    m_effect->SetView(m_view);
    m_effect->SetProjection(m_proj);

    m_msaaHelper->SetWindow(size);
}

void Graphics::OnDeviceLost()
{
    m_states.reset();
    m_effect.reset();
    m_batch.reset();
    m_inputLayout.Reset();
    m_font.reset();
    m_spriteBatch.reset();
    m_fxFactory.reset();
    m_shape.reset();
    m_msaaHelper->ReleaseDevice(); 
}

void Graphics::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
