#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <time.h>
#include <sstream>
#include <fstream>
#include <vector>

using namespace std;

int N = 50;
int max_steps = 10;
bool periodic = false;
int test = 1;
int imax, jmax;
vector<vector<bool>> grid, new_grid;
int rows, columns;
int id, p, tag_num = 1;

int num_neighbours(int ii, int jj)
{
    int ix, jx;
    int cnt = 0;
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            if (i != 0 || j != 0)
            {
                ix = (i + ii + imax) % imax;
                jx = (j + jj + jmax) % jmax;
                if (grid[ix][jx])
                    cnt++;
            }
        }
    }
    return cnt;
}

void grid_to_file(int it, int process_row, int process_column)
{
    stringstream fname;
    fname << "output"
          << "_"
          << "iteration_" << it << "_processrow_" << process_row << "_processcolumn_" << process_column << ".txt";
    fstream f1(fname.str().c_str(), ios_base::out);
    for (int i = 1; i < imax - 1; i++)
    {
        for (int j = 1; j < jmax - 1; j++)
        {
            f1 << grid[i][j] << "\t";
        }
        f1 << endl;
    }
    f1.close();
}

void do_iteration()
{
    for (int i = 0; i < imax; i++)
    {
        for (int j = 0; j < jmax; j++)
        {
            new_grid[i][j] = grid[i][j];
            int num_n = num_neighbours(i, j);
            if (grid[i][j])
            {
                if (num_n != 2 && num_n != 3)
                    new_grid[i][j] = false;
            }
            else if (num_n == 3)
                new_grid[i][j] = true;
        }
    }
    grid.swap(new_grid);
}

void find_dimensions(int p, int &rows, int &columns)
{
    int min_gap = p;
    for (int i = 1; i <= p / 2; i++)
    {
        if (p % i == 0)
        {
            int gap = abs(p / i - i);

            if (gap < min_gap)
            {
                min_gap = gap;
                rows = i;
                columns = p / i;
            }
        }
    }

    if (id == 0)
        cout << "Divide " << p << " into " << rows << " by " << columns << " grid" << endl;
}

void id_to_index(int id, int &id_row, int &id_column)
{
    id_column = id % columns;
    id_row = id / columns;
}

int id_from_index(int id_row, int id_column)
{
    if (periodic == true)
    {
        return ((id_row + rows) % rows) * columns + (id_column + columns) % columns;
    }

    if (id_row >= rows || id_row < 0)
    {
        test = -1;
    }
    if (id_column >= columns || id_column < 0)
    {
        test = -1;
    }

    return ((id_row + rows) % rows) * columns + (id_column + columns) % columns;
}

void generate_random_grid()
{
    srand(time(NULL) + 1000 * id);
    int b;
    for (int i = 1; i < N + 1; i++)
    {
        for (int j = 1; j < N + 1; j++)
        {
            int a = rand() % 10;
            if (a < 5)
            {
                b = 0;
            }
            else
            {
                b = 1;
            }
            grid[i][j] = b;
        }
    }
}

void send_receive_edges(const vector<bool> &send_data, vector<bool> &recv_data, int idsend, int idrecv, int tag_num)
{
    if (periodic == true || test == 1)
    {
        std::vector<char> send_data_copy(send_data.begin(), send_data.end());
        MPI_Isend(send_data_copy.data(), send_data_copy.size(), MPI_BYTE, idsend, tag_num, MPI_COMM_WORLD, MPI_REQUEST_NULL);
    }
    else
    {
        vector<bool> zeros(send_data.size(), false);
        MPI_Isend(zeros, zeros.size(), MPI_BYTE, idsend, tag_num, MPI_COMM_WORLD, MPI_REQUEST_NULL);
    }
    MPI_Irecv(recv_data, recv_data.size(), MPI_BYTE, idrecv, tag_num, MPI_COMM_WORLD, MPI_REQUEST_NULL);
}

void fill_boundaries()
{
    for (int i = 0; i < N; i++)
    {
        grid[0][i + 1] = top_edge_recv[i];
        grid[N + 1][i + 1] = bottom_edge_recv[i];
        grid[i + 1][0] = left_edge_recv[i];
        grid[i + 1][N + 1] = right_edge_recv[i];
    }


    grid[0][0] = left_top_recv[0];
    grid[0][N + 1] = right_top_recv[0];
    grid[N + 1][0] = left_bottom_recv[0];
    grid[N + 1][N + 1] = right_bottom_recv[0];
}

void perform_game_of_life()
{
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    srand(time(NULL) + id * 10);

    find_dimensions(p, rows, columns);
    id_to_index(id, id_row, id_column);

    imax = N + 2;
    jmax = N + 2;
    grid.resize(imax, vector<bool>(jmax));
    new_grid.resize(imax, vector<bool>(jmax));

    generate_random_grid();

    vector<MPI_Request> requests(8);
    vector<bool> left_edge(N), right_edge(N), right_edge_recv(N), left_edge_recv(N);
    vector<bool> top_edge(N), bottom_edge(N), bottom_edge_recv(N), top_edge_recv(N);

    vector<bool> left_top(1), left_bottom(1), right_top(1), right_bottom(1);
    vector<bool> left_top_recv(1), left_bottom_recv(1), right_top_recv(1), right_bottom_recv(1);

    vector<bool> zero(1, false);

    for (int n = 0; n < max_steps; n++)
    {
        // send and receive edges
        int idsend, idrecv;

        id_to_index(id, id_row, id_column);
        idsend = id_from_index(id_row, id_column - 1);
        send_receive_edges(left_edge, right_edge_recv, idsend, idrecv, tag_num = 1);

        id_to_index(id, id_row, id_column);
        idsend = id_from_index(id_row, id_column + 1);
        send_receive_edges(right_edge, left_edge_recv, idsend, idrecv, tag_num = 2);

        id_to_index(id, id_row, id_column);
        idsend = id_from_index(id_row - 1, id_column);
        send_receive_edges(top_edge, bottom_edge_recv, idsend, idrecv, tag_num = 3);

        id_to_index(id, id_row, id_column);
        idsend = id_from_index(id_row + 1, id_column);
        send_receive_edges(bottom_edge, top_edge_recv, idsend, idrecv, tag_num = 4);

        id_to_index(id, id_row, id_column);
        idsend = id_from_index(id_row + 1, id_column + 1);
        send_receive_edges(right_bottom, left_top_recv, idsend, idrecv, tag_num = 5);

        id_to_index(id, id_row, id_column);
        idsend = id_from_index(id_row - 1, id_column - 1);
        send_receive_edges(left_top, right_bottom_recv, idsend, idrecv, tag_num = 6);

        id_to_index(id, id_row, id_column);
        idsend = id_from_index(id_row - 1, id_column + 1);
        send_receive_edges(right_top, left_bottom_recv, idsend, idrecv, tag_num = 7);

        id_to_index(id, id_row, id_column);
        idsend = id_from_index(id_row + 1, id_column - 1);
        send_receive_edges(left_bottom, right_top_recv, idsend, idrecv, tag_num = 8);

        MPI_Waitall(8, requests.data(), MPI_STATUS_IGNORE);

        fill_boundaries();

        do_iteration();
        id_to_index(id, id_row, id_column);
        grid_to_file(n, id_row, id_column);
    }

    MPI_Finalize();
}

int main()
{
    perform_game_of_life();
    return 0;
}
