#include <iomanip>
#include "Matrix4.h"

Matrix4::Matrix4()
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            this->values[i][j] = 0;
        }
    }
}

Matrix4::Matrix4(double values[4][4])
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            this->values[i][j] = values[i][j];
        }
    }
}
Matrix4::Matrix4(const Matrix4 &other)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; i++)
        {
            this->values[i][j] = other.values[i][j];
        }
    }
}

std::ostream &operator<<(std::ostream &os, const Matrix4 &m)
{
        os << std::fixed << std::setprecision(6) << "|" << m.values[0][0] << "|" << m.values[0][1] << "|" << m.values[0][2] << "|" << m.values[0][3] << "|"
        << std::endl
        << "|" << m.values[1][0] << "|" << m.values[1][1] << "|" << m.values[1][2] << "|" << m.values[1][3] << "|"
        << std::endl
        << "|" << m.values[2][0] << "|" << m.values[2][1] << "|" << m.values[2][2] << "|" << m.values[2][3] << "|"
        << std::endl
        << "|" << m.values[3][0] << "|" << m.values[3][1] << "|" << m.values[3][2] << "|" << m.values[3][3] << "|";

    return os;
}

Matrix4 Matrix4::operator*(const Matrix4 &other)
{
    Matrix4 result;
    double total;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            total = 0;
            for (int k = 0; k < 4; k++)
            {
                total += this->values[i][k] * other.values[k][j];
            }

            result.values[i][j] = total;
        }
    }

    return result;
}
Matrix4 Matrix4::transpose()
{
    double transpose[4][4] = {{this->values[0][0], this->values[1][0], this->values[2][0], this->values[3][0]},
                              {this->values[0][1], this->values[1][1], this->values[2][1], this->values[3][1]},
                              {this->values[0][2], this->values[1][2], this->values[2][2], this->values[3][2]},
                              {this->values[0][3], this->values[1][3], this->values[2][3], this->values[3][3]}};
    return Matrix4(transpose);
}
Matrix4 Matrix4::operator*= (const Matrix4 &other)
{
    Matrix4 result;
    double total;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            total = 0;
            for (int k = 0; k < 4; k++)
            {
                total += this->values[i][k] * other.values[k][j];
            }

            result.values[i][j] = total;
        }
    }

    return result;
}