// vector.h - векторы и их вычисление
// P.S. Коменты на нашем потому что в ранних ТЧ был именно русский
#pragma once

// Undef some macros
#ifdef M_PI
#undef M_PI
#endif

#ifdef PI
#undef PI
#endif

// Select platform
#ifdef	_MSC_VER
#define	M_VISUAL
#endif
#ifdef	__BORLANDC__
#define M_BORLAND
#endif

//////////////////////////////////////////////
//#VERTVER: Т.к. Visual C++ Compiler не умеет вычислять геометрические,
// в ПЫС тупо сделали эти все числа константными.
//////////////////////////////////////////////
// Константы
#ifdef M_VISUAL
//////////////////////////////////////////////
constexpr float		EPS_S		= 0.0000001f;			// одна десяти миллионная 
constexpr float		EPS			= 0.0000100f;			// одна десяти тысячная
constexpr float		EPS_L		= 0.0010000f;			// одна тысячная
//////////////////////////////////////////////
#undef M_SQRT1_2
//////////////////////////////////////////////
constexpr float		M_SQRT1_2	= 0.7071067811865475244008443621048f;	// Одна вторая корня из двух
//////////////////////////////////////////////
constexpr float		M_PI		= 3.1415926535897932384626433832795f;	// Дополнительное число Пи
constexpr float		PI			= 3.1415926535897932384626433832795f;	// Число Пи
constexpr float		PI_MUL_2	= 6.2831853071795864769252867665590f;	// Два числа Пи
constexpr float		PI_MUL_3	= 9.4247779607693797153879301498385f;	// Три числа Пи
constexpr float		PI_MUL_4	= 12.566370614359172953850573533118f;	// Четыре числа Пи
constexpr float		PI_MUL_6	= 18.849555921538759430775860299677f;	// Шесть числа Пи
constexpr float		PI_MUL_8	= 25.132741228718345907701147066236f;	// Восемь числа Пи
constexpr float		PI_DIV_2	= 1.5707963267948966192313216916398f;	// Одна вторая числа Пи
constexpr float		PI_DIV_3	= 1.0471975511965977461542144610932f;	// Одна третяя числа Пи
constexpr float		PI_DIV_4	= 0.7853981633974483096156608458199f;	// Одна четвёртая числа Пи
constexpr float		PI_DIV_6	= 0.5235987755982988730771072305466f;	// Одна шестая числа Пи
constexpr float		PI_DIV_8	= 0.3926990816987241548078304229099f;	// Одна восьмая числа Пи
//////////////////////////////////////////////
#endif

//////////////////////////////////////////////
// Дефайны типов и пространств имён (CPU & FPU)
#include "_types.h"
#include "_math.h"
#include "_bitwise.h"
#include "_std_extensions.h"
//////////////////////////////////////////////

// Сравниваемые
IC BOOL  fsimilar		( float		a, float	b, float	cmp=EPS )		{ return _abs(a-b)<cmp;	}
IC BOOL  dsimilar		( double	a, double	b, double	cmp=EPS )		{ return _abs(a-b)<cmp;		}

IC BOOL  fis_zero		( float		val, float	cmp=EPS_S )					{ return _abs(val)<cmp;	}
IC BOOL  dis_zero		( double	val, double	cmp=EPS_S )					{ return _abs(val)<cmp;		}

// Возводим две радианы в градусы и обратно
//////////////////////////////////////////////
template <class T>	ICF T	deg2rad		( T val ) 	noexcept			{ return (val*T(M_PI)/T(180));	};
template <class T>	ICF T	rad2deg		( T val )	noexcept			{ return (val*T(180)/T(M_PI));	};

//////////////////////////////////////////////
// Клэмпим
template <class T>
constexpr  void clamp	( T& val, const T& _low, const T& _high ){
	if( val<_low ) val = _low; else if( val>_high ) val = _high;
};
//////////////////////////////////////////////
template <class T>
constexpr  T	clampr	( const T& val, const T& _low, const T& _high ){
	if		( val<_low	)	return _low; 
	else if	( val>_high )	return _high;
	else					return val;
};
//////////////////////////////////////////////
IC float snapto	( float value, float snap )	{
	if( snap<=0.f ) return value;
	return float(iFloor((value+(snap*0.5f)) / snap )) * snap;
};
//////////////////////////////////////////////

//////////////////////////////////////////////
// Пре-определения
template <class T> struct _quaternion;
//////////////////////////////////////////////
#pragma pack(push)
#pragma pack(1)
//////////////////////////////////////////////
#include "_random.h"
//////////////////////////////////////////////
#include "_color.h"
#include "_vector3d.h"
#include "_vector3d_ext.h"
#include "_vector2.h"
#include "_vector4.h"
#include "_matrix.h"
#include "_matrix33.h"
#include "_quaternion.h"
#include "_rect.h"
#include "_fbox.h"
#include "_fbox2.h"
#include "_obb.h"
#include "_sphere.h"
#include "_cylinder.h"
#include "_random.h"
#include "_compressed_normal.h"
#include "_plane.h"
#include "_plane2.h"
#include "_flags.h"
//////////////////////////////////////////////
#pragma pack(pop)
//////////////////////////////////////////////

