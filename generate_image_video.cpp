#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <cmath>

using namespace std;
using namespace cv;

/**
 * @brief Extracts the directory name from a file path.
 * @param fileNamePath The file path from which the directory name is extracted.
 * @return The directory name extracted from the file path.
 */
string extractDirectoryName(const string &fileNamePath)
{
    size_t firstSlashPos = fileNamePath.find('/');
    size_t secondSlashPos = fileNamePath.find('/', firstSlashPos + 1);
    return fileNamePath.substr(firstSlashPos + 1, secondSlashPos - firstSlashPos - 1);
}

/**
 * @brief Extracts the file name without extension from a file path.
 * @param fileNamePath The file path from which the file name without extension is extracted.
 * @return The file name without extension extracted from the file path.
 */
string extractFileNameWithoutExtension(const string &fileNamePath)
{
    size_t lastSlashPos = fileNamePath.find_last_of('/');
    size_t lastDotPos = fileNamePath.find_last_of('.');
    return fileNamePath.substr(lastSlashPos + 1, lastDotPos - lastSlashPos - 1);
}

/**
 * @brief Reads data from a file and extracts binary strings representing grid states.
 * @param fileNamePath The name of the input file containing binary strings.
 * @param gridSize [out] The size of the grid (number of rows/columns) extracted from the binary strings.
 * @return A vector of binary strings representing different grid states.
 */
vector<string> readBinaryStringsFromFile(const string &fileNamePath, int &gridSize)
{
    ifstream file(fileNamePath);
    if (!file)
    {
        cerr << "Could not open the input file: " << fileNamePath << endl;
        return {};
    }

    vector<string> binaryStrings;
    string line;
    while (getline(file, line))
    {
        istringstream iss(line);
        string identifier;
        string binaryString;
        if (iss >> identifier >> binaryString)
        {
            binaryStrings.push_back(binaryString);
            if (gridSize == 0)
            {
                gridSize = static_cast<int>(sqrt(binaryString.size()));
            }
        }
    }
    file.close();

    return binaryStrings;
}

/**
 * @brief Converts a binary string into an image (matrix) of size gridSize x gridSize.
 * @param binaryString The binary string representing the grid state.
 * @param gridSize The size of the grid (number of rows/columns).
 * @return The vector representation of the binary string as a grayscale image.
 */
vector<vector<uchar>> binaryStringToImage(const string &binaryString, int gridSize)
{
    vector<vector<uchar>> image(gridSize, vector<uchar>(gridSize, 0)); // Image with size gridSize x gridSize and pixels initialized to 0 (black)

    for (int i = 0; i < gridSize * gridSize; ++i)
    {
        if (binaryString[i] == '1')
        {
            int row = i / gridSize;
            int col = i % gridSize;
            image[row][col] = 255; // Set pixel to white (255) if we have a '1' in the binary string
        }
    }

    return image;
}

/**
 * @brief Converts a 2D vector of grayscale pixel values to a corresponding OpenCV Mat.
 * @param image The 2D vector representing the grayscale image.
 * @return The OpenCV Mat representation of the grayscale image.
 */
Mat vectorToMat(const vector<vector<uchar>> &image)
{
    int rows = image.size();
    int cols = image[0].size();
    Mat matImage(rows, cols, CV_8UC1);

    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            matImage.at<uchar>(i, j) = image[i][j];
        }
    }

    return matImage;
}

/**
 * @brief Generates a video based on binary strings representing grid states.
 * @param binaryStrings The vector of binary strings representing different grid states.
 * @param gridSize The size of the grid (number of rows/columns).
 * @param videoSize The desired size of the output video (width and height in pixels).
 * @param fileNamePath The name of the input file containing binary strings.
 * @return True if the video generation is successful, false otherwise.
 */
bool generateVideoFromBinaryStrings(const vector<string> &binaryStrings, int gridSize, int videoSize, const string &fileNamePath)
{
    string videoDirectory = extractDirectoryName(fileNamePath) + "/videos";
    string videoName = extractFileNameWithoutExtension(fileNamePath) + ".avi";

    string filename = "./" + videoDirectory + "/" + videoName;

    if (binaryStrings.empty())
    {
        cerr << "No valid data found in the input file!" << endl;
        return false;
    }

    VideoWriter video(filename, VideoWriter::fourcc('M', 'J', 'P', 'G'), 2, Size(videoSize, videoSize), true);
    if (!video.isOpened())
    {
        cerr << "Could not open the video file for writing!" << endl;
        return false;
    }

    for (const string &binaryString : binaryStrings)
    {
        vector<vector<uchar>> image = binaryStringToImage(binaryString, gridSize);

        Mat frame = vectorToMat(image);

        Mat resizedFrame;
        resize(frame, resizedFrame, Size(videoSize, videoSize), 0, 0, INTER_NEAREST);

        Mat bgrFrame;
        cvtColor(resizedFrame, bgrFrame, COLOR_GRAY2BGR);

        video.write(bgrFrame);
    }

    video.release();

    return true;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " input_file_name.txt" << endl;
        return 1;
    }
    string fileNamePath = argv[1];

    int gridSize = 0;
    vector<string> binaryStrings = readBinaryStringsFromFile(fileNamePath, gridSize);

    if (gridSize == 0)
    {
        cerr << "No valid data found in the input file!" << endl;
        return 1;
    }

    int videoSize = 800;
    if (!generateVideoFromBinaryStrings(binaryStrings, gridSize, videoSize, fileNamePath))
    {
        return 1;
    }

    return 0;
}
