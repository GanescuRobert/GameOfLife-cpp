// The header files for input-output operations, string operations, file operations, and math operations have been included.
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <cmath>

// `sys/stat.h` header is used to create a new directory in which output files will be saved as text files.
#include <sys/stat.h>

#include "../structures/Data.h"
#include "../utils.cpp"
#include "../constants.h"

using namespace std;
using namespace chrono;
using namespace Constants;
namespace fs = filesystem;

/// @brief Validates if the cell is inside the grid.
/// @param neighbRow the line the neighbor is on
/// @param neighbCol the column the neighbor is on
/// @param size the number of rows and columns in the square grid
/// @return true if the input indices of neighbour cell exist inside the bound of rows and columns of the grid else false
bool isValidCell(int neighbRow, int neighbCol, const int size)
{
	return (static_cast<unsigned>(neighbRow) < static_cast<unsigned>(size)) &
		   (static_cast<unsigned>(neighbCol) < static_cast<unsigned>(size));
}

/// @brief Calculates the number of alive neighbouring cells for the particular cell at the given position.
/// @param grid John Conway's Game of Life ( The grid )
/// @param currRow the line on which the rules apply
/// @param currCol the column on which the rules apply
/// @return the number of alive neighbouring cells
int getNeighboursAlive(const vector<vector<int>> &grid, const int currRow, const int currCol)
{
	const int coordinatesNeighbors[8][2] = {
		{-1, -1}, // Left Up
		{-1, 0},  // Mid Up
		{-1, 1},  // Right Up
		{0, -1},  // Left Mid
		{0, 1},	  // Right Mid
		{1, -1},  // Down Left
		{1, 0},	  // Down Mid
		{1, 1}	  // Down Right
	};
	const int size = grid.size();
	int neighboursAlive = 0;
	const int coordinatesNeighborsSize = 8;
	for (int neighbor = 0; neighbor < coordinatesNeighborsSize; neighbor++)
	{
		const int neighbRow = currRow + coordinatesNeighbors[neighbor][0];
		const int neighbCol = currCol + coordinatesNeighbors[neighbor][1];
		if (isValidCell(neighbRow, neighbCol, size))
		{
			neighboursAlive += grid[neighbRow][neighbCol] == LIVE;
		}
	}
	return neighboursAlive;
}

/// @brief Calculates the status of the cell at the given indices for the next generation by checking the number of alive neighbouring cells.
/// @param grid John Conway's Game of Life ( The grid )
/// @param currRow the line on which the rules apply
/// @param currCol the column on which the rules apply
/// @return the status of the cell
char getCurrentState(const vector<vector<int>> &grid, const int currRow, const int currCol)
{
	const int neighboursAlive = getNeighboursAlive(grid, currRow, currCol);
	return grid[currRow][currCol] ? ((neighboursAlive > 1 && neighboursAlive < 4) ? LIVE : DEAD) : ((neighboursAlive == 3) ? LIVE : DEAD);
}

/// @brief Updates the grid with the new status of each cell from the next generation.
/// @param grid John Conway's Game of Life ( The grid )
/// @param nextGrid next generation of John Conway's Game of Life ( The grid )
void updateGrid(vector<vector<int>> &grid, const vector<vector<int>> &nextGrid)
{
	const int size = grid.size();
	for (int row = 0; row < size; row++)
	{
		for (int col = 0; col < size; col++)
		{
			grid[row][col] = nextGrid[row][col];
		}
	}
}

/// @brief Creates a new grid based on the current grid by applying rules of the Game of Life.
/// @param grid John Conway's Game of Life ( The grid )
/// @return the new grid based on the current grid by applying rules
vector<vector<int>> getNextGrid(vector<vector<int>> &grid)
{
	static vector<vector<int>> nextGrid(grid.size(), vector<int>(grid[0].size()));
	const int size = grid.size();
	for (int i = 0; i < size * size; ++i)
	{
		const int row = i / size;
		const int col = i % size;
		nextGrid[row][col] = getCurrentState(grid, row, col);
	}
	return nextGrid;
}

