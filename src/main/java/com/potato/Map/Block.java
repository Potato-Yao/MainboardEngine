package com.potato.Map;

public class Block {
    private int id;
    private int x;
    private int y;

    public Block(int id, int x, int y) {
        this.id = id;
        this.x = x;
        this.y = y;
    }

    public int getId() {
        return id;
    }

    public int getX() {
        return x;
    }

    public int getY() {
        return y;
    }
}
