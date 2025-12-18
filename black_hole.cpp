#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

struct Vec3 {
    double x, y, z;
    Vec3 operator+(const Vec3& v) const { return {x+v.x, y+v.y, z+v.z}; }
    Vec3 operator-(const Vec3& v) const { return {x-v.x, y-v.y, z-v.z}; }
    Vec3 operator*(double s) const { return {x*s, y*s, z*s}; }
    double dot(const Vec3& v) const { return x*v.x + y*v.y + z*v.z; }
    double mag2() const { return x*x + y*y + z*z; }
    Vec3 norm() const { double m = std::sqrt(mag2()); return {x/m, y/m, z/m}; }
};

int main() {
    const int width = 1920, height = 1080;
    const double rs = 1.0;
    const double dt = 0.1;

    std::ofstream out("black_hole.ppm");
    out << "P3\n" << width << " " << height << "\n255\n";

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            // Camera setup: rays moving toward the +X direction
            Vec3 pos = {-15.0, (double(height)/2.0 - j)/20.0, (double(i) - width/2.0)/20.0};
            Vec3 vel = {1.0, 0.0, 0.0}; 
            
            int r = 0, g = 0, b = 0; // Default background color (Black)

            for (int step = 0; step < 300; step++) {
                double r2 = pos.mag2();
                if (r2 < rs * rs) break; // Hit event horizon (remains black)
                if (r2 > 400.0) {        
                    // Use the final direction of the ray to "pick" a star
                    double starDensity = sin(pos.y * 10.0) * cos(pos.z * 10.0);
                    if (starDensity > 0.98) { // Only the "peaks" of the math function become stars
                        r = g = b = 200; 
                    } else {
                        r = g = b = 0;
                    }
                    break; 
                }

                // Gravity: Accel = -1.5 * rs * L^2 / r^5
                double dist = std::sqrt(r2);
                double accel = -1.5 * rs / (dist * dist * dist);
                vel = vel + pos * (accel * dt);
                pos = pos + vel * dt;

                // Check for Accretion Disk (plane at y=0)
                if (std::abs(pos.y) < 0.08 && dist > 2.2 * rs && dist < 7.0 * rs) {
                    double intensity = 1.0 / (dist - rs); 
                    if (intensity > 1.0) intensity = 1.0;

                    // --- NEW: Relativistic Beaming Approximation ---
                    // pos.z represents the left/right side of the disk.
                    // We'll use it to brighten one side and dim the other.
                    double doppler = 1.0 + (pos.z / dist); // Simplified boost factor
                    intensity *= (doppler * doppler);      // Square it for a stronger effect
                    
                    double shift = (dist - 2.0 * rs) / 5.0; 
                    if (shift < 0) shift = 0;

                    r = static_cast<int>(std::min(255.0, 255 * intensity));
                    g = static_cast<int>(std::min(255.0, 180 * intensity * shift * doppler));
                    b = static_cast<int>(std::min(255.0, 100 * intensity * shift * shift * doppler));
                    break;
                }
            }
            out << r << " " << g << " " << b << " ";
        }
        out << "\n";
    }
    std::cout << "Render complete: black_hole.ppm" << std::endl;
    return 0;
}