/// @brief If it meets specific criteria, it saves the current generation of the grid to a file.
/// @param grid John Conway's Game of Life ( The grid )
/// @param inputFilename the name of the input file
/// @param generation the generation number
/// @return true if the current generation is saved else false
bool saveCurrentGeneration(Data &configuration, int generation)
{
	const vector<vector<int>> grid = configuration.grid;
	const int size = grid.size();

	if (size <= 4 * BORDER_SIZE)
	{
		return false;
	}

	const string folderName = "output/";
	if (!fs::exists(folderName))
	{
		fs::create_directory(folderName);
	}

	const string fileExtension = ".txt";
	const string outputFilename = configuration.inputFilename + "_" + to_string(configuration.numGenerations);
	const string filePath = folderName + outputFilename + fileExtension;

	ofstream outfile(filePath, ios::app);
	if (!outfile)
	{
		return false;
	}

	outfile << generation << ": ";
	for (int row = BORDER_SIZE; row < size - BORDER_SIZE; ++row)
	{
		for (int col = BORDER_SIZE; col < size - BORDER_SIZE; ++col)
		{
			outfile << grid[row][col];
		}
	}
	outfile << endl;
	return true;
}

/// @brief Converts a string representation to a 2D vector of integers.
/// @param str The string representation of the grid.
/// @param size the number of rows and columns in the square grid
/// @return The 2D vector of integers representing the grid.
vector<vector<int>> stringToVector2D(const string &str, const int size)
{
	vector<vector<int>> grid(size, vector<int>(size, 0));
	for (int i = 0; i < str.size(); ++i)
	{
		int row = i / size;
		int col = i % size;
		grid[row][col] = str[i] - '0';
	}
	return grid;
}

/// @brief Splits the input data into individual words and returns a list with each word as an element.
/// @param inputData the input data as a string
/// @return the configurations and grid.
vector<string> deconstructInputData(const string &inputData)
{
	istringstream iss(inputData);
	vector<string> words;
	words.reserve(3);
	string word;
	while (iss >> word)
	{
		words.emplace_back(move(word));
	}
	return words;
}

/// @brief Adds a border of two zeros around its edges.
/// @param grid John Conway's Game of Life ( The grid )
void addBoarder(vector<vector<int>> &grid)
{
	const int size = grid.size();

	// Add 0's at the beginning and end of each row
	for (int i = 0; i < size; i++)
	{
		grid[i].insert(grid[i].begin(), BORDER_SIZE, 0);
		grid[i].insert(grid[i].end(), BORDER_SIZE, 0);
	}

	// Add new border rows at the beginning and end of the grid
	const int numBorderCols = size + 2 * BORDER_SIZE;
	vector<int> newBorder(numBorderCols, 0);
	grid.insert(grid.begin(), BORDER_SIZE, newBorder);
	grid.insert(grid.end(), BORDER_SIZE, newBorder);
}

/// @brief Returns a list of elements that represent the lines and columns that will be cleaned.
/// @param size the maximum number of rows or columns
/// @return the list of elements that will be cleaned
vector<int> toBeCleaned(const int size)
{
	return vector<int>{0, 1, size - 2, size - 1};
}

/// @brief Cleans all the columns of the grid that are marked as "to be cleaned" with value 0.
/// @param grid John Conway's Game of Life ( The grid )
void cleanCols(vector<vector<int>> &grid)
{
	const int size = grid.size();
	vector<int> colsToBeCleaned = toBeCleaned(size);
	for (int row = 0; row < size; ++row)
	{
		for (auto &colToBeCleaned : colsToBeCleaned)
		{
			grid[row][colToBeCleaned] = 0;
		}
	}
}

/// @brief Cleans all the rows of the grid that are marked as "to be cleaned" with value 0.
/// @param grid John Conway's Game of Life ( The grid )
void cleanRows(vector<vector<int>> &grid)
{
	const int size = grid.size();
	vector<int> rowsToBeCleaned = toBeCleaned(size);
	for (int col = 0; col < size; ++col)
	{
		for (auto &rowToBeCleaned : rowsToBeCleaned)
		{
			grid[rowToBeCleaned][col] = 0;
		}
	}
}

/// @brief Zero out all values ​​in the specific rows and columns of the grid.
/// @param grid John Conway's Game of Life ( The grid )
void cleanIt(vector<vector<int>> &grid)
{
	cleanRows(grid);
	cleanCols(grid);
}

/// @brief Check if there is a value of 1 on the added border, on the top or bottom border.
/// @param grid John Conway's Game of Life ( The grid )
/// @return true if top or bottom border contains a value of 1 else false
bool isOnTopOrBottomBorder(vector<vector<int>> &grid)
{
	const int size = grid.size();
	for (int row = 0; row < size; ++row)
	{
		if (grid[row][0] == 1 || grid[row][size - 1] == 1)
		{
			return true;
		}
	}
	return false;
}