// Нормализация угла от 0 до 2Пи
ICF float		angle_normalize_always	(float a)
{
	float		div	 =	a/PI_MUL_2;
	int			rnd  =	(div>0)?iFloor(div):iCeil(div);
	float		frac =	div-rnd;
	if (frac<0)	frac +=	1.f;
	return		frac *	PI_MUL_2;
}

// Нормализация угла от 0 до 2Пи
ICF float		angle_normalize	(float a)
{
	if (a>=0 && a<=PI_MUL_2)	return	a;
	else						return	angle_normalize_always(a);
}

// от -Пи до +Пи
ICF float		angle_normalize_signed(float a)
{
	if (a>=(-PI) && a<=PI)		return		a;
	float angle = angle_normalize_always	(a);
	if (angle>PI) angle-=PI_MUL_2;
	return angle;
}

// от -Пи до Пи
ICF float		angle_difference_signed(float a, float b)
{
	float diff	= angle_normalize_signed(a) - angle_normalize_signed(b);
	if (diff>0) {
		if (diff>PI)
			diff	-= PI_MUL_2;
	} else {
		if (diff<-PI)	
			diff	+= PI_MUL_2;
	}
	return diff;
}

// от нуля до Пи
ICF float		angle_difference(float a, float b)
{
	return _abs	(angle_difference_signed(a,b));
}

IC bool			are_ordered		( float const value0, float const value1, float const value2 )
{
	if ( (value1 >= value0) && (value1 <= value2) )
		return	true;

	if ( (value1 <= value0) && (value1 >= value2) )
		return	true;

	return		false;
}

IC bool			is_between		( float const value, float const left, float const right )
{
	return		are_ordered( left, value, right );
}

// c - текущий, t - цель, s - скорость, dt - dt
IC bool			angle_lerp		(float& c, float t, float s, float dt)
{
	float const before = c;
	float diff	= t - c;
	if (diff>0) {
		if (diff>PI)	
			diff	-= PI_MUL_2;
	} else {
		if (diff<-PI)	
			diff	+= PI_MUL_2;
	}
	float diff_a	= _abs(diff);

	if (diff_a<EPS_S)	
		return true;

	float mot		= s*dt;
	if (mot>diff_a) mot=diff_a;
	c				+= (diff/diff_a)*mot;

	if ( is_between(c,before,t) )
		return		false;

	if (c<0)				
		c+=PI_MUL_2;
	else if (c>PI_MUL_2)	
		c-=PI_MUL_2;

	return false;
}

// Тупо lerp :)	ожидает нормализованные углы в радиусе от 0 до 2Пи
ICF float		angle_lerp		(float A, float B, float f)
{
	float diff		= B - A;
	if (diff>PI)		diff	-= PI_MUL_2;
	else if (diff<-PI)	diff	+= PI_MUL_2;

	return			A + diff*f;
}

IC float		angle_inertion	(float src, float tgt, float speed, float clmp, float dt)
{
	float a			= angle_normalize_signed	(tgt);
	angle_lerp		(src,a,speed,dt);
	src				= angle_normalize_signed	(src);
	float dH		= angle_difference_signed	(src,a);
	float dCH		= clampr					(dH,-clmp,clmp);
	src				-= dH-dCH;
	return			src;
}

IC float		angle_inertion_var(float src, float tgt, float min_speed, float max_speed, float clmp, float dt)
{
	tgt				= angle_normalize_signed	(tgt);
	src				= angle_normalize_signed	(src);
	float speed		= _abs((max_speed-min_speed)*angle_difference(tgt,src)/clmp)+min_speed;
	angle_lerp		(src,tgt,speed,dt);
	src				= angle_normalize_signed	(src);
	float dH		= angle_difference_signed	(src,tgt);
	float dCH		= clampr					(dH,-clmp,clmp);
	src				-= dH-dCH;
	return			src;
}

template <class T>
IC _matrix<T>& _matrix<T>::rotation	(const _quaternion<T> &Q) 
{
	T xx = Q.x*Q.x; T yy = Q.y*Q.y; T zz = Q.z*Q.z;
	T xy = Q.x*Q.y; T xz = Q.x*Q.z; T yz = Q.y*Q.z;
	T wx = Q.w*Q.x; T wy = Q.w*Q.y; T wz = Q.w*Q.z;

	_11 = 1 - 2 * ( yy + zz );	_12 =     2 * ( xy - wz );	_13 =     2 * ( xz + wy );	_14 = 0;
	_21 =     2 * ( xy + wz );	_22 = 1 - 2 * ( xx + zz );	_23 =     2 * ( yz - wx );	_24 = 0;
	_31 =     2 * ( xz - wy );	_32 =     2 * ( yz + wx );	_33 = 1 - 2 * ( xx + yy );	_34 = 0;
	_41 = 0;					_42 = 0;					_43 = 0;					_44 = 1;
	return *this;
}

