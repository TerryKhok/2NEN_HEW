#pragma once


/// <summary>
/// 2次元ベクトルクラス。
/// </summary>
class Vector2 {
public:
	//メモリ確保禁止
	void* operator new(size_t) = delete;

	static const Vector2 Zero;
	/// <summary>
	/// コンストラクタ。
	/// </summary>
	explicit Vector2()
	{
		x = 0.0f;
		y = 0.0f;
	}
	/// <summary>
	/// コンストラクタ。
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	Vector2(float x, float y)
	{
		this->x = x;
		this->y = y;
	}

	/// <summary>
	/// 代入演算子
	/// </summary>
	/// <param name="_v"></param>
	/// <returns></returns>
	Vector2& operator=(const Vector2& _v)
	{
		vec = _v.vec;
		return *this;
	}

	/// <summary>
	/// 加算演算
	/// </summary>
	/// <param name="_v"></param>
	/// <returns></returns>
	Vector2 operator+(const Vector2& _v)
	{
		return { vec.x + _v.x, vec.y + _v.y };
	}
	/// <summary>
	/// 加算代入演算
	/// </summary>
	/// <param name="_v"></param>
	/// <returns></returns>
	Vector2& operator+=(const Vector2& _v)
	{
		vec.x += _v.x;
		vec.y += _v.y;
		return *this;
	}
	/// <summary>
	/// 加算代入演算
	/// </summary>
	/// <param name="_v"></param>
	/// <returns></returns>
	Vector2& operator+=(const float& _v)
	{
		vec.x += _v;
		vec.y += _v;
		return *this;
	}
	/// <summary>
	/// 乗算演算
	/// </summary>
	/// <param name="_v"></param>
	/// <returns></returns>
	Vector2 operator*(const float& _v)
	{
		return{ vec.x * _v, vec.y * _v };
	}
	/// <summary>
	/// 乗算代入演算
	/// </summary>
	/// <param name="_v"></param>
	/// <returns></returns>
	Vector2& operator*=(const float& _v)
	{
		vec.x *= _v;
		vec.y *= _v;
		return *this;
	}
	/// <summary>
	/// 減算代入演算
	/// </summary>
	/// <param name="_v"></param>
	/// <returns></returns>
	Vector2 operator-(const Vector2& _v)
	{
		return { vec.x - _v.x, vec.y - _v.y };
	}
	/// <summary>
	/// 減算代入演算
	/// </summary>
	/// <param name="_v"></param>
	/// <returns></returns>
	Vector2& operator-=(const Vector2& _v)
	{
		vec.x -= _v.x;
		vec.y -= _v.y;
		return *this;
	}
	/// <summary>
	/// 減算代入演算
	/// </summary>
	/// <param name="_v"></param>
	/// <returns></returns>
	Vector2& operator-=(const float& _v)
	{
		vec.x -= _v;
		vec.y -= _v;
		return *this;
	}
	/// <summary>
	/// 徐算演算
	/// </summary>
	/// <param name="_v"></param>
	/// <returns></returns>
	Vector2 operator/(const float& _v)
	{
		return{ vec.x / _v, vec.y / _v };
	}
	/// <summary>
	/// 徐算演算
	/// </summary>
	/// <param name="_v"></param>
	/// <returns></returns>
	Vector2 operator/(const Vector2& _v)
	{
		return{ vec.x / _v.x, vec.y / _v.y };
	}
	/// <summary>
	/// 徐算代入演算
	/// </summary>
	/// <param name="_v"></param>
	/// <returns></returns>
	Vector2& operator/=(const float& _v)
	{
		vec.x /= _v;
		vec.y /= _v;
		return *this;
	}

	bool operator==(const Vector2& _v)
	{
		if (x == _v.x && y == _v.y)
			return true;
		
		return false;
	}

	void Set(float _x, float _y)
	{
		vec.x = _x;
		vec.y = _y;
	}
	union {
		DirectX::XMFLOAT2 vec;
		struct { float x, y; };
		float v[2];
	};

