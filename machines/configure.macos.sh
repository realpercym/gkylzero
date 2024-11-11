#: "${PREFIX:=$HOME/Desktop/PROJECT_DR_YAN/gkyl}"
#./configure --prefix=$PREFIX


: "${PREFIX:=$HOME/Desktop/PROJECT_DR_YAN/gkyl}"
./configure --prefix=$PREFIX \
            CC=/usr/bin/clang \
            --use-mpi=yes \
            --mpi-inc=/opt/homebrew/Cellar/open-mpi/5.0.5/include \
            --mpi-lib=/opt/homebrew/Cellar/open-mpi/5.0.5/lib \
            --lapack-inc="-framework Accelerate" \
            --lapack-lib="-framework Accelerate"


