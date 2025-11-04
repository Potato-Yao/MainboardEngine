package com.potato.Utils;

import com.potato.Variable.KeyCode;

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

    public KeyCode getFirstPressedKey() {
        ArrayList<KeyCode> pressedKeys = engine.keyboardManager.getCurrentPressedKeys();
        if (!pressedKeys.isEmpty()) {
            return pressedKeys.get(0);
        }
        return null;
    }
}
