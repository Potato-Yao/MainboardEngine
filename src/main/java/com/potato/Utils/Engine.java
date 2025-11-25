package com.potato.Utils;

import com.potato.Config;
import com.potato.Map.MapManager;
import com.potato.NativeUtils.NativeCaller;

import java.io.File;
import java.util.ArrayList;

public class Engine {
    private NativeCaller caller;
    private ArrayList eventProcessers = new ArrayList<EventProcesser>();
    private boolean hasStarted = false;
    public final EngineHelper engineHelper = new EngineHelper(this);
    public final KeyboardManager keyboardManager = KeyboardManager.getManager();
    public final CursorManager cursorManager = CursorManager.getManager();
    public final MapManager mapManager = MapManager.getMapManager();

    public Engine() {
        this.caller = new NativeCaller();
    }

    public void start(int isFullScreen, int x, int y, int width, int height, String title) {
        Config.init();
        caller.initializeEngine();
        caller.createWindow(isFullScreen, x, y, width, height, title);
        Config.gameContext.adjustContext("ENGINE_START");

        mapManager.registerMap(Config.DEFAULT_MAP_ID, new File("./test/map/main_map.toml"));
        mapManager.loadMap(Config.DEFAULT_MAP_ID, caller);
        registerEventProcessor(((_, caller) -> {
            mapManager.renderMap(caller);
        }));
        caller.processEvents(eventProcessers);

        hasStarted = true;
    }

    public void shutdown() {
        caller.destroyWindow();
    }

    public void registerEventProcessor(EventProcesser eventProcesser) {
        if (hasStarted) {
            throw new RuntimeException("Cannot register event processor after engine has started.");
        }
        eventProcessers.add(eventProcesser);
    }
}
