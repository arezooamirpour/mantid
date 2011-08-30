#ifndef MANTID_KERNEL_VMD_H_
#define MANTID_KERNEL_VMD_H_
    
#include "MantidKernel/System.h"
#include "MantidKernel/Tolerance.h"
#include "MantidKernel/V3D.h"
#include <cstddef>
#include <stdexcept>
#include <sstream>


namespace Mantid
{
namespace Kernel
{

  /** Simple vector class for multiple dimensions (i.e. > 3).
    
    @author Janik Zikovsky
    @date 2011-08-30

    Copyright &copy; 2011 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

    This file is part of Mantid.

    Mantid is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    Mantid is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    File change history is stored at: <https://svn.mantidproject.org/mantid/trunk/Code/Mantid>
    Code Documentation is available at: <http://doxygen.mantidproject.org>
  */
  class DLLExport VMD 
  {
  public:
    //-------------------------------------------------------------------------------------------
    /** Constructor
     * @param nd :: number of dimensions  */
    VMD(size_t nd)
    : nd(nd)
    {
      if (nd <= 0) throw std::invalid_argument("nd must be > 0");
      data = new double[nd];
      for (size_t d=0; d<nd; d++) data[d] = 0.0;
    }

    //-------------------------------------------------------------------------------------------
    /** 2D Constructor
     * @param valX :: value at this dimension*/
    VMD(double val0, double val1)
    : nd(2)
    {
      data = new double[nd];
      data[0] = val0;
      data[1] = val1;
    }

    //-------------------------------------------------------------------------------------------
    /** 3D Constructor
     * @param valX :: value at this dimension*/
    VMD(double val0, double val1, double val2)
    : nd(3)
    {
      data = new double[nd];
      data[0] = val0;
      data[1] = val1;
      data[2] = val2;
    }

    //-------------------------------------------------------------------------------------------
    /** 4D Constructor
     * @param valX :: value at this dimension*/
    VMD(double val0, double val1, double val2, double val3)
    : nd(4)
    {
      data = new double[nd];
      data[0] = val0;
      data[1] = val1;
      data[2] = val2;
      data[3] = val3;
    }

    //-------------------------------------------------------------------------------------------
    /** Copy constructor
     * @param other :: other to copy */
    VMD(const VMD & other)
    : nd(other.nd)
    {
      if (nd <= 0) throw std::invalid_argument("nd must be > 0");
      data = new double[nd];
      for (size_t d=0; d<nd; d++) data[d] = other.data[d];
    }

    //-------------------------------------------------------------------------------------------
    /** Constructor
     * @param nd :: number of dimensions
     * @param bareData :: pointer to a nd-sized bare data array */
    VMD(size_t nd, const double * bareData)
    : nd(nd)
    {
      if (nd <= 0) throw std::invalid_argument("nd must be > 0");
      data = new double[nd];
      for (size_t d=0; d<nd; d++) data[d] = bareData[d];
    }

    //-------------------------------------------------------------------------------------------
    /** Constructor
     * @param vector :: V3D */
    VMD(const V3D & vector)
    : nd(3)
    {
      data = new double[nd];
      for (size_t d=0; d<nd; d++) data[d] = vector[d];
    }

    //-------------------------------------------------------------------------------------------
    /** Constructor
     * @param vector :: vector of doubles */
    VMD(std::vector<double> vector)
    : nd(vector.size())
    {
      if (nd <= 0) throw std::invalid_argument("nd must be > 0");
      data = new double[nd];
      for (size_t d=0; d<nd; d++) data[d] = vector[d];
    }

    //-------------------------------------------------------------------------------------------
    /** Constructor
     * @param vector :: vector of floats */
    VMD(std::vector<float> vector)
    : nd(vector.size())
    {
      if (nd <= 0) throw std::invalid_argument("nd must be > 0");
      data = new double[nd];
      for (size_t d=0; d<nd; d++) data[d] = double(vector[d]);
    }

    //-------------------------------------------------------------------------------------------
    /// Destructor
    virtual ~VMD()
    {
      delete [] data;
    }
    
    //-------------------------------------------------------------------------------------------
    /// @return the number of dimensions
    size_t getNumDims() const
    { return nd; }

    /** @return the value at the index */
    const double& operator[](const size_t index) const
    { return data[index]; }

    /** @return the value at the index */
    double& operator[](const size_t index)
    { return data[index]; }

    //-------------------------------------------------------------------------------------------
    /** Return a simple string representation of the vector */
    std::string toString() const
    {
      std::ostringstream mess;
      for (size_t d=0; d<nd; d++)
        mess << (d>0?" ":"")
        << data[d];
      return mess.str();
    }



    //-------------------------------------------------------------------------------------------
    /** Equals operator with tolerance factor
      @param v :: VMD for comparison
      @return true if the items are equal
     */
    bool operator==(const VMD& v) const
    {
      using namespace std;
      if (v.nd != nd) return false;
      for (size_t d=0; d<nd; d++)
        if ((fabs(data[d]-v.data[d]) > Tolerance))
          return false;
      return true;
    }

