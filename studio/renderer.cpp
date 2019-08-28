#include "renderer.hpp"

/*
** Renderer
*/
Renderer::Renderer() :
	m_linkedCameraState(nullptr),
	m_musicTime(0.0f),
	m_sequenceTime(0.0f),
	m_mediaTime(0.0f),
	m_bassLvl(0.0f),
	m_bassLvlTime(0.0f),
	m_bassLvlSequenceTime(0.0f),
	m_noiseLvl(0.0f),
	m_noiseLvlTime(0.0f),
	m_noiseLvlSequenceTime(0.0f)
{
}
