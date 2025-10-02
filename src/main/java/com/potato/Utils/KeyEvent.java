package com.potato.Utils;

import java.awt.*;

public class KeyEvent extends java.awt.event.KeyEvent {
    private int clickCount = 0;

    public KeyEvent(Component source, int id, long when, int modifiers, int keyCode, char keyChar, int keyLocation) {
        super(source, id, when, modifiers, keyCode, keyChar, keyLocation);
    }

    public KeyEvent(Component source, int id, long when, int modifiers, int keyCode, char keyChar) {
        super(source, id, when, modifiers, keyCode, keyChar);
    }

    public int getClickCount() {
        return this.clickCount;
    }
}
