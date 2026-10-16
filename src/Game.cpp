#include "Game.hpp"

namespace game
{
void Game::init()
{
  // Initialize game resources
  Window::initializeGLFW();
  mWindow.initWindow();
}

void Game::gameplayLoop()
{
  while(!mWindow.getWindowShouldClose())
  {

  }
}

void Game::terminate()
{
  // Destroy game resources
  mWindow.terminateWindow();
  Window::terminateGLFW();
}

}
