// The header files for input-output operations, string operations, file operations, and math operations have been included.
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

// `sys/stat.h` header is used to create a new directory in which output files will be saved as text files.
#include <sys/stat.h>

using namespace std;

// Two global constant variables `LIVE` and `DEAD` are assigned with integer values to represent the status of living cell and dead cell.
const char LIVE = 1;
const char DEAD = 0;

struct Data
{
    string inputFilename;
    int numGenerations;
    vector<vector<int>> grid;
};

// One global constant variable `BORDER_SIZE` is assigned with integer value that represent the size of grid border.
const int BORDER_SIZE = 2;

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

/// @brief Validates if the cell is inside the grid.
/// @param neighbRow the line the neighbor is on
/// @param neighbCol the column the neighbor is on
/// @param rows the number of grid lines
/// @param cols the number of grid columns
/// @return true if the input indices of neighbour cell exist inside the bound of rows and columns of the grid else false
bool isValidCell(int neighbRow, int neighbCol, const int rows, const int cols)
{
    return (static_cast<unsigned>(neighbRow) < static_cast<unsigned>(rows)) &
           (static_cast<unsigned>(neighbCol) < static_cast<unsigned>(cols));
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
        {0, 1},   // Right Mid
        {1, -1},  // Down Left
        {1, 0},   // Down Mid
        {1, 1}    // Down Right
    };
    const int rows = grid.size();
    const int cols = grid[0].size();
    int neighboursAlive = 0;
    const int coordinatesNeighborsSize = 8;
    for (int neighbor = 0; neighbor < coordinatesNeighborsSize; neighbor++)
    {
        const int neighbRow = currRow + coordinatesNeighbors[neighbor][0];
        const int neighbCol = currCol + coordinatesNeighbors[neighbor][1];
        if (isValidCell(neighbRow, neighbCol, rows, cols))
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
    const int rows = grid.size();
    const int cols = grid[0].size();
    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
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
    const int rows = grid.size();
    const int cols = grid[0].size();
    for (int i = 0; i < rows * cols; ++i)
    {
        const int row = i / cols;
        const int col = i % cols;
        nextGrid[row][col] = getCurrentState(grid, row, col);
    }
    return nextGrid;
}

/// @brief If it meets specific criteria, it saves the current iteration of the grid to a file.
/// @param grid John Conway's Game of Life ( The grid )
/// @param inputFilename the name of the input file
/// @param iteration the generation number
/// @return true if the current iteration is saved else false
bool saveCurrentIteration(vector<vector<int>> &grid, string inputFilename, int iteration)
{
    const int cols = grid[0].size();
    const int rows = grid.size();
    if (rows <= 4 * BORDER_SIZE || cols <= 4 * BORDER_SIZE)
        return false;

    string filename = "output\\" + inputFilename + to_string(rows - 2 * BORDER_SIZE) + "x" + to_string(cols - 2 * BORDER_SIZE) + ".txt";
    ofstream outfile(filename, ios::app);
    if (!outfile)
        return false;

    outfile << iteration << ": ";
    for (int row = BORDER_SIZE; row < rows - BORDER_SIZE; ++row)
    {
        for (int col = BORDER_SIZE; col < cols - BORDER_SIZE; ++col)
        {
            outfile << grid[row][col];
        }
    }
    outfile << endl;
    return true;
}

/// @brief Initializes the grid based on the values stored in a string.
/// @param grid John Conway's Game of Life ( The grid )
/// @param strGrid the string representation of the grid
void initGrid(vector<vector<int>> &grid, const string &strGrid)
{
    const int numRows = grid.size();
    const int numCols = grid[0].size();
    for (int i = 0; i < numRows * numCols; ++i)
    {
        int row = i / numCols;
        int col = i % numCols;
        grid[row][col] = strGrid[i] - '0';
    }
}

