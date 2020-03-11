#ifndef MAZEGEN_H
#define MAZEGEN_H
#include <vector>
#include <iostream>
#include "point.h"
#include "wall.h"

using namespace std;

class MazeGen
{
public:
    MazeGen();
    vector<Wall *> generateBorder();
};

#endif // MAZEGEN_H
