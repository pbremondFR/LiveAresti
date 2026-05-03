#include <LiveAresti.hpp>
#include <Processing.NDI.Lib.h>

#include "raylib.h"

void ndi_export()
{
	Image output_image_ndi = LoadImageFromTexture(g_state.test_output_target.texture);
	ImageFormat(&output_image_ndi, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
	ImageFlipVertical(&output_image_ndi);
	NDIlib_video_frame_v2_t NDI_video_frame;
	NDI_video_frame.xres = g_state.test_output_target.texture.width;
	NDI_video_frame.yres = g_state.test_output_target.texture.height;
    
	NDI_video_frame.frame_rate_N = 60000;
	NDI_video_frame.frame_rate_D = 1000;
	NDI_video_frame.FourCC = NDIlib_FourCC_type_RGBA;
    
	NDI_video_frame.p_data = static_cast<uint8_t*>(output_image_ndi.data);
	NDI_video_frame.line_stride_in_bytes = g_state.test_output_target.texture.width * 4;
		
	NDIlib_send_send_video_v2(g_state.NDI_send_ptr, &NDI_video_frame);
	UnloadImage(output_image_ndi);

}
