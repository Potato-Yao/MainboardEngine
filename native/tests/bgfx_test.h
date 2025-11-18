// #define me_bgfx_test
#ifdef me_bgfx_test

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <bgfx/bgfx.h>
#include <mainboard_engine.h>
#include <event_message_type.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Structure to hold image and texture data
struct ImageData {
    std::string path;
    bgfx::TextureHandle texture;
    float width;
    float height;
    unsigned char* data;
};

// Helper function to load shader
static bgfx::ShaderHandle loadShader(const char* filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "Failed to open shader file: " << filename << std::endl;
        return BGFX_INVALID_HANDLE;
    }

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    const bgfx::Memory* mem = bgfx::alloc(uint32_t(size + 1));
    file.read(reinterpret_cast<char *>(mem->data), size);
    mem->data[size] = '\0';
    file.close();

    return bgfx::createShader(mem);
}

// Helper function to load a single image
static ImageData loadImage(const std::string& path) {
    ImageData img;
    img.path = path;
    img.data = nullptr;

    int imgWidth, imgHeight, imgChannels;
    img.data = stbi_load(path.c_str(), &imgWidth, &imgHeight, &imgChannels, 4);

    if (!img.data) {
        std::cout << "ERROR: Failed to load image from " << path << std::endl;
        img.texture = BGFX_INVALID_HANDLE;
        img.width = 0;
        img.height = 0;
        return img;
    }

    std::cout << "Image loaded: " << path << " - " << imgWidth << "x" << imgHeight << std::endl;

    img.width = float(imgWidth);
    img.height = float(imgHeight);

    // Create texture with point filtering for sharp pixel art
    img.texture = bgfx::createTexture2D(imgWidth, imgHeight, false, 1, bgfx::TextureFormat::RGBA8,
                                        BGFX_TEXTURE_NONE | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT,
                                        bgfx::copy(img.data, imgWidth * imgHeight * 4));

    if (!bgfx::isValid(img.texture)) {
        std::cout << "ERROR: Failed to create texture for " << path << std::endl;
        stbi_image_free(img.data);
        img.data = nullptr;
    }

    return img;
}

// Helper function to load multiple images
static std::vector<ImageData> loadImages(const std::vector<std::string>& paths) {
    std::vector<ImageData> images;
    for (const auto& path : paths) {
        images.push_back(loadImage(path));
    }
    return images;
}

