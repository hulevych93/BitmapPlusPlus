#include <iostream>
#include "BitmapPlusPlus.hpp"
#include <random>
#include <filesystem>

int main() {
  try {
    // Generate a bitmap of 10% white and 90% black pixels
    bmp::Bitmap image(512, 512);

    std::random_device seed{};
    std::default_random_engine eng{seed()};
    std::bernoulli_distribution dist(0.10); // 10% White, 90% Black

    const auto size = image.size();
    for (int i = 0; i < size; ++i) {
      const bmp::Pixel color = dist(eng) ? bmp::White : bmp::Black;
      image[i] = color;
    }

    const auto p1 = std::filesystem::path(BIN_DIR) / "bernoulli.bmp";
    const auto s1 = p1.string();
    image.save(s1.c_str());

    return EXIT_SUCCESS;
  } catch (const bmp::Exception& e) {
    std::cerr << "[BMP ERROR]: " << e.what() << '\n';
    return EXIT_FAILURE;
  }
}
