#include <thread>
#include <vector>

#include <SFML/Graphics.hpp>

class Population {
public:
	Population();
	void setup(const int in_width, const int in_height);
	void draw(sf::RenderWindow& window);
	void update(std::vector<std::thread>& threads, int numThreads);
	~Population();
private:
	class PopulationImpl;
	PopulationImpl* populationImpl;
};