	/// <summary>
	/// 線形補完。
	/// </summary>
	/// <param name="t">補完率</param>
	/// <param name="v0">補完開始のベクトル。</param>
	/// <param name="v1">補完終了のベクトル。</param>
	void Lerp(float t, const Vector2& v0, const Vector2& v1)
	{
		x = v0.x + (v1.x - v0.x) * t;
		y = v0.y + (v1.y - v0.y) * t;
	}

	float Distance(const Vector2& v)
	{
		return sqrt(powf((x - v.x), 2) + powf((y - v.y), 2));
	}
	/// <summary>
	/// 正規化。
	/// </summary>
	void Normalize()
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat2(&vec);
		xmv = DirectX::XMVector2Normalize(xmv);
		DirectX::XMStoreFloat2(&vec, xmv);
	}
	/// <summary>
	/// データの先頭ポインタ取得
	/// </summary>
	/// <returns></returns>
	float* data()
	{
		return v;
	}

private:
	template<class Archive>
	void serialize(Archive& ar)
	{
		ar(CEREAL_NVP(x), CEREAL_NVP(y));
	}

	friend class cereal::access;
};
/// <summary>
/// 3次元ベクトル。
/// </summary>
class Vector3 {
public:
	//メモリ確保禁止
	void* operator new(size_t) = delete;

	union {
		DirectX::XMFLOAT3 vec;
		float v[3];
		struct { float x, y, z; };
	};
	static const Vector3 Zero;
	static const Vector3 Right;
	static const Vector3 Left;
	static const Vector3 Up;
	static const Vector3 Down;
	static const Vector3 Front;
	static const Vector3 Back;
	static const Vector3 AxisX;
	static const Vector3 AxisY;
	static const Vector3 AxisZ;
	static const Vector3 One;
private:
	GENERATE_SERIALIZE(x, y, z)

public:
	/// <summary>
	/// XMVECTORへの暗黙の変換。
	/// </summary>
	/// <returns></returns>
	operator DirectX::XMVECTOR() const
	{
		return DirectX::XMLoadFloat3(&vec);
	}
	/// <summary>
	/// XMFLOAT3への暗黙の変換。
	/// </summary>
	/// <returns></returns>
	operator DirectX::XMFLOAT3() const
	{
		return { this->x,this->y,this->z };
	}
	/// <summary>
	/// Vector2への暗黙の変換。
	/// </summary>
	/// <returns></returns>
	operator Vector2() const
	{
		return Vector2(vec.x, vec.y);
	}
	/// <summary>
	/// Vector3への変換。
	/// </summary>
	/// <returns></returns>
	Vector3& operator=(const Vector2& vec)
	{
		this->x = vec.x;
		this->y = vec.y;
		return *this;
	}
	/// <summary>
	/// 代入演算子。
	/// </summary>
	/// <param name="_v"></param>
	/// <returns></returns>
	Vector3& operator=(const Vector3& _v)
	{
		vec = _v.vec;
		return *this;
	}

