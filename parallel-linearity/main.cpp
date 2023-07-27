#include <iostream>
#include <fstream>
#include <vector>
#include <mpi.h>
#include <ctime>
#include <string>
#include "generate_image_video.cpp"

#include "../structures/Data.h"
#include "../utils.cpp"
#include "../constants.h"

using namespace std;
using namespace chrono;
using namespace Constants;

int SIZE = 0;

MPI_Comm comm;
int size, rank;

int shape_grid_rows;
int shape_grid_cols;

int distribution;

vector<vector<short>> workspace;
vector<vector<short>> neighbor;
vector<vector<vector<vector<short>>>> neighbor_id;

void timing(vector<char> (*f)())
{
    double time1, time2;
    if (rank == 0)
    {
        time1 = MPI_Wtime();
    }
    f();
    if (rank == 0)
    {
        time2 = MPI_Wtime();
        cout << "Function took " << (time2 - time1) * 1000.0 << " ms" << endl;
    }
}

class Game;

class Engine
{
public:
    Engine(Game *game);

    vector<vector<vector<vector<short>>>> make_neighbor_indices();

    void count_neighbors();

    void up();

    void down();

    void clear_border();

    void set_ghost();

    void update_workspace();

    void next_state();

private:
    Game *_game;
    vector<vector<short>> _neighbor;
    vector<vector<vector<vector<short>>>> _neighbor_id;
};

class Game
{
public:
    vector<vector<short>> _workspace;

    Game(const vector<vector<short>> &workspace);

    void animate(int no_iter);

private:
    Engine _engine;

    void gatherGrid();

    vector<vector<short>> preprocess_grid(const vector<vector<vector<vector<short>>>> &gatherGrid);
};

Engine::Engine(Game *game)
{
    _game = game;
    _neighbor.resize(game->_workspace.size(), vector<short>(game->_workspace[0].size(), 0));
    _neighbor_id = make_neighbor_indices();
}

vector<vector<vector<vector<short>>>> Engine::make_neighbor_indices()
{
    vector<vector<vector<vector<short>>>> out(8, vector<vector<vector<short>>>(2, vector<vector<short>>(_game->_workspace.size(), vector<short>(_game->_workspace[0].size(), 0))));
    vector<vector<short>> d = {vector<short>(), vector<short>(_game->_workspace[0].size(), 0), vector<short>(_game->_workspace[0].size(), _game->_workspace.size() - 1)};
    vector<pair<int, int>> d2 = {{0, 1}, {1, 1}, {1, 0}, {1, -1}};
    for (int i = 0; i < 8; i++)
    {
        int x = d2[i].first;
        int y = d2[i].second;
        out[i][0] = d[x];
        out[i][1] = d[y];
        out[7 - i][0] = d[-x];
        out[7 - i][1] = d[-y];
    }
    return out;
}

void Engine::count_neighbors()
{
    for (int i = 0; i < _game->_workspace.size(); i++)
    {
        for (int j = 0; j < _game->_workspace[0].size(); j++)
        {
            _neighbor[i][j] = 0;
        }
    }

    vector<vector<short>> &workspace = _game->_workspace;
    vector<vector<vector<vector<short>>>> &n_id = _neighbor_id;
    vector<vector<short>> &n = _neighbor;

    for (int i = 0; i < 8; i++)
    {
        vector<short> &neighbor_ptr_x = workspace[n_id[i][0][0][0]];
        vector<short> &neighbor_ptr_y = workspace[n_id[i][1][0][0]];

        for (int row = 0; row < workspace.size(); row++)
        {
            for (int col = 0; col < workspace[0].size(); col++)
            {
                short neighbor_mat = neighbor_ptr_x[col];
                short zero_short = 0;

                if (neighbor_mat != zero_short)
                {
                    n[row][col] += neighbor_mat;
                }
            }
        }
    }
}

void Engine::up()
{
    vector<vector<short>> &w = _game->_workspace;
    if (rank != size - 1)
    {
        MPI_Send(w[shape_grid_rows], w[shape_grid_rows].size() * sizeof(short), MPI_BYTE, rank + 1, 0, comm);
        MPI_Recv(w[shape_grid_rows + 1].data(), w[shape_grid_rows + 1].size() * sizeof(short), MPI_BYTE, rank + 1, 0, comm, MPI_STATUS_IGNORE);
    }
}

void Engine::down()
{
    vector<vector<short>> &w = _game->_workspace;
    if (rank != 0)
    {
        MPI_Send(w[1].data(), w[1].size() * sizeof(short), MPI_BYTE, rank - 1, 0, comm);
        MPI_Recv(w[0].data(), w[0].size() * sizeof(short), MPI_BYTE, rank - 1, 0, comm, MPI_STATUS_IGNORE);
    }
}

