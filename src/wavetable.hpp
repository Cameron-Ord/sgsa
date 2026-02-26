#ifndef WAVE_TABLE_HPP
#define WAVE_TABLE_HPP
#include "typedef.hpp"
#include <memory>

enum table_locations {
    TABLE_TRIANGLE,
    TABLE_SQUARE,
    TABLE_PULSE_25,
    TABLE_PULSE_125,
    TABLE_END
};

class Wave_Table {
public:
    Wave_Table(size_t table_size);
    ~Wave_Table(void);
private:
    std::unique_ptr<f32[]> tables[TABLE_END];
};

#endif