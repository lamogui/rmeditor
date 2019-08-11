#include "renderer.hpp"

/*
** Renderer
*/

Renderer::Renderer()
{
  setTimeVariables(0, 0, 0);
  setBassVariables(0, 0, 0);
  setNoiseVariables(0, 0, 0);
}

void Renderer::setTimeVariables(float music, float sequence, float media)
{
  musicTime = music;
  sequenceTime = sequence;
  mediaTime = media;
}

void Renderer::setBassVariables(float bass, float bassTime, float bassSequenceTime)
{
  bassLvl = bass;
  bassLvlTime = bassTime;
  bassLvlSequenceTime = bassSequenceTime;
}

void Renderer::setNoiseVariables(float noise, float noiseTime, float noiseSequenceTime)
{
  noiseLvl = noise;
  noiseLvlTime = noiseTime;
  noiseLvlSequenceTime = noiseSequenceTime;
}
