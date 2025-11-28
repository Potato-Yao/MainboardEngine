package com.potato.NativeUtils;

import com.sun.jna.platform.win32.WinDef;

public class CursorPoint {
    private int x;
    private int y;

    public CursorPoint() {
        this.x = 0;
        this.y = 0;
    }

    public CursorPoint(int x, int y) {
        this.x = x;
        this.y = y;
    }

    public static CursorPoint fromWinPoint(WinDef.POINT point) {
        return new CursorPoint(point.x, point.y);
    }

    public static boolean isSamePosition(CursorPoint pointA, CursorPoint pointB) {
        return pointA.x == pointB.x && pointA.y == pointB.y;
    }

    public void setCursor(int x, int y) {
        this.x = x;
        this.y = y;
    }

    public void setX(int x) {
        this.x = x;
    }

    public void setY(int y) {
        this.y = y;
    }

    public int getX() {
        return x;
    }

    public int getY() {
        return y;
    }
}