    //-------------------------------------------------------------------------------------------
    /** Not-equals operator with tolerance factor
      @param v :: VMD for comparison
      @return true if the items are equal
     */
    bool operator!=(const VMD& v) const
    {
      return !operator==(v);
    }


    //-------------------------------------------------------------------------------------------
    /** Add two vectors together
     * @param v :: other vector, must match number of dimensions  */
    VMD operator+(const VMD& v) const
    {
      VMD out(*this);
      out += v;
      return out;
    }

    //-------------------------------------------------------------------------------------------
    /** Add two vectors together
     * @param v :: other vector, must match number of dimensions  */
    VMD & operator+=(const VMD& v)
    {
      if (v.nd != this->nd) throw std::runtime_error("Mismatch in number of dimensions in operation between two VMD vectors.");
      for (size_t d=0; d<nd; d++) data[d] += v.data[d];
      return *this;
    }

    //-------------------------------------------------------------------------------------------
    /** Subtract two vectors
     * @param v
     *  :: other vector, must match number of dimensions  */
    VMD operator-(const VMD& v) const
    {
      VMD out(*this);
      out -= v;
      return out;
    }

    //-------------------------------------------------------------------------------------------
    /** Subtract two vectors
     * @param v :: other vector, must match number of dimensions  */
    VMD & operator-=(const VMD& v)
    {
      if (v.nd != this->nd) throw std::runtime_error("Mismatch in number of dimensions in operation between two VMD vectors.");
      for (size_t d=0; d<nd; d++) data[d] -= v.data[d];
      return *this;
    }

    //-------------------------------------------------------------------------------------------
    /** Inner product of two vectors (element-by-element)
     * @param v :: other vector, must match number of dimensions  */
    VMD operator*(const VMD& v) const
    {
      VMD out(*this);
      out *= v;
      return out;
    }

    //-------------------------------------------------------------------------------------------
    /** Inner product of two vectors (element-by-element)
     * @param v :: other vector, must match number of dimensions  */
    VMD & operator*=(const VMD& v)
    {
      if (v.nd != this->nd) throw std::runtime_error("Mismatch in number of dimensions in operation between two VMD vectors.");
      for (size_t d=0; d<nd; d++) data[d] *= v.data[d];
      return *this;
    }

    //-------------------------------------------------------------------------------------------
    /** Inner division of two vectors (element-by-element)
     * @param v :: other vector, must match number of dimensions  */
    VMD operator/(const VMD& v) const
    {
      VMD out(*this);
      out /= v;
      return out;
    }

    //-------------------------------------------------------------------------------------------
    /** Inner division of two vectors (element-by-element)
     * @param v :: other vector, must match number of dimensions  */
    VMD & operator/=(const VMD& v)
    {
      if (v.nd != this->nd) throw std::runtime_error("Mismatch in number of dimensions in operation between two VMD vectors.");
      for (size_t d=0; d<nd; d++) data[d] /= v.data[d];
      return *this;
    }

    //-------------------------------------------------------------------------------------------
    /** Multiply by a scalar
     * @param scalar :: double scalar to multiply each element  */
    VMD operator*(const double scalar) const
    {
      VMD out(*this);
      out *= scalar;
      return out;
    }

    //-------------------------------------------------------------------------------------------
    /** Multiply by a scalar
     * @param scalar :: double scalar to multiply each element  */
    VMD & operator*=(const double scalar)
    {
      for (size_t d=0; d<nd; d++) data[d] *= scalar;
      return *this;
    }

    //-------------------------------------------------------------------------------------------
    /** Divide by a scalar
     * @param scalar :: double scalar to Divide each element  */
    VMD operator/(const double scalar) const
    {
      VMD out(*this);
      out /= scalar;
      return out;
    }

    //-------------------------------------------------------------------------------------------
    /** Divide by a scalar
     * @param scalar :: double scalar to Divide each element  */
    VMD & operator/=(const double scalar)
    {
      for (size_t d=0; d<nd; d++) data[d] /= scalar;
      return *this;
    }

    //-------------------------------------------------------------------------------------------
    /** Scalar product of two vectors
     * @param v :: other vector, must match number of dimensions  */
    double scalar_prod(const VMD& v) const
    {
      double out=0;
      if (v.nd != this->nd) throw std::runtime_error("Mismatch in number of dimensions in operation between two VMD vectors.");
      for (size_t d=0; d<nd; d++)
        out += (data[d] * v.data[d]);
      return out;
    }

    //-------------------------------------------------------------------------------------------
    /** @return the length of this vector */
    double length() const
    {
      return sqrt(this->scalar_prod(*this));
    }

    //-------------------------------------------------------------------------------------------
    /** Normalize this vector to unity length
     * @return the length of this vector BEFORE normalizing */
    double normalize()
    {
      double length = this->length();
      for (size_t d=0; d<nd; d++)
        data[d] /= length;
      return length;
    }

  protected:
    /// Number of dimensions
    size_t nd;
    /// Data, an array of size nd
    double * data;
  };



  // Overload operator <<
  MANTID_KERNEL_DLL std::ostream& operator<<(std::ostream&, const VMD&);

} // namespace Kernel
} // namespace Mantid

#endif  /* MANTID_KERNEL_VMD_H_ */
