#include "BitmapPlusPlus.hpp"

namespace bmp {

namespace {

#pragma pack(push, 1)
struct BitmapHeader {
  /* Bitmap file header structure */
  std::uint16_t magic;       /* Magic number for file always BM which is 0x4D42 */
  std::uint32_t file_size;   /* Size of file */
  std::uint16_t reserved1;   /* Reserved */
  std::uint16_t reserved2;   /* Reserved */
  std::uint32_t offset_bits; /* Offset to bitmap data */
  /* Bitmap file info structure */
  std::uint32_t size;              /* Size of info header */
  std::int32_t width;              /* Width of image */
  std::int32_t height;             /* Height of image */
  std::uint16_t planes;            /* Number of color planes */
  std::uint16_t bits_per_pixel;    /* Number of bits per pixel */
  std::uint32_t compression;       /* Type of compression to use */
  std::uint32_t size_image;        /* Size of image data */
  std::int32_t x_pixels_per_meter; /* X pixels per meter */
  std::int32_t y_pixels_per_meter; /* Y pixels per meter */
  std::uint32_t clr_used;          /* Number of colors used */
  std::uint32_t clr_important;     /* Number of important colors */
};
// Sanity check
static_assert(sizeof(BitmapHeader) == 54, "Bitmap header size must be 54 bytes");
#pragma pack(pop)

} // namespace

Pixel makePixel(const std::int32_t rgb) {
  Pixel p;
  p.r = ((rgb >> 16) & 0xff);
  p.g = ((rgb >> 8) & 0xff);
  p.b = ((rgb >> 0x0) & 0xff);
  return p;
}

Bitmap::Bitmap() noexcept : m_pixels(), m_width(0), m_height(0) {
}

Bitmap::Bitmap(const char* filename) : m_pixels(), m_width(0), m_height(0) {
  this->load(filename);
}

Bitmap::Bitmap(const std::int32_t width, const std::int32_t height)
    : m_pixels(static_cast<std::size_t>(width) * static_cast<std::size_t>(height)),
      m_width(width),
      m_height(height) {
  if (width == 0 || height == 0)
    throw Exception("Bitmap width and height must be > 0");
}

Bitmap::Bitmap(Bitmap &&other) noexcept
    : m_pixels(std::move(other.m_pixels)),
      m_width(std::exchange(other.m_width, 0)),
      m_height(std::exchange(other.m_height, 0)) {
}

void Bitmap::draw_line(std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2, const Pixel color) {
  const std::int32_t dx = std::abs(x2 - x1);
  const std::int32_t dy = std::abs(y2 - y1);
  const std::int32_t sx = (x1 < x2) ? 1 : -1;
  const std::int32_t sy = (y1 < y2) ? 1 : -1;
  std::int32_t err = dx - dy;
  while (true) {
    m_pixels[IX(x1, y1)] = color;

    if (x1 == x2 && y1 == y2) {
      break;
    }

    int e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x1 += sx;
    }
    if (e2 < dx) {
      err += dx;
      y1 += sy;
    }
  }
}


void Bitmap::fill_rect(const std::int32_t x, const std::int32_t y, const std::int32_t width, const std::int32_t height,
                       const Pixel color) {
  if (!in_bounds(x, y) || !in_bounds(x + (width - 1), y + (height - 1)))
    throw Exception(
        "Bitmap::fill_rect(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(width) + ", " +
        std::to_string(height) + "): x,y,w or h out of bounds");

  for (std::int32_t dx = x; dx < x + width; ++dx) {
    for (std::int32_t dy = y; dy < y + height; ++dy) {
      m_pixels[IX(dx, dy)] = color;
    }
  }
}

void Bitmap::draw_rect(const std::int32_t x, const std::int32_t y, const std::int32_t width, const std::int32_t height,
                       const Pixel color) {
  if (!in_bounds(x, y) || !in_bounds(x + (width - 1), y + (height - 1)))
    throw Exception(
        "Bitmap::draw_rect(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(width) + ", " +
        std::to_string(height) + "): x,y,w or h out of bounds");

  for (std::int32_t dx = x; dx < x + width; ++dx) {
    m_pixels[IX(dx, y)] = color;              // top
    m_pixels[IX(dx, y + height - 1)] = color; // bottom
  }
  for (std::int32_t dy = y; dy < y + height; ++dy) {
    m_pixels[IX(x, dy)] = color;             // left
    m_pixels[IX(x + width - 1, dy)] = color; // right
  }
}


