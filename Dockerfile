FROM ubuntu:22.04

ARG CMAKE_OPTIONS
ARG MAKE_OPTIONS

ENV hal_path=/hal
RUN mkdir -p $hal_path
WORKDIR $hal_path

RUN echo 'debconf debconf/frontend select Noninteractive' | debconf-set-selections
RUN sed -i -e 's/#force_color_prompt=yes/force_color_prompt=yes/g' /root/.bashrc

RUN apt-get update -y && \
    apt-get install -y lsb-release

COPY . .
RUN ./install_dependencies.sh

RUN mkdir build
WORKDIR ${hal_path}/build/
RUN cmake .. ${CMAKE_OPTIONS}
RUN make ${MAKE_OPTIONS}

VOLUME [ "/projects" ]

CMD ["./bin/hal", "-g"]
