#pragma once

namespace legit
{
  namespace Colors
  {
    //https://flatuicolors.com/palette/defo
#define RGBA_LE(col) (((col & 0xff000000) >> (3 * 8)) + ((col & 0x00ff0000) >> (1 * 8)) + ((col & 0x0000ff00) << (1 * 8)) + ((col & 0x000000ff) << (3 * 8)))
    const static uint32_t turqoise = RGBA_LE(0x1abc9cffu);
    const static uint32_t greenSea = RGBA_LE(0x16a085ffu);

    const static uint32_t emerald = RGBA_LE(0x2ecc71ffu);
    const static uint32_t nephritis = RGBA_LE(0x27ae60ffu);

    const static uint32_t peterRiver = RGBA_LE(0x3498dbffu); //blue
    const static uint32_t belizeHole = RGBA_LE(0x2980b9ffu);

    const static uint32_t amethyst = RGBA_LE(0x9b59b6ffu);
    const static uint32_t wisteria = RGBA_LE(0x8e44adffu);

    const static uint32_t sunFlower = RGBA_LE(0xf1c40fffu);
    const static uint32_t orange = RGBA_LE(0xf39c12ffu);

    const static uint32_t carrot = RGBA_LE(0xe67e22ffu);
    const static uint32_t pumpkin = RGBA_LE(0xd35400ffu);

    const static uint32_t alizarin = RGBA_LE(0xe74c3cffu);
    const static uint32_t pomegranate = RGBA_LE(0xc0392bffu);

    const static uint32_t clouds = RGBA_LE(0xecf0f1ffu);
    const static uint32_t silver = RGBA_LE(0xbdc3c7ffu);
    const static uint32_t imguiText = RGBA_LE(0xF2F5FAFFu);
    static uint32_t getColor(uint32_t index) {
      switch (index) {
      case 0: return turqoise;
      case 1: return greenSea;
      case 2: return emerald;
      case 3: return nephritis;
      case 4: return peterRiver;
      case 5: return belizeHole;
      case 6: return amethyst;
      case 7: return wisteria;
      case 8: return sunFlower;
      case 9: return orange;
      case 10: return carrot;
      case 11: return pumpkin;
      case 12: return alizarin;
      case 13: return pomegranate;
      case 14: return clouds;
      case 15: return silver;
      case 16: return imguiText;
      default:
        std::cerr << "Invalid index!" << std::endl;
        return 0; // Return a default or invalid value
      }
    }
  }
  struct ProfilerTask
  {
    double startTime;
    double endTime;
    std::string name;
    uint32_t color;
    double GetLength()
    {
      return endTime - startTime;
    }
  };
}