/// @brief Check if there is a value of 1 on the added border, on the left or right border.
/// @param grid John Conway's Game of Life ( The grid )
/// @return true if left or right border contains a value of 1 else false
bool isOnLeftOrRightBorder(vector<vector<int>> &grid)
{
	const int size = grid.size();
	for (int col = 0; col < size; ++col)
	{
		if (grid[0][col] == 1 || grid[size - 1][col] == 1)
		{
			return true;
		}
	}
	return false;
}

/// @brief Checks if there is a one on any of the borders of the grid.
/// @param grid John Conway's Game of Life ( The grid )
/// @return true if left or right border contains a value of 1 else false
bool isOneOnBorder(vector<vector<int>> &grid)
{
	if (isOnTopOrBottomBorder(grid))
		return true;
	return isOnLeftOrRightBorder(grid);
}

/// @brief Zero out all values in the border rows and columns of the grid if border contains one.
/// @param grid John Conway's Game of Life ( The grid )
void cleanBoarder(vector<vector<int>> &grid)
{
	if (isOneOnBorder(grid))
		cleanIt(grid);
}

/// @brief Takes command-line input arguments from the user
/// @param argc the number of arguments entered on the command line
/// @param argv the arguments entered on the command line
/// @return the input filename (if present) or 'null' (if not).
pair<string, int> getInputData(int argc, char **argv)
{
	if (argc != 3)
	{
		return make_pair("null", 0);
	}
	return make_pair(argv[1], stoi(argv[2]));
}

/// @brief Prepares a tuple containing useful data needed for the Game of Life simulation.
/// @param argc the number of arguments entered on the command line
/// @param argv the arguments entered on the command line
/// @return the structure containing the input data filename, the number of generations and John Conway's Game of Life ( The grid )
Data prepareGameOfLife(int argc, char **argv)
{
	Data configuration;
	auto [inputFilename, numGenerations] = getInputData(argc, argv);
	if (inputFilename == "null")
	{
		configuration.inputFilename = inputFilename;
		configuration.numGenerations = numGenerations;
		configuration.grid = vector<vector<int>>();
		return configuration;
	}
	setSysStdout(inputFilename, numGenerations);
	const string gridStr = readFile(inputFilename);
	const int gridSize = static_cast<int>(sqrt(gridStr.size()));

	vector<vector<int>> grid = stringToVector2D(gridStr, gridSize);
	addBoarder(grid);

	configuration.inputFilename = inputFilename;
	configuration.numGenerations = numGenerations;
	configuration.grid = grid;
	return configuration;
}

/// @brief Simulates the Game of Life for a given number of generations, updates and saves the grid on each generation.
/// @param configuration {	inputFilename: the input data filename
///							numGenerations: the number of generations
///							grid: John Conway's Game of Life ( The grid )
///						}
void saveGameOfLife(Data configuration)
{
	for (int generation = 0; generation < configuration.numGenerations; generation++)
	{
		if (saveCurrentGeneration(configuration, generation))
		{
			vector<vector<int>> nextGrid = getNextGrid(configuration.grid);
			updateGrid(configuration.grid, nextGrid);
			cleanBoarder(configuration.grid);
		}
	}
}

/// @brief Simulates the Game of Life for a given number of generations and updates the grid on each generation.
/// @param configuration {	inputFilename: the input data filename
///							numGenerations: the number of generations
///							grid: John Conway's Game of Life ( The grid )
///						}
void playGameOfLife(Data configuration)
{
	int generation = 0;
	while (generation < configuration.numGenerations)
	{
		vector<vector<int>> nextGrid = getNextGrid(configuration.grid);
		updateGrid(configuration.grid, nextGrid);
		cleanBoarder(configuration.grid);

		generation++;
	}
}

/// @brief This is a Python code that represents the main() function.
/// The purpose of this function is to simulate Conway's Game of Life, which is a cellular automation program.
int main(int argc, char **argv)
{
	vector<high_resolution_clock::time_point> timePoints;

	timePoints.emplace_back(high_resolution_clock::now());
	Data configuration = prepareGameOfLife(argc, argv);
	if (configuration.inputFilename == "null")
	{
		cout << "The number of arguments is not correct\n";
		cout << "Game of life did not complete successfully";
		return 0;
	}

	timePoints.emplace_back(high_resolution_clock::now());
	saveGameOfLife(configuration);

	timePoints.emplace_back(high_resolution_clock::now());
	playGameOfLife(configuration);

	timePoints.emplace_back(high_resolution_clock::now());
	measureExecutionTime(timePoints);

	cout << "Game of life completed successfully";
}
