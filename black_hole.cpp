#include <iostream>
#include <vector>
#include <cmath>

struct Vector3 {
    double x, y, z;
    
    Vector3 operator+(const Vector3& v) const { return {x + v.x, y + v.y, z + v.z}; }
    Vector3 operator*(double s) const { return {x * s, y * s, z * s}; }
    double magSquared() const { return x*x + y*y + z*z; }
};

// Simulation Parameters
const double rs = 1.0;      // Schwarzschild radius
const double dt = 0.1;      // Time step
const int maxSteps = 500;   // Max iterations per ray

bool simulateRay(Vector3 pos, Vector3 vel) {
    for (int i = 0; i < maxSteps; ++i) {
        double r2 = pos.magSquared();
        double r = std::sqrt(r2);

        if (r < rs * 1.01) return true; // Ray hit the Event Horizon
        if (r > 50.0) return false;    // Ray escaped to space

        // Gravity "bending" logic
        // We apply a force proportional to 1/r^3 to simulate light bending
        double bendingForce = (1.5 * rs) / (r2 * r); 
        Vector3 acceleration = pos * (-bendingForce);

        vel = vel + acceleration * dt;
        pos = pos + vel * dt;
    }
    return false;
}

int main() {
    std::cout << "Black Hole Ray Tracer Initialized..." << std::endl;
    // Example: Fire a ray from x=-10, slightly offset on y
    Vector3 cameraPos = {-10.0, 0.5, 0.0};
    Vector3 direction = {1.0, 0.0, 0.0};

    if (simulateRay(cameraPos, direction)) {
        std::cout << "The light ray was captured by the black hole!" << std::endl;
    } else {
        std::cout << "The light ray escaped to infinity." << std::endl;
    }

    return 0;
}