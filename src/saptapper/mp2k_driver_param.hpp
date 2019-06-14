// Saptapper: Automated GSF ripper for MusicPlayer2000.

#ifndef SAPTAPPER_MP2K_DRIVER_PARAM_HPP_
#define SAPTAPPER_MP2K_DRIVER_PARAM_HPP_

#include <array>
#include <string>
#include "tabulate.hpp"
#include "types.hpp"

namespace saptapper {

class Mp2kDriverParam {
 public:
  Mp2kDriverParam() = default;

  bool ok() const noexcept {
    return song_count_ != 0 && init_fn_ != agbnullptr &&
           main_fn_ != agbnullptr && vsync_fn_ != agbnullptr &&
           select_song_fn_ != agbnullptr;
  }

  int song_count() const noexcept { return song_count_; }

  agbptr_t song_table() const noexcept { return song_table_; }

  agbptr_t init_fn() const noexcept { return init_fn_; }

  agbptr_t main_fn() const noexcept { return main_fn_; }

  agbptr_t vsync_fn() const noexcept { return vsync_fn_; }

  agbptr_t select_song_fn() const noexcept { return select_song_fn_; }

  void set_song_count(int count) noexcept { song_count_ = count; }

  void set_song_table(agbptr_t address) noexcept { song_table_ = address; }

  void set_init_fn(agbptr_t address) noexcept { init_fn_ = address; }

  void set_main_fn(agbptr_t address) noexcept { main_fn_ = address; }

  void set_vsync_fn(agbptr_t address) noexcept { vsync_fn_ = address; }

  void set_select_song_fn(agbptr_t address) noexcept {
    select_song_fn_ = address;
  }

  std::ostream& WriteAsTable(std::ostream& stream) const {
    using row_t = std::array<std::string, 2>;
    const row_t header{"Name", "Address / Value"};
    std::array items{
        row_t{"m4aSoundVSync", to_string(this->vsync_fn())},
        row_t{"m4aSoundInit", to_string(this->init_fn())},
        row_t{"m4aSoundMain", to_string(this->main_fn())},
        row_t{"m4aSongNumStart", to_string(this->select_song_fn())},
        row_t{"song_table", to_string(this->song_table())},
        row_t{"len(song_table)", std::to_string(this->song_count())},
    };

    tabulate(stream, header, items);
    return stream;
  }

 private:
  int song_count_ = 0;
  agbptr_t song_table_ = agbnullptr;
  agbptr_t init_fn_ = agbnullptr;
  agbptr_t main_fn_ = agbnullptr;
  agbptr_t vsync_fn_ = agbnullptr;
  agbptr_t select_song_fn_ = agbnullptr;
};

}  // namespace saptapper

#endif
