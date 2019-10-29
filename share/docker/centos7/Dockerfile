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

RUN yum -y clean all
RUN yum -y clean metadata
RUN yum -y update

RUN yum -y install epel-release
#RUN yum -y install cmake
#RUN ls /usr/bin/cmake
RUN yum -y install cmake3
RUN yum -y install make
RUN yum -y install tar
RUN yum -y install gcc-c++
RUN yum -y install curl
RUN yum -y install xz-devel
RUN yum -y install git
RUN yum -y install freeglut-devel
# Aren't we supposed to be distributing libexpat in our third-party tar???
RUN yum -y install expat-devel
RUN yum -y install libquadmath-devel
RUN yum -y install python3-pip
RUN yum -y install libXrender-devel
RUN yum -y install libSM-devel
RUN yum -y install fontconfig-devel
RUN pip3 install gdown

RUN mkdir -p /glade/p/VAST/VAPOR/third-party

RUN gdown https://drive.google.com/uc?id=1_JdUuiy_iQUuIDoPyBn2pupBTz-LS4pM
RUN mv /apps-2017-Centos7.tar.xz /glade/p/VAST/VAPOR/third-party

#RUN chmod 777 /glade/p/VAST/VAPOR/third-party/apps-2017-Centos7.tar.xz
RUN tar xf /glade/p/VAST/VAPOR/third-party/apps-2017-Centos7.tar.xz \
           -C /glade/p/VAST/VAPOR/third-party

RUN ls /glade/p/VAST/VAPOR/third-party/apps-2017

#RUN git clone https://github.com/g-truc/glm.git \
#    /glade/p/VAST/VAPOR/third-party/apps-2017/Linux_x86_64/include/glm

#RUN mv /glade/p/VAST/VAPOR/third-party/apps-2017/Linux_x86_64/include/glm/glm/* \
#    /glade/p/VAST/VAPOR/third-party/apps-2017/Linux_x86_64/include/glm

RUN git clone https://github.com/NCAR/VAPOR.git
RUN cp /VAPOR/site_files/site.NCAR /VAPOR/site.local
RUN mkdir /VAPOR/build

RUN export CMAKE_CXX_COMPILER=g++ && \
    cd /VAPOR/build && \
    cmake3 .. && \
    make && pwd && ls && ls bin

RUN /VAPOR/build/bin/vaporpychecker

WORKDIR /VAPOR