	/// <summary>
	/// コンストラクタ。
	/// </summary>
	/// <remarks>
	/// x,y,z全ての要素0で初期化されます。
	/// </remarks>
	explicit Vector3()
	{
		x = y = z = 0.0f;
	}
	/// <summary>
	/// コンストラクタ。
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	/// <param name="z"></param>
	Vector3(float x, float y, float z)
	{
		Set(x, y, z);
	}
	/// <summary>
	/// データ配列をポインタで返す
	/// </summary>
	/// <returns></returns>
	float* data()
	{
		return v;
	}
	/// <summary>
	/// 線形補完
	/// </summary>
	/// <remarks>
	/// this = v0 + (v1-v0) * t;
	/// </remarks>
	/// <param name="t">補完率。</param>
	/// <param name="v0">補完開始のベクトル。</param>
	/// <param name="v1">補完終了のベクトル。</param>
	void Lerp(float t, const Vector3& v0, const Vector3& v1)
	{
		DirectX::XMVECTOR _v = DirectX::XMVectorLerp(
			DirectX::XMLoadFloat3(&v0.vec),
			DirectX::XMLoadFloat3(&v1.vec),
			t);
		DirectX::XMStoreFloat3(&vec, _v);
	}
	/// <summary>
	/// ベクトルのコピー。
	/// </summary>
	template<class TVector>
	void CopyTo(TVector& dst) const
	{
		dst.x = x;
		dst.y = y;
		dst.z = z;
	}
	/// <summary>
	/// ベクトルの設定。
	/// </summary>
	/// <param name="_x"></param>
	/// <param name="_y"></param>
	/// <param name="_z"></param>
	void Set(float _x, float _y, float _z)
	{
		vec.x = _x;
		vec.y = _y;
		vec.z = _z;
	}
	template<class TVector>
	void Set(TVector& _v)
	{
		Set(_v.x, _v.y, _v.z);
	}

