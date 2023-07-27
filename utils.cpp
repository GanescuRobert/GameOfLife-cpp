#include <vector>
#include <iostream>
#include <fstream>
#include <chrono>
#include <filesystem>

#include "./constants.h"

using namespace chrono;
using namespace std;
using namespace Constants;
namespace fs = filesystem;

/// @brief Prints the elements of the 2D grid in the console.
/// @param grid vector<vector<int>> &
void printGrid(vector<vector<int>> &grid)
{
    const int cols = grid[0].size();
    const int rows = grid.size();
    if (rows <= 2 * BORDER_SIZE || cols <= 2 * BORDER_SIZE)
        return;
    for (int row = BORDER_SIZE; row < rows - BORDER_SIZE; ++row)
    {
        for (int col = BORDER_SIZE; col < cols - BORDER_SIZE; ++col)
        {
            cout << grid[row][col] << " ";
        }
        cout << endl;
    }
    cout << endl;
}

/// @brief Sets the standard output (stdout) stream of the C++ interpreter to write to a file.
/// @param inputFilename the name of the input file
void setSysStdout(const string &inputFilename, const int numGenerations)
{
    string folderName = "time_measurements/";
    if (!fs::exists(folderName))
    {
        fs::create_directory(folderName);
    }
    string fileExtension = ".txt";
    const string timeMeasurementsFileame = inputFilename + "_" + to_string(numGenerations);
    string filePath = folderName + timeMeasurementsFileame + fileExtension;
    freopen(filePath.c_str(), "a", stdout);
}

/// @brief Measures the execution time of the methods that I have as a target.
/// @param times time points of the methods that I have as a target.
void measureExecutionTime(vector<high_resolution_clock::time_point> &timePoints)
{
    auto elapsedPrepareGameOfLife = duration_cast<nanoseconds>(timePoints[1] - timePoints[0]);
    auto elapsedSaveGameOfLife = duration_cast<nanoseconds>(timePoints[2] - timePoints[1]);
    auto elapsedPlayGameOfLife = duration_cast<nanoseconds>(timePoints[3] - timePoints[2]);
    cout << "Function prepareGameOfLife = " << elapsedPrepareGameOfLife.count() * 1e-9 << " seconds\n";
    cout << "Function saveGameOfLife = " << elapsedSaveGameOfLife.count() * 1e-9 << " seconds\n";
    cout << "Function playGameOfLife = " << elapsedPlayGameOfLife.count() * 1e-9 << " seconds\n";
}

/// @brief Reads the content of the input file as a string.
/// @param filename the name of the input file
/// @return the string representation of the grid
string readFile(const string &filename)
{
    ifstream file("../../inputData/" + filename + ".txt");
    if (!file.is_open())
    {
        throw runtime_error("Could not open file " + filename + ".txt");
    }
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}