void Bitmap::draw_triangle(const std::int32_t x1, const std::int32_t y1,
                           const std::int32_t x2, const std::int32_t y2,
                           const std::int32_t x3, const std::int32_t y3,
                           const Pixel color) {
  if (!in_bounds(x1, y1) || !in_bounds(x2, y2) || !in_bounds(x3, y3))
    throw Exception("Bitmap::draw_triangle: One or more points are out of bounds");

  draw_line(x1, y1, x2, y2, color);
  draw_line(x2, y2, x3, y3, color);
  draw_line(x3, y3, x1, y1, color);
}

void Bitmap::fill_triangle(const std::int32_t x1, const std::int32_t y1,
                           const std::int32_t x2, const std::int32_t y2,
                           const std::int32_t x3, const std::int32_t y3,
                           const Pixel color) {
  if (!in_bounds(x1, y1) || !in_bounds(x2, y2) || !in_bounds(x3, y3))
    throw Exception("Bitmap::fill_triangle: One or more points are out of bounds");

  // Sort the vertices by y-coordinate (top to bottom)
  std::vector<std::pair<std::int32_t, std::int32_t> > vertices = {
      {x1, y1},
      {x2, y2},
      {x3, y3}};
  std::sort(vertices.begin(), vertices.end(), [](const auto &a, const auto &b) {
    return a.second < b.second;
  });

  const auto [x_top, y_top] = vertices[0];
  const auto [x_mid, y_mid] = vertices[1];
  const auto [x_bot, y_bot] = vertices[2];

  // Calculate the slopes of the left and right edges
  const float slope_left = static_cast<float>(x_mid - x_top) / (y_mid - y_top);
  const float slope_right = static_cast<float>(x_bot - x_top) / (y_bot - y_top);

  // Initialize the starting and ending x-coordinates for each scanline
  std::vector<std::pair<std::int32_t, std::int32_t> > scanlines(y_mid - y_top + 1);
  for (std::int32_t y = y_top; y <= y_mid; ++y) {
    const auto x_start = static_cast<std::int32_t>(x_top + (y - y_top) * slope_left);
    const auto x_end = static_cast<std::int32_t>(x_top + (y - y_top) * slope_right);
    scanlines[y - y_top] = {x_start, x_end};
  }

  // Fill the upper part of the triangle
  for (std::int32_t y = y_top; y <= y_mid; ++y) {
    const std::int32_t x_start = scanlines[y - y_top].first;
    const std::int32_t x_end = scanlines[y - y_top].second;
    draw_line(x_start, y, x_end, y, color);
  }

  // Update the slope for the right edge of the triangle
  const float new_slope_right = static_cast<float>(x_bot - x_mid) / (y_bot - y_mid);

  // Update the x-coordinates for the scanlines in the lower part of the triangle
  for (std::int32_t y = y_mid + 1; y <= y_bot; ++y) {
    const auto x_start = static_cast<std::int32_t>(x_mid + (y - y_mid) * slope_left);
    const auto x_end = static_cast<std::int32_t>(x_top + (y - y_top) * new_slope_right);
    scanlines[y - y_top] = {x_start, x_end};
  }

  // Fill the lower part of the triangle
  for (std::int32_t y = y_mid + 1; y <= y_bot; ++y) {
    const std::int32_t x_start = scanlines[y - y_top].first;
    const std::int32_t x_end = scanlines[y - y_top].second;
    draw_line(x_start, y, x_end, y, color);
  }
}

