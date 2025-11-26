package com.potato.Utils;

import com.potato.Variable.KeyCode;
import com.potato.NativeUtils.WindowRect;
import org.junit.jupiter.api.Test;

import java.time.Duration;
import java.time.Instant;

class EngineTest {
    @Test
    void test() {
        EngineBuilder engineBuilder = new EngineBuilder();
        engineBuilder.configFile("./test/engine_config.toml");
        Engine engine = engineBuilder.build();
        engine.registerEventProcessor((context, caller) -> {
            if (context.getCurrentContext() == GameContext.CommonContext.ENGINE_START.name()) {
                engine.keyboardManager.poll();
                if (engine.engineHelper.hasKeyDown()) {
                    Instant start = Instant.now();
                    caller.setTitle(engine.engineHelper.getCurrPressedKeys().get(0).name());
                    if (engine.engineHelper.getFirstPressedKey() == KeyCode.MINUS) {
                        WindowRect rect = caller.getWindowSize();
                        caller.setWindowSize(rect.getWidth() - 10, rect.getHeight() - 10);
                    } else if (engine.engineHelper.getFirstPressedKey() == KeyCode.EQUALS) {
                        WindowRect rect = caller.getWindowSize();
                        caller.setWindowSize(rect.getWidth() + 10, rect.getHeight() + 10);
                    }
                    Instant end = Instant.now();
                    System.out.println("Time taken: " + Duration.between(start, end).toNanos() + " ns");
                }
            }
        });
        engine.start(0, 0, 0, 800, 600, "Test Window");

        engine.shutdown();
    }
}