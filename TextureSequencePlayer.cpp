#include "TextureSequencePlayer.h"

#define M_PI 3.14159265358979323846

hiveVG::CTextureSequencePlayer::CTextureSequencePlayer(const std::string& vTexturePath, int vSequenceRows, int vSequenceCols) 
							: m_TexturePath(vTexturePath),m_SequenceRows(vSequenceRows),m_SequenceCols(vSequenceCols)	
{
	m_ValidFrames = m_SequenceRows * m_SequenceCols;


}
