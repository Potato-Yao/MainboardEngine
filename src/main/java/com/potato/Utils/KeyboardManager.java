package com.potato.Utils;

import com.sun.jna.platform.win32.User32;

public class KeyboardManager {
    private final int STATE_ARRAY_LENGTH = 256;
    private final double DOUBLE_PRESS_GAP = 300;  // in millisecond
    private static KeyboardManager keyboardManager;
//    private final static KeyAction[] keyActions = new KeyAction[4];

    private final boolean[] currState = new boolean[STATE_ARRAY_LENGTH];
    private final boolean[] prevState = new boolean[STATE_ARRAY_LENGTH];

//    public interface KeyAction {
//        void action(int keyCode);
//    }

    public void poll() {
        System.arraycopy(currState, 0, prevState, 0, STATE_ARRAY_LENGTH);
        for (int i = 0; i < STATE_ARRAY_LENGTH; ++i) {
            currState[i] = isKeyDownRaw(i);
        }
    }

    private boolean isKeyDownRaw(int keyCode) {
        // 0x8000 is 1000000000000000 in binary, to isolate only the high-order bit
        return (User32.INSTANCE.GetAsyncKeyState(keyCode) & 0x8000) != 0;
    }

//    public enum KeyActionType {
//        DOWN,
//        PRESSED,
//        RELEASED,
//        MULTIPLE_PRESSED,
//    }

    public static KeyboardManager getKeyboardManager() {
        if (keyboardManager == null) {
            keyboardManager = new KeyboardManager();
        }
        return keyboardManager;
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

//    public static void registerKeyAction(KeyActionType type, KeyAction action) {
//        keyActions[type.ordinal()] = action;
//    }
//
//    public void keyDown(int keyCode) {
//        keyActions[KeyActionType.DOWN.ordinal()].action(keyCode);
//    }
//
//    // TODO implement multiple press
//    public void keyPressed(int keyCode) {
//        keyActions[KeyActionType.PRESSED.ordinal()].action(keyCode);
//    }
//
//    public void keyReleased(int keyCode) {
//        keyActions[KeyActionType.RELEASED.ordinal()].action(keyCode);
//
//    }
}
