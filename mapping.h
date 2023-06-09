#ifndef MAPPING_H
#define MAPPING_H

#include <map>

namespace mapping {
  
  typedef std::pair<uint8_t, uint8_t> key_t;
  typedef const std::map<key_t, uint8_t> nixieMapping_t;
  typedef const std::vector<uint8_t> neonMap_t;
  
  //four lamps:
  
  
  nixieMapping_t nixieDigitMappingFourLamp{
    //{{lamp(from left),digit}, serOut+8*chipID}
    {{0, 0}, 44},
    {{0, 1}, 41},
    {{0, 2}, 40},
    {{0, 3}, 23},
    {{0, 4}, 22},
    {{0, 5}, 21},
    {{0, 6}, 20},
    {{0, 7}, 19},
    {{0, 8}, 18},
    {{0, 9}, 45},
    {{1, 0}, 50},
    {{1, 1}, 47},
    {{1, 2}, 46},
    {{1, 3}, 17},
    {{1, 4}, 16},
    {{1, 5}, 15},
    {{1, 6}, 14},
    {{1, 7}, 13},
    {{1, 8}, 12},
    {{1, 9}, 51},
    {{2, 0}, 56},
    {{2, 1}, 53},
    {{2, 2}, 52},
    {{2, 3}, 11},
    {{2, 4}, 10},
    {{2, 5}, 9},
    {{2, 6}, 8},
    {{2, 7}, 7},
    {{2, 8}, 6},
    {{2, 9}, 57},
    {{3, 0}, 62},
    {{3, 1}, 59},
    {{3, 2}, 58},
    {{3, 3}, 5},
    {{3, 4}, 4},
    {{3, 5}, 3},
    {{3, 6}, 2},
    {{3, 7}, 1},
    {{3, 8}, 0},
    {{3, 9}, 63}
  };
  
  nixieMapping_t nixieDpMappingFourLamp{
    {{0, 0}, 21}, //left
    {{0, 1}, 20}, //right
    {{1, 0}, 15}, //left
    {{1, 1}, 14}, //right
    {{2, 0}, 9 }, //left
    {{2, 1}, 8 }, //right
    {{3, 0}, 3 }, //left
    {{3, 1}, 2 }  //right
  };
  
  neonMap_t neonTubesFourLamp{39};
  
  
  //six lamps:
  
  nixieMapping_t nixieDigitMappingSixLamp{
    //{{lamp(from left),digit}, serOut+8*chipID}
    {{0, 0}, 0 },
    {{0, 1}, 1 },
    {{0, 2}, 2 },
    {{0, 3}, 3 },
    {{0, 4}, 4 },
    {{0, 5}, 5 },
    {{0, 6}, 6 },
    {{0, 7}, 7 },
    {{0, 8}, 8 },
    {{0, 9}, 9 },
    {{1, 0}, 10},
    {{1, 1}, 11},
    {{1, 2}, 12},
    {{1, 3}, 13},
    {{1, 4}, 14},
    {{1, 5}, 15},
    {{1, 6}, 16},
    {{1, 7}, 17},
    {{1, 8}, 18},
    {{1, 9}, 19},
    {{2, 0}, 20},
    {{2, 1}, 21},
    {{2, 2}, 22},
    {{2, 3}, 23},
    {{2, 4}, 24},
    {{2, 5}, 25},
    {{2, 6}, 26},
    {{2, 7}, 27},
    {{2, 8}, 28},
    {{2, 9}, 29},
    {{3, 0}, 30},
    {{3, 1}, 31},
    {{3, 2}, 32},
    {{3, 3}, 33},
    {{3, 4}, 34},
    {{3, 5}, 35},
    {{3, 6}, 36},
    {{3, 7}, 37},
    {{3, 8}, 38},
    {{3, 9}, 39},
    {{4, 0}, 40},
    {{4, 1}, 41},
    {{4, 2}, 42},
    {{4, 3}, 43},
    {{4, 4}, 44},
    {{4, 5}, 45},
    {{4, 6}, 46},
    {{4, 7}, 47},
    {{4, 8}, 48},
    {{4, 9}, 49},
    {{5, 0}, 50},
    {{5, 1}, 51},
    {{5, 2}, 52},
    {{5, 3}, 53},
    {{5, 4}, 54},
    {{5, 5}, 55},
    {{5, 6}, 56},
    {{5, 7}, 57},
    {{5, 8}, 58},
    {{5, 9}, 59}
  };
  
  nixieMapping_t nixieDpMappingSixLamp{};
  
  neonMap_t neonTubesSixLamp{};
}

#endif //MAPPING_H
