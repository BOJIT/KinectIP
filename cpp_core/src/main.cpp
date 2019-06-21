
#include "main.h"

int main(int argc, char* argv[])
{	// Not required, but "correct" (see the SDK documentation)
	
	////////Initialise Kinect/////////
	/*std::string program_path(argv[0]);
  std::cerr << "Version: " << LIBFREENECT2_VERSION << std::endl;
  std::cerr << "Environment variables: LOGFILE=<protonect.log>" << std::endl;
  std::cerr << "Usage: " << program_path << " [-gpu=<id>] [gl | cl | clkde | cuda | cudakde | cpu] [<device serial>]" << std::endl;
  std::cerr << "        [-noviewer] [-norgb | -nodepth] [-help] [-version]" << std::endl;
  std::cerr << "        [-frames <number of frames to process>]" << std::endl;
  std::cerr << "To pause and unpause: pkill -USR1 Protonect" << std::endl;
  size_t executable_name_idx = program_path.rfind("Protonect");

  std::string binpath = "/";

  if(executable_name_idx != std::string::npos)
  {
    binpath = program_path.substr(0, executable_name_idx);
  }*/
	
	//////////////////////////////////
	if (Kinect_Discover() == -1) {
		printf("Cannot find Kinect \n");
		return 0;
	}

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
	printf("Transmission Complete \n");
	std::this_thread::sleep_until(std::chrono::high_resolution_clock::now() + std::chrono::seconds(30));

	// Destroy the NDI sender
	for(int i=0; i<4; i++) {
		NDIlib_send_destroy(sender[i]);
	}

	// Not required, but nice
	NDIlib_destroy();
	return true;
}
int Kinect_Discover() {
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

	// current plan is to initialise all streams regardless of whether they
	// are allocated to an NDI stream (streams can be enabled without re-initialisation)

	return 0;
}