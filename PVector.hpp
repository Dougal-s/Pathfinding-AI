class PVector {
public:
    float x;
    float y;

    PVector();
    PVector(float a, float b);

    static PVector fromAngle(float angle);
    void operator+=(PVector rhs);
    void limit(int max);
    float magSq();
    float dist(PVector v);

private:

};

float dist(PVector v1, PVector v2);
float distSq(PVector v1, PVector v2);
