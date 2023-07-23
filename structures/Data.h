#ifndef DATA_H
#define DATA_H

#include <string>
#include <vector>

using namespace std;

struct Data
{
    string inputFilename;
    int numGenerations;
    vector<vector<int>> grid;
};

#endif