#ifndef MAIN
  #define MAIN 

  //#define HEADLESS_GPU

  #include <iostream>
  #include <string>
  #include <stdlib.h>
  #include <chrono>
  #include <thread>
  #include <signal.h>

  #include <NDI/Processing.NDI.Lib.h>
  #include <PicoPNG/picopng.hpp>

  #include <libfreenect2/libfreenect2.hpp>
  #include <libfreenect2/frame_listener_impl.h>
  #include <libfreenect2/registration.h>
  #include <libfreenect2/packet_pipeline.h>
  #include <libfreenect2/logger.h>

  bool loadTestPatterns();

  int Kinect_Discover(bool enable_rgb, bool enable_depth);

  int cast_Vector();

  /*struct Dev_Config
  {
     const std::string dev_serial;
     libfreenect2::PacketPipeline dev_pipeline;
  };*/

#endif