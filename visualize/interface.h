#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// UI Control
struct Setting
{
    int testIndex;
    int objectNum;
    int samples;
    glm::ivec2 screenSize;
    glm::fvec2 focusOffset;
    float positionOffset;

    Setting()
        : testIndex(0)
        , objectNum(10)
		, samples(4)
        , screenSize(720, 720)
        , focusOffset(0.f, 0.f)
        , positionOffset(0.f)
    {}
};

class Observer
{
public:

	virtual void handleScreenSizeChange(const glm::ivec2 &newScreenSize)
    {
        assert(0 && "This function should NOT be called before override !!!");
    }
	virtual void handleFocusOffsetChange(const glm::fvec2 &newFocusOffset)
    {
        assert(0 && "This function should NOT be called before override !!!");
    }
	virtual void handlePositionOffsetChange(float newPositionOffset)
    {
        assert(0 && "This function should NOT be called before override !!!");
    }
	virtual void handleObjectNumChange(int newObjectNum)
    {
        assert(0 && "This function should NOT be called before override !!!");
    }
	virtual void handleTestIndexChange(int newTestIndex)
    {
        assert(0 && "This function should NOT be called before override !!!");
    }
	virtual void handleSampleCountChange(int newTestIndex)
    {
        assert(0 && "This function should NOT be called before override !!!");
    }
	virtual void handleNewRenderResult(void* newData, int newSize)
    {
        assert(0 && "This function should NOT be called before override !!!");
    }
};

