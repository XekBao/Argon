#include "renderer/imgui_pass2d.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

namespace argon {
	void ImGuiPass2D::execute(const RenderFrame2D& frame, Renderer& renderer) {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Argon");
		ImGui::Text("Hello from ImGuiPass!");
		const auto& s = renderer.stats();
		ImGui::Text("drawCalls: %u", s.drawCalls);
		ImGui::Text("batchFlushes: %u", s.batchFlushes);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}


}