#include "main.h"

//**********PREPROCESSOR**********//
	//#define hideLogs
	#define debug_Time 30
	#define enable_rgb
	#define enable_depth
//**************END***************//

//************GLOBALS*************//
	/*std::vector<unsigned char> rgb_data;
	std::vector<unsigned char> depth_data;
	std::vector<unsigned char> index_data;
	std::vector<unsigned char> ir_data;*/
	// Kinect Device Context
	libfreenect2::Freenect2 freenect2;
	libfreenect2::Freenect2Device *dev = 0;
	libfreenect2::PacketPipeline *pipeline = 0;
//**************END***************//

//*********SIGNAL HANDLERS********//
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
//**************END***************//

//**********MAIN PROGRAM**********//
	int main(int argc, char* argv[])
	{
		#ifdef hideLogs
			libfreenect2::setGlobalLogger(NULL);
		#endif
		
		signal(SIGINT, sigint_handler);
		#ifdef SIGUSR1
			signal(SIGUSR1, sigusr1_handler);
		#endif
		protonect_shutdown = false;
		
		// initialise Kinect
		if (Kinect_Config() == -1) {
			std::cout << "Error Initialising Kinect" << std::endl;
			return 0;
		}
		
		/// [listeners]
		int types = 0;
		
		types |= libfreenect2::Frame::Color | libfreenect2::Frame::Ir | libfreenect2::Frame::Depth;

		libfreenect2::SyncMultiFrameListener listener(types);
		libfreenect2::FrameMap frames;

		dev->setColorFrameListener(&listener);
		dev->setIrAndDepthFrameListener(&listener);
		/// [listeners]

		/// [start]
		if (!dev->start()) return -1;

		std::cout << "device serial: " << dev->getSerialNumber() << std::endl;
		std::cout << "device firmware: " << dev->getFirmwareVersion() << std::endl;
		/// [start]

		/// [registration setup]
		libfreenect2::Registration* registration = new libfreenect2::Registration(dev->getIrCameraParams(), dev->getColorCameraParams());
		libfreenect2::Frame undistorted(512, 424, 4), registered(512, 424, 4);
		/// [registration setup]

		std::cout << "KinectIP:" << std::endl;
		if (!NDIlib_initialize()) {
			std::cout << "Cannot Run NDI" << std::endl;	// CPU does not support NDI encoding
			return 0;
		}
		
		NDIlib_send_instance_t sender[4];		// array holds stream instances
		const char source[4][10] = { "Colour", "Depth", "Index", "Infrared"};
		for(int i=0; i<4; i++) {
			if(parseLine(i + 4) == "OFF")		// config file lines are zero-referenced
			{
				std::cout << source[i] << " stream not required" << std::endl;
				sender[i] = 0;
			}
			else
			{
				std::cout << source[i] << " stream required" << std::endl;
				sender[i] = loadStreams(source[i]);
			}
		}
		std::cout << "--------Streams Initialised--------" << std::endl;

		// if debugging is enabled, the while loop will break after the specified period
		#ifdef debug_Time
			std::chrono::system_clock::time_point debug_Period = std::chrono::system_clock::now();
		#endif

		NDIlib_video_frame_v2_t NDI_frame = createFrame(source[1], false);			// collect metadata with initial frame
		if (!listener.waitForNewFrame(frames, 10*1000)) // 10 seconds
			{
				std::cout << "timeout! Device Not Responding" << std::endl;
				return -1;
			}
		libfreenect2::Frame *rgb = frames[libfreenect2::Frame::Color];
		NDI_frame.xres = rgb->width;
		NDI_frame.yres = rgb->height;
		NDI_frame.p_data = &rgb->data[0];						// pointer to image data
		NDI_frame.line_stride_in_bytes = rgb->width * 4;

		while(protonect_shutdown == false) {
			// indefinite loop goes here ...
			/* printf("flag");
			if (!listener.waitForNewFrame(frames, 10*1000)) // 10 seconds
			{
				std::cout << "timeout! Device Not Responding" << std::endl;
				return -1;
			}
			libfreenect2::Frame *rgb = frames[libfreenect2::Frame::Color];
			//libfreenect2::Frame *ir = frames[libfreenect2::Frame::Ir];
			//libfreenect2::Frame *depth = frames[libfreenect2::Frame::Depth];

			NDI_frame.p_data = &rgb->data[0];						// pointer to image data
			*/		
			// Submit frame. Note that this call will be clocked so that we end up submitting at the desired framerate.
			NDIlib_send_send_video_v2(sender[1], &NDI_frame);	
			listener.release(frames);
			//printf("flag");

			// ... and ends here!
			#ifdef debug_Time
				if(std::chrono::system_clock::now() >= debug_Period + std::chrono::seconds(debug_Time)) break;
			#endif
		}
		// Shutdown Routine
		std::cout << "-------Destroying Senders--------" << std::endl;

		// Destroy all open NDI senders
		for(int i=0; i<4; i++) {
			if(sender[i] != 0) {
				NDIlib_send_destroy(sender[i]);
			}	
		}

		// safely stop application-specific processes
		dev->stop();
		dev->close();
		NDIlib_destroy();
		return 0;
	}
