#include <math.h>

#include "PVector.hpp"

PVector::PVector() {};

PVector::PVector(float a, float b) {
    x = a;
    y = b;
};

PVector PVector::fromAngle(float angle) {
    return PVector(cosf(angle), sinf(angle));
}

void PVector::operator+=(PVector rhs) {
    x += rhs.x;
    y += rhs.y;
}

void PVector::limit(int max) {
    if (magSq() > max*max) {
        x *= max/sqrtf(magSq());
        y *= max/sqrtf(magSq());
    }
}

float PVector::magSq() {
    return x*x+y*y;
}

float PVector::dist(PVector v) {
    float dx = x - v.x;
    float dy = y - v.y;
    return sqrtf(dx*dx + dy*dy);
 }

 float dist(PVector v1, PVector v2) {
    float dx = v1.x - v2.x;
    float dy = v1.y - v2.y;
    return sqrtf(dx*dx + dy*dy);
}

float distSq(PVector v1, PVector v2) {
   float dx = v1.x - v2.x;
   float dy = v1.y - v2.y;
   return dx*dx + dy*dy;
}
