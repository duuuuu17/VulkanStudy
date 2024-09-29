#pragma once

#include "lve_device.hpp"
#include "lve_game_object.hpp"
#include "lve_window.hpp"
#include "lve_renderer.hpp"
#include "./RenderSysytems/Simple_Render_System.hpp"
#include "lve_descriptors.hpp"

// std
#include <memory>
#include <vector>

namespace lve {
	class FirstApp {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		FirstApp();
		~FirstApp();

		FirstApp(const FirstApp&) = delete;
		FirstApp& operator=(const FirstApp&) = delete;

		void run();

	private:
		void loadGameObjects();

		LveWindow lveWindow{ WIDTH, HEIGHT, "Vulkan Tutorial" };
		LveDevice lveDevice{ lveWindow };
		LveRenderer lveRenderer{ lveWindow, lveDevice };
		std::unique_ptr<LveDescriptorPool> gobalDescriptorPool{};
		LveGameObject::Map gameObjects;
	};
}  // namespace lve