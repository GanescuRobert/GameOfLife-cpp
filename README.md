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