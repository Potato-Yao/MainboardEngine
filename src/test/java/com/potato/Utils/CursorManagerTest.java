package com.potato.Utils;

import org.junit.jupiter.api.Disabled;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

class CursorManagerTest {
    @Test
//    @Disabled
    void test() {
        CursorManager cursorManager = new CursorManager();
        while (true) {
            cursorManager.poll();
        }
    }
}