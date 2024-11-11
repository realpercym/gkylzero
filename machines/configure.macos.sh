#: "${PREFIX:=$HOME/Desktop/PROJECT_DR_YAN/gkyl}"
#./configure --prefix=$PREFIX


: "${PREFIX:=$HOME/Desktop/PROJECT_DR_YAN/gkyl}"
./configure --prefix=$PREFIX \
            CC=/usr/bin/clang \
            --use-mpi=yes \
            --superlu-inc=${PREFIX}/superlu-5.2.2/include \
            --superlu-lib=${PREFIX}/superlu-5.2.2/lib/libsuperlu.a \
            --mpi-inc=/opt/homebrew/Cellar/open-mpi/5.0.5/include \
            --mpi-lib=/opt/homebrew/Cellar/open-mpi/5.0.5/lib
