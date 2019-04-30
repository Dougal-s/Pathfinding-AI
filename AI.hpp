#include <thread>
#include <vector>

#include <SFML/Graphics.hpp>

void setup(const int in_width, const int in_height);
void draw(sf::RenderWindow& window);
void update(std::vector<std::thread>& threads, int numThreads);
