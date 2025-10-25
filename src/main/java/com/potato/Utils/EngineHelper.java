package com.potato.Utils;

import java.util.ArrayList;

public class EngineHelper {
    private Engine engine;

    public EngineHelper(Engine engine) {
        this.engine = engine;
    }

    public boolean hasKeyDown() {
        return !engine.keyboardManager.getCurrentPressedKeys().isEmpty();
    }

    public ArrayList<KeyCode> getCurrPressedKeys() {
        return engine.keyboardManager.getCurrentPressedKeys();
    }
}
