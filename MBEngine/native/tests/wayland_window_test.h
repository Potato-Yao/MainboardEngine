#define me_wayland_window_test
#ifdef me_wayland_window_test

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"

// --- Globals ---
static wl_compositor *compositor = nullptr;
static wl_shm *shm = nullptr;
static xdg_wm_base *xdg_wm_base = nullptr;
static bool running = true;

// --- Callbacks ---

// Registry listener
static void registry_global_handler(void *data, wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        compositor = static_cast<struct wl_compositor *>(wl_registry_bind(registry, name, &wl_compositor_interface, 4));
    } else if (strcmp(interface, wl_shm_interface.name) == 0) {
        shm = static_cast<struct wl_shm *>(wl_registry_bind(registry, name, &wl_shm_interface, 1));
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        xdg_wm_base = static_cast<struct xdg_wm_base *>(wl_registry_bind(registry, name, &xdg_wm_base_interface, 1));
    }
}

static void registry_global_remove_handler(void *data, wl_registry *registry, uint32_t name) {}

static const wl_registry_listener registry_listener = {
    .global = registry_global_handler,
    .global_remove = registry_global_remove_handler
};

// XDG WM Base listener
static void xdg_wm_base_ping_handler(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping_handler,
};

// XDG Surface listener
static void xdg_surface_configure_handler(void *data, xdg_surface *xdg_surface, uint32_t serial) {
    xdg_surface_ack_configure(xdg_surface, serial);
}

static const xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure_handler,
};

// XDG Toplevel listener
static void xdg_toplevel_configure_handler(void *data, xdg_toplevel *xdg_toplevel, int32_t width, int32_t height, struct wl_array *states) {
    if (width > 0 && height > 0) {
        printf("Window configured: %dx%d\n", width, height);
    }
}

static void xdg_toplevel_close_handler(void *data, xdg_toplevel *xdg_toplevel) {
    printf("Window close requested\n");
    running = false;
}

static const xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_configure_handler,
    .close = xdg_toplevel_close_handler,
};

// --- Shared Memory Helper ---
static int create_anonymous_file(off_t size) {
    char path[] = "/tmp/wayland-XXXXXX";
    int fd = mkstemp(path);
    if (fd < 0) return -1;

    if (ftruncate(fd, size) < 0) {
        close(fd);
        return -1;
    }

    unlink(path);
    return fd;
}

// --- Main ---
int execute() {
    printf("=== Native Wayland Window Test ===\n");

    wl_display *display = wl_display_connect(NULL);
    if (!display) {
        fprintf(stderr, "ERROR: Failed to connect to Wayland display\n");
        return 1;
    }

    wl_registry *registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, nullptr);
    wl_display_roundtrip(display);

    if (!compositor || !shm || !xdg_wm_base) {
        fprintf(stderr, "ERROR: Required Wayland globals not available\n");
        if (!compositor) fprintf(stderr, "  - Missing: wl_compositor\n");
        if (!shm) fprintf(stderr, "  - Missing: wl_shm\n");
        if (!xdg_wm_base) fprintf(stderr, "  - Missing: xdg_wm_base\n");
        wl_display_disconnect(display);
        return 1;
    }

    xdg_wm_base_add_listener(xdg_wm_base, &xdg_wm_base_listener, nullptr);

    wl_surface *surface = wl_compositor_create_surface(compositor);
    if (!surface) {
        fprintf(stderr, "ERROR: Failed to create surface\n");
        wl_display_disconnect(display);
        return 1;
    }

    xdg_surface *xdg_surface = xdg_wm_base_get_xdg_surface(xdg_wm_base, surface);
    if (!xdg_surface) {
        fprintf(stderr, "ERROR: Failed to create xdg_surface\n");
        wl_surface_destroy(surface);
        wl_display_disconnect(display);
        return 1;
    }
    xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, NULL);
    printf("✓ Created XDG surface\n");

    xdg_toplevel *xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);
    if (!xdg_toplevel) {
        fprintf(stderr, "ERROR: Failed to create xdg_toplevel\n");
        xdg_surface_destroy(xdg_surface);
        wl_surface_destroy(surface);
        wl_display_disconnect(display);
        return 1;
    }
    xdg_toplevel_add_listener(xdg_toplevel, &xdg_toplevel_listener, NULL);
    xdg_toplevel_set_title(xdg_toplevel, "Mainboard Engine - Wayland Window");
    xdg_toplevel_set_app_id(xdg_toplevel, "mainboard.engine");
    printf("✓ Created XDG toplevel (window)\n");

    // Initial commit to trigger configure
    wl_surface_commit(surface);

    int width = 800, height = 600;
    int stride = width * 4;
    int size = stride * height;

    int fd = create_anonymous_file(size);
    if (fd < 0) {
        fprintf(stderr, "ERROR: Failed to create shared memory file\n");
        xdg_toplevel_destroy(xdg_toplevel);
        xdg_surface_destroy(xdg_surface);
        wl_surface_destroy(surface);
        wl_display_disconnect(display);
        return 1;
    }

    void *shm_data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm_data == MAP_FAILED) {
        fprintf(stderr, "ERROR: Failed to mmap shared memory\n");
        close(fd);
        xdg_toplevel_destroy(xdg_toplevel);
        xdg_surface_destroy(xdg_surface);
        wl_surface_destroy(surface);
        wl_display_disconnect(display);
        return 1;
    }

    wl_shm_pool *pool = wl_shm_create_pool(shm, fd, size);
    wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_XRGB8888);
    wl_shm_pool_destroy(pool);
    close(fd);

    // Draw a nice gradient
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint32_t *pixel = (uint32_t *)shm_data + y * width + x;
            uint8_t r = (x * 255) / width;
            uint8_t g = (y * 255) / height;
            uint8_t b = 128;
            *pixel = (r << 16) | (g << 8) | b;
        }
    }

    printf("✓ Created buffer (%dx%d)\n", width, height);

    wl_surface_attach(surface, buffer, 0, 0);
    wl_surface_commit(surface);

    printf("\n=== Window Created Successfully! ===\n");
    printf("Close the window to exit...\n\n");

    while (running && wl_display_dispatch(display) != -1) {
        // Event loop
    }

    // Cleanup
    printf("\nCleaning up...\n");
    munmap(shm_data, size);
    wl_buffer_destroy(buffer);
    xdg_toplevel_destroy(xdg_toplevel);
    xdg_surface_destroy(xdg_surface);
    wl_surface_destroy(surface);
    xdg_wm_base_destroy(xdg_wm_base);
    wl_compositor_destroy(compositor);
    wl_shm_destroy(shm);
    wl_registry_destroy(registry);
    wl_display_disconnect(display);

    printf("✓ Done!\n");
    return 0;
}

#endif