void Engine::clear_border()
{
    vector<vector<short>> &w = _game->_workspace;
    for (int col = 0; col < w[0].size(); col++)
    {
        w[0][col] = 0;
        w[w.size() - 1][col] = 0;
    }
}

void Engine::set_ghost()
{
    if (rank == 0)
    {
        up();
    }
    else if (rank == size - 1)
    {
        down();
    }
    else
    {
        up();
        down();
    }
}

void Engine::update_workspace()
{
    vector<vector<short>> &w = _game->_workspace;
    vector<vector<short>> &n = _neighbor;

    for (int i = 0; i < w.size(); i++)
    {
        for (int j = 0; j < w[0].size(); j++)
        {
            w[i][j] = (n[i][j] == 2) || (n[i][j] == 3) ? w[i][j] : 0;
        }
    }
}

void Engine::next_state()
{
    clear_border();
    set_ghost();
    count_neighbors();
    update_workspace();
}

Game::Game(const vector<vector<short>> &workspace) : _workspace(workspace), _engine(this)
{
}

void Game::animate(int no_iter)
{
    while (no_iter)
    {
        gatherGrid();
        _engine.next_state();
        no_iter--;
    }
}

void Game::gatherGrid()
{
    vector<vector<short>> workspace_extended;
    if (rank == 0)
    {
        workspace_extended = vector<vector<short>>(SIZE, vector<short>(SIZE, 0));
    }
    vector<vector<vector<vector<short>>>> gatherGrid(size);
    MPI_Gather(&_workspace[0][0], SIZE * shape_grid_rows, MPI_SHORT,
               &workspace_extended[0][0], SIZE * shape_grid_rows, MPI_SHORT, 0, comm);
    if (rank == 0)
    {
        gatherGrid = preprocess_grid(workspace_extended);
    }
    MPI_Scatter(&gatherGrid[0][0][0][0], SIZE * shape_grid_rows, MPI_SHORT,
                &_workspace[0][0], SIZE * shape_grid_rows, MPI_SHORT, 0, comm);
}

vector<vector<short>> Game::preprocess_grid(const vector<vector<vector<vector<short>>>> &gatherGrid)
{
    vector<vector<short>> workspace_extended = gatherGrid[0][0];
    for (int i = 1; i < size; i++)
    {
        for (int j = 0; j < shape_grid_rows; j++)
        {
            for (int k = 0; k < SIZE; k++)
            {
                workspace_extended[i * shape_grid_rows + j][k] = gatherGrid[i][j][k][0];
            }
        }
    }
    return workspace_extended;
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

/// @brief Prepares a tuple containing useful data needed for the Game of Life simulation.
/// @param argc the number of arguments entered on the command line
/// @param argv the arguments entered on the command line
/// @return the structure containing the input data filename, the number of generations, and John Conway's Game of Life (The grid)
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
    setSysStdout(inputFilename);
    const string inputData = readFile(inputFilename);
    vector<string> words = deconstructInputData(inputData);

    const int size = stoi(words[0]);
    const int numGenerations = stoi(words[1]);
    const string strGrid = words[2];
    vector<vector<int>> grid(size, vector<int>(size, 0));

    initGrid(grid, strGrid);
    addBoarder(grid);

    configuration.inputFilename = inputFilename;
    configuration.numGenerations = numGenerations;
    configuration.grid = grid;
    return configuration;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    comm = MPI_COMM_WORLD;
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);

    if (configuration.inputFilename == "null")
    {
        if (rank == 0)
        {
            cout << "The number of arguments is not correct\n";
            cout << "Game of life did not complete successfully";
        }
        MPI_Finalize();
        return 0;
    }

    if (rank == 0)
    {
        // Prepare the Game of Life simulation using prepareGameOfLife function
        Data configuration = prepareGameOfLife(argc, argv);
        workspace = configuration.grid;
        SIZE = configuration.grid.size();

        shape_grid_rows = SIZE / size;
        shape_grid_cols = SIZE;
        
        distribution = SIZE / size;
    }

    // Broadcast the workspace data to all ranks
    MPI_Bcast(&workspace[0][0], SIZE * shape_grid_rows, MPI_SHORT, 0, comm);

    neighbor = vector<vector<short>>(shape_grid_rows + 2, vector<short>(shape_grid_cols, 0));
    neighbor_id = Engine(nullptr).make_neighbor_indices();

    timing([]()
           {
        Game game(workspace);
        game.animate(configuration.generations); });

    MPI_Finalize();

    return 0;
}