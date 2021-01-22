FROM ubuntu:focal

ENV build_path=/home/build
RUN mkdir -p $build_path
WORKDIR $build_path
RUN echo 'debconf debconf/frontend select Noninteractive' | debconf-set-selections
RUN sed -i -e 's/#force_color_prompt=yes/force_color_prompt=yes/g' /root/.bashrc
RUN apt-get update && apt-get install -y apt-transport-https ca-certificates gnupg software-properties-common curl \
    build-essential lsb-release git cmake pkgconf libspdlog-dev libboost-all-dev qt5-default libpython3.8-dev \
    pybind11-dev  build-essential libyaml-cpp-dev ccache autoconf autotools-dev flex bison libsodium-dev libqt5svg5-dev libqt5svg5* ninja-build lcov gcovr graphviz \
    python3-sphinx doxygen python3-sphinx-rtd-theme python3-jedi flex bison devscripts debhelper dh-make pkgconf gnupg2 pybind11-dev python3-pybind11 \
    python3-paramiko rapidjson-dev libspdlog-dev libigraph0-dev python3-dateutil libz3-dev libomp-dev\
      && apt-get clean \
      && rm -rf /var/lib/apt/lists/*
CMD cmake -G Ninja /home/src -DCMAKE_BUILD_TYPE=MinSizeRel -DBUILD_ALL_PLUGINS=OFF -DPL_GRAPH_ALGORITHM=ON -DPL_GATE_DECORATOR_SYSTEM=ON -DBUILD_TESTS=ON -PL_GUI=ON -DCMAKE_INSTALL_PREFIX=/usr/ -DCPACK_OUTPUT_FILE_PREFIX=/home/build-output/ && \
    cmake --build /home/build --target package --clean-first -- -j4