	/// <summary>
	/// ベクトルの加算。
	/// </summary>
	/// <remarks>
	/// this += _v;
	/// </remarks>
	/// <param name="_v">加算されるベクトル。</param>
	void Add(const Vector3& _v)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&_v.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorAdd(xmv0, xmv1);
		DirectX::XMStoreFloat3(&vec, xmvr);
	}
	/// <summary>
	/// ベクトルの加算。
	/// </summary>
	/// <remarks>
	/// this = v0 + v1;
	/// </remarks>
	void Add(const Vector3& v0, const Vector3& v1)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&v0.vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&v1.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorAdd(xmv0, xmv1);
		DirectX::XMStoreFloat3(&vec, xmvr);
	}
	/// <summary>
	/// ベクトルの減算。
	/// </summary>
	/// <remarks>
	/// this -= _v;
	/// </remarks>
	void Subtract(const Vector3& _v)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&_v.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorSubtract(xmv0, xmv1);
		DirectX::XMStoreFloat3(&vec, xmvr);
	}
	/// <summary>
	/// ベクトルの減算。
	/// </summary>
	/// <remarks>
	/// this = v0 - v1;
	/// </remarks>
	void Subtract(const Vector3& v0, const Vector3& v1)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&v0.vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&v1.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorSubtract(xmv0, xmv1);
		DirectX::XMStoreFloat3(&vec, xmvr);
	}

	/// <summary>
	/// 内積を計算。
	/// </summary>
	/// <remarks>
	/// float d = this->x * _v.x + this->y * _v.y + this->z * _v.z;
	/// return d;
	/// </remarks>
	float Dot(const Vector3& _v) const
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&_v.vec);
		return DirectX::XMVector3Dot(xmv0, xmv1).m128_f32[0];
	}

	/// <summary>
	/// 外積。
	/// </summary>
	/// <remarks>
	/// Vector3 v ;
	/// v.x = this->y * _v.z - this->z * _v.y;
	/// v.y = thiz->z * _v.x - this->x * _v.z;
	/// v.z = this->x * _v.y - this->y * _v.x;
	/// this = v;
	/// </remarks>
	void Cross(const Vector3& _v)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&_v.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVector3Cross(xmv0, xmv1);
		DirectX::XMStoreFloat3(&vec, xmvr);
	}
	/// <summary>
	/// 外積。
	/// </summary>
	/// <remarks>
	/// this->x = v0.y * v1.z - v0.z * v1.y;
	/// this->y = v0.z * v1.x - v0.x * v1.z;
	/// this->z = v0.x * v1.y - v0.y * v1.x; 
	/// </remarks>
	void Cross(const Vector3& v0, const Vector3& v1)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&v0.vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&v1.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVector3Cross(xmv0, xmv1);
		DirectX::XMStoreFloat3(&vec, xmvr);
	}
	/// <summary>
	/// ベクトルの長さを取得
	/// </summary>
	float Length() const
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat3(&vec);
		return DirectX::XMVector3Length(xmv).m128_f32[0];
	}
	/// <summary>
	/// ベクトルの長さの二乗を取得。
	/// </summary>
	float LengthSq() const
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat3(&vec);
		return DirectX::XMVector3LengthSq(xmv).m128_f32[0];
	}

	/// <summary>
	/// ベクトルをスカラーで拡大。
	/// </summary>
	/// <remarks>
	/// 下記のような処理が行われています。
	/// this->x *= s;
	/// this->y *= s;
	/// this->z *= s;
	/// </remarks>
	void Scale(float s)
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat3(&vec);
		xmv = DirectX::XMVectorScale(xmv, s);
		DirectX::XMStoreFloat3(&vec, xmv);
	}

	/// <summary>
	/// ベクトルを正規化。
	/// </summary>
	/// <remarks>
	///		下記のような処理が行われています。
	///		//ベクトルの大きさを求める。
	///		float len = this->x * this->x + this->y * this->y + this->z * this->z;
	///		len = sqrt(len);
	///		//ベクトルの大きさで、各要素を除算する。
	///		this->x /= len;
	///		this->y /= len;
	///		this->z /= len;
	/// </remarks>
	void Normalize()
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat3(&vec);
		xmv = DirectX::XMVector3Normalize(xmv);
		DirectX::XMStoreFloat3(&vec, xmv);
	}
	/// <summary>
	/// ベクトルをスカラーで除算。
	/// </summary>
	/// <remarks>
	///		下記のような処理が行われています。
	///		this->x /= d;
	///		this->y /= d;
	///		this->z /= d;
	/// </remarks>
	void Div(float d)
	{
		float scale = 1.0f / d;
		Scale(scale);
	}

	/// <summary>
	/// 最大値を設定。
	/// </summary>
	/// <remarks>
	/// 下記のような処理が行われています。
	/// this->x = max( this->x, v.x );
	/// this->y = max( this->y, v.y );
	/// this->z = max( this->z, v.z );
	/// </remarks>
	void Max(const Vector3& v)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&v.vec);
		DirectX::XMStoreFloat3(&vec, DirectX::XMVectorMax(xmv0, xmv1));
	}

	/// <summary>
	/// 最小値を設定。
	/// </summary>
	/// <remarks>
	/// 下記のような処理が行われています。
	/// this->x = min( this->x, v.x );
	/// this->y = min( this->y, v.y );
	/// this->z = min( this->z, v.z );
	/// </remarks>
	void Min(const Vector3& v)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&v.vec);
		DirectX::XMStoreFloat3(&vec, DirectX::XMVectorMin(xmv0, xmv1));
	}
	/// <summary>
	/// 加算演算子。
	/// </summary>
	/// <param name="_v"></param>
	/// <returns></returns>
	Vector3 operator+(const Vector3& _v)
	{
		return { x + _v.x,y + _v.y,z + _v.z };
	}
	/// <summary>
	/// 乗算演算子。
	/// </summary>
	/// <param name="_v"></param>
	/// <returns></returns>
	Vector3 operator*(const float& s)
	{
		return { x * s,y * s,z * s };
	}
	/// <summary>
	/// 減算演算子。
	/// </summary>
	/// <param name="_v"></param>
	/// <returns></returns>
	Vector3 operator-(const Vector3& _v)
	{
		return { x - _v.x,y - _v.y,z - _v.z };
	}
	/// <summary>
	/// 徐算演算子。
	/// </summary>
	/// <param name="_v"></param>
	/// <returns></returns>
	Vector3 operator/(const float& s)
	{
		return { x / s,y / s,z / s };
	}

	/// <summary>
	/// 加算代入演算子。
	/// </summary>
	/// <remarks>
	///	ベクトルクラスに下記のような加算代入の機能を提供します。
	///	Vector3 v0 = {20.0f, 30.0f, 40.0f};
	///	Vector3 v1 = {10.0f, 20.0f, 30.0f};
	///	v0 += v1;
	/// </remarks>
	const Vector3& operator+=(const Vector3& _v)
	{
		Add(_v);
		return *this;
	}
	/// <summary>
