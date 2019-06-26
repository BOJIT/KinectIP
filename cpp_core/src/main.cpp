#include "main.h"

// output vectors (GLOBAL):
std::vector<unsigned char> rgb_data;
std::vector<unsigned char> ir_data;
std::vector<unsigned char> depth_data;


// Signal Handler/Terminal Protection Functions
bool protonect_shutdown = false; ///< Whether the running application should shut down.

void sigint_handler(int s) {
  protonect_shutdown = true;
}

bool protonect_paused = false;
libfreenect2::Freenect2Device *devtopause;

//Doing non-trivial things in signal handler is bad. If you want to pause,
//do it in another thread.
//Though libusb operations are generally thread safe, I cannot guarantee
//everything above is thread safe when calling start()/stop() while
//waitForNewFrame().
void sigusr1_handler(int s) {
  if (devtopause == 0) {
		return;
	}
	/// [pause]
  if (protonect_paused) {
    devtopause->start();
	}
  else {
    devtopause->stop();
	}
  protonect_paused = !protonect_paused;
	/// [pause]
}

int main(int argc, char* argv[])
{	// Not required, but "correct" (see the SDK documentation)
	
	//////////////CONFIG//////////////
	// disable logging
	//libfreenect2::setGlobalLogger(NULL);

	////////Initialise Kinect/////////
	if (Kinect_Discover( true,true ) == -1) {
		printf("Error Initialising Kinect \n");
		return 0;
	}

	////////Test Frames///////////////
		std::cout << "NDI Test Patterns:" << std::endl;
	if (!NDIlib_initialize())
	{	// Cannot run NDI. Most likely because the CPU is not sufficient (see SDK documentation).
		// you can check this directly with a call to NDIlib_is_supported_CPU()
		printf("Cannot run NDI. \n");
		return 0;
	}

	if(!loadTestPatterns())
	{
		printf("Test Pattern Not Found \n");
		return 0;
	}
	
	return 0;
}

// loads PNGs into vectors
bool loadTestPatterns() {
	NDIlib_send_instance_t sender[4];
	char source[4][10] = { "Colour", "Depth", "Index", "Infrared"};
	for(int i=0; i<4; i++) {
		std::string source_name = source[i];
		std::string filepath = "./img/NDI_" + source_name + ".png";
		//std::cout << filepath << std::endl;
		std::vector<unsigned char> png_data;

		// Load Relevant Test Pattern
		loadFile(png_data, filepath);
		if (png_data.empty()) {
			printf("No PNG Found \n");
			return false;
		}

		// Decode the PNG file
		std::vector<unsigned char> image_data;
		unsigned long xres = 0, yres = 0;
		if (decodePNG(image_data, xres, yres, &png_data[0], png_data.size(), true)) {
			printf("Cannot Decode PNG \n");
			return false;
		}	

		// Create an NDI source that is clocked to the video.
		NDIlib_send_create_t NDI_send_create_desc;
		NDI_send_create_desc.p_ndi_name = source[i];

		// We create the NDI sender
		NDIlib_send_instance_t pNDI_send = NDIlib_send_create(&NDI_send_create_desc);
		if (!pNDI_send) {
			printf("Cannot Create NDI Sender \n");
			return false;
		}

		// We are going to create a frame
		NDIlib_video_frame_v2_t NDI_video_frame;
		NDI_video_frame.xres = xres;
		NDI_video_frame.yres = yres;
		NDI_video_frame.frame_rate_N = 30000;
		NDI_video_frame.frame_rate_D = 1001;
		NDI_video_frame.FourCC = NDIlib_FourCC_type_RGBA;
		NDI_video_frame.p_data = &image_data[0];
		NDI_video_frame.line_stride_in_bytes = xres * 4;
		NDI_video_frame.frame_format_type = NDIlib_frame_format_type_progressive;

		// We now submit the frame. Note that this call will be clocked so that we end up submitting at exactly 29.97fps.
		NDIlib_send_send_video_v2(pNDI_send, &NDI_video_frame);

		sender[i] = pNDI_send;

	}
	// Lets measure the performance for one minute
	std::this_thread::sleep_until(std::chrono::high_resolution_clock::now() + std::chrono::seconds(30));
	printf("Transmission Complete \n");
	
	// Destroy the NDI sender
	for(int i=0; i<4; i++) {
		NDIlib_send_destroy(sender[i]);
	}

	// Not required, but nice
	NDIlib_destroy();
	return true;
}

