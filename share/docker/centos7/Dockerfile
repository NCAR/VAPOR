# To build:
#   docker image build -t centos7:1.0 .

FROM centos:centos7
MAINTAINER The CentOS Project <cloud-ops@centos.org>

CMD [ "/bin/bash" ]


RUN yum -y clean all \
    && yum -y clean metadata \
    && yum -y update

RUN yum -y install epel-release \
    && yum -y install cmake3 \
    && yum -y install make \
    && yum -y install bsdtar \
    && yum -y install gcc-c++ \
    && yum -y install curl \
    && yum -y install xz-devel \
    && yum -y install git \
    && yum -y install freeglut-devel \
# Aren't we supposed to be distributing libexpat in our third-party tar???
    && yum -y install expat-devel \
    && yum -y install libquadmath-devel \
    && yum -y install python3-pip \
    && yum -y install libXrender-devel \
    && yum -y install libSM-devel \
    && yum -y install fontconfig-devel \
    && pip3 install gdown

# All this to default to CMake3
RUN alternatives --install /usr/local/bin/cmake cmake /usr/bin/cmake 10 \
    --slave /usr/local/bin/ctest ctest /usr/bin/ctest \
    --slave /usr/local/bin/cpack cpack /usr/bin/cpack \
    --slave /usr/local/bin/ccmake ccmake /usr/bin/ccmake \
    --family cmake

RUN alternatives --install /usr/local/bin/cmake cmake /usr/bin/cmake3 20 \
    --slave /usr/local/bin/ctest ctest /usr/bin/ctest3 \
    --slave /usr/local/bin/cpack cpack /usr/bin/cpack3 \
    --slave /usr/local/bin/ccmake ccmake /usr/bin/ccmake3 \
    --family cmake

RUN mkdir -p /usr/local/VAPOR-Deps

#Hack to download a file from google drive
RUN fileid="1e7F3kDoKctBmB3NOF4dES2395oScb9_0" \
    && filename="/usr/local/VAPOR-Deps/2019-Aug-CentOS.tar.xz" \
    && curl -c ./cookie -s -L "https://drive.google.com/uc?export=download&id=${fileid}" > /dev/null \
    && curl -Lb ./cookie \
    "https://drive.google.com/uc?export=download&confirm=`awk '/download/ {print $NF}' ./cookie`&id=${fileid}" \
    -o ${filename}

RUN ls -lrth /usr/local/VAPOR-Deps/

RUN bsdtar -xf /usr/local/VAPOR-Deps/2019-Aug-CentOS.tar.xz -C /usr/local/VAPOR-Deps

RUN chown -R root:root /usr
RUN chmod -R 777 /usr

RUN git clone https://github.com/NCAR/VAPOR.git \
    && cp /VAPOR/site_files/site.NCAR /VAPOR/site.local \
    && mkdir /VAPOR/build \
    && export CMAKE_CXX_COMPILER=g++ \
    && cd /VAPOR/build \
    && cmake3 .. \
    && make && pwd && ls && ls bin

RUN /VAPOR/build/bin/vaporpychecker

WORKDIR /VAPOR
