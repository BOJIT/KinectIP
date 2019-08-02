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

  
  #include <sys/shm.h>		//Used for shared memory
  #include <sys/sem.h>		//Used for semaphores

  #include <NDI/Processing.NDI.Lib.h>
  #include <PicoPNG/picopng.hpp>

  #include <libfreenect2/libfreenect2.hpp>
  #include <libfreenect2/frame_listener_impl.h>
  #include <libfreenect2/registration.h>
  #include <libfreenect2/packet_pipeline.h>
  #include <libfreenect2/logger.h>

  NDIlib_send_instance_t loadStreams(const char stream_name[]);   // loads NDI streams and fills in test images

  int Kinect_Config();  // connect and configure Kinect

  std::string parseLine(uint8_t line);  // read line from config file

  NDIlib_video_frame_v2_t createFrame(const char stream_name[], bool testImg);  // gets basic frame information from PHP config file

  void initialiseSHMEM(); //Shared Memory Initialise Declaration 
  	
  int ReadSHMEM(unsigned int Index);
	
	void WriteSHMEM(unsigned int Index, uint8_t Value);

  //----- SEMAPHORE -----
  //On linux systems this union is probably already defined in the included sys/sem.h, but if not use this default basic definition:
  union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
  };

  //----- SHARED MEMORY -----
  struct shared_memory1_struct {
    char shared_bytes[8];
  };
  #define	SEMAPHORE_KEY			291623581  			//Semaphore unique key (MAKE DIFFERENT TO PHP KEY)
  #define	SHARED_MEMORY_KEY 		672213396   		//Shared memory unique key (SAME AS PHP KEY)

  static int semaphore1_get_access(void);
  static int semaphore1_release_access(void);

  static int semaphore1_id;
  void *shared_memory1_pointer = (void *)0;
  struct shared_memory1_struct *shared_memory1;
  int shared_memory1_id;

  //----- SHARED MEMORY INDEX -----
  #define UPDATE_SETTINGS 0
  #define RESTART_SYSTEM 1
  #define STREAM_ACTIVE 2
  #define TEST_PATTERNS 3

#endif