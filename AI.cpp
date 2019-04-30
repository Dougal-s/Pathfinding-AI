#include <algorithm>
#include <thread>
#include <chrono>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <random>
#include <vector>
#include <complex>

#include <SFML/Graphics.hpp>

#include "AI.hpp"

typedef std::complex<float> Target;

namespace {
	int width = 800;
	int height = 800;

	constexpr int radius = 4;

	constexpr int maxNumberSteps = 400;

	std::mt19937 generator;

	const float targetx = width/2;
	const float targety = 50;
}

class Obstacle{
public:
	Obstacle() = default;

	Obstacle(int w, int h, int x, int y) {
		rectangle.setSize(sf::Vector2f(w, h));
		rectangle.setPosition(x-w/2, y-h/2);
		rectangle.setFillColor(sf::Color::Blue);
		boundingBox = rectangle.getGlobalBounds();
	}

	void init(int w, int h, int x, int y) {
		rectangle.setSize(sf::Vector2f(w, h));
		rectangle.setPosition(x-w/2, y-h/2);
		rectangle.setFillColor(sf::Color::Blue);
		boundingBox = rectangle.getGlobalBounds();
	}

	void draw(sf::RenderWindow& window) const {
		window.draw(rectangle);
	}

	bool contains(std::complex<float> point) const {
		return boundingBox.contains(sf::Vector2f(point.real(), point.imag()));
	}
private:
	sf::RectangleShape rectangle;
	sf::FloatRect boundingBox;
};

class Brain {
public:
	std::vector<std::complex<float>> directions;
	int step = 0;

	void init(int size, bool random = true) {
		directions.resize(size);
		if (random) randomize();
	}

	void randomize() {
		std::uniform_real_distribution<float> distribution(0.0, 2*M_PI);
		std::generate(directions.begin(), directions.end(), [&](){
			return std::polar(1.f, distribution(generator));
		});
	}

	Brain clone() {
		Brain clone;
		clone.init(directions.size(), false);

		std::copy(directions.begin(), directions.end(), clone.directions.begin());

		return clone;
	}

	void mutate() {
		std::uniform_real_distribution<float> mutationDist(0.0, 1.0);
		std::uniform_real_distribution<float> angleDist(0.0, 2*M_PI);
		float mutationRate = 0.01;

		for (auto& direction : directions) {
			if (mutationDist(generator) < mutationRate)
				direction = std::polar(1.f, angleDist(generator));
		}
	}
private:

};

class Dot {
public:
	std::complex<float> pos{(float)width/2.0f, (float)height - 50.0f};
	std::complex<float> vel{0.f, 0.f};
	std::complex<float> acc{0.f, 0.f};
	Brain brain;
	bool dead = false;

	bool reachedTarget = false;

	float fitness;

	Dot(bool randomizeDirections = true) {
		brain.init(maxNumberSteps, randomizeDirections);
	}

	void update(Target target, std::vector<Obstacle>& obstacles) {
		if (!dead && !reachedTarget) {
			move();
			if (pos.real() < radius || pos.imag() < radius || pos.real() > width-radius || pos.imag() > height-radius) {
				dead = true;
			} else if (std::abs(pos-target) < 3*radius) {
				reachedTarget = true;
			} else {
				for (const auto& obstacle : obstacles) {
					if (obstacle.contains(pos))
						dead = true;
				}
			}
		}
	}

	void calculateFitness(Target target) {
		if (reachedTarget)
			fitness = 1.0f/16.0f + 10000.0f/(float)(brain.step * brain.step);
		else
			fitness = 1.0f/std::norm(pos-target);
	}

	Dot createChild() {
		Dot child;
		child.brain = brain.clone();
		return child;
	}

private:
	void move() {
		if (brain.directions.size() > brain.step) {
			acc = brain.directions[brain.step];
			++brain.step;
		} else {
			dead = true;
		}

		vel += acc;
		if (std::norm(vel) > 5*5)
			vel *= 5/sqrtf(std::norm(vel));
		pos += vel;
	}
};

class Population::PopulationImpl {
public:

	void initialize(int size) {
		dots.resize(size);
		target = {targetx, targety};
	}

	void initialize(int size, const std::vector<Obstacle>& obstacleList) {
		dots.resize(size);
		target = {targetx, targety};

		obstacles.resize(obstacleList.size());
		std::copy(obstacleList.begin(), obstacleList.end(), obstacles.begin());
	}

	void replaceObstacle(int index, const Obstacle& obstacle) {
		obstacles[index] = obstacle;
	}

	void update(int numThreads, int index) {
		for (int i = index; i < dots.size(); i += numThreads) {
			if (dots[i].brain.step > maxSteps)
				dots[i].dead = true;
			else
				dots[i].update(target, obstacles);
		}
	}

