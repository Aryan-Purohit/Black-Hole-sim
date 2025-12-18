#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <algorithm>
#include <omp.h>
#include <iostream>

struct Vec3 {
    double x, y, z;
    Vec3 operator+(const Vec3& v) const { return {x+v.x, y+v.y, z+v.z}; }
    Vec3 operator*(double s) const { return {x*s, y*s, z*s}; }
    double mag2() const { return x*x + y*y + z*z; }
};

int main() {
    const int W = 1920, H = 1080;
    sf::RenderWindow window(sf::VideoMode(W, H), "Black Hole: Nebula & Gravitational Redshift");
    sf::Texture texture;
    texture.create(W, H);
    sf::Sprite sprite(texture);
    std::vector<sf::Uint8> pixels(W * H * 4);

    double time = 0;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) { 
            if (event.type == sf::Event::Closed) window.close(); 
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::S) {
                texture.copyToImage().saveToFile("black_hole_nebula.png");
                std::cout << "Cinematic shot saved!" << std::endl;
            }
        }

        time += 0.03;
        double tilt = 0.4; 
        Vec3 camPos = {0.0, 7.0, -20.0}; 

        #pragma omp parallel for schedule(dynamic)
        for (int j = 0; j < H; j++) {
            for (int i = 0; i < W; i++) {
                double aspect = (double)W / H;
                double u = (2.0 * i / W - 1.0) * aspect;
                double v = (2.0 * j / H - 1.0);
                
                Vec3 pos = camPos;
                Vec3 vel = {u, v - 0.3, 1.8}; 
                double v_mag = std::sqrt(vel.mag2());
                vel = {vel.x/v_mag, vel.y/v_mag, vel.z/v_mag};

                double r = 0, g = 0, b = 0;
                double glow = 0;
                double nebula = 0;
                double closestDist = 100.0;

                for (int step = 0; step < 240; step++) {
                    double r2 = pos.mag2();
                    double d = std::sqrt(r2);
                    if (d < closestDist) closestDist = d;
                    
                    if (d < 1.15) break; // Event Horizon

                    // 1. NEBULA CALCULATION: Rays pick up color from "gas" clouds
                    // We use a math function to create wispy blue/purple shapes
                    //double gas = (sin(pos.x * 0.2) + cos(pos.z * 0.2 + time * 0.5)) * 0.5;
                    //if (gas > 0.4) nebula += (gas - 0.4) * 0.05;

                    vel = vel + pos * (-1.5 / (r2 * d) * 0.2);
                    pos = pos + vel * 0.2;

                    double diskDist = std::abs(pos.y);
                    if (d > 2.4 && d < 9.0) {
                        glow += 0.12 / (0.2 + diskDist * diskDist * 6.0); 

                        if (diskDist < 0.04) {
                            // 1. Physical Rotation: Rotate the coordinates based on time
                            // This makes the entire disk structure spin, not just the colors.
                            double spinSpeed = 5.0; // Increase this for faster rotation
                            double angle = atan2(pos.z, pos.x) + (time * spinSpeed);
                            
                            // 2. Multi-arm Swirl: Create the "arms" of the accretion disk
                            double arms = 4.0; 
                            double swirl = sin(angle * arms + d * 2.0); // d * 2.0 adds a spiral curve
                            
                            // 3. Relativistic Beaming (Doppler)
                            // We use the ray's velocity to determine if it's hitting the "incoming" side
                            double doppler = 1.0 + (vel.x * 0.9);
                            
                            // 4. Color Assignment
                            double brightness = (swirl > 0 ? 1.2 : 0.5); 
                            r = 255 * doppler * brightness;
                            g = 140 * doppler * brightness;
                            b = 20 * doppler * brightness;
                            break; 
                        }
                    }
                    if (r2 > 1200.0) break;
                }

                // 2. STARFIELD WITH DYNAMIC REDSHIFT
                if (r == 0 && g == 0 && b == 0) {
                    double seed = sin(vel.x * 120.0 + vel.y * 120.0 + vel.z * 120.0) * 43758.5;
                    if (seed - floor(seed) > 0.997) {
                        // Closer pass = Redder stars (Longer wavelength)
                        // Farther pass = Bluer stars (Shorter wavelength)
                        double zShift = std::min(1.0, (closestDist - 1.2) / 6.0);
                        r = 255; 
                        g = 150 + 105 * zShift; 
                        b = 100 + 155 * zShift;
                    }
                }

                int idx = (j * W + i) * 4;
                // Combine colors: Base + Disk Glow + Purple Nebula
                pixels[idx]   = std::min(255.0, r + glow * 50 + nebula * 100); 
                pixels[idx+1] = std::min(255.0, g + glow * 20); 
                pixels[idx+2] = std::min(255.0, b + glow * 10 + nebula * 255); // Purple/Blue shift
                pixels[idx+3] = 255;
            }
        }

        texture.update(pixels.data());
        window.clear();
        window.draw(sprite);
        window.display();
    }
    return 0;
}