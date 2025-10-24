package com.potato.Utils;

import java.util.ArrayList;

public class GameContext {
    private static GameContext gameContext;
    private static ArrayList context = new ArrayList<String>();
    private static String currentContext = "NO_CONTEXT";

    public enum CommonContext {
        ENGINE_START,
        ENGINE_SHUTDOWN;
    }

    public static GameContext getGameContext() {
        if (gameContext == null) {
            gameContext = new GameContext();

            context.add("ENGINE_START");
            context.add("ENGINE_SHUTDOWN");
        }
        return gameContext;
    }

    public void registerContext(String key) {
        context.add(key);
    }

    public void adjustContext(String key) {
        if (context.contains(key)) {
            currentContext = key;
        } else {
            throw new RuntimeException("Context " + key + " not registered.");
        }
    }

    public String getCurrentContext() {
        return currentContext;
    }
}
