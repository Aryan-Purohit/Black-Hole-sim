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
    const int W = 800, H = 400;
    sf::RenderWindow window(sf::VideoMode(W, H), "Black Hole - Press 'S' to Save");
    sf::Texture texture;
    texture.create(W, H);
    sf::Sprite sprite(texture);
    std::vector<sf::Uint8> pixels(W * H * 4);

    double time = 0;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) { 
            if (event.type == sf::Event::Closed) window.close(); 
            // Save Screenshot logic
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::S) {
                sf::Image screenshot = texture.copyToImage();
                if (screenshot.saveToFile("black_hole_render.png")) {
                    std::cout << "Screenshot saved to black_hole_render.png!" << std::endl;
                }
            }
        }

        time += 0.05;
        double tilt = 0.5; // Adjusted tilt
        double camDist = 20.0;
        Vec3 camPos = {0.0, camDist * sin(tilt), -camDist * cos(tilt)};

        #pragma omp parallel for schedule(dynamic)
        for (int j = 0; j < H; j++) {
            for (int i = 0; i < W; i++) {
                double u = (double(i) - W/2.0) / (W/2.0);
                double v = (double(j) - H/2.0) / (W/2.0);
                
                Vec3 pos = camPos;
                // Fixed ray direction for the tilted camera
                Vec3 vel = {u * 1.5, -v - 0.5, 1.5}; 
                double v_mag = std::sqrt(vel.mag2());
                vel = {vel.x/v_mag, vel.y/v_mag, vel.z/v_mag};

                double r = 0, g = 0, b = 0;
                double glow = 0;
                bool hitSomething = false;

                for (int step = 0; step < 250; step++) {
                    double r2 = pos.mag2();
                    double d = std::sqrt(r2);
                    
                    if (r2 < 1.2) { hitSomething = true; break; } // Hit Event Horizon

                    // NEW STAR LOGIC: Based on iteration count or "escaping"
                    if (step == 249 || r2 > 1000.0) { 
                        // Direction-based stars
                        double starX = vel.x, starY = vel.y, starZ = vel.z;
                        double seed = sin(starX * 100.0 + starY * 100.0 + starZ * 100.0) * 43758.5;
                        if (seed - floor(seed) > 0.998) r = g = b = 220;
                        hitSomething = true;
                        break;
                    }

                    vel = vel + pos * (-1.5 / (r2 * d) * 0.18);
                    pos = pos + vel * 0.18;

                    double diskDist = std::abs(pos.y);
                    if (d > 2.5 && d < 8.0) {
                        glow += 0.15 / (0.2 + diskDist * diskDist); 
                        if (diskDist < 0.1) {
                            double angle = atan2(pos.z, pos.x);
                            double swirl = sin(angle * 4.0 + time * 2.0);
                            double doppler = 1.0 + (vel.x); // Relative to ray direction
                            
                            r = 255 * doppler * (swirl > 0 ? 1.0 : 0.6);
                            g = 140 * doppler;
                            b = 40 * doppler;
                            hitSomething = true;
                            break; 
                        }
                    }
                }

                int idx = (j * W + i) * 4;
                pixels[idx]   = std::min(255.0, r + glow * 35);
                pixels[idx+1] = std::min(255.0, g + glow * 15);
                pixels[idx+2] = std::min(255.0, b + glow * 5);  
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