#ifndef _imglib_convolve_h_
#define _imglib_convolve_h_

#include "imglib_export.h"
#include "Eigen/Core"

namespace imglib
{

enum class Direction
{
    X,
    Y
};

///
/// Convolves two matrices
///
template <typename Derived1_, typename Derived2_>
Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> convolveMatrix(const Eigen::MatrixBase<Derived1_>& matrix1, const Eigen::MatrixBase<Derived2_> &matrix2)
{
    size_t cols=matrix1.cols()+matrix2.cols()-1;
    size_t rows=matrix1.rows()+matrix2.rows()-1;

    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> output=Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>::Zero(rows, cols);

    size_t matrix1Row=0;
    size_t matrix2Row=matrix2.rows()-1;
    size_t rowSize=1;

    for(size_t y=0; y<rows; ++y)
    {
        size_t matrix1Col=0;
        size_t matrix2Col=matrix2.cols()-1;
        size_t colSize=1;

        for(size_t x=0; x<cols; ++x)
        {
            Derived1_::Scalar b=(matrix1.block(matrix1Row, matrix1Col, rowSize, colSize).cwiseProduct(matrix2.block(matrix2Row, matrix2Col, rowSize, colSize))).sum();
            output.coeffRef(y, x)=b;

            if((matrix1Col<=matrix1.cols()-1)&&(matrix2Col>=0))
            {
                if((colSize<matrix1.cols())&&(colSize<matrix2.cols()))
                    colSize++;
                matrix2Col--;
            }
            else
            {
                matrix1Col++;
                if(colSize > matrix1.cols()-matrix1Col)
                    colSize--;
            }
        }

        if((matrix1Row<=matrix1.rows()-1)&&(matrix2Row>0))
        {
            if((rowSize<matrix1.rows())&&(rowSize<matrix2.rows()))
                rowSize++;
            matrix2Row--;
        }
        else
        {
            matrix1Row++;
            if(rowSize > matrix1.rows()-matrix1Row)
                rowSize--;
        }
    }

    return output;
}

struct IMGLIB_EXPORT Kernel
{
    uint8_t *buffer;
    size_t width;
    size_t height;
};

struct IMGLIB_EXPORT SeparableKernel
{
    uint8_t *kenel1;
    uint8_t *kenel2;
    size_t size;
};

}//namespace imglib

#include "convolve.inl"

#endif //_imglib_convolve_h_