template <class T>
IC _matrix<T>& _matrix<T>::mk_xform	(const _quaternion<T> &Q, const Tvector &V) 
{
	T xx = Q.x*Q.x; T yy = Q.y*Q.y; T zz = Q.z*Q.z;
	T xy = Q.x*Q.y; T xz = Q.x*Q.z; T yz = Q.y*Q.z;
	T wx = Q.w*Q.x; T wy = Q.w*Q.y; T wz = Q.w*Q.z;

	_11 = 1 - 2 * ( yy + zz );	_12 =     2 * ( xy - wz );	_13 =     2 * ( xz + wy );	_14 = 0;
	_21 =     2 * ( xy + wz );	_22 = 1 - 2 * ( xx + zz );	_23 =     2 * ( yz - wx );	_24 = 0;
	_31 =     2 * ( xz - wy );	_32 =     2 * ( yz + wx );	_33 = 1 - 2 * ( xx + yy );	_34 = 0;
	_41 = V.x;					_42 = V.y;					_43 = V.z;					_44 = 1;
	return *this;
}

#define TRACE_QZERO_TOLERANCE	0.1f
template <class T>
IC _quaternion<T>& _quaternion<T>::set(const _matrix<T>& M)
{

	float trace, s;
	
	trace = float(M._11 + M._22 + M._33);
	if (trace > 0.0f){
		s = _sqrt(trace + 1.0f);
		w = s * 0.5f;
		s = 0.5f / s;

		x = (M._32 - M._23) * s;
		y = (M._13 - M._31) * s;
		z = (M._21 - M._12) * s;
	}else{
		int biggest;
		enum {A,E,I};
		if (M._11 > M._22){
			if (M._33 > M._11)
				biggest = I;	
			else
				biggest = A;
		}else{
			if (M._33 > M._11)
				biggest = I;
			else
				biggest = E;
		}

		// В необычном кейсе оригиналный след фейлит на построение правильного квадратного корня
		switch (biggest){
		case A:
			s = float(std::sqrt( M._11 - (M._22 + M._33) + 1.0));
			if (s > TRACE_QZERO_TOLERANCE){
				x = s * 0.5f;
				s = 0.5f / s;
				w = (M._32 - M._23) * s;
				y = (M._12 + M._21) * s;
				z = (M._13 + M._31) * s;
				break;
			}
			// I
			s = float(std::sqrt( M._33 - (M._11 + M._22) + 1.0));
			if (s > TRACE_QZERO_TOLERANCE){
				z = s * 0.5f;
				s = 0.5f / s;
				w = (M._21 - M._12) * s;
				x = (M._31 + M._13) * s;
				y = (M._32 + M._23) * s;
				break;
			}
			// E
			s = float(std::sqrt( M._22 - (M._33 + M._11) + 1.0));
			if (s > TRACE_QZERO_TOLERANCE){
				y = s * 0.5f;
				s = 0.5f / s;
				w = (M._13 - M._31) * s;
				z = (M._23 + M._32) * s;
				x = (M._21 + M._12) * s;
				break;
			}
			break;
		case E:
			s = float(std::sqrt( M._22 - (M._33 + M._11) + 1.0));
			if (s > TRACE_QZERO_TOLERANCE){
				y = s * 0.5f;
				s = 0.5f / s;
				w = (M._13 - M._31) * s;
				z = (M._23 + M._32) * s;
				x = (M._21 + M._12) * s;
				break;
			}
			// I
			s = float(std::sqrt( M._33 - (M._11 + M._22) + 1.0));
			if (s > TRACE_QZERO_TOLERANCE){
				z = s * 0.5f;
				s = 0.5f / s;
				w = (M._21 - M._12) * s;
				x = (M._31 + M._13) * s;
				y = (M._32 + M._23) * s;
				break;
			}
			// A
			s = float(std::sqrt( M._11 - (M._22 + M._33) + 1.0));
			if (s > TRACE_QZERO_TOLERANCE){
				x = s * 0.5f;
				s = 0.5f / s;
				w = (M._32 - M._23) * s;
				y = (M._12 + M._21) * s;
				z = (M._13 + M._31) * s;
				break;
			}
			break;
		case I:
			s = float(std::sqrt( M._33 - (M._11 + M._22) + 1.0));
			if (s > TRACE_QZERO_TOLERANCE){
				z = s * 0.5f;
				s = 0.5f / s;
				w = (M._21 - M._12) * s;
				x = (M._31 + M._13) * s;
				y = (M._32 + M._23) * s;
				break;
			}
			// A
			s = float(std::sqrt( M._11 - (M._22 + M._33) + 1.0));
			if (s > TRACE_QZERO_TOLERANCE){
				x = s * 0.5f;
				s = 0.5f / s;
				w = (M._32 - M._23) * s;
				y = (M._12 + M._21) * s;
				z = (M._13 + M._31) * s;
				break;
			}
			// E
			s = float(std::sqrt( M._22 - (M._33 + M._11) + 1.0));
			if (s > TRACE_QZERO_TOLERANCE){
				y = s * 0.5f;
				s = 0.5f / s;
				w = (M._13 - M._31) * s;
				z = (M._23 + M._32) * s;
				x = (M._21 + M._12) * s;
				break;
			}
			break;
		}
	}
	return *this;
}