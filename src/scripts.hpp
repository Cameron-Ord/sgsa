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

struct Synth_Cfg;
struct Oscilator_Cfg;

enum ENTRY_TYPE {
  BOOL,
  INT,
  FLOAT,
  STRING,
  LUA_TABLE,
};

struct Entry {
  Entry(std::string str, ENTRY_TYPE val) : name(str), type(val), offset(0) {}
  Entry(std::string str, ENTRY_TYPE val, size_t _offset)
      : name(str), type(val), offset(_offset) {}
  std::string name;
  ENTRY_TYPE type;
  size_t offset;
};

struct Field_Cluster {
  Field_Cluster(std::vector<Entry> flds,
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

class Lua_Container;

class Cfg_Builder {
public:
  Cfg_Builder(Field_Cluster flds, Lua_Container &lc_);
  void build_base_fields(Synth_Cfg &synth);
  void build_oscilator_fields(std::vector<Oscilator_Cfg> &oscs);

private:
  Lua_Container &lc_;
  Field_Cluster fields;
};

class Lua_Container {
public:
  Lua_Container(void) = default;
  ~Lua_Container(void) = default;
  bool load_cfg(const char *filepath, Synth_Cfg &synth,
                std::vector<Oscilator_Cfg> &oscs);
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
