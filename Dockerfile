FROM ubuntu:focal

ENV build_path=/home/build
RUN mkdir -p $build_path
WORKDIR $build_path
RUN echo 'debconf debconf/frontend select Noninteractive' | debconf-set-selections
RUN sed -i -e 's/#force_color_prompt=yes/force_color_prompt=yes/g' /root/.bashrc
RUN  apt-get update && bash /home/src/install_dependencies.sh \
      && apt-get clean \
      && rm -rf /var/lib/apt/lists/*
CMD cmake -G Ninja /home/src -DCMAKE_BUILD_TYPE=MinSizeRel -DBUILD_ALL_PLUGINS=OFF -DPL_GRAPH_ALGORITHM=ON -DPL_GATE_DECORATOR_SYSTEM=ON -DBUILD_TESTS=ON -DWITH_GUI=ON -DCMAKE_INSTALL_PREFIX=/usr/ -DCPACK_OUTPUT_FILE_PREFIX=/home/build-output/ && \
    cmake --build /home/build --target package --clean-first -- -j4
