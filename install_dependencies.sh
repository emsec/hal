#!/usr/bin/env bash

platform='unknown'
unamestr=$(uname)
distribution='unknown'
release='unknown'
if [[ -f "/.dockerenv" ]]; then
   platform='docker'
   distribution=$(lsb_release -is)
   release=$(lsb_release -rs)
elif [[ "$unamestr" == 'Linux' ]]; then
   platform='linux'
   distribution=$(lsb_release -is)
   release=$(lsb_release -rs)
elif [[ "$unamestr" == 'Darwin' ]]; then
   platform='macOS'
fi

if [[ "$platform" == 'macOS' ]]; then
    echo "Executing brew bundle"
    brew bundle
    pip3 install -r requirements.txt
    BREW_PREFIX=$(brew --prefix)
    if [ -n "$($SHELL -c 'echo $ZSH_VERSION')" ]; then
        grep -Fxq 'export PATH="$BREW_PREFIX/opt/qt@5/bin:$PATH"' ~/.zshrc
        if ! [[ $? -eq 0 ]]; then
            echo 'export PATH="$BREW_PREFIX/opt/qt@5/bin:$PATH"' >> ~/.zshrc
        fi

        grep -Fxq 'export PATH="$BREW_PREFIX/opt/llvm@14/bin:$PATH"' ~/.zshrc
        if ! [[ $? -eq 0 ]]; then
            echo 'export PATH="$BREW_PREFIX/opt/llvm@14/bin:$PATH"' >> ~/.zshrc
        fi

        grep -Fxq 'export PATH="$BREW_PREFIX/opt/flex/bin:$PATH"' ~/.zshrc
        if ! [[ $? -eq 0 ]]; then
            echo 'export PATH="$BREW_PREFIX/opt/flex/bin:$PATH"' >> ~/.zshrc
        fi

        grep -Fxq 'export PATH="$BREW_PREFIX/opt/bison/bin:$PATH"' ~/.zshrc
        if ! [[ $?  -eq 0 ]]; then
            echo 'export PATH="$BREW_PREFIX/opt/bison/bin:$PATH"' >> ~/.zshrc
        fi
        source ~/.zshrc
    elif [ -n "$($SHELL -c 'echo $BASH_VERSION')" ]; then
        grep -Fxq 'export PATH="$BREW_PREFIX/opt/qt@5/bin:$PATH"' ~/.bash_profile
        if ! [[ $? -eq 0 ]]; then
            echo 'export PATH="$BREW_PREFIX/opt/qt@5/bin:$PATH"' >> ~/.bash_profile
        fi

        grep -Fxq 'export PATH="$BREW_PREFIX/opt/llvm@14/bin:$PATH"' ~/.bash_profile
        if ! [[ $? -eq 0 ]]; then
            echo 'export PATH="$BREW_PREFIX/opt/llvm@14/bin:$PATH"' >> ~/.bash_profile
        fi

        grep -Fxq 'export PATH="$BREW_PREFIX/opt/flex/bin:$PATH"' ~/.bash_profile
        if ! [[ $? -eq 0 ]]; then
            echo 'export PATH="$BREW_PREFIX/opt/flex/bin:$PATH"' >> ~/.bash_profile
        fi

        grep -Fxq 'export PATH="$BREW_PREFIX/opt/bison/bin:$PATH"' ~/.bash_profile
        if ! [[ $?  -eq 0 ]]; then
            echo 'export PATH="$BREW_PREFIX/opt/bison/bin:$PATH"' >> ~/.bash_profile
        fi
        source ~/.bash_profile
    else
        echo "Unknown User Shell: abort!"
        exit 255
    fi
elif [[ "$platform" == 'linux' ]]; then
    if [ "$distribution" == 'Ubuntu' ] || [ "$distribution" == 'LinuxMint' ]; then

        sudo apt-get update && sudo apt-get install -y build-essential verilator \
        lsb-release git cmake pkgconf libboost-all-dev qtbase5-dev \
        libpython3-dev ccache autoconf autotools-dev libsodium-dev \
        libqt5svg5-dev libqt5svg5* ninja-build lcov gcovr python3-sphinx \
        doxygen python3-sphinx-rtd-theme python3-jedi python3-pip \
        pybind11-dev python3-pybind11 rapidjson-dev libspdlog-dev libz3-dev z3 \
        libreadline-dev \
        $additional_deps \
        graphviz libomp-dev libsuitesparse-dev # For documentation
        sudo pip3 install -r requirements.txt
    elif [[ "$distribution" == "Arch" ]]; then
        yay -S --needed base-devel lsb-release git verilator cmake boost-libs pkgconf \
        qt5-base python ccache autoconf libsodium qt5-svg ninja lcov \
        gcovr python-sphinx doxygen python-sphinx_rtd_theme python-jedi \
        python-pip pybind11 rapidjson spdlog graphviz boost \
        python-dateutil z3
    else
       echo "Unsupported Linux distribution: abort!"
       exit 255
    fi
elif [[ "$platform" == 'docker' ]]; then
    # We can assume that we are in a ubuntu container, because of the official Dockerfile in the hal project
    apt-get update && apt-get install -y build-essential verilator \
    lsb-release git cmake pkgconf libboost-all-dev qtbase5-dev \
    libpython3-dev ccache autoconf autotools-dev libsodium-dev \
    libqt5svg5-dev libqt5svg5* ninja-build lcov gcovr python3-sphinx \
    doxygen python3-sphinx-rtd-theme python3-jedi python3-pip \
    pybind11-dev python3-pybind11 rapidjson-dev libspdlog-dev libz3-dev libreadline-dev \
    graphviz libomp-dev libsuitesparse-dev # For documentation
    pip3 install -r requirements.txt
fi
