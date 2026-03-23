#define WINDOW_TITLE "ICP App"

// fps meter config
#define FPS_METER_INTERVAL 1000 
#define FPS_TEXT_FONT_SCALE 1.0
#define FPS_TEXT_LINE_WIDTH 2
#define FPS_TEXT_FONT cv::FONT_HERSHEY_SIMPLEX

//face detect config
#define DETECT_SIZE_SCALE_FACTOR 0.8
#define DETECT_SCALE_FACTOR 1.1
#define MIN_FACE_SIZE 60
#define DETECT_MIN_NEIGHBORS 6
#define FRAMES_PER_DETECTION 5

//GL config
#define NEAR_CLIP_PLANE 0.1f
#define NEAR_CLIP_PLANE 0.1f
#define FAR_CLIP_PLANE 20000.0f

//file path defines 
#define TEXTURE_SHADER_PATH_VERT "../resources/shaders/tex.vert"
#define TEXTURE_SHADER_PATH_FRAG "../resources/shaders/tex.frag"

//screenshot config
#define SCREENSHOT_FILE_NAME "Screenshot"
#define SCREENSHOT_TIMESTAMP_FORMAT "%F_%H-%M-%S"

//audio config
#define AUDIO_MIN_DISTANCE 0.5f
#define AUDIO_MAX_DISTANCE 100.0f
#define AUDIO_DEF_VOLUME 2.0f

//mic config
#define MIC_BUFFER_FRAMES 2048

//webcam config
#define WEBCAM_SCALE_FACTOR 0.25f
#define WEBCAM_CAPTURE_SIZE 512

//game config 
#define N_TARGETS 5
#define ROOM_SIZE 15.0f
#define ROOM_HEIGHT 6.0f
#define BOXES_Z_POSITION 0.0f
#define BOXES_COUNT 15
#define BOXES_START_X -7.0f
#define BOXES_SPACING 1.0f
#define BOXES_Y_POSITION 0.5f