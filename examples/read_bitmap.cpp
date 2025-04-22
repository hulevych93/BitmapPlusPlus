#include "BitmapPlusPlus.hpp"
#include <iostream>
#include <filesystem>

int main() {
  try {
    bmp::Bitmap image;

    // Load penguin.bmp bitmap
    const auto p0 = std::filesystem::path(ROOT_DIR) / "images" / "penguin.bmp";
    const auto s0 = p0.string();
    image.load(s0.c_str());

    // Modify loaded image (makes half of the image black)
    for (std::int32_t y = 0; y < image.height(); ++y) {
      for (std::int32_t x = 0; x < image.width() / 2; ++x) {
        image.set(x, y, bmp::Black);
      }
    }

    // Save
    const auto p1 = std::filesystem::path(BIN_DIR) / "modified-penguin.bmp";
    const auto s1 = p1.string();
    image.save(s1.c_str());

    return EXIT_SUCCESS;
  } catch (const bmp::Exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
