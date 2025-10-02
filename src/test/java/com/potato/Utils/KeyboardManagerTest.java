package com.potato.Utils;

import org.junit.jupiter.api.Disabled;
import org.junit.jupiter.api.Test;

class KeyboardManagerTest {
    @Test
//    @Disabled
    void test() {
        var manager = KeyboardManager.getKeyboardManager();
        while (true) {
            manager.poll();

            if (manager.isKeyPressed(KeyCode.A)) {
                System.out.println("hihihi");
            }
        }
    }
}