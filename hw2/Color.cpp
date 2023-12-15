#include <iomanip>
#include "Color.h"

Color::Color() {
    this->r = 0;
    this->g = 0;
    this->b = 0;
}

Color::Color(double r, double g, double b)
{
    this->r = r;
    this->g = g;
    this->b = b;
}

Color::Color(const Color &other)
{
    this->r = other.r;
    this->g = other.g;
    this->b = other.b;
}

Color Color::operator+(const Color &other)
{
    return Color(this->r + other.r, this->g + other.g, this->b + other.b);
}

Color Color::operator-(const Color &other)
{
    return Color(this->r - other.r, this->g - other.g, this->b - other.b);
}

Color Color::operator*(double scalar)
{
    return Color(this->r * scalar, this->g * scalar, this->b * scalar);
}

Color Color::operator/(double scalar)
{
    return Color(this->r / scalar, this->g / scalar, this->b / scalar);
}
double clamp(double value, double min, double max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

Color Color::round()
{
    Color newColor;
    newColor.r = (int)(this->r + 0.5);
    newColor.g = (int)(this->g + 0.5);
    newColor.b = (int)(this->b + 0.5);
    newColor.r = clamp(newColor.r, 0, 255);
    newColor.g = clamp(newColor.g, 0, 255);
    newColor.b = clamp(newColor.b, 0, 255);
    return newColor;
}

std::ostream &operator<<(std::ostream &os, const Color &c)
{
    os << std::fixed << std::setprecision(0) << "rgb(" << c.r << ", " << c.g << ", " << c.b << ")";
    return os;
}
