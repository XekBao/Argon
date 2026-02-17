#pragma once

namespace argon {

    class CameraController2D;

    class CameraSystem {
    public:
        void update(CameraController2D& camCtl, float dt);
    };

} // namespace argon