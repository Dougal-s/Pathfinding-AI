#include <algorithm>
#include <thread>
#include <chrono>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <random>
#include <vector>
#include <complex>

#include <SFML/Graphics.hpp>

#include "AI.hpp"

typedef std::complex<float> Target;

int width = 800;
int height = 800;

constexpr int radius = 4;

constexpr int maxNumberSteps = 400;

std::mt19937 generator;

const float targetx = width/2;
const float targety = 50;

class Obstacle{
public:
    Obstacle() {}

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

    void draw(sf::RenderWindow& window) {
        window.draw(rectangle);
    }

    bool contains(std::complex<float> point) {
        return boundingBox.contains(sf::Vector2f(point.real(), point.imag()));
    }
private:
	sf::RectangleShape rectangle;
	sf::FloatRect boundingBox;
};

class Brain {
public:
    std::vector<std::complex<float>> Directions;
    int step = 0;

    void init(int size, bool random = true) {
        Directions.resize(size);
        if (random) {
            randomize();
        }
    }

    void randomize() {
        std::uniform_real_distribution<float> distribution(0.0, 2*M_PI);
		std::generate(Directions.begin(), Directions.end(), [&](){
			return std::polar(1.f, distribution(generator));
		});
    }

    Brain clone() {
        Brain clone;
        clone.init(Directions.size(), false);

        std::copy(Directions.begin(), Directions.end(), clone.Directions.begin());

        return clone;
    }

    void mutate() {
        std::uniform_real_distribution<float> mutationDist(0.0, 1.0);
        std::uniform_real_distribution<float> angleDist(0.0, 2*M_PI);
        float mutationRate = 0.01;

        for (int i = 0; i < Directions.size(); i++) {
            float rand = mutationDist(generator);
            if (rand < mutationRate) {
                Directions[i] = std::polar(1.f, angleDist(generator));
            }
        }
    }
private:

};

class Dot {
public:
    std::complex<float> pos, vel, acc;
    Brain brain;
    bool dead = false;

    bool reachedTarget = false;

    float fitness;

    Dot(bool randomizeDirections = true) {
        pos = {(float)width/2.0f, (float)height - 50.0f};
        vel = {0.0f, 0.0f};
        acc = {0.0f, 0.0f};

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
                for (int i = 0; i < obstacles.size(); i++) {
                    if (obstacles[i].contains(pos)) {
                        dead = true;
                    }
                }
            }
        }
    }

    void draw(sf::RenderWindow& window) {
        sf::CircleShape circle(radius);
        circle.setFillColor(sf::Color::Black);
        circle.setPosition(pos.real()-radius, pos.imag()-radius);
        window.draw(circle);
    }

    void calculateFitness(Target target) {
        if (reachedTarget) {
            fitness = 1.0f/16.0f + 10000.0f/(float)(brain.step * brain.step);
        } else {
            float distanceToTargetSq = std::norm(pos-target);
            fitness = 1.0f/distanceToTargetSq;
        }
    }

    Dot createChild() {
        Dot child;
        child.brain = brain.clone();
        return child;
    }

private:
    void move() {
        if (brain.Directions.size() > brain.step) {
            acc = brain.Directions[brain.step];
            brain.step++;
        } else {
            dead = true;
        }

        vel += acc;
        if (std::norm(vel) > 5*5) {
	        vel *= 5/sqrtf(std::norm(vel));
	    }
        pos += vel;
    }


};

class Population {
public:

    void initialize(int size) {
        dots.resize(size);
        target = Target{targetx, targety};
    }

    void initialize(int size, std::vector<Obstacle>& obstacleList) {
        dots.resize(size);
        target = Target{targetx, targety};

        obstacles.resize(obstacleList.size());
        std::copy(obstacleList.begin(), obstacleList.end(), obstacles.begin());
    }

    void replaceObstacle(int index, Obstacle& obstacle) {
        obstacles[index] = obstacle;
    }

    void update(int numThreads, int index) {
        for (int i = index; i < dots.size(); i+=numThreads) {
            if (dots[i].brain.step > minSteps) {
                dots[i].dead = true;
            } else {
                dots[i].update(target, obstacles);
            }
        }
    }

    void draw(sf::RenderWindow& window) {

        // Draw Obstacles
        for (int i = 0; i < obstacles.size(); i++) {
            obstacles[i].draw(window);
        }

        //Draw Dots
        sf::CircleShape circle(radius);
        circle.setFillColor(sf::Color::Black);
        if (gen != 0) {
            for (int i = 1; i < dots.size(); i++) {
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
        // Draw Targets
        circle.setRadius(2*radius);
        circle.setFillColor(sf::Color::Red);
        circle.setOutlineThickness(radius/2);
        circle.setOutlineColor(sf::Color::Black);
        circle.setPosition(target.real() - 2*radius, target.imag() - 2*radius);
        window.draw(circle);
    }

    bool allDotsDead() {
        return !std::any_of(dots.begin(), dots.end(), [](Dot dot){
            return !dot.dead && !dot.reachedTarget;
        });
    }

    void calculateFitness() {
        for (int i = 0; i < dots.size(); i++) {
            dots[i].calculateFitness(target);
        }
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

        dots = newDots;

        gen++;
    }

    void mutate() {
        for (int i = 2; i < dots.size(); i++) {
            dots[i].brain.mutate();
        }
    }

    void addObstacle(int w, int h, int x, int y) {
        obstacles.push_back(Obstacle(w, h, x, y));
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
    int minSteps = maxNumberSteps;

    void calculateFitnessSum() {
        fitnessSum = 0.f;
		for (const auto& dot : dots) {
			fitnessSum += dot.fitness;
		}
    }

    Dot selectParent() {
        std::uniform_real_distribution<float> distribution(0.0, fitnessSum);
        float rand = distribution(generator);
        float runningSum = 0;

        for (int i = 0; i < dots.size(); i++) {
            runningSum += dots[i].fitness;
            if (runningSum > rand)
                return dots[i];
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
            minSteps = dots[maxIndex].brain.step;
            std::cout << "step: " << std::setw(3) << std::left << minSteps;
        }
        std::cout << std::endl;

        bestDot = maxIndex;
    }

} population;

void setup(const int in_width, const int in_height) {
	width = in_width;
	height = in_height;

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    generator.seed(seed);

    std::vector<Obstacle> obstacles = {
        Obstacle(500, 50, width/2, height/2-150),
        Obstacle(300, 50, 150, height/2+140),
        Obstacle(300, 50, width-150, height/2+140),
        Obstacle(50, 75, 300-25, height/2+85),
        Obstacle(50, 75, width-300+25, height/2+85),
        Obstacle(50, 100, width/2-550/2+50, height/2-75),
        Obstacle(50, 100, width/2+550/2-50, height/2-75),
        Obstacle(250, 50, 125, height/2-300),
        Obstacle(250, 50, width-125, height/2-300),
        Obstacle(100, 25, width/2, 100),
        Obstacle(250, 40, width/2, height/2+260)
    };

    population.initialize(2000, obstacles);
}

void update(std::vector<std::thread>& threads, int numThreads) {
    if (population.allDotsDead()) {
        //genetic algorithm
        population.calculateFitness();
        population.selectNextGeneration();
        population.mutate();
    } else {
        for (int i = 0; i < numThreads; i++) {
            threads[i] = population.spawnUpdateThread(numThreads+1, i);
        }

        population.update(numThreads+1, numThreads);

        for (int i = 0; i < numThreads; i++) {
            threads[i].join();
        }
    }
}

void draw(sf::RenderWindow& window) {
    population.draw(window);
}
