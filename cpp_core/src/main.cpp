#include "main.h"

//**********PREPROCESSOR**********//
	//#define hideLogs
	#define debug_Time 30
	#define enable_rgb
	//#define enable_depth
	#define LED_pin 100
	#define Switch1 82
	#define Switch2 83

	//#define OPENGL
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

		// initialise shared memory sector
		initialiseSHMEM();
		
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
		
		#ifdef enable_rgb
			types |= libfreenect2::Frame::Color;
		#endif
		#ifdef enable_depth
			types |= libfreenect2::Frame::Ir | libfreenect2::Frame::Depth;
		#endif
		libfreenect2::SyncMultiFrameListener listener(types);
		libfreenect2::FrameMap frames;

		dev->setColorFrameListener(&listener);
		dev->setIrAndDepthFrameListener(&listener);
		/// [listeners]

		/// [start]
		#if defined enable_rgb && defined enable_depth
			if (!dev->start()) return -1;						// start both streams
		#elif defined enable_rgb
			if (!dev->startStreams(1,0)) return -1;	// start RGB only
		#else
			if (!dev->startStreams(0,1)) return -1;	// start Depth only
		#endif

		std::cout << "device serial: " << dev->getSerialNumber() << std::endl;
		std::cout << "device firmware: " << dev->getFirmwareVersion() << std::endl;
		/// [start]
/*
		/// [registration setup]
		libfreenect2::Registration* registration = new libfreenect2::Registration(dev->getIrCameraParams(), dev->getColorCameraParams());
		libfreenect2::Frame undistorted(512, 424, 4), registered(512, 424, 4);
		/// [registration setup]
*/
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

		listener.release(frames);

		WriteSHMEM(STREAM_ACTIVE, 255);

		std::cout << "--------Streams Initialised--------" << std::endl;
		// if debugging is enabled, the while loop will break after the specified period
		#ifdef debug_Time
			std::chrono::system_clock::time_point debug_Period = std::chrono::system_clock::now();
		#endif

		while(protonect_shutdown == false) {
			// indefinite loop starts here ...
			if (!NDIlib_send_get_no_connections(sender[0], 10000)) {}
			else
			{	
				if (!listener.waitForNewFrame(frames, 10*1000)) // 10 seconds
				{
					std::cout << "timeout! Device Not Responding" << std::endl;
					return -1;
				}
				libfreenect2::Frame *rgb = frames[libfreenect2::Frame::Color];
				NDI_frame.p_data = &rgb->data[0];	
				NDIlib_send_send_video_v2(sender[0], &NDI_frame);
				listener.release(frames);
			}
			if(ReadSHMEM(RESTART_SYSTEM) != 0) {
				break;
			}
			// ... and ends here!
			#ifdef debug_Time
				if(std::chrono::system_clock::now() >= debug_Period + std::chrono::seconds(debug_Time)) break;
			#endif
		}
		// Shutdown Routine
		std::cout << "-------Destroying Senders--------" << std::endl;

		WriteSHMEM(STREAM_ACTIVE, 0);

		// Destroy all open NDI senders
		for(int i=0; i<4; i++) {
			if(sender[i] != 0) {
				NDIlib_send_send_video_v2(sender[i], NULL);
				NDIlib_send_destroy(sender[i]);
			}	
		}

		// safely stop application-specific processes
		dev->stop();
		dev->close();
		NDIlib_destroy();

		//----- DETACH SHARED MEMORY -----
		//Detach and delete
		if (shmdt(shared_memory1_pointer) == -1)
		{
			fprintf(stderr, "shmdt failed\n");
			//exit(EXIT_FAILURE);
		}
		if (shmctl(shared_memory1_id, IPC_RMID, 0) == -1)
		{
			fprintf(stderr, "shmctl(IPC_RMID) failed\n");
			//exit(EXIT_FAILURE);
		}
		//Delete the Semaphore
		//It's important not to unintentionally leave semaphores existing after program execution. It also may cause problems next time you run the program.
		union semun sem_union_delete;
		if (semctl(semaphore1_id, 0, IPC_RMID, sem_union_delete) == -1)
			fprintf(stderr, "Failed to delete semaphore\n");

		return 0;
	}
//**************END***************//

