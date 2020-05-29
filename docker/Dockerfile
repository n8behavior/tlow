FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update -q
RUN apt dist-upgrade -y -q
RUN apt install -y -q --no-install-recommends \
        git ca-certificates cmake build-essential \
        libpng-dev libx11-dev libgl1-mesa-dev
