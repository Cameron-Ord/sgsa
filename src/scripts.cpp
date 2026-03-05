#include "scripts.hpp"
#include <iostream>


Literals::Literals(std::vector<std::string> f, std::vector<std::string> sf) 
  : fields_array(f), osc_subfields_array(sf) {}

const std::vector<std::string> BASE_LITERALS = { 
  "lfo_on", "lfo_rate", "lfo_depth", "lfo_timer", "lfo_mode", "env_attack",
  "env_decay", "env_sustain", "env_release", "volume", "gain", "channels", 
  "sample_rate", "voicings", "wave_table_size", "tempo", "note_duration",
  "filter_cutoff_low", "filter_cutoff_high", "use_filter", "oscilators"
};

const std::vector<std::string> OSC_LITERALS = {
  "duty_cycle", "detune", "volume", "octave_step", "waveform"
};

const Literals FIELDS(BASE_LITERALS, OSC_LITERALS);

void Lua_Container::pop(void){
  lua_pop(L, 1);
}

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
    pop();
    return false;
  }
  return true;
}

bool Lua_Container::get_field(const char *key) {
  if(lua_getfield(L, -1, key) == LUA_TNIL){
    std::cerr << "Lua error: field '" << key << "' is nil" << std::endl;
    pop();
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
  return (f32)val;
}

void Lua_Container::raw_geti(i32 i){
  lua_rawgeti(L, -1, (lua_Integer)i);
}

size_t Lua_Container::raw_len(void) {
  return (size_t)lua_rawlen(L, -1);
}

i32 Lua_Container::get_int(void){
  const lua_Integer val = lua_tointeger(L, -1);
  return (i32)val;
}

std::string Lua_Container::get_str(void){
  const std::string val = std::string(lua_tostring(L, -1));
  return val;
}

Lua_Cfg Lua_Container::load_cfg(const char * filepath){
  if(!do_file(filepath)){
    return Lua_Cfg(Cfg_Maps(), false);
  }
  
  Cfg_Builder c(FIELDS, *this);
  return c.loop_fields().build();
}


Cfg_Builder::Cfg_Builder(Literals flds, Lua_Container& lc) 
    :  lc_(lc), fields(flds), maps()
{}

Lua_Cfg Cfg_Builder::build(void){
  return Lua_Cfg(maps, true);
}

Cfg_Builder& Cfg_Builder::loop_fields(void){
  for(size_t i = 0; i < fields.fields_array.size(); i++){
    if(!lc_.get_field(fields.fields_array[i].c_str()))
      continue;
    
    switch(lc_.get_type()){
        default : {
          lc_.pop();
        } break;
        case LUA_TTABLE: {
          const size_t len = lc_.raw_len();
          maps.cfg_osc_floats.resize(len);
          maps.cfg_osc_ints.resize(len);
          for(size_t j = 1; j <= len; j++){
            lc_.raw_geti((i32)j);
            for(size_t k = 0; k < fields.osc_subfields_array.size(); k++){
              if(lc_.get_field(fields.osc_subfields_array[k].c_str())){
                if(lc_.get_type() != LUA_TNUMBER){
                  lc_.pop();
                  continue;
                } 
                if(lc_.is_int()){
                  maps.cfg_osc_floats[j - 1].emplace(fields.osc_subfields_array[k], lc_.get_int());
                } else {
                  maps.cfg_osc_floats[j - 1].emplace(fields.osc_subfields_array[k], lc_.get_float());
                }
                lc_.pop();
              }
            }
            lc_.pop();
          }
        }break;
        case LUA_TNUMBER: {
            if(lc_.is_int()){
              maps.cfg_ints.emplace(fields.fields_array[i], lc_.get_int());
              lc_.pop();
            } else {
              maps.cfg_ints.emplace(fields.fields_array[i], lc_.get_float());
              lc_.pop();
            }
        }break;
        case LUA_TSTRING:{
            maps.cfg_strings.emplace(fields.fields_array[i], lc_.get_str());
            lc_.pop();
        }break;
        case LUA_TBOOLEAN: {
            lc_.pop();
        } break;
      }

  }
  return *this;
}

