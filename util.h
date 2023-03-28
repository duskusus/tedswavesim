template <typename T>
inline T min(T x1, T x2) {
    return x1 > x2 ? x2 : x1;
}
template <typename T>
inline T max(T x1, T x2) {
    return x1 > x2 ? x1 : x2;
}
template <typename T>
inline T clamp(T x, T min, T max) {
    assert(max > min);
    if(x > max) return max;
    if(x < min) return min;
    return x;
}