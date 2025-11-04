package com.potato;

import com.sun.jna.Structure;

import java.util.List;

public class WindowRect extends Structure {
    public int top, bottom, left, right;

    public static class ByValue extends WindowRect implements Structure.ByValue {
    }

    public static class ByReference extends WindowRect implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return List.of("top", "bottom", "left", "right");
    }

    public int getWidth() {
        return right - left;
    }


    public int getHeight() {
        return bottom - top;
    }
}
