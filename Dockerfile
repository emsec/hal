FROM ubuntu:latest
MAINTAINER Sebastian Wallat "sebastian.wallat@rub.de"

ENV build_path=/home/build
RUN mkdir -p $build_path
WORKDIR $build_path
RUN echo 'debconf debconf/frontend select Noninteractive' | debconf-set-selections
RUN sed -i -e 's/#force_color_prompt=yes/force_color_prompt=yes/g' /root/.bashrc
RUN apt-get update && apt-get install -y build-essential lsb-release git cmake pkgconf libspdlog-dev libboost-all-dev qt5-default libpython3.6-dev pybind11-dev  build-essential \
    libyaml-cpp-dev ccache autoconf autotools-dev flex bison libbdd-dev libbdd0c2 libsodium-dev libqt5svg5-dev libqt5svg5* ninja-build lcov gcovr graphviz python3-sphinx doxygen \
    python3-sphinx-rtd-theme python3-jedi flex bison
RUN git clone https://github.com/gabime/spdlog.git && cd spdlog && git checkout tags/v0.13.0 && mkdir build && cd build && cmake .. && make install && cd ../../
CMD cmake -G Ninja /home/src -DCMAKE_BUILD_TYPE=MinSizeRel -DBUILD_ALL_PLUGINS=OFF -DPL_GRAPH_ALGORITHM=ON -DPL_GATE_DECORATOR_SYSTEM=ON -DBUILD_TESTS=ON -DWITH_GUI=ON -DCMAKE_INSTALL_PREFIX=/usr/ -DCPACK_OUTPUT_FILE_PREFIX=/home/build-output/ && \
    cmake --build /home/build --target package --clean-first -- -j4
