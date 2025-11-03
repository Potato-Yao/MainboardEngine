#ifndef MAINBOARD_ENGINE_ENGINE_H
#define MAINBOARD_ENGINE_ENGINE_H

namespace MainBoardEngine {
    class Engine {
    public:
        virtual ~Engine() = default;

        virtual bool Initialize() = 0;

        virtual void Shutdown() = 0;

    };
}



#endif //MAINBOARD_ENGINE_ENGINE_H