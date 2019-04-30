#include <vector>
#include <thread>
#include <iostream>

#include <SFML/Graphics.hpp>

#include "AI.hpp"

constexpr int width = 800;
constexpr int height = 800;

class DotGame {
public:
	void run() {
		init();
		mainLoop();
		cleanup();
	}
private:
	sf::RenderWindow window;
	std::vector<std::thread> threads;

	void init() {
		threads.resize(std::thread::hardware_concurrency());

		sf::ContextSettings settings;
    	settings.antialiasingLevel = 8;

		window.create(sf::VideoMode(width, height), "Dot Game", sf::Style::Default, settings);
		window.setVerticalSyncEnabled(true);

		setup(width, height);
	}

	void mainLoop() {

		while (window.isOpen()) {
			sf::Event event;
			while (window.pollEvent(event)) {
				if (event.type == sf::Event::Closed)
					window.close();
			}

			update(threads, threads.size());

			window.clear(sf::Color::White);
			draw(window);
			window.display();
		}
	}

	void cleanup() {

	}
};

int main(int argc, char *argv[]) {
	DotGame app;

	try {
		app.run();
	} catch (const char * error) {
		std::cerr << error << std::endl;
	}

	return 0;
}
