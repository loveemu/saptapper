/// @file
/// Saptapper CLI: Automated GSF ripper for MusicPlayer2000.

#include "args.hxx"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include "saptapper/cartridge.hpp"
#include "saptapper/saptapper.hpp"

using namespace saptapper;

int main(int argc, const char** argv) {
  try {
    args::ArgumentParser parser(
        "An automated GSF ripper for MusicPlayer2000 driver by Nintendo.", "");
    args::HelpFlag help(parser, "help", "Show this help message and exit",
                        {'h', "help"});
    args::Positional<std::filesystem::path> input_arg(
        parser, "input.gba", "The GBA ROM to be analyzed");

    try {
      parser.ParseCLI(argc, argv);
    } catch (args::Help&) {
      std::cout << parser;
      return EXIT_SUCCESS;
    }

    const auto in_path = args::get(input_arg);
    if (!exists(in_path)) {
      std::cerr << in_path.string() << ": File does not exist" << std::endl;
      return EXIT_FAILURE;
    }

    Cartridge cartridge = Cartridge::LoadFromFile(in_path);
    Saptapper saptapper;
    saptapper.Inspect(cartridge);
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
