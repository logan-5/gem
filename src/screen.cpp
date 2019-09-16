#include "screen.hpp"

#include <SFML/Graphics.hpp>

#include <array>

namespace gem {

struct Screen::Impl {
    explicit Impl() {
        std::array<u8, Screen::Width * Screen::Height * 4> texData;
        for (std::size_t i = 0; i < Screen::Width * Screen::Height; ++i) {
            texData[0u + i * 4u] = 0xFF;
            texData[1u + i * 4u] = 0x00;
            texData[2u + i * 4u] = 0xFF;
            texData[3u + i * 4u] = 0xFF;
        }
        sf::Image screenImage;
        screenImage.create(Screen::Width, Screen::Height, texData.data());
        screenTexture.loadFromImage(screenImage);
        screenSprite.setTexture(screenTexture);
        screenSprite.setScale(static_cast<float>(Window::Scale),
                              static_cast<float>(Window::Scale));
    }

    void renderLine(const std::array<u8, Screen::Width * 4>& line,
                    const unsigned y) {
        screenTexture.update(line.data(), Screen::Width, 1u, 0, y);
    }

    sf::Texture screenTexture;
    sf::Sprite screenSprite;
};

Screen::Screen(Window& window)
    : window{window}, impl{std::make_unique<Impl>()} {}
Screen::~Screen() = default;

void Screen::renderLine(const std::array<u8, Width * 4>& line,
                        const unsigned y) {
    impl->renderLine(line, y);
}

void Screen::vblank() {
    window.get().draw(*this);
}

struct Window::Impl {
    explicit Impl()
        : window{sf::VideoMode{Screen::Width * Scale, Screen::Height * Scale},
                 "gem"} {}

    bool isOpen() const { return window.isOpen(); }
    void processEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
    }

    void draw(const Screen& screen) {
        window.clear();
        window.draw(screen.getImpl().screenSprite);
        window.display();
    }

    sf::RenderWindow window;
};

Window::Window() : impl{std::make_unique<Impl>()} {}
Window::~Window() = default;

bool Window::isOpen() const {
    return impl->isOpen();
}
void Window::processEvents() {
    impl->processEvents();
}
void Window::draw(const Screen& screen) {
    impl->draw(screen);
}

}  // namespace gem
