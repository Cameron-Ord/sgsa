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
  Literals(std::string n, std::vector<std::string> f);
  std::string name;
  std::vector<std::string> fields_array;
};

struct Cfg_Maps {
  Cfg_Maps(void) : cfg_floats(), cfg_ints(), cfg_strings() {}
  bool valid;
  std::unordered_map<std::string, f32> cfg_floats;
  std::unordered_map<std::string, i32> cfg_ints;
  std::unordered_map<std::string, std::string> cfg_strings;
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
  Cfg_Builder(std::vector<Literals> flds, Lua_Container& lc_);
  Cfg_Builder& loop_tables(void);
  Lua_Cfg build();
private:
  Lua_Container& lc_;
  std::vector<Literals> fields;
  Cfg_Maps maps;
  void loop_field_array(const std::vector<std::string>& fields_array);
};

class Lua_Container {
public:
  Lua_Container(void) = default;
  ~Lua_Container(void) = default;
  Lua_Cfg load_cfg(const char * filepath);
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