int Kinect_Discover(bool enable_rgb, bool enable_depth) {

	std::string serial = "";

	libfreenect2::Freenect2 freenect2;
	libfreenect2::Freenect2Device *dev = 0;
	libfreenect2::PacketPipeline *pipeline = 0;

	if(freenect2.enumerateDevices() == 0)
  {
    std::cout << "no device connected!" << std::endl;
    return -1;
  }
  if (serial == "")
  {
    serial = freenect2.getDefaultDeviceSerialNumber();
  }
	if(!pipeline)
	{
		#if defined LIBFREENECT2_WITH_OPENGL_SUPPORT && defined HEADLESS_GPU 	// note currently openGL is not utilised, future optimisation
			pipeline = new libfreenect2::OpenGLPacketPipeline();
		#else
    	std::cout << "OpenGL pipeline is not supported in current configuration!" << std::endl;
			pipeline = new libfreenect2::CpuPacketPipeline();
		#endif
	}

	  if(pipeline)
  {
    dev = freenect2.openDevice(serial, pipeline);		// specified pipeline
  }
  else
  {
    dev = freenect2.openDevice(serial);		// unspecified pipeline
  }

  if(dev == 0)
  {
    std::cout << "failure opening device!" << std::endl;
    return -1;
  }

	devtopause = dev;

  signal(SIGINT,sigint_handler);
		#ifdef SIGUSR1
  		signal(SIGUSR1, sigusr1_handler);
	#endif
  protonect_shutdown = false;

	/// [listeners]
  int types = 0;
  if (enable_rgb)
    types |= libfreenect2::Frame::Color;
  if (enable_depth)
    types |= libfreenect2::Frame::Ir | libfreenect2::Frame::Depth;
  libfreenect2::SyncMultiFrameListener listener(types);
  libfreenect2::FrameMap frames;

  dev->setColorFrameListener(&listener);
  dev->setIrAndDepthFrameListener(&listener);
	/// [listeners]

	// current plan is to initialise all streams regardless of whether they
	// are allocated to an NDI stream (streams can be enabled without re-initialisation)

	/// [start]
  if (enable_rgb && enable_depth)
  {
    if (!dev->start())
      return -1;
  }
  else
  {
    if (!dev->startStreams(enable_rgb, enable_depth))
      return -1;
  }

  std::cout << "device serial: " << dev->getSerialNumber() << std::endl;
  std::cout << "device firmware: " << dev->getFirmwareVersion() << std::endl;
	/// [start]

	/// [registration setup]
  libfreenect2::Registration* registration = new libfreenect2::Registration(dev->getIrCameraParams(), dev->getColorCameraParams());
  libfreenect2::Frame undistorted(512, 424, 4), registered(512, 424, 4);
	/// [registration setup]


	// frame capture test

	if (!listener.waitForNewFrame(frames, 10*1000)) // 10 sconds
  {
    std::cout << "timeout!" << std::endl;
    return -1;
  }
  libfreenect2::Frame *rgb = frames[libfreenect2::Frame::Color];
  libfreenect2::Frame *ir = frames[libfreenect2::Frame::Ir];
  libfreenect2::Frame *depth = frames[libfreenect2::Frame::Depth];


	// Create an NDI source that is clocked to the video.
	NDIlib_send_create_t NDI_send_create_desc;
	NDI_send_create_desc.p_ndi_name = "test";

	// We create the NDI sender
	NDIlib_send_instance_t pNDI_send = NDIlib_send_create(&NDI_send_create_desc);
	if (!pNDI_send) {
		printf("Cannot Create NDI Sender \n");
		return false;
	}

	// We are going to create a frame
	NDIlib_video_frame_v2_t NDI_video_frame;
	NDI_video_frame.xres = rgb->width;
	NDI_video_frame.yres = rgb->height;
	NDI_video_frame.frame_rate_N = 30000;
	NDI_video_frame.frame_rate_D = 1001;
	NDI_video_frame.FourCC = NDIlib_FourCC_type_BGRX;
	NDI_video_frame.p_data = rgb->data;
	NDI_video_frame.line_stride_in_bytes = rgb->width * 4;
	NDI_video_frame.frame_format_type = NDIlib_frame_format_type_progressive;

	// We now submit the frame. Note that this call will be clocked so that we end up submitting at exactly 29.97fps.
	NDIlib_send_send_video_v2(pNDI_send, &NDI_video_frame);

	/* // We create the NDI sender
	libfreenect2::Frame *rgb = frames[libfreenect2::Frame::Color];
	libfreenect2::Frame *ir = frames[libfreenect2::Frame::Ir];
	libfreenect2::Frame *depth = frames[libfreenect2::Frame::Depth];

	NDIlib_send_create_t NDI_send_create_desc;
	NDI_send_create_desc.p_ndi_name = "test";

	NDIlib_send_instance_t pNDI_send = NDIlib_send_create(&NDI_send_create_desc);
	if (!pNDI_send) return 0;

	// We are going to create a 1920x1080 interlaced frame at 29.97Hz.
	NDIlib_video_frame_v2_t NDI_video_frame;
	NDI_video_frame.xres = 1920;
	NDI_video_frame.yres = 1080;
	NDI_video_frame.frame_rate_N = 30000;
	NDI_video_frame.frame_rate_D = 1001;
	NDI_video_frame.FourCC = NDIlib_FourCC_type_BGRX;
	NDI_video_frame.p_data = rgb->data;

	size_t framemax = -1;
	size_t framecount = 0;

	// Run for one minute
	using namespace std::chrono;
	for (const auto start = high_resolution_clock::now(); high_resolution_clock::now() - start < seconds(30);)
	{	// Get the current time
		const auto start_send = high_resolution_clock::now();

	// Send 200 frames
	for (int idx = 200; idx; idx--) {	
		while(!protonect_shutdown && (framemax == (size_t)-1 || framecount < framemax))
  	{
		if (!listener.waitForNewFrame(frames, 10*1000)) // 10 sconds
		{
			std::cout << "timeout!" << std::endl;
			return -1;
		}
		libfreenect2::Frame *rgb = frames[libfreenect2::Frame::Color];
		libfreenect2::Frame *ir = frames[libfreenect2::Frame::Ir];
		libfreenect2::Frame *depth = frames[libfreenect2::Frame::Depth];
		//memset((void*)NDI_video_frame.p_data, (idx & 1) ? 255 : 0, NDI_video_frame.xres*NDI_video_frame.yres * 4);
		//NDI_video_frame.p_data = rgb->data;
		// We now submit the frame. Note that this call will be clocked so that we end up submitting at exactly 29.97fps.
		NDIlib_send_send_video_v2(pNDI_send, &NDI_video_frame);
		
		}

		// Just display something helpful
		printf("200 frames sent, at %1.2ffps\n", 200.0f / duration_cast<duration<float>>(high_resolution_clock::now() - start_send).count());
	}

	// Free the video frame
	free(NDI_video_frame.p_data);

	}*/

	listener.release(frames);

	dev->stop();
  dev->close();

	return 0;
}