/// 加算代入演算子。
/// </summary>
/// <remarks>
///	ベクトルクラスに下記のような加算代入の機能を提供します。
///	Vector3 v0 = {20.0f, 30.0f, 40.0f};
///	Vector3 v1 = {10.0f, 20.0f, 30.0f};
///	v0 += v1;
/// </remarks>
	const Vector3& operator+=(const Vector2& _v)
	{
		vec.x += _v.x;
		vec.y += _v.y;
		return *this;
	}
	/// <summary>
	/// 乗算代入演算子。
	/// </summary>
	/// <remarks>
	///	ベクトルクラスに下記のような乗算代入演算の機能を提供します。
	/// Vector3 v = {20.0f, 30.0f, 40.0f};
	///	v *= 10;
	/// </remarks>
	const Vector3& operator*=(float s)
	{
		Scale(s);
		return *this;
	}
	/// <summary>
	/// 減算代入演算子。
	/// </summary>
	/// <remarks>
	/// ベクトルクラスに下記のような減算代入演算の機能を提供します。
	/// Vector3 v0 = {20.0f, 30.0f, 40.0f};
	///	Vector3 v1 = {10.0f, 20.0f, 30.0f};
	///	v0 -= v1;
	/// </remarks>
	const Vector3& operator-=(const Vector3& _v)
	{
		Subtract(_v);
		return *this;
	}

	const Vector3& operator-=(const Vector2& _v)
	{
		vec.x -= _v.x;
		vec.y -= _v.y;
		return *this;
	}
	/// <summary>
	/// 除算代入演算子。
	/// </summary>
	/// <remarks>
	/// ベクトルクラスに下記のような除算代入演算の機能を提供します。
	/// Vector3 v = {20.0f, 30.0f, 40.0f};
	///	v /= 10;
	/// </remarks>
	const Vector3& operator/=(const float s)
	{
		Div(s);
		return *this;
	}

	const bool operator==(const Vector3& vec)
	{
		if (x == vec.x && y == vec.y && z == vec.z)
			return true;

		return false;
	}
};
/// <summary>
/// 4次元ベクトルクラス。
/// </summary>
class Vector4 {
public:
	//メモリ確保禁止
	void* operator new(size_t) = delete;

	union {
		DirectX::XMFLOAT4 vec;
		struct { float x, y, z, w; };
		struct { float r, g, b, a; };
		float v[4];
	};
	static const Vector4 White;
	static const Vector4 Black;
	static const Vector4 Yellow;
	static const Vector4 Gray;
public:
	/// <summary>
	/// XMVECTORへの暗黙の型変換。
	/// </summary>
	/// <returns></returns>
	operator DirectX::XMVECTOR() const
	{
		return DirectX::XMLoadFloat4(&vec);
	}
	/// <summary>
	/// コンストラクタ。
	/// </summary>
	/// <remarks>
	/// x,y,zが0.0、wは1.0で初期化されます。
	/// </remarks>
	explicit Vector4()
	{
		x = y = z = 0.0f;
		w = 1.0f;
	}
	/// <summary>
	/// ベクトルのコピー。
	/// </summary>
	template<class TVector4>
	void CopyTo(TVector4& dst) const
	{
		dst.x = x;
		dst.y = y;
		dst.z = z;
		dst.w = w;
	}

	/// <summary>
	/// 代入演算子。
	/// </summary>
	/// <remarks>
	/// ベクトルクラスに下記のような代入演算の機能を提供します。
	/// Vector4 v0 = {10.0f, 20.0f, 30.0f, 10.0f};
	/// Vector4 v1;
	/// v1 = v0;
	/// </remarks>
	Vector4& operator=(const Vector4& _v)
	{
		vec = _v.vec;
		return *this;
	}

