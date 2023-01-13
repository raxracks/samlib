#include "pch.h"

#include "Game.h"
#include <Graphics.h>
#include <SimpleMath.h>

using namespace DirectX;

Game::Game() {

}

Game::Game(Graphics* graphics) {
	m_graphics = graphics;
}

void Game::Update(float dt) {

}

void Game::Render() {
	m_graphics->BeginDrawing();
		m_graphics->ClearBackground(Colors::White.v);

		m_graphics->BeginMode2D();

			m_graphics->DrawGrid(10, 5.0f);

		m_graphics->EndMode2D();

		m_graphics->DrawCube(0, 0, 0, 2, 2, 2, Colors::Red.v);
		
		m_graphics->BeginMode2D();

			m_graphics->DrawText(std::to_wstring(m_graphics->m_timer.GetFramesPerSecond()) + L" FPS", 10, 10, Colors::Green.v);
		
		m_graphics->EndMode2D();

	m_graphics->EndDrawing();
}