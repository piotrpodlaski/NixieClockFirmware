#ifndef MAPPING_H
#define MAPPING_H

#include <map>

namespace mapping {

  typedef std::pair<uint8_t, uint8_t> key_t;
  typedef const std::map<key_t, uint8_t> nixieMapping_t;
  typedef const std::vector<uint8_t> neonMap_t;

  nixieMapping_t nixieDigitMapping{
    //{{lamp(from left),digit}, srOut+8*chipID}
    {{0, 0}, 0 + 0 * 8},
    {{0, 1}, 1 + 0 * 8},
    {{0, 2}, 2 + 0 * 8},
    {{0, 3}, 3 + 0 * 8},
    {{0, 4}, 4 + 0 * 8},
    {{0, 5}, 5 + 0 * 8},
    {{0, 6}, 6 + 0 * 8},
    {{0, 7}, 7 + 0 * 8},
    {{0, 8}, 0 + 1 * 8},
    {{0, 9}, 1 + 1 * 8},
    {{1, 0}, 4 + 1 * 8},
    {{1, 1}, 5 + 1 * 8},
    {{1, 2}, 6 + 1 * 8},
    {{1, 3}, 7 + 1 * 8},
    {{1, 4}, 0 + 2 * 8},
    {{1, 5}, 1 + 2 * 8},
    {{1, 6}, 2 + 2 * 8},
    {{1, 7}, 3 + 2 * 8},
    {{1, 8}, 4 + 2 * 8},
    {{1, 9}, 5 + 2 * 8},
    {{2, 0}, 0 + 3 * 8},
    {{2, 1}, 1 + 3 * 8},
    {{2, 2}, 2 + 3 * 8},
    {{2, 3}, 3 + 3 * 8},
    {{2, 4}, 4 + 3 * 8},
    {{2, 5}, 5 + 3 * 8},
    {{2, 6}, 6 + 3 * 8},
    {{2, 7}, 7 + 3 * 8},
    {{2, 8}, 0 + 4 * 8},
    {{2, 9}, 1 + 4 * 8},
    {{3, 0}, 4 + 4 * 8},
    {{3, 1}, 5 + 4 * 8},
    {{3, 2}, 6 + 4 * 8},
    {{3, 3}, 7 + 4 * 8},
    {{3, 4}, 0 + 5 * 8},
    {{3, 5}, 1 + 5 * 8},
    {{3, 6}, 2 + 5 * 8},
    {{3, 7}, 3 + 5 * 8},
    {{3, 8}, 4 + 5 * 8},
    {{3, 9}, 5 + 5 * 8}
  
  };
  
  nixieMapping_t nixieDpMapping{
    {{0, 0}, 2 + 1 * 8}, //left decimal point
    {{0, 1}, 3 + 1 * 8}, //right decimal point
    {{1, 0}, 6 + 2 * 8}, //left decimal point
    {{1, 1}, 7 + 2 * 8}, //right decimal point
    {{2, 0}, 2 + 4 * 8}, //left decimal point
    {{2, 1}, 3 + 4 * 8}, //right decimal point
    {{3, 0}, 6 + 5 * 8}, //left decimal point
    {{3, 1}, 7 + 5 * 8} //right decimal point
  };

neonMap_t neonTubes{0 + 6 * 8, 1 + 6 * 8};
}

#endif //MAPPING_H