	/// <summary>
	/// コンストラクタ。
	/// </summary>
	Vector4(float x, float y, float z, float w)
	{
		Set(x, y, z, w);
	}
	/// <summary>
	/// 3次元のベクトルデータを引数で受け取るコンストラクタ
	/// </summary>
	/// <remarks>
	/// wには1.0が格納されます。
	/// </remarks>
	Vector4(const Vector3& v)
	{
		Set(v);
	}
	/// <summary>
	/// ベクトルの各要素を設定。
	/// </summary>
	void Set(float _x, float _y, float _z, float _w)
	{
		this->x = _x;
		this->y = _y;
		this->z = _z;
		this->w = _w;
	}
	/// <summary>
	/// ベクトルを正規化。
	/// </summary>
	void Normalize()
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat4(&vec);
		xmv = DirectX::XMVector4Normalize(xmv);
		DirectX::XMStoreFloat4(&vec, xmv);
	}
	/// <summary>
	/// ベクトルを設定。
	/// </summary>
	/// <param name="_v"></param>
	void Set(const Vector4& _v)
	{
		*this = _v;
	}
	/// <summary>
	/// 3次元ベクトルデータをもとに、ベクトルを設定。
	/// </summary>
	/// <remarks>
	/// wには1.0が設定されます。
	/// </remarks>
	void Set(const Vector3& _v)
	{
		this->x = _v.x;
		this->y = _v.y;
		this->z = _v.z;
		this->w = 1.0f;
	}
	/// <summary>
	/// ベクトルを加算。
	/// </summary>
	/// <remarks>
	/// this += _v;
	/// </remarks>
	void Add(const Vector4& _v)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat4(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat4(&_v.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorAdd(xmv0, xmv1);
		DirectX::XMStoreFloat4(&vec, xmvr);
	}
	/// <summary>
	/// ベクトルの加算。
	/// </summary>
	/// <remarks>
	/// this = v0 + v1;
	/// </remarks>
	void Add(const Vector4& v0, const Vector4& v1)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat4(&v0.vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat4(&v1.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorAdd(xmv0, xmv1);
		DirectX::XMStoreFloat4(&vec, xmvr);
	}
	/// <summary>
	/// ベクトルの減算。
	/// </summary>
	/// <remarks>
	/// this -= _v;
	/// </remarks>
	void Subtract(const Vector4& _v)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat4(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat4(&_v.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorSubtract(xmv0, xmv1);
		DirectX::XMStoreFloat4(&vec, xmvr);
	}
	/// <summary>
	/// ベクトルの減算。
	/// </summary>
	/// <remarks>
	/// this = v0 - v1;
	/// </remarks>
	void Subtract(const Vector4& v0, const Vector4& v1)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat4(&v0.vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat4(&v1.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorSubtract(xmv0, xmv1);
		DirectX::XMStoreFloat4(&vec, xmvr);
	}

	/// <summary>
	/// 内積を計算。
	/// </summary>
	/// <remarks>
	/// float d = this->x * _v.x + this->y * _v.y + this->z * _v.z + this->w * _v.w;
	/// return d;
	/// </remarks>
	float Dot(const Vector4& _v) const
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat4(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat4(&_v.vec);
		return DirectX::XMVector4Dot(xmv0, xmv1).m128_f32[0];
	}
	/// <summary>
	/// ベクトルの長さを取得
	/// </summary>
	float Length()
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat4(&vec);
		return DirectX::XMVector4Length(xmv).m128_f32[0];
	}
	/// <summary>
	/// ベクトルの長さの二乗を取得。
	/// </summary>
	float LengthSq()
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat4(&vec);
		return DirectX::XMVector4LengthSq(xmv).m128_f32[0];
	}
	/// <summary>
	/// ベクトルをスカラーで拡大。
	/// </summary>
	/// <remarks>
	/// 下記のような処理が行われています。
	/// this->x *= s;
	/// this->y *= s;
	/// this->z *= s;
	/// this->w *= s;
	/// </remarks>
	void Scale(float s)
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat4(&vec);
		xmv = DirectX::XMVectorScale(xmv, s);
		DirectX::XMStoreFloat4(&vec, xmv);
	}
};

