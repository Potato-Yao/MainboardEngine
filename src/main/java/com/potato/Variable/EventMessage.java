package com.potato.Variable;

public enum EventMessage {
    NO_EVENT(0),
    QUIT(1),
    CANNOT_GET_EVENT(-1);

    private final int code;

    EventMessage(int code) {
        this.code = code;
    }

    public int getCode() {
        return code;
    }
}
