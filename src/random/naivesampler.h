#ifndef NAIVE_SAMPLERS_H_ 
#define NAIVE_SAMPLERS_H_ 

#include "2d.h"
#include "3d.h"
#include "stringprint.h"
#include "randoms.h"
#include "samplers.h"

namespace mei
{

    class NaiveSampler : public Sampler
    {
    public:

        NaiveSampler(int ns, int seed = 0);
        virtual void StartPixel(const Point2i &) override;
        virtual Float Get1D() override;
        virtual Point2f Get2D() override;

        std::unique_ptr<Sampler> Clone(int seed);

    private:
        RNG rng;
    };

    Sampler *CreateNaiveSampler(int n = 4);
}// namespace
#endif