const Vector2 g_vec2Zero = { 0.0f, 0.0f };
const Vector3 g_vec3Zero = { 0.0f,  0.0f,  0.0f };
const Vector3 g_vec3Right = { 1.0f,  0.0f,  0.0f };
const Vector3 g_vec3Left = { -1.0f,  0.0f,  0.0f };
const Vector3 g_vec3Up = { 0.0f,  1.0f,  0.0f };
const Vector3 g_vec3Down = { 0.0f, -1.0f,  0.0f };
const Vector3 g_vec3Front = { 0.0f,   0.0f,  1.0f };
const Vector3 g_vec3Back = { 0.0f,   0.0f, -1.0f };
const Vector3 g_vec3AxisX = { 1.0f,  0.0f,  0.0f };
const Vector3 g_vec3AxisY = { 0.0f,  1.0f,  0.0f };
const Vector3 g_vec3AxisZ = { 0.0f,  0.0f,  1.0f };
const Vector3 g_vec3One = { 1.0f, 1.0f, 1.0f };

const Vector4 g_vec4White = { 1.0f, 1.0f, 1.0f, 1.0f };
const Vector4 g_vec4Black = { 0.0f, 0.0f, 0.0f, 1.0f };
const Vector4 g_vec4Gray = { 0.5f, 0.5f, 0.5f, 1.0f };
const Vector4 g_vec4Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };

//整数型のベクトルクラス。
__declspec(align(16)) class Vector4i {
public:
	union {
		struct { int x, y, z, w; };
		int v[4];
	};
};
/// <summary>
/// ベクトル同士の加算。
/// </summary>
static inline Vector3 operator+(const Vector3& v0, const Vector3& v1)
{
	Vector3 result;
	result.Add(v0, v1);
	return result;
}
/// <summary>
/// ベクトルとスカラーの乗算。
/// </summary>

static inline Vector3 operator*(const Vector3& v, float s)
{
	Vector3 result;
	result = v;
	result.Scale(s);
	return result;
}
/// <summary>
/// ベクトルとスカラーの除算。
/// </summary>
static inline Vector3 operator/(const Vector3& v, float s)
{
	Vector3 result;
	result = v;
	result.Div(s);
	return result;
}
/// <summary>
/// ベクトル同士の減算。
/// </summary>
static inline Vector3 operator-(const Vector3& v0, const Vector3& v1)
{
	Vector3 result;
	result.Subtract(v0, v1);
	return result;
}

/// <summary>
/// 外積を計算。
/// </summary>
static inline Vector3 Cross(const Vector3& v0, const Vector3& v1)
{
	Vector3 result;
	result.Cross(v0, v1);
	return result;
}
/// <summary>
/// 内積を計算。
/// </summary>
static inline float Dot(const Vector3& v0, const Vector3& v1)
{
	return v0.Dot(v1);
}

/// <summary>
/// ベクトル同士の加算。
/// </summary>

static inline Vector4 operator+(const Vector4& v0, const Vector4& v1)
{
	Vector4 result;
	result.Add(v0, v1);
	return result;
}
/// <summary>
/// ベクトルとスカラーの乗算。
/// </summary>

static inline Vector4 operator*(const Vector4& v, float s)
{
	Vector4 result;
	result = v;
	result.Scale(s);
	return result;
}

/// <summary>
/// ベクトル同士の減算。
/// </summary>
static inline Vector4 operator-(const Vector4& v0, const Vector4& v1)
{
	Vector4 result;
	result.Subtract(v0, v1);
	return result;
}


/// <summary>
/// 内積を計算。
/// </summary>
static inline float Dot(const Vector4& v0, const Vector4& v1)
{
	return v0.Dot(v1);
}