int execute() {
    using namespace bgfx;
    using namespace std;

    ME_Initialize();

    uint32_t windowWidth = 1200;
    uint32_t windowHeight = 800;

    auto window = ME_CreateWindow(0, 100, 100, windowWidth, windowHeight, "BGFX Multi-Image Renderer");
    Init bgfx_init;
    bgfx_init.type = RendererType::Count;
    bgfx_init.resolution.width = windowWidth;
    bgfx_init.resolution.height = windowHeight;
    bgfx_init.resolution.reset = BGFX_RESET_VSYNC;
    PlatformData platformData;
    platformData.nwh = ME_GetMEWindowHandle(window);
    bgfx_init.platformData = platformData;

    auto stat = init(bgfx_init);
    cout << "bgfx init state: " << stat << endl;

    if (!stat) {
        cout << "ERROR: bgfx failed to initialize!" << endl;
        ME_DestroyWindow(window);
        return -1;
    }

    // Load multiple images
    vector<string> imagePaths = {
        "../tests/Ice_Block_(placed).png",
        "../tests/Cobalt_Brick_(placed).png",
        // "../tests/third_image.png",
    };

    vector<ImageData> images = loadImages(imagePaths);

    if (images.empty() || !isValid(images[0].texture)) {
        cout << "ERROR: No valid images loaded!" << endl;
        shutdown();
        ME_DestroyWindow(window);
        return -1;
    }

    cout << "Loaded " << images.size() << " images successfully!" << endl;

    // Create vertex buffer for a fullscreen quad
    struct PosTexCoord {
        float x, y, z;
        float u, v;
    };

    static PosTexCoord quadVertices[] = {
        {-1.0f,  1.0f, 0.0f, 0.0f, 0.0f},
        { 1.0f,  1.0f, 0.0f, 1.0f, 0.0f},
        {-1.0f, -1.0f, 0.0f, 0.0f, 1.0f},
        { 1.0f, -1.0f, 0.0f, 1.0f, 1.0f}
    };

    VertexLayout layout;
    layout.begin()
        .add(Attrib::Position, 3, AttribType::Float)
        .add(Attrib::TexCoord0, 2, AttribType::Float)
        .end();

    VertexBufferHandle vbh = createVertexBuffer(makeRef(quadVertices, sizeof(quadVertices)), layout);

    static const uint16_t quadIndices[] = {0, 1, 2, 1, 3, 2};
    IndexBufferHandle ibh = createIndexBuffer(makeRef(quadIndices, sizeof(quadIndices)));

    // Create uniforms
    UniformHandle s_tex = createUniform("s_tex", UniformType::Sampler);
    UniformHandle u_resolution = createUniform("u_resolution", UniformType::Vec4);

    // Load shaders - try multiple paths
    ShaderHandle vsh = BGFX_INVALID_HANDLE;
    ShaderHandle fsh = BGFX_INVALID_HANDLE;

    // Determine renderer type for shader path
    auto renderer = getRendererType();
    const char* shaderDir = nullptr;

    switch (renderer) {
        case RendererType::Direct3D11:
        case RendererType::Direct3D12:
            shaderDir = "dx11";
            break;
        case RendererType::OpenGL:
            shaderDir = "glsl";
            break;
        case RendererType::Vulkan:
            shaderDir = "spirv";
            break;
        default:
            shaderDir = "dx11";
            break;
    }

    cout << "Renderer: " << getRendererName(renderer) << " using shader dir: " << shaderDir << endl;

    // Try loading compiled shaders
    string vsPath = string("../tests/shaders/") + shaderDir + "/vs_fullscreen.bin";
    string fsPath = string("../tests/shaders/") + shaderDir + "/fs_tiled.bin";

    vsh = loadShader(vsPath.c_str());
    fsh = loadShader(fsPath.c_str());

    ProgramHandle program = BGFX_INVALID_HANDLE;

    if (isValid(vsh) && isValid(fsh)) {
        program = createProgram(vsh, fsh, true);
        cout << "Shaders loaded successfully!" << endl;
        cout << "Tileset will scale with window size - try resizing!" << endl;
    } else {
        cout << "ERROR: Failed to load shaders from " << vsPath << " and " << fsPath << endl;
        cout << "Please compile shaders first using shaderc" << endl;
    }

    setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x443355FF, 1.0f, 0);

    uint32_t frameCount = 0;
    uint32_t lastWidth = windowWidth;
    uint32_t lastHeight = windowHeight;
    bool needsResize = false;
    uint32_t currentImageIndex = 0;

    cout << "Rendering " << images.size() << " images in grid layout" << endl;
    cout << "Press number keys to switch between images (if multiple are loaded)" << endl;

    while (true) {
        if (ME_ProcessEvents(window) == ME_QUIT_MESSAGE) {
            cout << "Quit message received after " << frameCount << " frames" << endl;
            break;
        }

        // Check if window size changed by polling bgfx stats
        const Stats* stats = getStats();
        uint32_t currentWidth = stats->width;
        uint32_t currentHeight = stats->height;

        if (currentWidth != lastWidth || currentHeight != lastHeight) {
            lastWidth = currentWidth;
            lastHeight = currentHeight;
            needsResize = true;
            cout << "Window resized to: " << lastWidth << "x" << lastHeight << endl;

            // Update bgfx resolution
            reset(lastWidth, lastHeight, BGFX_RESET_VSYNC);

            // Update view rectangle immediately
            setViewRect(0, 0, 0, uint16_t(lastWidth), uint16_t(lastHeight));
        }

        // Always set view rect to ensure proper rendering
        setViewRect(0, 0, 0, uint16_t(lastWidth), uint16_t(lastHeight));

        // Render all images in a grid layout
        if (isValid(program)) {
            uint32_t numImages = images.size();
            uint32_t cols, rows;

            // Determine grid layout based on number of images
            if (numImages == 1) {
                cols = 1;
                rows = 1;
            } else if (numImages == 2) {
                cols = 2;
                rows = 1;
            } else if (numImages <= 4) {
                cols = 2;
                rows = 2;
            } else if (numImages <= 6) {
                cols = 3;
                rows = 2;
            } else {
                cols = 3;
                rows = (numImages + cols - 1) / cols;
            }

            for (uint32_t i = 0; i < numImages; ++i) {
                if (!isValid(images[i].texture)) continue;

                uint32_t col = i % cols;
                uint32_t row = i / cols;

                // Calculate position in grid
                float cellWidth = float(lastWidth) / cols;
                float cellHeight = float(lastHeight) / rows;
                uint16_t viewX = uint16_t(col * cellWidth);
                uint16_t viewY = uint16_t(row * cellHeight);
                uint16_t viewWidth = uint16_t(cellWidth);
                uint16_t viewHeight = uint16_t(cellHeight);

                // Use a unique view ID for each image
                uint8_t viewId = i;

                // Set view rectangle for this specific cell
                setViewRect(viewId, viewX, viewY, viewWidth, viewHeight);

                // Set the view clear (only clear on first image)
                if (i == 0) {
                    setViewClear(viewId, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x443355FF, 1.0f, 0);
                }

                // Set resolution uniform with cell size AND image size
                float resolution[4] = { cellWidth, cellHeight, images[i].width, images[i].height };
                setUniform(u_resolution, resolution);

                setVertexBuffer(0, vbh);
                setIndexBuffer(ibh);
                setTexture(0, s_tex, images[i].texture);
                setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
                submit(viewId, program);
            }
        }

        touch(0);
        frame();
        frameCount++;

        if (needsResize) {
            cout << "Rerendered frame " << frameCount << " at new resolution " << lastWidth << "x" << lastHeight << endl;
            needsResize = false;
        }

        if (frameCount % 100 == 0) {
            cout << "Frame " << frameCount << " rendered at " << lastWidth << "x" << lastHeight << endl;
        }
    }

    // Clean up
    if (isValid(program)) destroy(program);
    if (isValid(u_resolution)) destroy(u_resolution);
    if (isValid(s_tex)) destroy(s_tex);
    if (isValid(vbh)) destroy(vbh);
    if (isValid(ibh)) destroy(ibh);

    for (auto& img : images) {
        if (isValid(img.texture)) destroy(img.texture);
        if (img.data) stbi_image_free(img.data);
    }

    shutdown();
    ME_DestroyWindow(window);

    return 0;
}

#endif
