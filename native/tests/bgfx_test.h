#ifdef me_bgfx_test

#include <iostream>
#include <string>
#include <fstream>
#include <bgfx/bgfx.h>
#include <mainboard_engine.h>
#include <event_message_type.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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
    file.read((char*)mem->data, size);
    mem->data[size] = '\0';
    file.close();

    return bgfx::createShader(mem);
}

int execute() {
    using namespace bgfx;
    using namespace std;

    ME_Initialize();

    uint32_t windowWidth = 800;
    uint32_t windowHeight = 600;

    auto window = ME_CreateWindow(0, 100, 100, windowWidth, windowHeight, "BGFX Tileset Renderer - Resize to see tiles adjust!");
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

    // Load the PNG image using stb_image
    int imgWidth, imgHeight, imgChannels;
    unsigned char *imageData = nullptr;

    auto path = "../tests/Ice_Block_(placed).png";
    imageData = stbi_load(path, &imgWidth, &imgHeight, &imgChannels, 4);

    if (!imageData) {
        cout << "ERROR: Failed to load image!" << endl;
        shutdown();
        ME_DestroyWindow(window);
        return -1;
    }

    cout << "Image loaded: " << imgWidth << "x" << imgHeight << " channels: " << imgChannels << endl;

    // Create texture with point filtering for sharp pixel art
    // Default wrapping mode is REPEAT (texture will tile automatically)
    TextureHandle texture = createTexture2D(imgWidth, imgHeight, false, 1, TextureFormat::RGBA8,
                                            BGFX_TEXTURE_NONE | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT,
                                            copy(imageData, imgWidth * imgHeight * 4));

    // Store texture dimensions for shader
    float textureWidth = float(imgWidth);
    float textureHeight = float(imgHeight);

    stbi_image_free(imageData);

    if (!isValid(texture)) {
        cout << "ERROR: Failed to create texture!" << endl;
        shutdown();
        ME_DestroyWindow(window);
        return -1;
    }

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

        // If we have a valid program, render with shaders
        if (isValid(program)) {
            // Set resolution uniform with window size AND texture size
            // x = window width, y = window height, z = texture width, w = texture height
            float resolution[4] = { float(lastWidth), float(lastHeight), textureWidth, textureHeight };
            setUniform(u_resolution, resolution);

            setVertexBuffer(0, vbh);
            setIndexBuffer(ibh);
            setTexture(0, s_tex, texture);
            setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
            submit(0, program);
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
    if (isValid(texture)) destroy(texture);

    shutdown();
    ME_DestroyWindow(window);

    return 0;
}

#endif
