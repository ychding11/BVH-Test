#include "samplers.h"

#include "camera/camera.h"

namespace mei
{
	// Sampler Method Definitions
	Sampler::~Sampler() {}

	Sampler::Sampler(int64_t samplesPerPixel) : samplesPerPixel(samplesPerPixel) {}

	//< pRaster = {(0,0),(0,1),(1,0),(1,1)}
	CameraSample Sampler::GetCameraSample(const Point2i &pRaster)
	{
		CameraSample cs;
		cs.pFilm = (Point2f)pRaster + Get2D();
		cs.time = Get1D();
		cs.pLens = Get2D();
		return cs;
	}

	void Sampler::StartPixel(const Point2i &p)
	{
		currentPixel = p;
		currentPixelSampleIndex = 0;

		//< Reset array offsets for next pixel sample
		array1DOffset = array2DOffset = 0;
	}

	bool Sampler::StartNextSample()
	{
		// Reset array offsets for next pixel sample
		array1DOffset = array2DOffset = 0;
		return ++currentPixelSampleIndex < samplesPerPixel;
	}

	bool Sampler::SetSampleNumber(int64_t sampleNum)
	{
		// Reset array offsets for next pixel sample
		array1DOffset = array2DOffset = 0;
		currentPixelSampleIndex = sampleNum;
		return currentPixelSampleIndex < samplesPerPixel;
	}

	void Sampler::Request1DArray(int n)
	{
		CHECK_EQ(RoundCount(n), n);
		samples1DArraySizes.push_back(n);
		sampleArray1D.push_back(std::vector<Float>(n * samplesPerPixel));
	}

	void Sampler::Request2DArray(int n)
	{
		CHECK_EQ(RoundCount(n), n);
		samples2DArraySizes.push_back(n);
		sampleArray2D.push_back(std::vector<Point2f>(n * samplesPerPixel));
	}

	const Float *Sampler::Get1DArray(int n)
	{
		if (array1DOffset == sampleArray1D.size()) return nullptr;
		CHECK_EQ(samples1DArraySizes[array1DOffset], n);
		CHECK_LT(currentPixelSampleIndex, samplesPerPixel);
		return &sampleArray1D[array1DOffset++][currentPixelSampleIndex * n];
	}

	const Point2f *Sampler::Get2DArray(int n)
	{
		if (array2DOffset == sampleArray2D.size()) return nullptr;
		CHECK_EQ(samples2DArraySizes[array2DOffset], n);
		CHECK_LT(currentPixelSampleIndex, samplesPerPixel);
		return &sampleArray2D[array2DOffset++][currentPixelSampleIndex * n];
	}




	PixelSampler::PixelSampler(int64_t samplesPerPixel, int nSampledDimensions)
		: Sampler(samplesPerPixel)
	{
		for (int i = 0; i < nSampledDimensions; ++i)
		{
			samples1D.push_back(std::vector<Float>(samplesPerPixel));
			samples2D.push_back(std::vector<Point2f>(samplesPerPixel));
		}
	}

	bool PixelSampler::StartNextSample()
	{
		current1DDimension = current2DDimension = 0;
		return Sampler::StartNextSample();
	}

	bool PixelSampler::SetSampleNumber(int64_t sampleNum)
	{
		current1DDimension = current2DDimension = 0;
		return Sampler::SetSampleNumber(sampleNum);
	}

	Float PixelSampler::Get1D()
	{
		CHECK_LT(currentPixelSampleIndex, samplesPerPixel);
		if (current1DDimension < samples1D.size())
			return samples1D[current1DDimension++][currentPixelSampleIndex];
		else
			return rng.UniformFloat();
	}

	Point2f PixelSampler::Get2D()
	{
		CHECK_LT(currentPixelSampleIndex, samplesPerPixel);
		if (current2DDimension < samples2D.size())
			return samples2D[current2DDimension++][currentPixelSampleIndex];
		else
			return Point2f(rng.UniformFloat(), rng.UniformFloat());
	}


}