void Bitmap::draw_circle(const std::int32_t center_x, const std::int32_t center_y, const std::int32_t radius,
                         const Pixel color) {
  if (!in_bounds(center_x - radius, center_y - radius) || !in_bounds(center_x + radius, center_y + radius))
    throw Exception("Bitmap::draw_circle: Circle exceeds bounds");

  std::int32_t x = radius;
  std::int32_t y = 0;
  std::int32_t err = 0;

  while (x >= y) {
    // Draw pixels in all octants
    m_pixels[IX(center_x + x, center_y + y)] = color;
    m_pixels[IX(center_x + y, center_y + x)] = color;
    m_pixels[IX(center_x - y, center_y + x)] = color;
    m_pixels[IX(center_x - x, center_y + y)] = color;
    m_pixels[IX(center_x - x, center_y - y)] = color;
    m_pixels[IX(center_x - y, center_y - x)] = color;
    m_pixels[IX(center_x + y, center_y - x)] = color;
    m_pixels[IX(center_x + x, center_y - y)] = color;

    // Update error and y for the next pixel
    if (err <= 0) {
      y += 1;
      err += 2 * y + 1;
    }

    // Update error and x for the next pixel
    if (err > 0) {
      x -= 1;
      err -= 2 * x + 1;
    }
  }
}

void Bitmap::fill_circle(const std::int32_t center_x, const std::int32_t center_y, const std::int32_t radius,
                         const Pixel color) {
  if (!in_bounds(center_x - radius, center_y - radius) || !in_bounds(center_x + radius, center_y + radius))
    throw Exception("Bitmap::fill_circle: Circle exceeds bounds");

  std::int32_t x = radius;
  std::int32_t y = 0;
  std::int32_t err = 0;

  while (x >= y) {
    // Fill scanlines in all octants
    for (std::int32_t i = center_x - x; i <= center_x + x; ++i) {
      m_pixels[IX(i, center_y + y)] = color;
      m_pixels[IX(i, center_y - y)] = color;
    }

    for (std::int32_t i = center_x - y; i <= center_x + y; ++i) {
      m_pixels[IX(i, center_y + x)] = color;
      m_pixels[IX(i, center_y - x)] = color;
    }

    // Update error and y for the next pixel
    if (err <= 0) {
      y += 1;
      err += 2 * y + 1;
    }

    // Update error and x for the next pixel
    if (err > 0) {
      x -= 1;
      err -= 2 * x + 1;
    }
  }
}

Pixel & Bitmap::get(const std::int32_t x, const std::int32_t y) {
  if (!in_bounds(x, y))
    throw Exception("Bitmap::get(" + std::to_string(x) + ", " + std::to_string(y) + "): x,y out of bounds");
  return m_pixels[IX(x, y)];
}

const Pixel& Bitmap::get(const std::int32_t x, const std::int32_t y) const {
  if (!in_bounds(x, y))
    throw Exception("Bitmap::get(" + std::to_string(x) + ", " + std::to_string(y) + "): x,y out of bounds");
  return m_pixels[IX(x, y)];
}

std::int32_t Bitmap::width() const noexcept { return m_width; }

std::int32_t Bitmap::height() const noexcept { return m_height; }


void Bitmap::clear(const Pixel pixel) {
  std::fill(m_pixels.begin(), m_pixels.end(), pixel);
}

const Pixel & Bitmap::operator[](const std::size_t i) const { return m_pixels[i]; }

Pixel & Bitmap::operator[](const std::size_t i) { return m_pixels[i]; }

bool Bitmap::operator!() const noexcept { return (m_pixels.empty()) || (m_width == 0) || (m_height == 0); }

Bitmap::operator bool() const noexcept { return !(*this); }

bool Bitmap::operator==(const Bitmap &image) const {
  if (this == std::addressof(image)) {
    return true;
  }
  return (m_width == image.m_width) &&
         (m_height == image.m_height) &&
         (std::memcmp(m_pixels.data(), image.m_pixels.data(), sizeof(Pixel) * m_pixels.size()) == 0);
}

bool Bitmap::operator!=(const Bitmap &image) const { return !(*this == image); }

std::uint32_t Bitmap::size() const { return m_pixels.size(); }

Bitmap & Bitmap::operator=(const Bitmap &image) // Copy assignment operator
{
  if (this != std::addressof(image)) {
    m_width = image.m_width;
    m_height = image.m_height;
    m_pixels = image.m_pixels;
  }
  return *this;
}

