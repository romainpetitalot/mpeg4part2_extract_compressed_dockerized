from extract_mvs import av_register_all_w, avcodec_register_all_w

from extract_mvs import init_AVFormatContext, init_AVCodec, init_AVStream, init_AVCodecContext, init_AVFrame, init_AVDictionary, init_AVPacket

from extract_mvs import free_AVFormatContext, free_AVCodec, free_AVStream, free_AVCodecContext, free_AVFrame, free_AVDictionary, free_AVPacket

from extract_mvs import avformat_open_input_w, avformat_find_stream_info_w, av_find_best_stream_w, av_get_stream, avcodec_alloc_context3_w, avcodec_parameters_to_context_w, av_dict_set_w, avcodec_open2_w, av_frame_alloc_w, av_get_width, av_get_height

from extract_mvs import av_read_frame_w, av_get_packet_stream_idx, avcodec_send_packet_w, avcodec_receive_frame_w, av_is_error, av_packet_unref_w, read_frame

import cv2
import numpy as np

def convert(img: object,
            img_min: int = None,
            img_max: int = None,
            target_type_min: int = 0,
            target_type_max: int = 255,
            target_type: object = None):
    """ Converts an image to a given range of values.
    
        # Arguments:
            - img: The image to convert, a numpy array.
            - img_min: The min possible value of the image, if None, the min of the image is used.
            - img_max: The max possible value of the image, if None, the max of the image is used.
            - target_type_min: Target min value once converted.
            - target_max_type: Target max value once converted.
            - target_type: The numpy type to convert to.
        
        # Returns
            The converted image. The converted image is not a pointer to the original image.
    """
    if img_min is None:
        img_min = img.min()

    if img_max is None:
        img_max = img.max()

    if target_type is None:
        target_type = np.uint8

    # Scaling factor
    a = (target_type_max - target_type_min) / (img_max - img_min)
    # Centering factor
    b = target_type_max - a * img_max

    # Put the image in the desired range and convert to the required type
    new_img = (a * img + b).astype(target_type)

    return new_img


class MVSVideoReader(object):

    def __init__(self, file):
        """ Initialize the video stream to be ready to extract frames.
        
        # Arguments:
            -file: The mp4 file to be read.
        """
        self.is_done = False

        av_register_all_w()
        avcodec_register_all_w()

        self.fmt_ctx = init_AVFormatContext()
        self.dec = init_AVCodec()
        self.stream_idx = None
        self.st = init_AVStream()
        self.dec_ctx = init_AVCodecContext()
        self.opts = init_AVDictionary()
        self.pkt = init_AVPacket()

        ret = avformat_open_input_w(file, self.fmt_ctx)

        if ret < 0:
            raise RuntimeError("Could not open source file {}.".format(file))

        ret = avformat_find_stream_info_w(self.fmt_ctx)

        if ret < 0:
            raise RuntimeError("Could not find stream information.")

        ret = av_find_best_stream_w(self.fmt_ctx, self.dec)

        if ret < 0:
            raise RuntimeError("Could not find stream in input file {}".format(file))
        
        self.stream_idx = ret

        av_get_stream(self.fmt_ctx, self.stream_idx, self.st)

        avcodec_alloc_context3_w(self.dec, self.dec_ctx)

        ret = avcodec_parameters_to_context_w(self.dec_ctx, self.st)

        if ret < 0:
            raise RuntimeError("Failed to copy codec parameters to codec context.")

        av_dict_set_w(self.opts)

        ret = avcodec_open2_w(self.dec_ctx, self.dec, self.opts)

        if ret < 0:
            raise RuntimeError("Failed to open codec.")

        self.frame = av_frame_alloc_w()

        self.width = av_get_width(self.dec_ctx)
        self.height = av_get_height(self.dec_ctx)
    
    def close(self):
        """ Deallocate all the av stuff that was allocated."""
        free_AVFormatContext(self.fmt_ctx)
        free_AVCodec(self.dec)
        free_AVStream(self.st)
        free_AVCodecContext(self.dec_ctx)
        free_AVDictionary(self.opts)
        free_AVPacket(self.pkt)

    def read(self):
        """ Reads a frame from the stream. """
        if self.is_done:
            return False, None

        print("yoy")

        ret = av_read_frame_w(self.fmt_ctx, self.pkt)
        print("yoy")

        # If < 0, eof or error
        if ret < 0:
            self.is_done = True
            av_packet_unref_w(self.pkt)

            ret = avcodec_send_packet_w(self.dec_ctx, None)

            if ret < 0:
                raise RuntimeError("Error while sending a packet to the decoder: {}.".format(ret))

            while (ret >= 0):
                ret = avcodec_receive_frame_w(self.dec_ctx, self.frame)

                if av_is_error(ret):
                    self.is_done = True
                    return False, None
                elif ret < 0:
                    raise RuntimeError("Error while receiving a frame from the decoder: {}".format(ret))

                if ret >= 0:
                    mv_frame, mv_target = read_frame(self.frame, self.width, self.height)
                    return True, [mv_frame, mv_target]
                
        else:
            print("yoy")
            if av_get_packet_stream_idx(self.pkt) == self.stream_idx:
                ret = avcodec_send_packet_w(self.dec_ctx, self.pkt)

                if ret < 0:
                    raise RuntimeError("Error while sending a packet to the decoder: {}.".format(ret))

                print("yay")
                while (ret >= 0):
                    ret = avcodec_receive_frame_w(self.dec_ctx, self.frame)
                    if av_is_error(ret):
                        self.is_done = True
                        return False, None
                    elif ret < 0:
                        raise RuntimeError("Error while receiving a frame from the decoder: {}".format(ret))

                    print("yay")
                    if ret >= 0:
                        mv_frame, mv_target = read_frame(self.frame, self.width, self.height)
                        
                        return True, [mv_frame, mv_target]
                
            av_packet_unref_w(self.pkt)

        

if __name__ == "__main__":

    file_path = "/d2/thesis/datasets/mnist_qtv/01_linear_0-9/01_000/train/rgb_data/02491_rgb_data.mp4"
    file_path = "/data/thesis/datasets/qtv/05_parsed_data/01_data/E73.332A/2020_01_28/08_00_01/rgb_data/257_rgb_data.mp4"
    
    video_reader = MVSVideoReader(file_path)

    ret = True
    i = 0

    while ret:
        ret, frame = video_reader.read()

        if ret:
            mv_frame = frame[0]

            print(mv_frame.shape)
            print(mv_frame.dtype)
            
            w, h, _ = mv_frame.shape

            temp_frame = np.zeros((w, h, 3))
            temp_frame[:, :, :2] = mv_frame

            # min/max frame convertion, but its only for display
            mv_frame = convert(temp_frame)

            cv2.imshow('Motion Vectors', mv_frame)

            if cv2.waitKey(20) & 0xFF == ord('q'):
                break

        # print(frame[0].shape)
        # print(frame[1].shape)

    video_reader.close()

