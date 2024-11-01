#pragma once

class Math {
public:
	//円周率
	static constexpr float PI = 3.14159265358979323846f;
	//円周率の２乗
	static constexpr float PI2 = PI * 2.0f;

	/// <summary>
	/// 角度をDegree単位からRadian単位に変換する。
	/// </summary>
	/// <param name="deg">Degree単位の角度。</param>
	/// <returns>Radian単位の角度。</returns>
	constexpr static inline float DegToRad(float deg)
	{
		return deg * (PI / 180.0f);
	}
	/// <summary>
	/// 角度をRadian単位からDegree単位に変換する。
	/// </summary>
	/// <param name="rad">Radian単位の角度。</param>
	/// <returns>Degree単位の角度。</returns>
	constexpr static inline float RadToDeg(float rad)
	{
		return rad / (PI / 180.0f);
	}
	/// <summary>
	/// t0とt1の間を線形補完。
	/// </summary>
	/// <param name="rate">補間率は0.0～1.0</param>
	/// <param name="t0">補間開始の値。</param>
	/// <param name="t1">補間終了の値。</param>
	/// <returns>補間された値。</returns>
	template<class T> static inline T Lerp(float rate, T t0, T t1)
	{
		T ret;
		ret.Lerp(rate, t0, t1);
		return ret;
	}
	/// <summary>
	/// t0とt1の間を線形補完。
	/// </summary>
	/// <param name="rate">補間率は0.0～1.0</param>
	/// <param name="t0">補間開始の値。</param>
	/// <param name="t1">補間終了の値。</param>
	/// <returns>補間された値。</returns>
	template<> static inline float Lerp(float rate, float t0, float t1)
	{
		return t0 + (t1 - t0) * rate;
	}
	/// <summary>
	/// 二つの点の距離を求める
	/// </summary>
	template<typename T>
	static T PointDistance(T x, T y, T x2, T y2) {
		T distance = sqrt((x2 - x) * (x2 - x) + (y2 - y) * (y2 - y));

		return distance;
	}
	/// <summary>
	/// 二点間の角度をラジアンで取得
	/// </summary>
	template<typename T>
	static T PointRadian(T x, T y, T x2, T y2) {
		T radian = atan2(y2 - y, x2 - x);
		return radian;
	}
};
