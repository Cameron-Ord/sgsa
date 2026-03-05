#include "scripts.hpp"
#include "audio.hpp"
#include <iostream>

Literals::Literals(std::vector<Entry> flds,
                   std::vector<std::pair<Entry, std::vector<Entry>>> tbls)
    : fields_array(flds), tables_array(tbls) {}

const std::vector<Entry> BASE_ENTRIES = {
    Entry("lfo_on", ENTRY_TYPE::BOOL),
    Entry("lfo_rate", ENTRY_TYPE::FLOAT),
    Entry("lfo_depth", ENTRY_TYPE::FLOAT),
    Entry("lfo_timer", ENTRY_TYPE::FLOAT),
    Entry("lfo_mode", ENTRY_TYPE::INT),
    Entry("env_attack", ENTRY_TYPE::FLOAT),
    Entry("env_decay", ENTRY_TYPE::FLOAT),
    Entry("env_sustain", ENTRY_TYPE::FLOAT),
    Entry("env_release", ENTRY_TYPE::FLOAT),
    Entry("volume", ENTRY_TYPE::FLOAT),
    Entry("gain", ENTRY_TYPE::FLOAT),
    Entry("channels", ENTRY_TYPE::INT),
    Entry("sample_rate", ENTRY_TYPE::INT),
    Entry("voicings", ENTRY_TYPE::INT),
    Entry("wave_table_size", ENTRY_TYPE::INT),
    Entry("tempo", ENTRY_TYPE::FLOAT),
    Entry("note_duration", ENTRY_TYPE::FLOAT),
    Entry("filter_cutoff_low", ENTRY_TYPE::FLOAT),
    Entry("filter_cutoff_high", ENTRY_TYPE::FLOAT),
    Entry("use_filter", ENTRY_TYPE::BOOL),
};

const std::vector<Entry> OSC_TABLE_ENTRIES = {
    Entry("duty_cycle", ENTRY_TYPE::FLOAT), Entry("detune", ENTRY_TYPE::FLOAT),
    Entry("volume", ENTRY_TYPE::FLOAT), Entry("octave_step", ENTRY_TYPE::FLOAT),
    Entry("waveform", ENTRY_TYPE::INT)};

std::vector<std::pair<Entry, std::vector<Entry>>> TABLE_ENTRIES = {
    {Entry("oscilators", ENTRY_TYPE::LUA_TABLE), OSC_TABLE_ENTRIES}};

const Literals FIELDS(BASE_ENTRIES, TABLE_ENTRIES);

std::vector<Oscilator_Cfg> Cfg_Maps::make_internal_osc_cfg(void) {
  std::vector<Oscilator_Cfg> cfgs(osc_maps.size());
  for (size_t i = 0; i < osc_maps.size(); i++) {
    Osc_Map &map = osc_maps[i];
    COPY_FLOAT(cfgs[i], map, duty_cycle);
    COPY_FLOAT(cfgs[i], map, detune);
    COPY_FLOAT(cfgs[i], map, volume);
    COPY_FLOAT(cfgs[i], map, octave_step);
    COPY_INT(cfgs[i], map, waveform);
  }
  return cfgs;
}

Synth_Cfg Cfg_Maps::make_internal_base_cfg(void) {
  Synth_Cfg cfgs;
  COPY_BOOL(cfgs, base_cfg, lfo_on);
  COPY_BOOL(cfgs, base_cfg, use_filter);

  COPY_FLOAT(cfgs, base_cfg, lfo_rate);
  COPY_FLOAT(cfgs, base_cfg, lfo_depth);
  COPY_FLOAT(cfgs, base_cfg, lfo_timer);
  COPY_FLOAT(cfgs, base_cfg, volume);
  COPY_FLOAT(cfgs, base_cfg, gain);
  COPY_FLOAT(cfgs, base_cfg, tempo);
  COPY_FLOAT(cfgs, base_cfg, note_duration);
  COPY_FLOAT(cfgs, base_cfg, filter_cutoff_low);
  COPY_FLOAT(cfgs, base_cfg, filter_cutoff_high);
  COPY_FLOAT(cfgs, base_cfg, env_attack);
  COPY_FLOAT(cfgs, base_cfg, env_decay);
  COPY_FLOAT(cfgs, base_cfg, env_sustain);
  COPY_FLOAT(cfgs, base_cfg, env_release);

  COPY_INT(cfgs, base_cfg, channels);
  COPY_INT(cfgs, base_cfg, sample_rate);
  COPY_INT(cfgs, base_cfg, voicings);
  COPY_INT(cfgs, base_cfg, wave_table_size);
  COPY_INT(cfgs, base_cfg, lfo_mode);
  return cfgs;
}

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

Lua_Cfg Lua_Container::load_cfg(const char *filepath) {
  if (!do_file(filepath)) {
    return Lua_Cfg(Cfg_Maps(), false);
  }

  Cfg_Builder c(FIELDS, *this);
  return c.loop_fields().build();
}

Cfg_Builder::Cfg_Builder(Literals flds, Lua_Container &lc)
    : lc_(lc), fields(flds), maps() {}

Lua_Cfg Cfg_Builder::build(void) { return Lua_Cfg(maps, true); }

Cfg_Builder &Cfg_Builder::loop_fields(void) {

  for (size_t i = 0; i < fields.fields_array.size(); i++) {
    if (!lc_.get_field(fields.fields_array[i].name.c_str()))
      continue;

    switch (lc_.get_type()) {
    default: {
      lc_.pop();
    } break;

    case LUA_TNUMBER: {
      if (lc_.is_int()) {
        maps.base_cfg.cfg_ints.emplace(fields.fields_array[i].name,
                                       lc_.get_int());
        lc_.pop();
      } else {
        maps.base_cfg.cfg_floats.emplace(fields.fields_array[i].name,
                                         lc_.get_float());
        lc_.pop();
      }
    } break;

    case LUA_TSTRING: {
      maps.base_cfg.cfg_strings.emplace(fields.fields_array[i].name,
                                        lc_.get_str());
      lc_.pop();
    } break;

    case LUA_TBOOLEAN: {
      maps.base_cfg.cfg_bools.emplace(fields.fields_array[i].name,
                                      lc_.get_bool());
      lc_.pop();
    } break;
    }
  }

  for (size_t i = 0; i < fields.tables_array.size(); i++) {
    if (!lc_.get_field(fields.tables_array[i].first.name.c_str()))
      continue;

    switch (lc_.get_type()) {
    case LUA_TTABLE: {
      const size_t len = lc_.raw_len();
      maps.osc_maps.resize(len);
      for (size_t j = 1; j <= len; j++) {
        lc_.raw_geti((i32)j);
        for (size_t k = 0; k < fields.tables_array[i].second.size(); k++) {
          if (lc_.get_field(fields.tables_array[i].second[k].name.c_str())) {
            if (lc_.get_type() != LUA_TNUMBER) {
              lc_.pop();
              continue;
            }
            if (lc_.is_int()) {
              maps.osc_maps[j - 1].cfg_ints.emplace(
                  fields.tables_array[i].second[k].name, lc_.get_int());
            } else {
              maps.osc_maps[j - 1].cfg_floats.emplace(
                  fields.tables_array[i].second[k].name, lc_.get_float());
            }
            lc_.pop();
          }
        }
        lc_.pop();
      }
    } break;
    }
  }

  return *this;
}
