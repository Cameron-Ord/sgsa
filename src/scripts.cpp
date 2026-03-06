#include "scripts.hpp"
#include "audio.hpp"
#include <iostream>

Field_Cluster::Field_Cluster(
    std::vector<Entry> flds,
    std::vector<std::pair<Entry, std::vector<Entry>>> tbls)
    : fields_array(flds), tables_array(tbls) {}

const std::vector<Entry> BASE_FIELDS = {
    Entry("lfo_on", ENTRY_TYPE::BOOL, offsetof(Synth_Cfg, lfo_on)),
    Entry("lfo_rate", ENTRY_TYPE::FLOAT, offsetof(Synth_Cfg, lfo_rate)),
    Entry("lfo_depth", ENTRY_TYPE::FLOAT, offsetof(Synth_Cfg, lfo_depth)),
    Entry("lfo_timer", ENTRY_TYPE::FLOAT, offsetof(Synth_Cfg, lfo_timer)),
    Entry("lfo_mode", ENTRY_TYPE::INT, offsetof(Synth_Cfg, lfo_mode)),
    Entry("env_attack", ENTRY_TYPE::FLOAT, offsetof(Synth_Cfg, env_attack)),
    Entry("env_decay", ENTRY_TYPE::FLOAT, offsetof(Synth_Cfg, env_decay)),
    Entry("env_sustain", ENTRY_TYPE::FLOAT, offsetof(Synth_Cfg, env_sustain)),
    Entry("env_release", ENTRY_TYPE::FLOAT, offsetof(Synth_Cfg, env_release)),
    Entry("volume", ENTRY_TYPE::FLOAT, offsetof(Synth_Cfg, volume)),
    Entry("gain", ENTRY_TYPE::FLOAT, offsetof(Synth_Cfg, gain)),
    Entry("channels", ENTRY_TYPE::INT, offsetof(Synth_Cfg, channels)),
    Entry("sample_rate", ENTRY_TYPE::INT, offsetof(Synth_Cfg, sample_rate)),
    Entry("voicings", ENTRY_TYPE::INT, offsetof(Synth_Cfg, voicings)),
    Entry("wave_table_size", ENTRY_TYPE::INT,
          offsetof(Synth_Cfg, wave_table_size)),
    Entry("tempo", ENTRY_TYPE::FLOAT, offsetof(Synth_Cfg, tempo)),
    Entry("note_duration", ENTRY_TYPE::FLOAT,
          offsetof(Synth_Cfg, note_duration)),
    Entry("low_pass_cutoff", ENTRY_TYPE::FLOAT,
          offsetof(Synth_Cfg, low_pass_cutoff)),
    Entry("use_filter", ENTRY_TYPE::BOOL, offsetof(Synth_Cfg, use_filter)),
    Entry("duty_cycle", ENTRY_TYPE::FLOAT, offsetof(Synth_Cfg, duty_cycle))};

const std::vector<Entry> OSC_TABLE_FIELDS = {
    Entry("detune", ENTRY_TYPE::FLOAT, offsetof(Oscilator_Cfg, detune)),
    Entry("volume", ENTRY_TYPE::FLOAT, offsetof(Oscilator_Cfg, volume)),
    Entry("octave_step", ENTRY_TYPE::FLOAT,
          offsetof(Oscilator_Cfg, octave_step)),
    Entry("waveform", ENTRY_TYPE::INT, offsetof(Oscilator_Cfg, waveform))};

std::vector<std::pair<Entry, std::vector<Entry>>> TABLES = {
    {Entry("oscilators", ENTRY_TYPE::LUA_TABLE), OSC_TABLE_FIELDS}};

const Field_Cluster FIELDS(BASE_FIELDS, TABLES);

void Lua_Container::pop(void) { lua_pop(L, 1); }

bool Lua_Container::initialize(void) {
  if (!(L = luaL_newstate())) {
    std::cerr << "Failed to initialize lua state!" << std::endl;
    return false;
  }
  luaL_openlibs(L);
  return true;
}

bool Lua_Container::do_file(const char *filepath) {
  if (luaL_dofile(L, filepath) != LUA_OK) {
    std::cerr << lua_tostring(L, -1) << std::endl;
    pop();
    return false;
  }
  return true;
}

