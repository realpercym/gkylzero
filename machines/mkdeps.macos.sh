#cd install-deps
#: "${PREFIX:=$HOME/Desktop/PROJECT_DR_YAN/gkyl}"
#./mkdeps.sh --build-openblas=yes --build-superlu=yes --prefix=$PREFIX


cd install-deps
: "${PREFIX:=$HOME/Desktop/PROJECT_DR_YAN/gkyl}"
./mkdeps.sh --build-openblas=no --build-superlu=yes --prefix=$PREFIX

