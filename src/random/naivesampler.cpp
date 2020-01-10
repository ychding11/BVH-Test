#include "naivesampler.h"

namespace mei
{

    NaiveSampler::NaiveSampler(int ns, int seed) : Sampler(ns), rng(seed) {}

    Float NaiveSampler::Get1D()
    {
        CHECK_LT(currentPixelSampleIndex, samplesPerPixel);
        return rng.UniformFloat();
    }

    Point2f NaiveSampler::Get2D()
    {
        CHECK_LT(currentPixelSampleIndex, samplesPerPixel);
        return { rng.UniformFloat(), rng.UniformFloat() };
    }

    std::unique_ptr<Sampler> NaiveSampler::Clone(int seed)
    {
        NaiveSampler *rs = new NaiveSampler(*this);
        rs->rng.SetSequence(seed);
        return std::unique_ptr<Sampler>(rs);
    }

    //< It may take time
    void NaiveSampler::StartPixel(const Point2i &p)
    {
        for (size_t i = 0; i < sampleArray1D.size(); ++i)
            for (size_t j = 0; j < sampleArray1D[i].size(); ++j)
                sampleArray1D[i][j] = rng.UniformFloat();

        for (size_t i = 0; i < sampleArray2D.size(); ++i)
            for (size_t j = 0; j < sampleArray2D[i].size(); ++j)
                sampleArray2D[i][j] = { rng.UniformFloat(), rng.UniformFloat() };
        Sampler::StartPixel(p);
    }

    Sampler *CreateNaiveSampler(int n)
    {
        int ns = n;
        return new NaiveSampler(ns);
    }
} //<namespace