Bitmap & Bitmap::operator=(Bitmap &&image) noexcept {
  if (this != std::addressof(image)) {
    m_pixels = std::move(image.m_pixels);
    m_width = std::exchange(image.m_width, 0);
    m_height = std::exchange(image.m_height, 0);
  }
  return *this;
}

void Bitmap::set(const std::int32_t x, const std::int32_t y, const Pixel color) {
  if (!in_bounds(x, y)) {
    throw Exception("Bitmap::set(" + std::to_string(x) + ", " + std::to_string(y) + "): x,y out of bounds");
  }
  m_pixels[IX(x, y)] = color;
}


/**
    *	Vertically flips the bitmap and returns the flipped version
    *
    */
[[nodiscard("Bitmap::flip_v() is immutable")]]
Bitmap Bitmap::flip_v() const {
  Bitmap finished(m_width, m_height);
  for (std::int32_t x = 0; x < m_width; ++x) {
    for (std::int32_t y = 0; y < m_height; ++y) {
      // Calculate the reverse y-index
      finished.m_pixels[IX(x, y)] = m_pixels[IX(x, m_height - 1 - y)];
    }
  }
  return finished;
}

/**
    *	Horizontally flips the bitmap and returns the flipped version
    *
    */
[[nodiscard("Bitmap::flip_h() is immutable")]]
Bitmap Bitmap::flip_h() const {
  Bitmap finished(m_width, m_height);
  for (std::int32_t y = 0; y < m_height; ++y) {
    for (std::int32_t x = 0; x < m_width; ++x) {
      // Calculate the reverse x-index
      finished.m_pixels[IX(x, y)] = m_pixels[IX(m_width - 1 - x, y)];
    }
  }
  return finished;
}

/**
    *	Rotates the bitmap to the right and returns the rotated version
    *
    */
[[nodiscard("Bitmap::rotate_90_left() is immutable")]]
Bitmap Bitmap::rotate_90_left() const {
  Bitmap finished(m_height, m_width); // Swap dimensions

  for (std::int32_t y = 0; y < m_height; ++y) {
    const std::int32_t y_offset = y * m_width; // Precompute row start index
    for (std::int32_t x = 0; x < m_width; ++x) {
      // Original pixel at (x, y) moves to (y, m_width - 1 - x)
      finished.m_pixels[(m_width - 1 - x) * m_height + y] = m_pixels[y_offset + x];
    }
  }

  return finished;
}

/**
    *	Rotates the bitmap to the left and returns the rotated version
    *
    */
[[nodiscard("Bitmap::rotate_90_right() is immutable")]]
Bitmap Bitmap::rotate_90_right() const {
  Bitmap finished(m_height, m_width); // Swap dimensions
  for (std::int32_t y = 0; y < m_height; ++y) {
    const std::int32_t y_offset = y * m_width; // Precompute row start index
    for (std::int32_t x = 0; x < m_width; ++x) {
      finished.m_pixels[x * m_height + (m_height - 1 - y)] = m_pixels[y_offset + x];
    }
  }

  return finished;
}

/**
     *	Saves Bitmap pixels into a file
     *   @throws bmp::Exception on error
     */
