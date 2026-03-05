#ifndef SCRIPTS_HPP
#define SCRIPTS_HPP

#include "typedef.hpp"

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}
#include <string>
#include <unordered_map>
#include <vector>

#define COPY_FLOAT(cfg, map, name) cfg.name = map.cfg_floats[#name]
#define COPY_INT(cfg, map, name) cfg.name = map.cfg_ints[#name]
#define COPY_BOOL(cfg, map, name) cfg.name = map.cfg_bools[#name]

class Synth_Cfg;
class Oscilator_Cfg;

enum ENTRY_TYPE {
  BOOL,
  INT,
  FLOAT,
  STRING,
  LUA_TABLE,
};

struct Entry {
  Entry(std::string str, ENTRY_TYPE val) : name(str), type(val) {}
  std::string name;
  ENTRY_TYPE type;
};

struct Literals {
  Literals(std::vector<Entry> flds,
           std::vector<std::pair<Entry, std::vector<Entry>>> tbls);
  std::vector<Entry> fields_array;
  std::vector<std::pair<Entry, std::vector<Entry>>> tables_array;
};

struct Base_Maps {
  Base_Maps(void) : cfg_floats(), cfg_ints(), cfg_strings() {}
  std::unordered_map<std::string, f32> cfg_floats;
  std::unordered_map<std::string, i32> cfg_ints;
  std::unordered_map<std::string, std::string> cfg_strings;
  std::unordered_map<std::string, bool> cfg_bools;
};

struct Osc_Map {
  Osc_Map(void) : cfg_ints(), cfg_floats() {}
  std::unordered_map<std::string, i32> cfg_ints;
  std::unordered_map<std::string, f32> cfg_floats;
};

struct Cfg_Maps {
  Cfg_Maps(void) : osc_maps(), base_cfg() {}
  std::vector<Osc_Map> osc_maps;
  Base_Maps base_cfg;

  std::vector<Oscilator_Cfg> make_internal_osc_cfg(void);
  Synth_Cfg make_internal_base_cfg(void);
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
  Cfg_Builder(Literals flds, Lua_Container &lc_);
  Cfg_Builder &loop_fields(void);
  Lua_Cfg build();

private:
  Lua_Container &lc_;
  Literals fields;
  Cfg_Maps maps;
};

class Lua_Container {
public:
  Lua_Container(void) = default;
  ~Lua_Container(void) = default;
  Lua_Cfg load_cfg(const char *filepath);
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
  bool get_bool(void);
  std::string get_str(void);

private:
  lua_State *L = NULL;
};

#endif
