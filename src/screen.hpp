#ifndef GEM_SCREEN_HPP
#define GEM_SCREEN_HPP

#include "fwd.hpp"

#include <memory>
#include <utility>

namespace gem {

struct Window;

struct Screen {
   public:
    struct Impl;

    static constexpr unsigned Width = 160, Height = 144;

    explicit Screen(Window& window);

    ~Screen();

    Impl& getImpl() const { return *impl; }

    void renderLine(const std::array<u8, Width * 4>& line, const unsigned y);

    void vblank();

   private:
    std::reference_wrapper<Window> window;
    std::unique_ptr<Impl> impl;
};

struct Window {
   public:
    static constexpr unsigned Scale = 6;

    explicit Window();
    ~Window();

    bool isOpen() const;

    void processEvents();

    void draw(const Screen& screen);

   private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

}  // namespace gem

#endif