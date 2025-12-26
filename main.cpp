#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <algorithm>

const float PI = 3.14159265358979323846f;
const float TAU = 2.0f * PI;
const int WIDTH = 800;
const int HEIGHT = 600;

struct Sphere {
    sf::Vector3f position;
    float radius;
    sf::Color color;
};

class SphereRenderer {
private:
    sf::RenderWindow window;
    float f;
    sf::Clock clock;
    sf::Font font;
    sf::Text infoText;

    std::vector<Sphere> spheres;

    // Camera parameters
    float cameraDistance;
    float cameraAngleX, cameraAngleY;
    sf::Vector2f lastMousePos;

public:
    SphereRenderer() : window(sf::VideoMode(WIDTH, HEIGHT), "Flower Flip Effect - SFML"),
                       f(0.0f), cameraDistance(300.0f), cameraAngleX(0.3f), cameraAngleY(0.3f) {
        window.setFramerateLimit(60);

        if (!font.loadFromFile("arial.ttf")) {
            font = sf::Font();
        }

        infoText.setFont(font);
        infoText.setCharacterSize(14);
        infoText.setFillColor(sf::Color::White);
        infoText.setPosition(10, 10);

        generateSpheres();
    }

    void generateSpheres() {
        spheres.clear();

        int numPetals = 12; // 12 cánh
        int numStrands = 3; // 3 sợi mỗi cánh
        float petalAngle = TAU / numPetals;

        for (int petal = 0; petal < numPetals; petal++) {
            float baseAngle = petal * petalAngle;

            for (int strand = 0; strand < numStrands; strand++) {
                // Tạo các điểm dọc theo mỗi sợi với hiệu ứng lộn
                for (int point = 0; point < 25; point++) {
                    Sphere sphere;

                    float t = static_cast<float>(point) / 24.0f; // 0 đến 1

                    // HIỆU ỨNG LỘN - quan trọng nhất
                    float V = t * PI + f; // Thay đổi theo thời gian để tạo hiệu ứng lộn
                    float U = baseAngle + (strand - 1) * 0.2f; // Mỗi sợi hơi lệch góc

                    // Bán kính thay đổi theo hiệu ứng sóng
                    float r = 120.0f * (0.8f + 0.2f * sin(f * 2.0f + petal * 0.3f));

                    // Tính toán vị trí với hiệu ứng lộn
                    sphere.position.x = sin(U) * sin(V) * r;
                    sphere.position.y = cos(V) * r;
                    sphere.position.z = cos(U) * sin(V) * r;

                    // Kích thước có hiệu ứng sóng
                    sphere.radius = 5.0f + 3.0f * sin(U * 8.0f + f * 4.0f) * cos(V * 6.0f + f * 3.0f);

                    // Màu sắc theo cánh và thời gian
                    float red = 0.5f + 0.5f * sin(f + U);
                    float green = 0.5f + 0.5f * sin(f + V + PI/3);
                    float blue = 0.5f + 0.5f * sin(f + U + V + 2*PI/3);

                    sphere.color = sf::Color(
                        static_cast<sf::Uint8>(red * 255),
                        static_cast<sf::Uint8>(green * 255),
                        static_cast<sf::Uint8>(blue * 255)
                    );

                    spheres.push_back(sphere);
                }
            }
        }
    }

    void run() {
        while (window.isOpen()) {
            handleEvents();
            update();
            render();
        }
    }

private:
    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape)
                    window.close();
                else if (event.key.code == sf::Keyboard::R) {
                    cameraAngleX = 0.3f;
                    cameraAngleY = 0.3f;
                    cameraDistance = 300.0f;
                }
            }
            else if (event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.delta > 0)
                    cameraDistance -= 20.0f;
                else
                    cameraDistance += 20.0f;
                cameraDistance = std::max(100.0f, std::min(1000.0f, cameraDistance));
            }
            else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    lastMousePos = sf::Vector2f(
                        static_cast<float>(event.mouseButton.x),
                        static_cast<float>(event.mouseButton.y)
                    );
                }
            }
            else if (event.type == sf::Event::MouseMoved) {
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                    sf::Vector2f currentMousePos(
                        static_cast<float>(event.mouseMove.x),
                        static_cast<float>(event.mouseMove.y)
                    );

                    sf::Vector2f delta = currentMousePos - lastMousePos;
                    cameraAngleY += delta.x * 0.01f;
                    cameraAngleX += delta.y * 0.01f;

                    lastMousePos = currentMousePos;
                }
            }
        }
    }

    void update() {
        f += PI / 90.0f; // Tốc độ lộn nhanh hơn
        if (f > TAU) f -= TAU;

        generateSpheres();

        std::string info = "Flower Flip Effect - SFML\n";
        info += "12 petals flipping like bubbles\n";
        info += "Controls:\n";
        info += "- Mouse drag: Rotate camera\n";
        info += "- Mouse wheel: Zoom in/out\n";
        info += "- R: Reset camera\n";
        info += "- ESC: Exit";
        infoText.setString(info);
    }

    void render() {
        window.clear(sf::Color::Black);

        // Draw spheres với sắp xếp theo độ sâu
        std::vector<std::pair<float, const Sphere*>> sortedSpheres;

        for (const auto& sphere : spheres) {
            sf::Vector3f rotated = rotate3D(sphere.position);
            sortedSpheres.push_back(std::make_pair(rotated.z, &sphere));
        }

        std::sort(sortedSpheres.begin(), sortedSpheres.end(),
                  [](const std::pair<float, const Sphere*>& a,
                     const std::pair<float, const Sphere*>& b) {
                      return a.first > b.first;
                  });

        for (const auto& spherePair : sortedSpheres) {
            const Sphere& sphere = *spherePair.second;
            sf::Vector3f rotated = rotate3D(sphere.position);
            sf::Vector2f projected = project3D(rotated);

            float scale = 1.0f - (rotated.z + cameraDistance) / (2.0f * cameraDistance);
            scale = std::max(0.1f, scale);

            float displayRadius = sphere.radius * scale;

            if (displayRadius > 0.5f &&
                projected.x >= -displayRadius && projected.x <= WIDTH + displayRadius &&
                projected.y >= -displayRadius && projected.y <= HEIGHT + displayRadius) {

                sf::CircleShape circle(displayRadius);
                circle.setPosition(projected.x - displayRadius, projected.y - displayRadius);
                circle.setFillColor(sphere.color);

                window.draw(circle);
            }
        }

        window.draw(infoText);
        window.display();
    }

    sf::Vector3f rotate3D(const sf::Vector3f& point) {
        sf::Vector3f result = point;

        float tempX = result.x * cos(cameraAngleY) - result.z * sin(cameraAngleY);
        float tempZ = result.x * sin(cameraAngleY) + result.z * cos(cameraAngleY);
        result.x = tempX;
        result.z = tempZ;

        float tempY = result.y * cos(cameraAngleX) - result.z * sin(cameraAngleX);
        tempZ = result.y * sin(cameraAngleX) + result.z * cos(cameraAngleX);
        result.y = tempY;
        result.z = tempZ;

        return result;
    }

    sf::Vector2f project3D(const sf::Vector3f& point) {
        float scale = cameraDistance / (cameraDistance + point.z);
        return sf::Vector2f(
            point.x * scale + WIDTH / 2.0f,
            point.y * scale + HEIGHT / 2.0f
        );
    }
};

int main() {
    SphereRenderer renderer;
    renderer.run();
    return 0;
}
