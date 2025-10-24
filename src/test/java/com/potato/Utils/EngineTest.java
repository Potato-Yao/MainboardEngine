package com.potato.Utils;

import org.junit.jupiter.api.Test;

import java.time.Duration;
import java.time.Instant;

class EngineTest {
    @Test
    void test() {
        Engine engine = new Engine();
        engine.registerEventProcessor((context, caller) -> {
            if (context.getCurrentContext() == GameContext.CommonContext.ENGINE_START.name()) {
                engine.keyboardManager.poll();
                if (engine.keyboardManager.isKeyPressed(KeyCode.A)) {
                    Instant start = Instant.now();
                    caller.setTitle("A pressed");
                    Instant end = Instant.now();
                    System.out.println("Time taken: " + Duration.between(start, end).toNanos() + " ns");
                }
            }
        });
        engine.registerEventProcessor((context, caller) -> {
            if (context.getCurrentContext() == GameContext.CommonContext.ENGINE_START.name()) {
                engine.keyboardManager.poll();
                if (engine.keyboardManager.isKeyPressed(KeyCode.B)) {
                    Instant start = Instant.now();
                    caller.setTitle("B pressed");
                    Instant end = Instant.now();
                    System.out.println("Time taken: " + Duration.between(start, end).toNanos() + " ns");
                }
            }
        });
        engine.start(0, 0, 0, 800, 600, "Test Window");

        engine.shutdown();
    }
}