#include "Game.hpp"

#include "Logger.hpp"

namespace game
{
void Game::init()
{
  // Initialize game resources
  Logger::init();
  Window::initializeGLFW();
  mWindow.initWindow();
  mRenderer.initRenderer(); 
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
  Logger::shutdown();
}

}