	void draw(sf::RenderWindow& window) {
		// Draw Obstacles
		for (int i = 0; i < obstacles.size(); i++)
			obstacles[i].draw(window);

		//Draw Dots
		sf::CircleShape circle(radius);
		circle.setFillColor(sf::Color::Black);
		if (gen != 0) {
			for (int i = 2; i < dots.size(); i++) {
				circle.setPosition(dots[i].pos.real()-radius, dots[i].pos.imag()-radius);
				window.draw(circle);
			}
			// Draw Random
			circle.setOutlineThickness(radius/2);
			circle.setOutlineColor(sf::Color::Red);
			circle.setPosition(dots[1].pos.real()-radius, dots[1].pos.imag()-radius);
			window.draw(circle);
			// Draw champion
			circle.setFillColor(sf::Color::Green);
			circle.setOutlineThickness(0);
			circle.setPosition(dots[0].pos.real()-radius, dots[0].pos.imag()-radius);
			window.draw(circle);
		} else {
			for (int i = 0; i < dots.size(); i++) {
				circle.setPosition(dots[i].pos.real()-radius, dots[i].pos.imag()-radius);
				window.draw(circle);
			}
		}

		// Draw Target
		circle.setRadius(2*radius);
		circle.setFillColor(sf::Color::Red);
		circle.setOutlineThickness(radius/2);
		circle.setOutlineColor(sf::Color::Black);
		circle.setPosition(target.real() - 2*radius, target.imag() - 2*radius);
		window.draw(circle);
	}

	bool allDotsDead() {
		return !std::any_of(dots.begin(), dots.end(), [](const Dot& dot){
			return !dot.dead && !dot.reachedTarget;
		});
	}

	void calculateFitness() {
		for (auto& dot : dots)
			dot.calculateFitness(target);
	}

	void selectNextGeneration() {
		std::vector<Dot> newDots(dots.size());
		setBestDot();
		calculateFitnessSum();

		newDots[0] = dots[bestDot].createChild();
		// newDots[1] is random

		std::generate(newDots.begin()+2, newDots.end(), [=](){
			return selectParent().createChild();
		});

		dots = std::move(newDots);

		gen++;
	}

	void mutate() {
		for (int i = 2; i < dots.size(); i++)
			dots[i].brain.mutate();
	}

	void addObstacle(int w, int h, int x, int y) {
		obstacles.emplace_back(w, h, x, y);
	}

	std::thread spawnUpdateThread(int numThreads, int index) {
		return std::thread( [=] { update(numThreads, index); } );
	}

private:
	std::vector<Dot> dots;
	std::vector<Obstacle> obstacles;
	Target target;

	float fitnessSum;

	int gen = 0;
	int bestDot;
	int maxSteps = maxNumberSteps;

	void calculateFitnessSum() {
		fitnessSum = 0.f;
		for (const auto& dot : dots)
			fitnessSum += dot.fitness;
	}

	Dot selectParent() {
		std::uniform_real_distribution<float> distribution(0.0, fitnessSum);
		float rand = distribution(generator);
		float runningSum = 0;

		for (auto& dot : dots) {
			runningSum += dot.fitness;
			if (runningSum > rand) return dot;
		}

		return Dot();
	}

	void setBestDot() {
		float max = 0.0f;
		int maxIndex = 0;

		for (int i = 0; i < dots.size(); i++) {
			if (max < dots[i].fitness) {
				max = dots[i].fitness;
				maxIndex = i;
			}
		}

		std::cout << "Gen: " << std::setw(4) << std::left << gen << ' ';
		if (dots[maxIndex].reachedTarget) {
			maxSteps = dots[maxIndex].brain.step;
			std::cout << "step: " << std::setw(3) << std::left << maxSteps;
		}
		std::cout << std::endl;

		bestDot = maxIndex;
	}
};

Population::Population() {
	populationImpl = new PopulationImpl;
}

void Population::setup(const int inWidth, const int inHeight) {
	width = inWidth;
	height = inHeight;

	generator.seed(std::chrono::system_clock::now().time_since_epoch().count());

	std::vector<Obstacle> obstacles = {
		{500, 50, width/2, height/2-150},
		{300, 50, 150, height/2+140},
		{300, 50, width-150, height/2+140},
		{50, 75, 300-25, height/2+85},
		{50, 75, width-300+25, height/2+85},
		{50, 100, width/2-550/2+50, height/2-75},
		{50, 100, width/2+550/2-50, height/2-75},
		{250, 50, 125, height/2-300},
		{250, 50, width-125, height/2-300},
		{100, 25, width/2, 100},
		{250, 40, width/2, height/2+260}
	};

	populationImpl->initialize(2000, obstacles);
}

void Population::update(std::vector<std::thread>& threads, int numThreads) {
	if (populationImpl->allDotsDead()) {
		//genetic algorithm
		populationImpl->calculateFitness();
		populationImpl->selectNextGeneration();
		populationImpl->mutate();
	} else {
		for (int i = 0; i < numThreads; i++)
			threads[i] = populationImpl->spawnUpdateThread(numThreads+1, i);

		populationImpl->update(numThreads+1, numThreads);

		for (int i = 0; i < numThreads; i++)
			threads[i].join();
	}
}

void Population::draw(sf::RenderWindow& window) {
	populationImpl->draw(window);
}

Population::~Population() {
	delete populationImpl;
}
