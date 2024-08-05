#ifndef algebra_H
#define algebra_H
#include <cmath>
#include <array>
#include <stdint.h>
struct vec2
{
    float data[2];
    float& operator[](size_t index)
    {
        return this->data[index];
    }
};
struct vec3
{
    float data[3];
    float& operator[](size_t index)
    {
        return this->data[index];
    }
    void normalize()
    {
        float length = sqrt(data[0]*data[0]+data[1]*data[1]+data[2]*data[2]);
        data[0] /= length;
        data[1] /= length;
        data[2] /= length;
    }
};
struct vec4
{
    float data[4];
    float& operator[](size_t index)
    {
        return this->data[index];
    }
    void normalize()
    {
        float length = sqrt(data[0]*data[0]+data[1]*data[1]+data[2]*data[2]+data[3]*data[3]);
        data[0] /= length;
        data[1] /= length;
        data[2] /= length;
        data[3] /= length;
    }
};
struct mat3
{
    float data[9];
    float& operator[](size_t index)
    {
        return this->data[index];
    }
    void matrix_identity()
    {
        data[ 0] = 1.0;data[ 1] = 0.0;data[ 2] = 0.0;
        data[ 3] = 0.0;data[ 4] = 1.0;data[ 5] = 0.0;
        data[ 6] = 0.0;data[ 7] = 0.0;data[ 8] = 1.0;
    }
};
struct mat4
{
    float data[16];
    float& operator[](size_t index)
    {
        return this->data[index];
    }
    void operator=(mat4 m_1)
    {
        for(size_t i = 0; i < 16; i++)
            data[i] = m_1[i];
    }
    mat4 operator*(mat4 m_1)
    {
        mat4 m_rez;
        for(unsigned long y = 0; y < 4; y++)
        {
            for(unsigned long x = 0; x < 4; x++)
            {
                m_rez[4*y+x] =
                m_1[4*y+0]*data[4*0+x] +
                m_1[4*y+1]*data[4*1+x] +
                m_1[4*y+2]*data[4*2+x] +
                m_1[4*y+3]*data[4*3+x];
            }
        }
        return m_rez;
    }
    void matrix_identity()
    {
        data[ 0] = 1;data[ 1] = 0;data[ 2] = 0;data[ 3] = 0;
        data[ 4] = 0;data[ 5] = 1;data[ 6] = 0;data[ 7] = 0;
        data[ 8] = 0;data[ 9] = 0;data[10] = 1;data[11] = 0;
        data[12] = 0;data[13] = 0;data[14] = 0;data[15] = 1;
    }
    void matrix_scale(float x, float y, float z)
    {
        data[ 0] = x;data[ 1] = 0;data[ 2] = 0;data[ 3] = 0;
        data[ 4] = 0;data[ 5] = y;data[ 6] = 0;data[ 7] = 0;
        data[ 8] = 0;data[ 9] = 0;data[10] = z;data[11] = 0;
        data[12] = 0;data[13] = 0;data[14] = 0;data[15] = 1;
    }
    void matrix_translate(float x, float y, float z)
    {
        data[ 0] = 1;data[ 1] = 0;data[ 2] = 0;data[ 3] = 0;
        data[ 4] = 0;data[ 5] = 1;data[ 6] = 0;data[ 7] = 0;
        data[ 8] = 0;data[ 9] = 0;data[10] = 1;data[11] = 0;
        data[12] = x;data[13] = y;data[14] = z;data[15] = 1;
    }
    void matrix_rotate(float angle, char axis_of_rotation)
    {
        double Pi = 3.14159265359;
        angle = angle*Pi/180.0;
        if(axis_of_rotation == 'x')
        {
            data[ 0] = 1;data[ 1] = 0;         data[ 2] = 0;         data[ 3] = 0;
            data[ 4] = 0;data[ 5] = cos(angle);data[ 6] = sin(angle);data[ 7] = 0;
            data[ 8] = 0;data[ 9] =-sin(angle);data[10] = cos(angle);data[11] = 0;
            data[12] = 0;data[13] = 0;         data[14] = 0;         data[15] = 1;
        }
        else if(axis_of_rotation == 'y')
        {
            data[ 0] = cos(angle);data[ 1] = 0;data[ 2] =-sin(angle);data[ 3] = 0;
            data[ 4] = 0;         data[ 5] = 1;data[ 6] = 0;         data[ 7] = 0;
            data[ 8] = sin(angle);data[ 9] = 0;data[10] = cos(angle);data[11] = 0;
            data[12] = 0;         data[13] = 0;data[14] = 0;         data[15] = 1;
        }
        else if(axis_of_rotation == 'z')
        {
            data[ 0] = cos(angle);data[ 1] = sin(angle);data[ 2] = 0;data[ 3] = 0;
            data[ 4] =-sin(angle);data[ 5] = cos(angle);data[ 6] = 0;data[ 7] = 0;
            data[ 8] = 0;         data[ 9] = 0;         data[10] = 1;data[11] = 0;
            data[12] = 0;         data[13] = 0;         data[14] = 0;data[15] = 1;
        }
    }
    void matrix_perspective(float angle, float aspect, float n, float f)
    {
        double Pi = 3.14159265359;
        angle = angle*Pi/180.0;
        for(long i = 0; i < 15; i++)
            data[i] = 0;
        data[ 0] = 1.0/(tan(angle/2.0)*aspect);
        data[ 5] = 1.0/(tan(angle/2.0));
        data[10] = f/(f-n);data[11] = 1;
        data[14] = -(f*n)/(f-n);data[15] = 0;
    }
};
mat4 inverse_matrix(mat4 mat);
mat4 Invert2(mat4 mat);
mat4 matrix_multiplication(mat4 m_1, mat4 m_2);
mat4 gemm_v0(mat4 A, mat4 B);

void matrix_translate(float *matrix, float x, float y, float z);
mat4 matrix_translate(float x, float y, float z);
void matrix_scale(float *matrix, float x, float y, float z);
void matrix_rotate(float *matrix, float angle, char axis_of_rotation);
mat4 matrix_rotate(float angle, char axis_of_rotation);
void matrix_transpose(float *matrix);
struct Quaternion
{
	// real part
	float w = 1;
	// imaginary part
	float x = 0;
	float y = 0;
	float z = 0;
	Quaternion(){}
	Quaternion(float w, float x, float y, float z)
	{
	    this->w = w;
	    this->x = x;
	    this->y = y;
	    this->z = z;
	}
	float quaternion_magnitude()
	{
	    return float(sqrt(w*w + x*x + y*y + z*z));
	}
	Quaternion quaternion_conjugate()
	{
	    return Quaternion(w, -x, -y, -z);
	}
	Quaternion quaternion_normal()
	{
	    float data = sqrt(w*w + x*x + y*y + z*z);
	    return Quaternion(w/data, x/data, y/data, z/data);
	}
	Quaternion quaterion_inverse()
	{
	    Quaternion conjugate = {w, -x, -y, -z};
	    float squared_magnitude = w*w + x*x + y*y + z*z;
	    return Quaternion(conjugate.w/squared_magnitude, conjugate.x/squared_magnitude, conjugate.y/squared_magnitude, conjugate.z/squared_magnitude);
	}
};
mat4 MatrixFromQuaterion(Quaternion quat);
Quaternion QuaterionFromMatrix(mat4 matrix);
Quaternion QuaternionFromDirectionAndAngle(vec3 direction, float angle);
Quaternion product_of_quaternions(Quaternion quat_1, Quaternion quat_2);
#endif