bool Lua_Container::get_field(const char *key) {
  if (lua_getfield(L, -1, key) == LUA_TNIL) {
    std::cerr << "Lua error: field '" << key << "' is nil" << std::endl;
    pop();
    return false;
  }
  return true;
}

bool Lua_Container::is_int(void) { return lua_isinteger(L, -1); }

bool Lua_Container::is_float(void) { return lua_isnumber(L, -1); }

i32 Lua_Container::get_type(void) const { return lua_type(L, -1); }

f32 Lua_Container::get_float(void) {
  const lua_Number val = lua_tonumber(L, -1);
  return (f32)val;
}

bool Lua_Container::get_bool(void) {
  const i32 val = lua_toboolean(L, -1);
  return (bool)val;
}

void Lua_Container::raw_geti(i32 i) { lua_rawgeti(L, -1, (lua_Integer)i); }

size_t Lua_Container::raw_len(void) { return (size_t)lua_rawlen(L, -1); }

i32 Lua_Container::get_int(void) {
  const lua_Integer val = lua_tointeger(L, -1);
  return (i32)val;
}

std::string Lua_Container::get_str(void) {
  const std::string val = std::string(lua_tostring(L, -1));
  return val;
}

bool Lua_Container::load_cfg(const char *filepath, Synth_Cfg &synth,
                             std::vector<Oscilator_Cfg> &oscs) {
  if (!do_file(filepath)) {
    return false;
  }

  Cfg_Builder c(FIELDS, *this);
  c.build_base_fields(synth);
  c.build_oscilator_fields(oscs);

  return true;
}

Cfg_Builder::Cfg_Builder(Field_Cluster flds, Lua_Container &lc)
    : lc_(lc), fields(flds) {}

void Cfg_Builder::build_base_fields(Synth_Cfg &synth) {
  Synth_Cfg *ptr = &synth;
  for (size_t i = 0; i < fields.fields_array.size(); i++) {
    if (!lc_.get_field(fields.fields_array[i].name.c_str())) {
      continue;
    }

    switch (lc_.get_type()) {
    default: {
      lc_.pop();
    } break;

    case LUA_TNUMBER: {
      if (lc_.is_int()) {
        *(i32 *)((char *)ptr + fields.fields_array[i].offset) = lc_.get_int();
        lc_.pop();
      } else {
        *(f32 *)((char *)ptr + fields.fields_array[i].offset) = lc_.get_float();
        lc_.pop();
      }
    } break;

    case LUA_TBOOLEAN: {
      *(bool *)((char *)ptr + fields.fields_array[i].offset) = lc_.get_bool();
      lc_.pop();
    } break;
    }
  }
}

void Cfg_Builder::build_oscilator_fields(std::vector<Oscilator_Cfg> &oscs) {
  for (size_t i = 0; i < fields.tables_array.size(); i++) {
    if (!lc_.get_field(fields.tables_array[i].first.name.c_str())) {
      continue;
    }

    switch (lc_.get_type()) {
    default: {
      lc_.pop();
    } break;

    case LUA_TTABLE: {
      const size_t len = lc_.raw_len();
      oscs.resize(len);
      for (size_t j = 1; j <= len; j++) {
        lc_.raw_geti((i32)j);
        Oscilator_Cfg *ptr = &oscs[j - 1];
        for (size_t k = 0; k < fields.tables_array[i].second.size(); k++) {
          if (!lc_.get_field(fields.tables_array[i].second[k].name.c_str())) {
            continue;
          }

          switch (lc_.get_type()) {
          default: {
            lc_.pop();
          } break;
          case LUA_TNUMBER: {
            if (lc_.is_int()) {
              *(i32 *)((char *)ptr + fields.tables_array[i].second[k].offset) =
                  lc_.get_int();
              lc_.pop();
            } else {
              *(f32 *)((char *)ptr + fields.tables_array[i].second[k].offset) =
                  lc_.get_float();
              lc_.pop();
            }
          } break;
          }
        }
        lc_.pop();
      }
    } break;
    }
  }
}
