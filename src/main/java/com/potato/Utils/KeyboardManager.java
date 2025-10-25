package com.potato.Utils;

import com.potato.Config;
import com.potato.OSType;
import com.sun.jna.platform.win32.User32;

import java.util.ArrayList;

public class KeyboardManager {
    private final int STATE_ARRAY_LENGTH = 256;
    private final double DOUBLE_PRESS_GAP = 300;  // in millisecond
    private static KeyboardManager innerManager;

    private final boolean[] currState = new boolean[STATE_ARRAY_LENGTH];
    private final boolean[] prevState = new boolean[STATE_ARRAY_LENGTH];

    public static KeyboardManager getManager() {
        if (innerManager == null) {
            innerManager = new KeyboardManager();
        }
        return innerManager;
    }

    public void poll() {
        System.arraycopy(currState, 0, prevState, 0, STATE_ARRAY_LENGTH);
        for (int i = 0; i < STATE_ARRAY_LENGTH; ++i) {
            boolean isPressed = isKeyDownRaw(i);
            currState[i] = isPressed;
        }
    }

    private boolean isKeyDownRaw(int virtualKeyCode) {
        if (Config.os == OSType.Windows) {
            // 0x8000 is 1000000000000000 in binary, to isolate only the high-order bit
            return (User32.INSTANCE.GetAsyncKeyState(virtualKeyCode) & 0x8000) != 0;
        } else {
            // TODO implement for other OS
            return false;
        }
    }


    public boolean isKeyDown(KeyCode keyCode) {
        return currState[keyCode.getVirtualKeyCode()];
    }

    public boolean isKeyPressed(KeyCode keyCode) {
        // TODO multiple pressed needed
        return currState[keyCode.getVirtualKeyCode()] && !prevState[keyCode.getVirtualKeyCode()];
    }

    public boolean isKeyReleased(KeyCode keyCode) {
        return !currState[keyCode.getVirtualKeyCode()] && prevState[keyCode.getVirtualKeyCode()];
    }

    public ArrayList<KeyCode> getCurrentPressedKeys() {
        ArrayList pressedKeys = new ArrayList<KeyCode>();
        for (int i = 0; i < STATE_ARRAY_LENGTH; ++i) {
            if (currState[i]) {
                pressedKeys.add(KeyCode.getKeyCodeByVirtualKeyCode(i));
            }
        }

        return pressedKeys;
    }
}
