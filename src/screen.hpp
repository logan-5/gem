#ifndef GEM_SCREEN_HPP
#define GEM_SCREEN_HPP

#include "fwd.hpp"

#include <memory>

namespace gem {

struct Screen {
   public:
    struct Impl;

    static constexpr unsigned Width = 160, Height = 144;

    explicit Screen();

    ~Screen();

    Impl& getImpl() const { return *impl; }

    void renderLine(const std::array<u8, Width * 4>& line, const unsigned y);

   private:
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