/// @brief Splits the input data into individual words and returns a list with each word as an element.
/// @param inputData the input data as a string
/// @return the configurations and grid.
vector<string> deconstructInputData(const string &inputData)
{
    istringstream iss(inputData);
    vector<string> words;
    words.reserve(4);
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
    const int numRows = grid.size();
    const int numCols = grid[0].size();

    // Add 0's at the beginning and end of each row
    for (int i = 0; i < numRows; i++)
    {
        grid[i].insert(grid[i].begin(), BORDER_SIZE, 0);
        grid[i].insert(grid[i].end(), BORDER_SIZE, 0);
    }

    // Add new border rows at the beginning and end of the grid
    const int numBorderCols = numCols + 2 * BORDER_SIZE;
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
    const int numRows = grid.size();
    const int numCols = grid[0].size();
    vector<int> colsToBeCleaned = toBeCleaned(numCols);
    for (int row = 0; row < numRows; ++row)
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
    const int numRows = grid.size();
    const int numCols = grid[0].size();
    vector<int> rowsToBeCleaned = toBeCleaned(numRows);
    for (int col = 0; col < numCols; ++col)
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
    const int numRows = grid.size();
    const int numCols = grid[0].size();
    for (int row = 0; row < numRows; ++row)
    {
        if (grid[row][0] == 1 || grid[row][numCols - 1] == 1)
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
    const int numRows = grid.size();
    const int numCols = grid[0].size();
    for (int col = 0; col < numCols; ++col)
    {
        if (grid[0][col] == 1 || grid[numRows - 1][col] == 1)
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

/// @brief Reads the content of the input file as a string.
/// @param filename the name of the input file
/// @return the string representation of the grid
string readFile(const string &filename)
{
    ifstream file("input/" + filename + ".txt");
    if (!file.is_open())
    {
        throw runtime_error("Could not open file " + filename + ".txt");
    }
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

/// @brief Takes command-line input arguments from the user
/// @param argc the number of arguments entered on the command line
/// @param argv the arguments entered on the command line
/// @return the input filename (if present) or 'null' (if not).
string getInputFilename(int argc, char **argv)
{
    if (argc != 2)
    {
        return "null";
    }
    return argv[1];
}

Data prepareGameOfLife(int argc, char **argv)
{
    Data configuration;
    const string inputFilename = getInputFilename(argc, argv);
    if (inputFilename == "null")
    {
        configuration.inputFilename = inputFilename;
        configuration.numGenerations = 0;
        configuration.grid = vector<vector<int>>();
        return configuration;
    }
    const string inputData = readFile(inputFilename);
    vector<string> words = deconstructInputData(inputData);

    const int numRows = stoi(words[0]);
    const int numCols = stoi(words[1]);
    const int numGenerations = stoi(words[2]);
    const string strGrid = words[3];
    vector<vector<int>> grid(numRows, vector<int>(numCols, 0));

    initGrid(grid, strGrid);
    addBoarder(grid);

    configuration.inputFilename = inputFilename;
    configuration.numGenerations = numGenerations;
    configuration.grid = grid;
    return configuration;
}

void saveGameOfLife(Data configuration)
{
    for (int generation = 0; generation < configuration.numGenerations; generation++)
    {
        if (saveCurrentIteration(configuration.grid, configuration.inputFilename, generation))
        {

            vector<vector<int>> nextGrid = getNextGrid(configuration.grid);
            updateGrid(configuration.grid, nextGrid);
            cleanBoarder(configuration.grid);
        }
    }
}
void playGameOfLife(Data configuration)
{

    for (int _ = 0; _ < configuration.numGenerations; _++)
    {
        vector<vector<int>> nextGrid = getNextGrid(configuration.grid);
        updateGrid(configuration.grid, nextGrid);
        cleanBoarder(configuration.grid);
    }
}

/// @brief This is a Python code that represents the main() function.
/// The purpose of this function is to simulate Conway's Game of Life, which is a cellular automation program.
int main(int argc, char **argv)
{
    Data configuration = prepareGameOfLife(argc, argv);
    if (configuration.inputFilename == "null")
    {
        cout << "The number of arguments is not correct\n";
        cout << "Game of life did not complete successfully";
        return 0;
    }
    saveGameOfLife(configuration);
    playGameOfLife(configuration);
    cout << "Game of life completed successfully";
    return 1;
}
