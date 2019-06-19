#include <iostream>
#include <stdlib.h>
#include <chrono>
#include <thread>

#include <KinectIP/game_interface.h>
#include <NDI/Processing.NDI.Lib.h>
#include <PicoPNG/picopng.hpp>

int main(int argc, char* argv[])
{	// Not required, but "correct" (see the SDK documentation)
	std::cout << "hello world 2!" << std::endl;
	if (!NDIlib_initialize())
	{	// Cannot run NDI. Most likely because the CPU is not sufficient (see SDK documentation).
		// you can check this directly with a call to NDIlib_is_supported_CPU()
		printf("Cannot run NDI.");
		return 0;
	}
	// Lets load the file from disk
	std::vector<unsigned char> png_data;
	loadFile(png_data, "./img/NewTekNDI.png");
	if (png_data.empty()) {
		printf("No PNG Found");
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
	NDI_send_create_desc.p_ndi_name = "Colour";

	// We create the NDI sender
	NDIlib_send_instance_t pNDI_send = NDIlib_send_create(&NDI_send_create_desc);
	if (!pNDI_send) {
		printf("Cannot Create NDI Sender \n");
		return 0;
	}
	// We are going to create a frame
	NDIlib_video_frame_v2_t NDI_video_frame;
	NDI_video_frame.xres = xres;
	NDI_video_frame.yres = yres;
	NDI_video_frame.FourCC = NDIlib_FourCC_type_RGBA;
	NDI_video_frame.p_data = &image_data[0];
	NDI_video_frame.line_stride_in_bytes = xres * 4;

	// We now submit the frame. Note that this call will be clocked so that we end up submitting at exactly 29.97fps.
	NDIlib_send_send_video_v2(pNDI_send, &NDI_video_frame);

	// Lets measure the performance for one minute
	printf("Source is not on output !");
	std::this_thread::sleep_until(std::chrono::high_resolution_clock::now() + std::chrono::minutes(2));

	// Destroy the NDI sender
	NDIlib_send_destroy(pNDI_send);
	
	// Not required, but nice
	NDIlib_destroy();

	// Success
	return 0;
}