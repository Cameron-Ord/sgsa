#include "scripts.hpp"
#include "util.hpp"
#include <iostream>
Literals::Literals(std::string n, std::vector<std::string> f) : name(n), fields_array(f) {}

const std::string LFO = "lfo";
const std::string AUDIO = "audio";
const std::string ENVELOPE = "envelope";
const std::string OSC = "oscilators";

const std::vector<std::string> LFO_LITERALS = { 
  "on", "rate", "depth", "timer", "mode" 
};

const std::vector<std::string> AUDIO_LITERALS = {
  "volume", "gain", "channels", "sample_rate", "voicings", "wave_table_size",
  "tempo", "note_duration", "filter_cutoff_low", "filter_cutoff_high", "use_filter"
};

const std::vector<std::string> ENVELOPE_LITERALS = {
  "attack", "decay", "sustain", "release"
};

const std::vector<std::string> OSC_LITERALS = { 
  "duty_cycle", "detune", "volume", "octave_step", "waveform" 
};

const std::vector<Literals> FIELDS = {
  Literals(LFO, LFO_LITERALS),
  Literals(AUDIO, AUDIO_LITERALS),
  Literals(ENVELOPE, ENVELOPE_LITERALS),
  Literals(OSC, OSC_LITERALS)
};

bool Lua_Container::initialize(void){
  if(!(L = luaL_newstate())){
    std::cerr << "Failed to initialize lua state!" << std::endl;
    return false;
  }
  luaL_openlibs(L);
  return true;
}

bool Lua_Container::do_file(const char *filepath){
  if(luaL_dofile(L, filepath) != LUA_OK) {
    std::cerr << lua_tostring(L, -1) << std::endl;
    lua_pop(L, 1);
    return false;
  }
  return true;
}

bool Lua_Container::get_field(const char *key) {
  if(lua_getfield(L, -1, key) != LUA_OK){
    std::cerr << lua_tostring(L, -1) << std::endl;
    lua_pop(L, 1);
    return false;
  }
  return true;
}

bool Lua_Container::is_int(void){
  return lua_isinteger(L, -1);
}

bool Lua_Container::is_float(void){
  return lua_isnumber(L, -1);
}

i32 Lua_Container::get_type(void) const {
  return lua_type(L, -1);
}

f32 Lua_Container::get_float(void){
  const lua_Number val = lua_tonumber(L, -1);
  lua_pop(L, 1);
  return (f32)val;
}

i32 Lua_Container::get_int(void){
  const lua_Integer val = lua_tointeger(L, -1);
  lua_pop(L, 1);
  return (i32)val;
}

std::string Lua_Container::get_str(void){
  const std::string val = std::string(lua_tostring(L, -1));
  lua_pop(L, 1);
  return val;
}

Lua_Cfg Lua_Container::load_cfg(const char * filepath){
  if(!do_file(filepath)){
    return Lua_Cfg(Cfg_Maps(), false);
  }
  
  Cfg_Builder c(FIELDS, *this);
  return c.loop_tables().build();
}


Cfg_Builder::Cfg_Builder(std::vector<Literals> flds, Lua_Container& lc) 
    :  lc_(lc), fields(flds), maps()
{}

Lua_Cfg Cfg_Builder::build(void){
  return Lua_Cfg(maps, true);
}

Cfg_Builder& Cfg_Builder::loop_tables(void){
  for(size_t i = 0; i < fields.size(); i++){
    if(lc_.get_field(fields[i].name.c_str())){
      if(lc_.get_type() != LUA_TTABLE){
        continue;
      } else {
        loop_field_array(fields[i].fields_array);
      }
    } else {
      continue;
    }
  }
  return *this;
}

void Cfg_Builder::loop_field_array(const std::vector<std::string>& fields_array){
  for(size_t i = 0; i < fields_array.size(); i++){
    if(lc_.get_field(fields_array[i].c_str())){
      switch(lc_.get_type()){
        default: {
          continue; 
        }break;

        case LUA_TNUMBER:{
          if(lc_.is_int()){
            maps.cfg_ints.emplace(fields_array[i], lc_.get_int());
          } else {
            maps.cfg_floats.emplace(fields_array[i], lc_.get_float());
          }
        } break;

        case LUA_TSTRING:{
          maps.cfg_strings.emplace(fields_array[i], lc_.get_str());
        } break;

      }
    } else {
      continue;
    }
  }
}
