#ifndef MAIN
  #define MAIN 

  //#define HEADLESS_GPU

  #include <iostream>
  #include <cstring>
  #include <sstream>
  #include <stdlib.h>
  #include <chrono>
  #include <thread>
  #include <signal.h>
  #include <fstream>

  #include <NDI/Processing.NDI.Lib.h>
  #include <PicoPNG/picopng.hpp>

  #include <libfreenect2/libfreenect2.hpp>
  #include <libfreenect2/frame_listener_impl.h>
  #include <libfreenect2/registration.h>
  #include <libfreenect2/packet_pipeline.h>
  #include <libfreenect2/logger.h>

  NDIlib_send_instance_t loadStreams(const char stream_name[]);   // loads NDI streams and fills in test images

  int Kinect_Discover(bool enable_rgb, bool enable_depth);  // connect and configure Kinect

  std::string parseLine(uint8_t line);  // read line from config file

  NDIlib_video_frame_v2_t createFrame(const char stream_name[], bool testImg);

#endif