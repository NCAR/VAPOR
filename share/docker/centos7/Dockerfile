# To build:
#   docker image build -t centos7:1.0 .

FROM centos:centos7
MAINTAINER The CentOS Project <cloud-ops@centos.org>

CMD [ "/bin/bash" ]

#################################
#                               #
# Vapor configuration and build #
#                               #
#################################

RUN yum -y clean all \
    && yum -y clean metadata \
    && yum -y update

RUN yum -y install epel-release \
    && yum -y install cmake3 \
    && yum -y install make \
    && yum -y install tar \
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

RUN mkdir -p /glade/p/VAST/VAPOR/third-party

RUN gdown https://drive.google.com/uc?id=1_JdUuiy_iQUuIDoPyBn2pupBTz-LS4pM \
    && mv /apps-2017-Centos7.tar.xz /glade/p/VAST/VAPOR/third-party \
    && tar xf /glade/p/VAST/VAPOR/third-party/apps-2017-Centos7.tar.xz \
           -C /glade/p/VAST/VAPOR/third-party

RUN ls /glade/p/VAST/VAPOR/third-party/apps-2017

#RUN git clone https://github.com/g-truc/glm.git \
#    /glade/p/VAST/VAPOR/third-party/apps-2017/Linux_x86_64/include/glm

#RUN mv /glade/p/VAST/VAPOR/third-party/apps-2017/Linux_x86_64/include/glm/glm/* \
#    /glade/p/VAST/VAPOR/third-party/apps-2017/Linux_x86_64/include/glm

RUN git clone https://github.com/NCAR/VAPOR.git \
    && cp /VAPOR/site_files/site.NCAR /VAPOR/site.local \
    && mkdir /VAPOR/build \
    && export CMAKE_CXX_COMPILER=g++ \
    && cd /VAPOR/build \
    && cmake3 .. \
    && make && pwd && ls && ls bin

RUN /VAPOR/build/bin/vaporpychecker

WORKDIR /VAPOR
