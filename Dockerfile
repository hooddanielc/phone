FROM debian
RUN apt update
RUN apt full-upgrade -y
RUN apt install -y git clang
RUN cd $HOME && git clone https://github.com/JasonL9000/ib
RUN echo "export PATH=$HOME/ib" >> $HOME/.bashrc
CMD bash