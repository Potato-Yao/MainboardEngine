package com.potato.Utils;

import com.potato.Config;
import com.potato.NativeUtils.CursorPoint;
import com.potato.Variable.OSType;
import com.sun.jna.platform.win32.User32;
import com.sun.jna.platform.win32.WinDef;

public class CursorManager {
    private static CursorManager innerManager;
    private static CursorPoint prevPos = new CursorPoint();
    private static CursorPoint currPos = new CursorPoint();

    public static CursorManager getManager() {
        if (innerManager == null) {
            innerManager = new CursorManager();
        }
        return innerManager;
    }

    public void poll() {
        prevPos = currPos;
        currPos = getCursorRaw();
    }

    public boolean hasMoved() {
        return !CursorPoint.isSamePosition(prevPos, currPos);
    }

    public CursorPoint getPosition() {
        return currPos;
    }

    private CursorPoint getCursorRaw() {
        CursorPoint point = null;
        if (Config.os == OSType.Windows) {
            WinDef.POINT winPoint = new WinDef.POINT();
            User32.INSTANCE.GetCursorPos(winPoint);
            point = CursorPoint.fromWinPoint(winPoint);
        }
        // TODO implement for other OS
        return point;
    }
}
