package com.potato.Utils;

public enum EventMessage {
    QUIT(1);

    private final int code;

    EventMessage(int code) {
        this.code = code;
    }
}
