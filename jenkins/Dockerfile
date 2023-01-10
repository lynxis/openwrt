ARG BASE_IMAGE
FROM ${BASE_IMAGE}

ARG USER_ID
ARG GROUP_ID
RUN groupadd -g ${GROUP_ID} builder && useradd -u ${USER_ID} -g ${GROUP_ID} builder -m
RUN dnf install -y ncurses-devel perl-Thread-Queue rsync