//********STREAM FUNCTIONS********//

	NDIlib_send_instance_t loadStreams(const char stream_name[]) {
		std::string source_name = stream_name;
		std::string filepath = "/home/rock64/img/NDI_" + source_name + ".png";

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

	int Update_Settings() {
		printf("flag\n");
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
			#if defined LIBFREENECT2_WITH_OPENGL_SUPPORT && defined OPENGL
      	if(!pipeline)
        	pipeline = new libfreenect2::OpenGLPacketPipeline();
			#else
				std::cout << "OpenGL pipeline is not supported!" << std::endl;
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

	void initialiseSHMEM()
	{
		//-----------------------------------------------
		//----- CREATE SHARED MEMORY WITH SEMAPHORE -----
		//-----------------------------------------------
		printf("Creating shared memory with semaphore...\n");
		semaphore1_id = semget((key_t)SEMAPHORE_KEY, 1, 0666 | IPC_CREAT);		//Semaphore key, number of semaphores required, flags
		//	Semaphore key
		//		Unique non zero integer (usually 32 bit).  Needs to avoid clashing with another other processes semaphores (you just have to pick a random value and hope - ftok() can help with this but it still doesn't guarantee to avoid colision)

		//Initialize the semaphore using the SETVAL command in a semctl call (required before it can be used)
		union semun sem_union_init;
		sem_union_init.val = 1;
		if (semctl(semaphore1_id, 0, SETVAL, sem_union_init) == -1)
		{
			fprintf(stderr, "Creating semaphore failed to initialize\n");
			exit(EXIT_FAILURE);
		}

		//Create the shared memory
		shared_memory1_id = shmget((key_t)SHARED_MEMORY_KEY, sizeof(struct shared_memory1_struct), 0666 | IPC_CREAT);		//Shared memory key , Size in bytes, Permission flags
		//	Shared memory key
		//		Unique non zero integer (usually 32 bit).  Needs to avoid clashing with another other processes shared memory (you just have to pick a random value and hope - ftok() can help with this but it still doesn't guarantee to avoid colision)
		//	Permission flags
		//		Operation permissions 	Octal value
		//		Read by user 				00400
		//		Write by user 			00200
		//		Read by group 			00040
		//		Write by group 			00020
		//		Read by others 			00004
		//		Write by others			00002
		//		Examples:
		//			0666 Everyone can read and write

		if (shared_memory1_id == -1)
		{
			fprintf(stderr, "Shared memory shmget() failed\n");
			exit(EXIT_FAILURE);
		}

		//Make the shared memory accessible to the program
		shared_memory1_pointer = shmat(shared_memory1_id, (void *)0, 0);
		if (shared_memory1_pointer == (void *)-1)
		{
			fprintf(stderr, "Shared memory shmat() failed\n");
			exit(EXIT_FAILURE);
		}
		printf("Shared memory attached\n");

		//Assign the shared_memory segment
		shared_memory1 = (struct shared_memory1_struct *)shared_memory1_pointer;


		//----- SEMAPHORE GET ACCESS -----
		if (!semaphore1_get_access())
			exit(EXIT_FAILURE);

		//----- WRITE SHARED MEMORY -----
		unsigned int Index;
		for (Index = 0; Index < sizeof(struct shared_memory1_struct); Index++)
			shared_memory1->shared_bytes[Index] = 0x00;

		//Write initial values
		shared_memory1->shared_bytes[UPDATE_SETTINGS] = 0;
		shared_memory1->shared_bytes[RESTART_SYSTEM] = 0;
		shared_memory1->shared_bytes[STREAM_ACTIVE] = 0;
		shared_memory1->shared_bytes[TEST_PATTERNS] = 0;

		//----- SEMAPHORE RELEASE ACCESS -----
		if (!semaphore1_release_access())
			exit(EXIT_FAILURE);
	}

	//Stall if another process has the semaphore, then assert it to stop another process taking it
	static int semaphore1_get_access(void)
	{
		struct sembuf sem_b;
		sem_b.sem_num = 0;
		sem_b.sem_op = -1; /* P() */
		sem_b.sem_flg = SEM_UNDO;
		if (semop(semaphore1_id, &sem_b, 1) == -1)		//Wait until free
		{
			fprintf(stderr, "semaphore1_get_access failed\n");
			return(0);
		}
		return(1);
	}

	//Release the semaphore and allow another process to take it
	static int semaphore1_release_access(void)
	{
		struct sembuf sem_b;
		sem_b.sem_num = 0;
		sem_b.sem_op = 1; /* V() */
		sem_b.sem_flg = SEM_UNDO;
		if (semop(semaphore1_id, &sem_b, 1) == -1)
		{
			fprintf(stderr, "semaphore1_release_access failed\n");
			return(0);
		}
		return(1);
	}

	int ReadSHMEM(unsigned int Index) {
			//----- SEMAPHORE GET ACCESS -----
			uint8_t Value;
			if (!semaphore1_get_access())
				exit(EXIT_FAILURE);
			
			//----- ACCESS THE SHARED MEMORY -----
			Value = shared_memory1->shared_bytes[Index];
			
			//----- SEMAPHORE RELEASE ACCESS -----
			if (!semaphore1_release_access())
				exit(EXIT_FAILURE);
			
			return Value;
	}

	void WriteSHMEM(unsigned int Index, uint8_t Value) {
				//----- SEMAPHORE GET ACCESS -----
		if (!semaphore1_get_access())
			exit(EXIT_FAILURE);

		//----- WRITE SHARED MEMORY -----

		//Write initial values
		shared_memory1->shared_bytes[Index] = Value;

		//----- SEMAPHORE RELEASE ACCESS -----
		if (!semaphore1_release_access())
			exit(EXIT_FAILURE);
	}
//**************END***************//