void Bitmap::save(const char* path) const {
  std::filesystem::path filename{path};
  // Calculate row and bitmap size
  const std::int32_t row_size = m_width * 3 + m_width % 4;
  const std::uint32_t bitmap_size = row_size * m_height;

  // Construct bitmap header
  BitmapHeader header{};
  /* Bitmap file header structure */
  header.magic = BITMAP_BUFFER_MAGIC;
  header.file_size = bitmap_size + sizeof(BitmapHeader);
  header.reserved1 = 0;
  header.reserved2 = 0;
  header.offset_bits = sizeof(BitmapHeader);
  /* Bitmap file info structure */
  header.size = 40;
  header.width = m_width;
  header.height = m_height;
  header.planes = 1;
  header.bits_per_pixel = sizeof(Pixel) * 8; // 24bpp
  header.compression = 0;
  header.size_image = bitmap_size;
  header.x_pixels_per_meter = 0;
  header.y_pixels_per_meter = 0;
  header.clr_used = 0;
  header.clr_important = 0;

  // Save bitmap to output file
  if (std::ofstream ofs{filename, std::ios::binary}; ofs.good()) {
    // Write Header
    ofs.write(reinterpret_cast<const char *>(&header), sizeof(BitmapHeader));
    if (!ofs.good()) {
      throw Exception("Bitmap::save(\"" + filename.string() + "\"): Failed to write bitmap header to file.");
    }

    // Write Pixels
    std::vector<std::uint8_t> line(row_size);
    for (std::int32_t y = m_height - 1; y >= 0; --y) {
      std::size_t i = 0;
      for (std::int32_t x = 0; x < m_width; ++x) {
        const Pixel &color = m_pixels[IX(x, y)];
        line[i++] = color.b;
        line[i++] = color.g;
        line[i++] = color.r;
      }
      ofs.write(reinterpret_cast<const char *>(line.data()), line.size());
      if (!ofs.good()) {
        throw Exception("Bitmap::save(\"" + filename.string() + "\"): Failed to write bitmap pixels to file.");
      }
    }
  } else
    throw Exception("Bitmap::save(\"" + filename.string() + "\"): Failed to open file.");
}

/**
     *	Loads Bitmap from file
     *   @throws bmp::Exception on error
     */
void Bitmap::load(const char* path) {
  std::filesystem::path filename{path};

  m_pixels.clear();

  if (std::ifstream ifs{filename, std::ios::binary}; ifs.good()) {
    // Read Header
    std::unique_ptr<BitmapHeader> header(new BitmapHeader());
    ifs.read(reinterpret_cast<char *>(header.get()), sizeof(BitmapHeader));

    // Check if Bitmap file is valid
    if (header->magic != BITMAP_BUFFER_MAGIC) {
      throw Exception("Bitmap::load(\"" + filename.string() + "\"): Unrecognized file format.");
    }
    // Check if the Bitmap file has 24 bits per pixel (for now supporting only 24bpp bitmaps)
    if (header->bits_per_pixel != 24) {
      throw Exception("Bitmap::load(\"" + filename.string() + "\"): Only 24 bits per pixel bitmaps supported.");
    }

    // Seek the beginning of the pixels data
    // Note: We can't just assume we're there right after we read the BitmapHeader
    // Because some editors like Gimp might put extra information after the header.
    // Thanks to @seeliger-ec
    ifs.seekg(header->offset_bits);

    // Set width & height
    m_width = header->width;
    m_height = header->height;

    // Resize pixels size
    m_pixels.resize(static_cast<std::size_t>(m_width) * static_cast<std::size_t>(m_height), Black);

    // Read Bitmap pixels
    const std::int32_t row_size = m_width * 3 + m_width % 4;
    std::vector<std::uint8_t> line(row_size);
    for (std::int32_t y = m_height - 1; y >= 0; --y) {
      ifs.read(reinterpret_cast<char *>(line.data()), line.size());
      if (!ifs.good())
        throw Exception("Bitmap::load(\"" + filename.string() + "\"): Failed to read bitmap pixels from file.");
      std::size_t i = 0;
      for (std::int32_t x = 0; x < m_width; ++x) {
        Pixel color{};
        color.b = line[i++];
        color.g = line[i++];
        color.r = line[i++];
        m_pixels[IX(x, y)] = color;
      }
    }
  } else
    throw Exception("Bitmap::load(\"" + filename.string() + "\"): Failed to load bitmap pixels from file.");
}

[[nodiscard]] constexpr std::size_t Bitmap::IX(const std::int32_t x, const std::int32_t y) const noexcept {
  return static_cast<std::size_t>(x) + static_cast<std::size_t>(m_width) * static_cast<std::size_t>(y);
}

[[nodiscard]] constexpr bool Bitmap::in_bounds(const std::int32_t x, const std::int32_t y) const noexcept {
  return (x >= 0) && (x < m_width) && (y >= 0) && (y < m_height);
}

} // namespace bmp