//**************END***************//

//********STREAM FUNCTIONS********//

	NDIlib_send_instance_t loadStreams(const char stream_name[]) {
		std::string source_name = stream_name;
		std::string filepath = "./img/NDI_" + source_name + ".png";

		std::vector<unsigned char> png_data;
		// Load Relevant Test Pattern
		loadFile(png_data, filepath);
		if (png_data.empty()) {
			printf("No PNG Found \n");
			return 0;
		}

		// Decode the PNG file
		std::vector<unsigned char> image_data;
		unsigned long xres = 0, yres = 0;
		if (decodePNG(image_data, xres, yres, &png_data[0], png_data.size(), true)) {
			printf("Cannot Decode PNG \n");
			return 0;
		}	

		// Create an NDI source that is clocked to the video.
		NDIlib_send_create_t NDI_send_create_desc;
		NDI_send_create_desc.p_ndi_name = stream_name;

		// We create the NDI sender
		NDIlib_send_instance_t pNDI_send = NDIlib_send_create(&NDI_send_create_desc);
		if (!pNDI_send) {
			printf("Cannot Create NDI Sender \n");
			return 0;
		}

		// We are going to create a frame
		NDIlib_video_frame_v2_t NDI_video_frame = createFrame(stream_name, true);		// createFrame pulls the default framerates 
		NDI_video_frame.xres = xres;																									// and colour formats from the config files
		NDI_video_frame.yres = yres;
		NDI_video_frame.p_data = &image_data[0];						// pointer to image data
		NDI_video_frame.line_stride_in_bytes = xres * 4;
		
		// We now submit the first frame. Note that this call will be clocked so that we end up submitting at the desired framerate.
		NDIlib_send_send_video_v2(pNDI_send, &NDI_video_frame);		// this is the test pattern until the kinect is initialised

		return pNDI_send;		// return stream instance for future referencing
	}

	NDIlib_video_frame_v2_t createFrame(const char stream_name[], bool testImg) {
		NDIlib_video_frame_v2_t NDI_video_frame;
		// sets config file lines to read
		int index = 0;
		if( !strcmp( stream_name, "Colour" )) {
			index = 0;
		}
		else if( !strcmp( stream_name, "Depth" )) {
			index = 1;
		}
		else if( !strcmp( stream_name, "Index" )) {
			index = 2;
		}
		else if( !strcmp( stream_name, "Infrared" )) {
			index = 3;
		}

		// set framerate
		if(parseLine(index + 11) == "24p") {
			NDI_video_frame.frame_rate_N = 24000;
			NDI_video_frame.frame_rate_D = 1001;
			NDI_video_frame.frame_format_type = NDIlib_frame_format_type_progressive;
		}
		else if(parseLine(index + 11) == "25p") {
			NDI_video_frame.frame_rate_N = 25;
			NDI_video_frame.frame_rate_D = 1;
			NDI_video_frame.frame_format_type = NDIlib_frame_format_type_progressive;
		}
		else if(parseLine(index + 11) == "29.97p") {
			NDI_video_frame.frame_rate_N = 30000;
			NDI_video_frame.frame_rate_D = 1001;
			NDI_video_frame.frame_format_type = NDIlib_frame_format_type_progressive;
		}
		else if(parseLine(index + 11) == "50i") {
			NDI_video_frame.frame_rate_N = 25;
			NDI_video_frame.frame_rate_D = 1;
			NDI_video_frame.frame_format_type = NDIlib_frame_format_type_interleaved;
		}
		else if(parseLine(index + 11) == "60i") {
			NDI_video_frame.frame_rate_N = 30000;
			NDI_video_frame.frame_rate_D = 1001;
			NDI_video_frame.frame_format_type = NDIlib_frame_format_type_interleaved;
		}
		else {		// default frame settings
			NDI_video_frame.frame_rate_N = 30000;
			NDI_video_frame.frame_rate_D = 1001;
			NDI_video_frame.frame_format_type = NDIlib_frame_format_type_progressive;
		}
		
		// set image colour format
		if(testImg == true) {
			NDI_video_frame.FourCC = NDIlib_FourCC_type_RGBA;
		}
		else {
			NDI_video_frame.FourCC = NDIlib_FourCC_type_BGRX;
		}
		return NDI_video_frame;
	}
//**************END***************//

//********KINECT FUNCTIONS********//
	int Kinect_Config() {
		std::string serial = "";

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
		return 0;
	}
//**************END***************//

//*********FILE FUNCTIONS*********//
	std::string parseLine(uint8_t line) {
		std::string parameter;
		std::ifstream configFile;
		configFile.open("/var/www/html/config/settings.conf");
    if (configFile.is_open()) {
			std::string raw_string;
			for(int i=0; i<line + 1; i++) {
				getline(configFile, raw_string);
			}

			std::size_t pos = raw_string.find("=");
			parameter = raw_string.substr (pos + 1);
		}
    configFile.close();

		return parameter.c_str();
	}
//**************END***************//