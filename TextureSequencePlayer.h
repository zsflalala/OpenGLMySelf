#pragma once

#include <glm/glm.hpp>
#include <string>

namespace hiveVG 
{
	class CTextureSequencePlayer
	{
	public:
		CTextureSequencePlayer(const std::string& vTexturePath, int vSequenceRows, int vSequenceCols);
		~CTextureSequencePlayer() = default;

		void setLoopPlayback(bool vLoopTag);
		void setScreenScale(const glm::vec2& vScreenScale);
		void setScreenOffset(const glm::vec2& vScreenOffset);
		void setFramePerSecond(float vFramePerSecond);
		void setValidFrames(int vValidFrames);

		void draw();

	private:
		int               m_SequenceRows;
		int               m_SequenceCols;
		int               m_ValidFrames;
		float             m_FramePerSecond = 60.0f;
		bool              m_IsLoop = true;
		const std::string m_TexturePath;
		glm::vec2         m_ScreenScale  = glm::vec2(1.0f, 1.0f);
		glm::vec2         m_ScreenOffset = glm::vec2(0.0f, 0.0f);

		unsigned int      m_TextureHandle;
		unsigned int      m_ShaderProgram;
	};
}




