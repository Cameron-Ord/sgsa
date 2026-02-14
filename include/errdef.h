#ifndef ERRDEF_H
#define ERRDEF_H
enum RETURN_CODES {
    UNSET = -1,
    NO_MEM = 0,
    READ_FAILED = 1,
    WRITE_FAILED = 2,
    BAD_PARAM = 3,
    UNSPECIFIED = 4,
    OK = 5,
    FILE_OPEN_FAIL = 6,
    NO_MATCH = -2
};
#endif