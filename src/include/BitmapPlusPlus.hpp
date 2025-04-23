#pragma once

#include <fstream>    // std::*fstream
#include <vector>     // std::vector
#include <memory>     // std::unique_ptr
#include <algorithm>  // std::fill
#include <cstdint>    // std::int*_t
#include <cstddef>    // std::size_t
#include <string>     // std::string
#include <cstring>    // std::memcmp
#include <filesystem> // std::filesystem::path
#include <stdexcept>  // std::runtime_error
#include <utility>    // std::exchange

namespace bmp {
// Magic number for Bitmap .bmp 24 bpp files (24/8 = 3 = rgb colors only)
static constexpr std::uint16_t BITMAP_BUFFER_MAGIC = 0x4D42;

#pragma pack(push, 1)
struct Pixel {
  unsigned char r; /* Blue value */
  unsigned char g; /* Green value */
  unsigned char b; /* Red value */
};

Pixel makePixel(const std::int32_t rgb);

static_assert(sizeof(Pixel) == 3, "Bitmap Pixel size must be 3 bytes");
#pragma pack(pop)

static Pixel Aqua = {0, 255, 255};
static Pixel Beige = {245, 245, 220};
static Pixel Black = {0, 0, 0};
static Pixel Blue = {0, 0, 255};
static Pixel Brown = {165, 42, 42};
static Pixel Chocolate = {210, 105, 30};
static Pixel Coral = {255, 127, 80};
static Pixel Crimson = {220, 20, 60};
static Pixel Cyan = {0, 255, 255};
static Pixel Firebrick = {178, 34, 34};
static Pixel Gold = {255, 215, 0};
static Pixel Gray = {128, 128, 128};
static Pixel Green = {0, 255, 0};
static Pixel Indigo = {75, 0, 130};
static Pixel Lavender = {230, 230, 250};
static Pixel Lime = {0, 255, 0};
static Pixel Magenta = {255, 0, 255};
static Pixel Maroon = {128, 0, 0};
static Pixel Navy = {0, 0, 128};
static Pixel Olive = {128, 128, 0};
static Pixel Orange = {255, 165, 0};
static Pixel Pink = {255, 192, 203};
static Pixel Purple = {128, 0, 128};
static Pixel Red = {255, 0, 0};
static Pixel Salmon = {250, 128, 114};
static Pixel Silver = {192, 192, 192};
static Pixel Snow = {255, 250, 250};
static Pixel Teal = {0, 128, 128};
static Pixel Tomato = {255, 99, 71};
static Pixel Turquoise = {64, 224, 208};
static Pixel Violet = {238, 130, 238};
static Pixel White = {255, 255, 255};
static Pixel Wheat = {245, 222, 179};
static Pixel Yellow = {255, 255, 0};

class Exception : public std::runtime_error {
 public:
  explicit Exception(const std::string &message) : std::runtime_error(message) {
  }
};

class Bitmap {
 public:
  Bitmap() noexcept;

  explicit Bitmap(const char* filename);

  Bitmap(const std::int32_t width, const std::int32_t height);

  Bitmap(const Bitmap &other) = default; // Copy Constructor

  Bitmap(Bitmap &&other) noexcept;

  virtual ~Bitmap() noexcept = default;

 public: /* Draw Primitives */
  /**
     * Draw a line form (x1, y1) to (x2, y2)
     */
  void draw_line(std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2, const Pixel color);

  /**
     * Draw a filled rect
     */
  void fill_rect(const std::int32_t x, const std::int32_t y, const std::int32_t width, const std::int32_t height,
                 const Pixel color);

  /**
     * Draw a rect (not filled, border only)
     */
  void draw_rect(const std::int32_t x, const std::int32_t y, const std::int32_t width, const std::int32_t height,
                 const Pixel color);


  /**
     * Draw a triangle (not filled, border only)
     */
  void draw_triangle(const std::int32_t x1, const std::int32_t y1,
                     const std::int32_t x2, const std::int32_t y2,
                     const std::int32_t x3, const std::int32_t y3,
                     const Pixel color);

  /**
     * Draw a filled triangle
     */
  void fill_triangle(const std::int32_t x1, const std::int32_t y1,
                     const std::int32_t x2, const std::int32_t y2,
                     const std::int32_t x3, const std::int32_t y3,
                     const Pixel color);

  /**
     * Draw a circle with a given center and radius
     */
  void draw_circle(const std::int32_t center_x, const std::int32_t center_y, const std::int32_t radius,
                   const Pixel color);

  /**
     * Fill a circle with a given center and radius
     */
  void fill_circle(const std::int32_t center_x, const std::int32_t center_y, const std::int32_t radius,
                   const Pixel color);

 public: /* Accessors */
  /**
     *	Get pixel at position x,y
     */
  Pixel &get(const std::int32_t x, const std::int32_t y);

  /**
     *	Get const pixel at position x,y
     */
  [[nodiscard]] const Pixel &get(const std::int32_t x, const std::int32_t y) const;

  /**
     *	Returns the width of the Bitmap image
     */
  [[nodiscard]] std::int32_t width() const noexcept;

  /**
     *	Returns the height of the Bitmap image
     */
  [[nodiscard]] std::int32_t height() const noexcept;

  /**
     *	Clears Bitmap pixels with an rgb color
     */
  void clear(const Pixel pixel = Black);

 public: /* Operators */
  const Pixel &operator[](const std::size_t i) const;

  Pixel &operator[](const std::size_t i);

  bool operator!() const noexcept;

  explicit operator bool() const noexcept;

  bool operator==(const Bitmap &image) const;

  bool operator!=(const Bitmap &image) const;

  std::uint32_t size() const;

  Bitmap &operator=(const Bitmap &image);

  Bitmap &operator=(Bitmap &&image) noexcept;

 public: /* Modifiers */
  /**
     *	Sets rgb color to pixel at position x,y
     *   @throws bmp::Exception on error
     */
  void set(const std::int32_t x, const std::int32_t y, const Pixel color);


  /**
    *	Vertically flips the bitmap and returns the flipped version
    *
    */
  [[nodiscard("Bitmap::flip_v() is immutable")]]
  Bitmap flip_v() const;

  /**
    *	Horizontally flips the bitmap and returns the flipped version
    *
    */
  [[nodiscard("Bitmap::flip_h() is immutable")]]
  Bitmap flip_h() const;

  /**
    *	Rotates the bitmap to the right and returns the rotated version
    *
    */
  [[nodiscard("Bitmap::rotate_90_left() is immutable")]]
  Bitmap rotate_90_left() const;

  /**
    *	Rotates the bitmap to the left and returns the rotated version
    *
    */
  [[nodiscard("Bitmap::rotate_90_right() is immutable")]]
  Bitmap rotate_90_right() const;

  /**
     *	Saves Bitmap pixels into a file
     *   @throws bmp::Exception on error
     */
  void save(const char* path) const;

  /**
     *	Loads Bitmap from file
     *   @throws bmp::Exception on error
     */
  void load(const char* path);

 private: /* Utils */
  /**
     *	Converts 2D x,y coords into 1D index
     */
  [[nodiscard]] constexpr std::size_t IX(const std::int32_t x, const std::int32_t y) const noexcept;
  /**
     *	Returns true if x,y coords are within boundaries
     */
  [[nodiscard]] constexpr bool in_bounds(const std::int32_t x, const std::int32_t y) const noexcept;

 private:
  std::vector<Pixel> m_pixels;
  std::int32_t m_width;
  std::int32_t m_height;
};
} // namespace bmp
