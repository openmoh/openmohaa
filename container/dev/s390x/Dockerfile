FROM --platform=linux/s390x ubuntu:22.10

# install dev tools
RUN apt update && apt install -y ninja-build cmake make clang-15 lld-15 flex bison gdb gdbserver lldb \
 openssh-server rsync zip
# install some important libraries
RUN apt install -y --no-install-recommends ncurses-dev

# configure ssh to use X11
RUN sed -i "s/^.*X11Forwarding.*$/X11Forwarding yes/" /etc/ssh/sshd_config \
 && sed -i "s/^.*X11UseLocalhost.*$/X11UseLocalhost no/" /etc/ssh/sshd_config

RUN useradd -m buildmaster \
 && echo "buildmaster:buildmaster" | chpasswd

EXPOSE 22

CMD service ssh start && /bin/bash
