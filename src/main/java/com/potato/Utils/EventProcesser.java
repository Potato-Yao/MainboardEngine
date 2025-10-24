package com.potato.Utils;

import com.potato.MainboardNativeLibrary;

public interface EventProcesser {
    void process(GameContext gameContext, NativeCaller caller);
}
