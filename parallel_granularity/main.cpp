#include <iostream>
#include <fstream>
#include <vector>
#include <mpi.h>
#include <ctime>
#include <opencv2/opencv.hpp>
#include "generate_image_video.cpp"

using namespace std;
using namespace cv;

int SIZE;
int generations;
double ratio;

MPI_Comm comm;
int size, rank;

int shape_grid_rows;
int shape_grid_cols;

int distribution;

Mat workspace;
Mat neighbor;
vector<vector<Mat>> neighbor_id;

void timing(vector<char> (*f)())
{
    double time1, time2;
    if (::rank == 0)
    {
        time1 = MPI_Wtime();
    }
    f();
    if (::rank == 0)
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

    vector<vector<Mat>> make_neighbor_indices();

    void count_neighbors();

    void up();

    void down();

    void clear_border();

    void set_ghost();

    void update_workspace();

    void next_state();

private:
    Game *_game;
    Size _shape;
    Mat _neighbor;
    vector<vector<Mat>> _neighbor_id;
};

class Game
{
public:
    Mat _workspace;

    Game(const Mat &workspace);

    void animate(int no_iter);

private:
    Engine _engine;

    void gatherGrid();

    Mat preprocess_grid(const vector<Mat> &gatherGrid);
};

Engine::Engine(Game *game)
{
    _game = game;
    _shape = game->_workspace.size();
    _neighbor = Mat(_shape, CV_16S, Scalar(0));
    _neighbor_id = make_neighbor_indices();
}

vector<vector<Mat>> Engine::make_neighbor_indices()
{
    vector<vector<Mat>> out(8, vector<Mat>(2));
    vector<Mat> d = {Mat(), Mat(_shape, CV_16S, Scalar(0)), Mat(_shape, CV_16S, Scalar(_shape.height - 1))};
    vector<pair<int, int>> d2 = {{0, 1}, {1, 1}, {1, 0}, {1, -1}};
    for (int i = 0; i < 8; i++)
    {
        int x = d2[i].first;
        int y = d2[i].second;
        out[i][0] = d[x].clone();
        out[i][1] = d[y].clone();
        out[7 - i][0] = d[-x].clone();
        out[7 - i][1] = d[-y].clone();
    }
    return out;
}

void Engine::count_neighbors()
{
    _neighbor.setTo(0);
    cv::Mat workspace = _game->_workspace;
    std::vector<std::vector<cv::Mat>> &n_id = _neighbor_id;
    cv::Mat n = _neighbor;

    for (int i = 0; i < 8; i++)
    {
        short *neighbor_ptr_x = workspace.ptr<short>(n_id[i][0].at<short>(0, 0));
        short *neighbor_ptr_y = workspace.ptr<short>(n_id[i][1].at<short>(0, 0));

        for (int row = 0; row < workspace.rows; row++)
        {
            for (int col = 0; col < workspace.cols; col++)
            {
                cv::Vec3b neighbor_mat = cv::Vec3b(neighbor_ptr_x[col], neighbor_ptr_y[col], 0);
                cv::Vec3b zero_vec3b(0, 0, 0);

                n.setTo(n + cv::Scalar(neighbor_mat[0], neighbor_mat[1], neighbor_mat[2]), neighbor_mat != zero_vec3b);
            }

            neighbor_ptr_x += workspace.step1();
            neighbor_ptr_y += workspace.step1();
        }
    }
}

void Engine::up()
{
    Mat w = _game->_workspace;
    if (::rank != size - 1)
    {
        MPI_Send(w.row(distribution).data, SIZE * sizeof(short), MPI_BYTE, ::rank + 1, 0, comm);
        MPI_Recv(w.row(distribution + 1).data, SIZE * sizeof(short), MPI_BYTE, ::rank + 1, 0, comm, MPI_STATUS_IGNORE);
    }
}

void Engine::down()
{
    Mat w = _game->_workspace;
    if (::rank != 0)
    {
        MPI_Send(w.row(1).data, SIZE * sizeof(short), MPI_BYTE, ::rank - 1, 0, comm);
        MPI_Recv(w.row(0).data, SIZE * sizeof(short), MPI_BYTE, ::rank - 1, 0, comm, MPI_STATUS_IGNORE);
    }
}

void Engine::clear_border()
{
    Mat w = _game->_workspace;
    w.row(0).setTo(0);
    w.row(w.rows - 1).setTo(0);
}

void Engine::set_ghost()
{
    if (::rank == 0)
    {
        up();
    }
    else if (::rank == size - 1)
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
    Mat w = _game->_workspace;
    Mat n = _neighbor;
    w &= (n == 2);
    w |= (n == 3);
}

void Engine::next_state()
{
    clear_border();
    set_ghost();
    count_neighbors();
    update_workspace();
}

Game::Game(const Mat &workspace) : _workspace(workspace), _engine(this)
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
    Mat workspace_extended;
    if (::rank == 0)
    {
        workspace_extended = Mat(Size(SIZE, SIZE), CV_16S);
    }
    vector<Mat> gatherGrid(size);
    MPI_Gather(_workspace.data, SIZE * shape_grid_rows * sizeof(short), MPI_BYTE,
               workspace_extended.data, SIZE * shape_grid_rows * sizeof(short), MPI_BYTE, 0, comm);
    if (::rank == 0)
    {
        gatherGrid = preprocess_grid(workspace_extended);
    }
    MPI_Scatter(gatherGrid.data(), SIZE * shape_grid_rows * sizeof(short), MPI_BYTE,
                _workspace.data, SIZE * shape_grid_rows * sizeof(short), MPI_BYTE, 0, comm);
}

Mat Game::preprocess_grid(const vector<Mat> &gatherGrid)
{
    Mat workspace_extended = gatherGrid[0];
    for (int i = 1; i < size; i++)
    {
        Mat subgrid = gatherGrid[i];
        Rect roi(0, i * shape_grid_rows, SIZE, shape_grid_rows);
        subgrid.copyTo(workspace_extended(roi));
    }
    return workspace_extended;
}

vector<char> read_data()
{
    ifstream ifs("data.txt", ios::binary | ios::ate);
    ifstream::pos_type pos = ifs.tellg();
    int file_size = pos;
    vector<char> result(file_size);
    ifs.seekg(0, ios::beg);
    ifs.read(&result[0], file_size);
    return result;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    comm = MPI_COMM_WORLD;
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &::rank);

    SIZE = stoi(argv[1]);
    generations = stoi(argv[2]);
    ::ratio = stod(argv[3]);

    shape_grid_rows = SIZE / size;
    shape_grid_cols = SIZE;

    distribution = SIZE / size;

    workspace = Mat(shape_grid_rows + 2, shape_grid_cols, CV_16S);
    neighbor = Mat(shape_grid_rows + 2, shape_grid_cols, CV_16S);
    neighbor_id = Engine(nullptr).make_neighbor_indices();

    timing(read_data);

    MPI_Finalize();

    return 0;
}
