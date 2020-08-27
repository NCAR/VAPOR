#ifndef ftle_helper_h
#define ftle_helper_h
#include <limits>
#include <vector>

#include "GridMetaData.h"

namespace detail
{

class Vec3
{
public:
  Vec3() = default;

  Vec3(double x, double y, double z)
  {
    this->vector[0] = x;
    this->vector[1] = y;
    this->vector[2] = z;
  }

  Vec3(const Vec3& vec)
  {
    this->vector[0] = vec[0];
    this->vector[1] = vec[1];
    this->vector[2] = vec[2];
  }

  double operator[](const size_t component) const
  {
    return vector[component];
  }

  void operator=(const Vec3& vec)
  {
    this->vector[0] = vec[0];
    this->vector[1] = vec[1];
    this->vector[2] = vec[2];
  }

  Vec3 operator*(const Vec3& vec) const
  {
    return Vec3(this->vector[0]*vec[0], this->vector[1]*vec[1], this->vector[2]*vec[2]);
  }

  Vec3 operator+(const Vec3& vec) const
  {
    return Vec3(this->vector[0]+vec[0], this->vector[1]+vec[1], this->vector[2]+vec[2]);
  }

private:
  double vector[3];
};

std::ostream& operator<<(std::ostream &strm, const Vec3& vec) {
    return strm << "{" << vec[0] << "," << vec[1] << ", " << vec[2] << "}";
}

inline Vec3 GetVec3(const std::vector<flow::Particle>& data,
                    const long long int index)
{
  return Vec3{data.at(index).location.x, data.at(index).location.y, data.at(index).location.z};
}

class FTLECalculator
{
public:
  FTLECalculator() = default;

  virtual void CalculateCauchyGreenTensor(Vec3* jacobian) = 0;
  virtual Vec3 CalculateJacobi(Vec3* jacobian) = 0;
};

template<int dimensions>
class DimensionHelper;

template<>
class DimensionHelper<2> : public FTLECalculator
{
public:
  DimensionHelper() = default;

  void CalculateCauchyGreenTensor(Vec3* jacobian) override
  {
    Vec3 j1 = jacobian[0];
    Vec3 j2 = jacobian[1];

    // Left Cauchy Green Tensor is J*J^T
    // j1[0] j1[1] | j1[0] j2[0]
    // j2[0] j2[1] | j1[1] j2[1]
    double a = j1[0] * j1[0] + j1[1] * j1[1];
    double b = j1[0] * j2[0] + j1[1] * j2[1];

    double d = j2[0] * j2[0] + j2[1] * j2[1];

    jacobian[0] =  Vec3{a, b, 1};
    jacobian[1] =  Vec3{b, d, 1};
  }

  Vec3 CalculateJacobi(Vec3* jacobian) override
  {
    Vec3 j1 = jacobian[0];
    Vec3 j2 = jacobian[1];

    // Assume a symetric matrix
    // a b
    // b c
    double a = j1[0];
    double b = j1[1];
    double c = j2[1];

    double trace = (a + c) / 2.0f;
    double det = a * c - b * b;
    double sqrtr = std::sqrt(trace * trace - det);

    // Arrange eigen values from largest to smallest.
    double w0 = trace + sqrtr;
    double w1 = trace - sqrtr;
    return Vec3{w0, w1, -(std::numeric_limits<double>::infinity())};
  }
};

template<>
class DimensionHelper<3> : public FTLECalculator
{
public:
  DimensionHelper() = default;

  void CalculateCauchyGreenTensor(Vec3* jacobian) override
  {
    Vec3 j1 = jacobian[0];
    Vec3 j2 = jacobian[1];
    Vec3 j3 = jacobian[2];

    // Left Cauchy Green Tensor is J*J^T
    // j1[0]  j1[1] j1[2] |  j1[0]  j2[0]  j3[0]
    // j2[0]  j2[1] j2[2] |  j1[1]  j2[1]  j3[1]
    // j3[0]  j3[1] j3[2] |  j1[2]  j2[2]  j3[2]
    double a = j1[0] * j1[0] + j1[1] * j1[1] + j1[2] * j1[2];
    double b = j1[0] * j2[0] + j1[1] * j2[1] + j1[2] * j2[2];
    double c = j1[0] * j3[0] + j1[1] * j3[1] + j1[2] * j3[2];

    double d = j2[0] * j2[0] + j2[1] * j2[1] + j2[2] * j2[2];
    double e = j2[0] * j3[0] + j2[1] * j3[1] + j2[2] * j3[2];

    double f = j3[0] * j3[0] + j3[1] * j3[1] + j3[2] * j3[2];

    jacobian[0] =  Vec3{a, b, c};
    jacobian[1] =  Vec3{b, d, e};
    jacobian[2] =  Vec3{d, e, f};
  }

