#pragma once

namespace argon {

	class Window;
	class InputMap;
	class CameraController2D;
	class MaterialLibrary;

	struct FrameContext {
		Window& window;
		InputMap& input;
		CameraController2D& camCtl;
		MaterialLibrary& materials;
	};

}