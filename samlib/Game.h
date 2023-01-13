#pragma once

#include "Graphics.h"

class Game {
public:
	Game();
	Game(Graphics* graphics);
	void Update(float dt);
	void Render();
private:
	Graphics* m_graphics;
};