  Vec3 CalculateJacobi(Vec3* jacobian) override
  {
    Vec3 j1 = jacobian[0];
    Vec3 j2 = jacobian[1];
    Vec3 j3 = jacobian[2];
    // Assume a symetric matrix
    // a b c
    // b d e
    // c e f
    double a = j1[0];
    double b = j1[1];
    double c = j1[2];
    double d = j2[1];
    double e = j2[2];
    double f = j3[2];

    double x = (a + d + f) / 3.0f; // trace

    a -= x;
    d -= x;
    f -= x;

    // Det / 2;
    double q = (a * d * f + b * e * c + c * b * e - c * d * c - e * e * a - f * b * b) / 2.0f;
    double r = (a * a + b * b + c * c + b * b + d * d + e * e + c * c + e * e + f * f) / 6.0f;

    double D = (r * r * r - q * q);
    double phi = 0.0f;
    const double PI = std::atan(1.0)*4;

    if (D < DBL_EPSILON)
      phi = 0.0f;
    else
    {
      phi = std::atan(std::sqrt(D) / q) / 3.0f;
      if (phi < 0)
        phi += PI;
    }

    const double sqrt3 = std::sqrt(3.0f);
    const double sqrtr = std::sqrt(r);

    double sinphi = 0.0f, cosphi = 0.0f;
    sinphi = std::sin(phi);
    cosphi = std::cos(phi);

    double w0 = x + 2.0f * sqrtr * cosphi;
    double w1 = x - sqrtr * (cosphi - sqrt3 * sinphi);
    double w2 = x - sqrtr * (cosphi + sqrt3 * sinphi);

    // Arrange eigen values from largest to smallest.
    if (w1 > w0)
      std::swap(w0, w1);
    if (w2 > w0)
      std::swap(w0, w2);
    if (w2 > w1)
      std::swap(w1, w2);

    return Vec3{w0, w1, w2};
  }

};

FTLECalculator* GetFTLECalculator(const GridMetaData& metaData)
{
  if(metaData.IsTwoDimentional())
    return new DimensionHelper<2>();
  else
    return new DimensionHelper<3>();
}

void CalculateFTLE(const std::vector<flow::Particle>& startPositions,
                   const std::vector<flow::Particle>& endPositions,
                   const GridMetaData& metaData,
                   const double duration,
                   std::vector<double>& output)
{
  std::cout << "Calculating FTLE" << std::endl;
  const long long int numPoints = metaData.GetNumberOfPoints();
  long long int index;
  const double dur_by2_reci = 1.0f / (2.0*duration);

  FTLECalculator* expCalculator = GetFTLECalculator(metaData);

  #pragma omp parallel
  #pragma omp for
  for(index =0; index < numPoints; index++)
  {
    long long int neighbors[6];
    metaData.GetNeighborIndices(index, neighbors);

    // Gradient w.r.t X, Y, and Z.
    Vec3 xin1, xin2;
    xin1 = GetVec3(startPositions, neighbors[0]);
    xin2 = GetVec3(startPositions, neighbors[1]);
    Vec3 yin1, yin2;
    yin1 = GetVec3(startPositions, neighbors[2]);
    yin2 = GetVec3(startPositions, neighbors[3]);

    Vec3 xout1, xout2;
    xout1 = GetVec3(endPositions, neighbors[0]);
    xout2 = GetVec3(endPositions, neighbors[1]);
    Vec3 yout1, yout2;
    yout1 = GetVec3(endPositions, neighbors[2]);
    yout2 = GetVec3(endPositions, neighbors[3]);

    // 1. Calculate Gradiant
    double xDiff, yDiff;
    xDiff = 1.0 / (xin2[0] - xin1[0]);
    yDiff = 1.0 / (yin2[1] - yin1[1]);

    double zDiff = 0.;
    Vec3 zin1, zin2;
    Vec3 zout1, zout2;
    if(!metaData.IsTwoDimentional())
    {
      zin1 = GetVec3(startPositions, neighbors[4]);
      zin2 = GetVec3(startPositions, neighbors[5]);

      zout1 = GetVec3(endPositions, neighbors[4]);
      zout2 = GetVec3(endPositions, neighbors[5]);

      zDiff = 1.0 / (zin2[2] - zin1[2]);
    }

    // Total X gradient w.r.t X, Y, Z
    double f1x = (xout2[0] - xout1[0]) * xDiff;
    double f1y = (yout2[0] - yout1[0]) * yDiff;
    double f1z = (zout2[0] - zout1[0]) * zDiff;

    // Total Y gradient w.r.t X, Y, Z
    double f2x = (xout2[1] - xout1[1]) * xDiff;
    double f2y = (yout2[1] - yout1[1]) * yDiff;
    double f2z = (zout2[1] - zout1[1]) * zDiff;

    // Total Z gradient w.r.t X, Y, Z
    double f3x = (xout2[2] - xout1[2]) * xDiff;
    double f3y = (yout2[2] - yout1[2]) * yDiff;
    double f3z = (zout2[2] - zout1[2]) * zDiff;

    // 2. Form  the jacobian
    Vec3 jacobian[3];
    jacobian[0] = Vec3{f1x, f1y, f1z};
    jacobian[1] = Vec3{f2x, f2y, f2z};
    jacobian[2] = Vec3{f3x, f3y, f3z};

    // 3. Calculate Caunchy Green Tensor
    expCalculator->CalculateCauchyGreenTensor(jacobian);

    // 4. Calculate eigenvalues for the Cauchy Green Tensor
    Vec3 eigenValues = expCalculator->CalculateJacobi(jacobian);

    // 5. Allow rich set of exponents calculation
    double delta = eigenValues[0];
    // Return value for ftle computation
    double outputField = std::log(delta) * dur_by2_reci;

    output.push_back(outputField);
  }
}

} // namespace detail
#endif //ftle_helper_h
