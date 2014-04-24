#ifndef _INTERPOLATE_H
#define _INTEPROLATE_H

namespace Interpolate
{
	//// INTERPOLATION FUNCTIONS: Several based on http://wiki.unity3d.com/index.php?title=Interpolate ////

	template <typename T>
	inline T Lerp(T& start, T& end, float time)
	{
		return start*(1.0f - time) + end*time;
	}

	template <typename T>
	inline T QuadraticBezier(T& start, T& control_point, T& end, float t)
	{
		float oneMinusT = 1.0f - t;
		return start*oneMinusT*oneMinusT + control_point*2*oneMinusT*t + end*t*t;
	}

	template <typename T>
	inline T QuadraticBezierPassThrough(T& start, T& control_point, T& end, float t)
	{
		T real_cp = (control_point * 2) - (start + end) * 0.5f;
		return QuadraticBezier(start, real_cp, end, t);
	}

	template <typename T>
	inline T CubicBezier(T& start, T& control_point1, T& control_point2, T& end, float t)
	{
		float oneMinusT = 1.0f - t;		
		return start*oneMinusT*oneMinusT*oneMinusT + control_point1*oneMinusT*oneMinusT*t*3 + control_point2*oneMinusT*t*t*3 + end*t*t*t;
	}

	template <typename T>
	inline T NewCatmullRom(T* nodes, int num_nodes, int slices, float t, bool loop) {
		// need at least two nodes to spline between
		if (num_nodes < 2) {
			return nodes[num_nodes];
		}
		int last = num_nodes - 1;
		int current = (int)(t * (float)last);
		if (loop && current > last) {
			current = 0;
		}
		// handle edge cases for looping and non-looping scenarios
		// when looping we wrap around, when not looping use start for previous
		// and end for next when you at the ends of the nodes array
		int previous = (current == 0) ? ((loop) ? last : current) : current - 1;
		int start = current;
		int end = (current == last) ? ((loop) ? 0 : current) : current + 1;
		int next = (end == last) ? ((loop) ? 0 : end) : end + 1;
 
		float stepPercent = 1.0f/last;
		float actualT = MapValue(t, stepPercent*current, stepPercent*(current+1), 0.0f, 1.0f);
		return CatmullRom(nodes[previous],
							 nodes[start],
							 nodes[end],
							 nodes[next],
							 actualT);
		
		
	}

	template <typename T>
	inline T CatmullRom(T& previous, T& start, T& end, T& next, 
								float t) {
		// References used:
		// p.266 GemsV1
		//
		// tension is often set to 0.5 but you can use any reasonable value:
		// http://www.cs.cmu.edu/~462/projects/assn2/assn2/catmullRom.pdf
		//
		// bias and tension controls:
		// http://local.wasp.uwa.edu.au/~pbourke/miscellaneous/interpolation/
 
		float percentComplete = t;
		float percentCompleteSquared = percentComplete * percentComplete;
		float percentCompleteCubed = percentCompleteSquared * percentComplete;
 
		return previous * (-0.5f * percentCompleteCubed +
								   percentCompleteSquared -
							0.5f * percentComplete) +
				start   * ( 1.5f * percentCompleteCubed +
						   -2.5f * percentCompleteSquared + 1.0f) +
				end     * (-1.5f * percentCompleteCubed +
							2.0f * percentCompleteSquared +
							0.5f * percentComplete) +
				next    * ( 0.5f * percentCompleteCubed -
							0.5f * percentCompleteSquared);
	}

	/**
	 * Linear interpolation (same as Mathf.Lerp)
	 */
	inline float Linear(float start, float distance, float elapsedTime, float duration) {
		// clamp elapsedTime to be <= duration
		if (elapsedTime > duration) { elapsedTime = duration; }
		return distance * (elapsedTime / duration) + start;
	}
 
	/**
	 * quadratic easing in - accelerating from zero velocity
	 */
	inline float EaseInQuad(float start, float distance, float elapsedTime, float duration) {
		// clamp elapsedTime so that it cannot be greater than duration
		elapsedTime = (elapsedTime > duration) ? 1.0f : elapsedTime / duration;
		return distance * elapsedTime * elapsedTime + start;
	}
 
	/**
	 * quadratic easing out - decelerating to zero velocity
	 */
	inline float EaseOutQuad(float start, float distance, float elapsedTime, float duration) {
		// clamp elapsedTime so that it cannot be greater than duration
		elapsedTime = (elapsedTime > duration) ? 1.0f : elapsedTime / duration;
		return -distance * elapsedTime * (elapsedTime - 2) + start;
	}
}

#endif