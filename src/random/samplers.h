#ifndef SAMPLERS_H_ 
#define SAMPLERS_H_ 

#include "2d.h"
#include "3d.h"
#include "stringprint.h"
#include "randoms.h"

namespace mei
{
    struct CameraSample;

	// Sampler Declarations
	class Sampler
	{
	public:
		// Sampler Interface
		virtual ~Sampler();

		Sampler(int64_t samplesPerPixel);

		virtual void StartPixel(const Point2i &p);

		virtual Float Get1D() = 0;
		virtual Point2f Get2D() = 0;
		CameraSample GetCameraSample(const Point2i &pRaster);
        
		void Request1DArray(int n);
		void Request2DArray(int n);

		virtual int RoundCount(int n) const { return n; }

		const Float   *Get1DArray(int n);
		const Point2f *Get2DArray(int n);

		virtual bool StartNextSample();

		virtual std::unique_ptr<Sampler> Clone(int seed) = 0;

		virtual bool SetSampleNumber(int64_t sampleNum);

		std::string StateString() const
		{
			return StringPrintf("(%d,%d), sample %" PRId64, currentPixel.x, currentPixel.y, currentPixelSampleIndex);
		}

		int64_t CurrentSampleNumber() const { return currentPixelSampleIndex; }

		const int64_t samplesPerPixel;

	protected:
		Point2i currentPixel;
		int64_t currentPixelSampleIndex;
		std::vector<int> samples1DArraySizes, samples2DArraySizes;
		std::vector<std::vector<Float>> sampleArray1D;
		std::vector<std::vector<Point2f>> sampleArray2D;

	private:
		// Sampler Private Data
		size_t array1DOffset, array2DOffset;
	};

	class PixelSampler : public Sampler
	{
	public:
		// PixelSampler Public Methods
		PixelSampler(int64_t samplesPerPixel, int nSampledDimensions);
		bool StartNextSample();
		bool SetSampleNumber(int64_t);

		virtual Float   Get1D() override;
		virtual Point2f Get2D() override;

	protected:
		// PixelSampler Protected Data
		std::vector<std::vector<Float>> samples1D;
		std::vector<std::vector<Point2f>> samples2D;
		int current1DDimension = 0, current2DDimension = 0;
		RNG rng;
	};

	class GlobalSampler : public Sampler
	{
	public:
		bool StartNextSample();
		void StartPixel(const Point2i &);
		bool SetSampleNumber(int64_t sampleNum);

		virtual Float Get1D() override;
		virtual Point2f Get2D() override;

		GlobalSampler(int64_t samplesPerPixel) : Sampler(samplesPerPixel) {}

		virtual int64_t GetIndexForSample(int64_t sampleNum) const = 0;
		virtual Float   SampleDimension(int64_t index, int dimension) const = 0;

	private:
		int dimension;
		int64_t intervalSampleIndex;
		static const int arrayStartDim = 5;
		int arrayEndDim;
	};
}


#endif
