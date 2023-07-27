# GameOfLife-cpp
 
https://code.visualstudio.com/docs/cpp/config-mingw


### Set mpic++

If you don't have mpic++ installed on your Linux system, you may need to install an MPI implementation first.
Here are instructions for Ubuntu/Debian:

sudo apt-get update
sudo apt-get install mpich

After installing the MPI implementation, you should be able to use the mpic++ command. If it still doesn't work, it's possible that the MPI binaries are not in your system's PATH. In that case, you can try locating the mpic++ binary and use its full path when compiling your MPI program.

You can use the which command to locate the mpic++ binary. Open a terminal and run:
which mpic++


### Install packages for conda

The error message suggests that the `x86_64-conda-linux-gnu-c++` compiler is not found in your Conda environment. This issue can occur if the necessary build tools or C++ compiler are not installed within the Conda environment.

To resolve this issue, you can try the following steps:

1. Ensure that the necessary build tools and C++ compiler are installed in your Conda environment. You can install them using the following command:

   ```
   conda install -c conda-forge mpich mpich-devel
   ```

   This command will install the `mpich` package along with its development files, which include the C++ compiler needed for building MPI programs.

2. After installing the necessary packages, activate your Conda environment where you want to compile and run your MPI program:

   ```
   conda activate gol
   ```

   Replace `gol` with the name of your Conda environment.

3. Try compiling your MPI program again using `mpic++`.

If you continue to encounter the same error, it's possible that the necessary C++ compiler is not installed in your Conda environment. In that case, you may need to install a C++ compiler separately, such as `gcc` or `g++`, within your Conda environment.

You can install the C++ compiler using the following command:

```
conda install -c conda-forge gxx_linux-64
```

Once the C++ compiler is installed, try compiling your MPI program using `mpic++` again.

If you still face issues, make sure to review your Conda environment configuration and verify that the required packages are installed correctly.

<!-- https://github.com/TayfunKaraderi/Game-of-Life-In-Parallel/blob/master/Game_of_Life.cpp -->


### Install opencv2 for c++

In Linux, you can install OpenCV using the package manager. Here's how you can install OpenCV and resolve the "Package opencv was not found" error:

1. **Update Package Lists**: Open a terminal and update the package lists on your system by running the following command:

   ```
   sudo apt update
   ```

2. **Install OpenCV Packages**: Install the OpenCV packages using the package manager. Run the following command:

   ```
   sudo apt install libopencv-dev
   ```

   This command will install the necessary OpenCV development packages, including the package configuration files needed by `pkg-config`.

3. **Check OpenCV Installation**: Verify that OpenCV is installed correctly. Run the following command to display the installed OpenCV version:

   ```
   pkg-config --modversion opencv4
   ```

   If the command displays the version number of OpenCV, it means OpenCV is installed and configured properly.

4. **Link OpenCV Libraries**: To link against the OpenCV libraries, you need to specify the linker flags during the compilation process. Open a terminal and compile your C++ code with the following command:

   ```
   g++ your_code.cpp -o your_executable `pkg-config --cflags --libs opencv4`
   ```

   This command uses the `pkg-config` tool to automatically determine the necessary compiler and linker flags for OpenCV.

5. **Build and Run**: Finally, build your C++ code by running the generated executable:

   ```
   ./your_executable
   ```

These steps should help you install OpenCV, include the necessary headers in your C++ code, and compile and run your program using OpenCV functionality.


The error message suggests that the linker (`ld`) is unable to find the `libGL` library. This usually indicates that the OpenGL development libraries are missing on your system.

To resolve this issue, you can try installing the necessary OpenGL libraries using the package manager for your operating system. The package name may vary depending on your distribution.

For example, on Ubuntu or Debian, you can try installing the following packages:

```bash
sudo apt-get install libgl1-mesa-dev
sudo apt-get install libglu1-mesa-dev
```

On Fedora or CentOS, you can try:

```bash
sudo dnf install mesa-libGL-devel
sudo dnf install mesa-libGLU-devel
```

Once the OpenGL development libraries are installed, you can try compiling your code again with the `-lGL` flag.




## MPI

### Build
```
mpic++ -std=c++17 GOL_parallel_linearity.cpp -o GOL_parallel_linearity -L/usr/local/lib -L/usr/lib `pkg-config --cflags opencv4` `pkg-config --libs opencv4` `pkg-config --libs opencv4` -lGL
```

## Generate video ( OPENCV)

### Build
```
g++ generateVideos.cpp -o generateVideos.exe -I/home/rob/anaconda3/envs/gol/include/opencv4 -L/home/rob/anaconda3/envs/gol/lib -Wl,-rpath=/home/rob/anaconda3/envs/gol/lib -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_videoio
```
### Create video
```
./generateVideos.exe cpp/secvential
```

## Cpp

### Build
```
g++ -o main.exe main.cpp 
```
### Run 
```
./main.exe life
```



### Toti algoritmi se ruleaza din fisierul sau.