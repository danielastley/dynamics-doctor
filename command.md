# 1. Remove the build directory to clean previous builds
rm -rf build

# 1.5 The "Safer" CMake-Integrated Method
cmake --build build --target clean

# 2. Configure the project with Ninja as the generator (Release build for FL Studio)
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# 3. Build the project
cmake --build build