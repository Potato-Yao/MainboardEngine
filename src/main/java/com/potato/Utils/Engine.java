package com.potato.Utils;

import com.potato.Config;

import java.util.ArrayList;

public class Engine {
    private NativeCaller caller;
    private ArrayList eventProcessers = new ArrayList<EventProcesser>();
    private boolean hasStarted = false;
    public KeyboardManager keyboardManager = KeyboardManager.getManager();
    public CursorManager cursorManager = CursorManager.getManager();

    public Engine() {
        this.caller = new NativeCaller();
    }

    public void start(int isFullScreen, int x, int y, int width, int height, String title) {
        Config.init();
        caller.initializeEngine();
        caller.createWindow(isFullScreen, x, y, width, height, title);
        Config.gameContext.adjustContext("ENGINE_START");

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
