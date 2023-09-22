FROM gcc

WORKDIR /app

RUN apt update && apt -y install make yasm pkg-config libgl1 swig 
RUN apt -y install python3 python3-pip python3-numpy 

# RUN apt-get install libcunit1 libcunit1-doc libcunit1-dev

RUN pip install --break-system-packages numpy opencv-python scipy matplotlib

COPY . /app

RUN git clone https://github.com/FFmpeg/FFmpeg.git ffmpeg

RUN cd ffmpeg && git checkout release/6.0 

RUN cp -r mpeg42compressed/common/* ffmpeg/

RUN cd ffmpeg && ./configure --enable-shared --enable-debug=3 

RUN cd ffmpeg && make install

RUN cd ffmpeg && cp ./ffmpeg /bin/ && cp ./ffprobe /bin/

RUN cd ffmpeg && cp ./ffmpeg_g /bin/ && cp ./ffprobe_g /bin/

RUN echo "/usr/local/lib/" > /etc/ld.so.conf && ldconfig
