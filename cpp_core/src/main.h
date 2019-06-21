#ifndef MAIN
  #define MAIN

  #include <iostream>
  #include <string>
  #include <stdlib.h>
  #include <chrono>
  #include <thread>
  #include <signal.h>

  bool loadTestPatterns();

  int Kinect_Discover();

  struct Dev_Config
  {
     const std::string dev_serial;
     libfreenect2::PacketPipeline dev_pipeline;
  };

#endif