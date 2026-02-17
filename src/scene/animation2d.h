#pragma once
#include <cstdint>
#include <vector>

namespace argon {
	
	struct AnimationClip2D {
		std::vector<std::uint32_t> frames; // sprite sequence
		float fps = 10.0f;
		bool loop = true;
	};

	struct Animator2D {
		const AnimationClip2D* clip = nullptr;
		float time = 0.0f; // summed time
		bool playing = true;

		void play(const AnimationClip2D* c, bool restart = true) {
			clip = c;
			playing = true;
			if (restart) time = 0.0f;
		}

		void stop() { playing = false; }
	
		std::uint32_t update(float dt) {
			if (!playing || !clip || clip->frames.empty() || clip->fps <= 0.0f) return 0;
			time += dt;
		
			const float frameDur = 1.0f / clip->fps;
			int idx = (int)(time / frameDur);

			if (clip->loop) {
				idx %= (int)clip->frames.size();
			} else {
				if (idx >= (int)clip->frames.size()) idx = (int)clip->frames.size() - 1;
			}
			return clip->frames[(size_t)idx];
		}

	};

}