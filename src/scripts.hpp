#ifndef SCRIPTS_HPP
#define SCRIPTS_HPP

#include "typedef.hpp"

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}
#include <unordered_map>
#include <vector>
#include <string>

struct Literals {
  Literals(std::vector<std::string> f, std::vector<std::string> sf);
  std::vector<std::string> fields_array;
  std::vector<std::string> osc_subfields_array;
};

struct Cfg_Maps {
  Cfg_Maps(void) : cfg_floats(), cfg_ints(), cfg_strings() {}
  bool valid;
  std::unordered_map<std::string, f32> cfg_floats;
  std::unordered_map<std::string, i32> cfg_ints;
  std::unordered_map<std::string, std::string> cfg_strings;
  std::vector<std::unordered_map<std::string, f32>> cfg_osc_floats;
  std::vector<std::unordered_map<std::string, i32>> cfg_osc_ints;
};

class Lua_Cfg {
public:
  Lua_Cfg(void) : maps(), valid(false) {}
  Lua_Cfg(Cfg_Maps m, bool is_valid) : maps(m), valid(is_valid) {}
  bool get_state(void) { return valid; }
private:
  Cfg_Maps maps;
  bool valid;
};

class Lua_Container;

class Cfg_Builder {
public:
  Cfg_Builder(Literals flds, Lua_Container& lc_);
  Cfg_Builder& loop_fields(void);
  Lua_Cfg build();
private:
  Lua_Container& lc_;
  Literals fields;
  Cfg_Maps maps;
};

class Lua_Container {
public:
  Lua_Container(void) = default;
  ~Lua_Container(void) = default;
  Lua_Cfg load_cfg(const char * filepath);
  size_t raw_len(void);
  void raw_geti(i32 i);
  void pop(void);
  bool initialize(void);
  bool do_file(const char *filepath);
  bool get_field(const char *key);
  bool is_int(void);
  bool is_float(void);
  i32 get_type(void) const;
  i32 get_int(void);
  f32 get_float(void);
  std::string get_str(void);
private:
  lua_State *L = NULL;

};


#endif
