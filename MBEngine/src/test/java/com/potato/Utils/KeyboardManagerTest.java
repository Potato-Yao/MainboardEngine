package com.potato.Utils;

import com.potato.Variable.KeyCode;
import org.junit.jupiter.api.Test;

class KeyboardManagerTest {
    @Test
//    @Disabled
    void test() {
        var manager = KeyboardManager.getManager();
        while (true) {
            manager.poll();

            if (manager.isKeyPressed(KeyCode.A)) {
                System.out.println("hihihi");
            } else if (manager.isKeyPressed(KeyCode.MOUSE_LEFT)) {
                System.out.println("llll");
            }
        }
    }
}