FROM scratch
ADD ubuntu-xenial-core-cloudimg-amd64-root.tar.gz /

# a few minor docker-specific tweaks
# see https://github.com/docker/docker/blob/9a9fc01af8fb5d98b8eec0740716226fadb3735c/contrib/mkimage/debootstrap
RUN set -xe \
	\
# https://github.com/docker/docker/blob/9a9fc01af8fb5d98b8eec0740716226fadb3735c/contrib/mkimage/debootstrap#L40-L48
	&& echo '#!/bin/sh' > /usr/sbin/policy-rc.d \
	&& echo 'exit 101' >> /usr/sbin/policy-rc.d \
	&& chmod +x /usr/sbin/policy-rc.d \
	\
# https://github.com/docker/docker/blob/9a9fc01af8fb5d98b8eec0740716226fadb3735c/contrib/mkimage/debootstrap#L54-L56
	&& dpkg-divert --local --rename --add /sbin/initctl \
	&& cp -a /usr/sbin/policy-rc.d /sbin/initctl \
	&& sed -i 's/^exit.*/exit 0/' /sbin/initctl \
	\
# https://github.com/docker/docker/blob/9a9fc01af8fb5d98b8eec0740716226fadb3735c/contrib/mkimage/debootstrap#L71-L78
	&& echo 'force-unsafe-io' > /etc/dpkg/dpkg.cfg.d/docker-apt-speedup \
	\
# https://github.com/docker/docker/blob/9a9fc01af8fb5d98b8eec0740716226fadb3735c/contrib/mkimage/debootstrap#L85-L105
	&& echo 'DPkg::Post-Invoke { "rm -f /var/cache/apt/archives/*.deb /var/cache/apt/archives/partial/*.deb /var/cache/apt/*.bin || true"; };' > /etc/apt/apt.conf.d/docker-clean \
	&& echo 'APT::Update::Post-Invoke { "rm -f /var/cache/apt/archives/*.deb /var/cache/apt/archives/partial/*.deb /var/cache/apt/*.bin || true"; };' >> /etc/apt/apt.conf.d/docker-clean \
	&& echo 'Dir::Cache::pkgcache ""; Dir::Cache::srcpkgcache "";' >> /etc/apt/apt.conf.d/docker-clean \
	\
# https://github.com/docker/docker/blob/9a9fc01af8fb5d98b8eec0740716226fadb3735c/contrib/mkimage/debootstrap#L109-L115
	&& echo 'Acquire::Languages "none";' > /etc/apt/apt.conf.d/docker-no-languages \
	\
# https://github.com/docker/docker/blob/9a9fc01af8fb5d98b8eec0740716226fadb3735c/contrib/mkimage/debootstrap#L118-L130
	&& echo 'Acquire::GzipIndexes "true"; Acquire::CompressionTypes::Order:: "gz";' > /etc/apt/apt.conf.d/docker-gzip-indexes \
	\
# https://github.com/docker/docker/blob/9a9fc01af8fb5d98b8eec0740716226fadb3735c/contrib/mkimage/debootstrap#L134-L151
	&& echo 'Apt::AutoRemove::SuggestsImportant "false";' > /etc/apt/apt.conf.d/docker-autoremove-suggests

# delete all the apt list files since they're big and get stale quickly
RUN rm -rf /var/lib/apt/lists/*
# this forces "apt-get update" in dependent images, which is also good
# (see also https://bugs.launchpad.net/cloud-images/+bug/1699913)

# make systemd-detect-virt return "docker"
# See: https://github.com/systemd/systemd/blob/aa0c34279ee40bce2f9681b496922dedbadfca19/src/basic/virt.c#L434
RUN mkdir -p /run/systemd && echo 'docker' > /run/systemd/container

RUN export TERM=xterm

#################################
#                               #
# Vapor configuration and build #
#                               #
#################################

RUN apt-get update \
    && apt-get install -y curl \
    && apt-get install -y xz-utils \
    && apt-get install -y git \
    && apt-get install -y cmake \
    && apt-get install -y g++ \
# need freglut3-dev due to error Could NOT find OpenGL (missing: OPENGL_gl_LIBRARY OPENGL_INCLUDE_DIR)
# https://stackoverflow.com/questions/31170869/cmake-could-not-find-opengl-in-ubuntu
    && apt-get install -y freeglut3-dev \
# Aren't we supposed to be distributing libexpat in our third-party tar???
    && apt-get install -y libexpat1-dev

RUN cd / && \
    mkdir glade        && cd glade       && \
    mkdir p            && cd p           && \
    mkdir VAST         && cd VAST        && \
    mkdir VAPOR        && cd VAPOR       && \
    mkdir third-party  && cd third-party && \
    mkdir apps-2017    && cd apps-2017

# It seems that its not feasible to curl from google docs, so
# the docker image is currentl pulling a file down from my laptop.
# We need another host for our third party libraries (github!)
#ADD Linux_x86_64_Ubuntu16.04.tar.xz /glade/p/VAST/VAPOR/third-party/apps-2017
RUN curl -L -o \
    /glade/p/VAST/VAPOR/third-party/apps-2017/Linux_x86_64_Ubuntu16.04.tar.xz \
    "https://drive.google.com/open?id=0B0dQMtxB89M0azF5RW1RSE5qcTg"

#Hack to download a file from google drive
RUN fileid="0B0dQMtxB89M0azF5RW1RSE5qcTg" \
    && filename="/glade/p/VAST/VAPOR/third-party/apps-2017/Linux_x86_64_Ubuntu16.04.tar.xz" \
    && curl -c ./cookie -s -L "https://drive.google.com/uc?export=download&id=${fileid}" > /dev/null \
    && curl -Lb ./cookie \
    "https://drive.google.com/uc?export=download&confirm=`awk '/download/ {print $NF}' ./cookie`&id=${fileid}" \
    -o ${filename}

# Debug info - Make sure 3rd party libs are downloaded
RUN ls -lrth /glade/p/VAST/VAPOR/third-party/apps-2017
  
# Unpack        
RUN tar xf /glade/p/VAST/VAPOR/third-party/apps-2017/Linux_x86_64_Ubuntu16.04.tar.xz \
           -C /glade/p/VAST/VAPOR/third-party/apps-2017

# Update 3rd party libraries with glm
RUN git clone https://github.com/g-truc/glm.git \
    /glade/p/VAST/VAPOR/third-party/apps-2017/Linux_x86_64/include/glm

RUN mv /glade/p/VAST/VAPOR/third-party/apps-2017/Linux_x86_64/include/glm/glm/* \
    /glade/p/VAST/VAPOR/third-party/apps-2017/Linux_x86_64/include/glm

# Pull github repo
RUN cd / \
    && git clone https://github.com/NCAR/VAPOR.git \
    && cd VAPOR \
    && cp site_files/site.NCAR site.local \
    && mkdir build

# Build in Docker Image to speed up build on CircleCI
RUN cd /VAPOR/build \
    && export CMAKE_CXX_COMPILER=g++ \
    && cmake .. && make

WORKDIR /VAPOR
