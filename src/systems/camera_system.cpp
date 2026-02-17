#include "camera_system.h"
#include "gfx/camera_controller2d.h"

namespace argon {
	void argon::CameraSystem::update(CameraController2D& camCtl, float dt) {
		camCtl.update(dt);
	}
}