// Saptapper: Automated GSF ripper for MusicPlayer2000.

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include "args.hxx"
#include "saptapper/cartridge.hpp"
#include "saptapper/saptapper.hpp"

using namespace saptapper;
using namespace std::literals::string_literals;

inline static const std::string kAppName = "Saptapper"s;
inline static const std::string kAppVersion = "2.0"s;
inline static const std::string kAppCredits =
    "Original created by Caitsith2, reimplemented by loveemu from scratch."s
    "\nVisit <http://github.com/loveemu/saptapper> for details."s;

int main(int argc, const char** argv) {
  try {
    args::ArgumentParser parser(
        "An automated GSF ripper for MusicPlayer2000 driver by Nintendo "
        "(aka. m4a or Sappy).");
    args::HelpFlag help(parser, "help", "Show this help message and exit",
                        {'h', "help"});
    args::Flag inspect_arg(
        parser, "inspect",
        "Show the inspection result without saving files and quit",
        {"inspect"});
    args::Flag force_arg(parser, "force",
                         "Save all songs including duplicated ones",
                         {'f', "force"});
    args::ValueFlag<std::filesystem::path> outdir_arg(
        parser, "directory",
        "The output directory (the default is the working directory)",
        {'d', "outdir"});
    args::ValueFlag<std::filesystem::path> basename_arg(
        parser, "basename", "The output filename (without extension)", {'o'});
    args::ValueFlag<std::string> gsfby_arg(
        parser, "name", "The creator name to be tagged to minigsfs", {"gsfby"},
        args::Options::HiddenFromUsage | args::Options::HiddenFromDescription);
    args::Positional<std::filesystem::path> input_arg(
        parser, "romfile", "The ROM file to be processed",
        args::Options::Required);

    try {
      if (argc < 2) throw args::Help(help.Name());

      parser.ParseCLI(argc, argv);
    } catch (args::Help&) {
      std::cout << kAppName << " " << kAppVersion << std::endl << std::endl;
      std::cout << kAppCredits << std::endl << std::endl;
      std::cout << parser;
      return EXIT_SUCCESS;
    }

    const auto in_path = args::get(input_arg);
    if (!exists(in_path)) {
      std::cerr << in_path.string() << ": File does not exist" << std::endl;
      return EXIT_FAILURE;
    }

    Cartridge cartridge = Cartridge::LoadFromFile(in_path);

    if (inspect_arg) {
      Mp2kDriverParam param;
      MinigsfDriverParam minigsf;
      agbptr_t gsf_driver_addr = agbnullptr;
      Saptapper::Inspect(cartridge, param, minigsf, gsf_driver_addr);
      Saptapper::PrintParam(param, minigsf);
    } else {
      const std::filesystem::path basename{
          basename_arg ? args::get(basename_arg) : in_path.stem()};
      const std::filesystem::path outdir{args::get(outdir_arg)};

      std::string gsfby{args::get(gsfby_arg)};
      if (gsfby != "Caitsith2") {
        if (gsfby.empty()) {
          gsfby = "Saptapper";
        } else {
          gsfby.insert(0, "Saptapper, with help of ");
        }
      }

      bool keep_duplicated = force_arg;
      Saptapper::ConvertToGsfSet(cartridge, basename, outdir, gsfby,
                                 keep_duplicated);
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
