FROM ubuntu:18.04
LABEL Name=pillserial Version=0.0.1
RUN   apt-get update \
   && apt-get install -y  \
       git \
       build-essential \
       cmake \
       python3  

RUN apt-get install -y gcc-arm-none-eabi 
         

RUN mkdir /usr/local/src/pill_serial
WORKDIR /usr/local/src/pill_serial
COPY . .
RUN rm -rf libopencm3
RUN git config --global user.email "me@inimage" 
RUN git config --global user.name "ME IN IMAGE" 
RUN git init .  
RUN git add --all 
RUN git commit -m "git init inside image"




#RUN make

CMD ["bash"]