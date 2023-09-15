#define FLT_EPSILON 1.19e-07
#define PI float(3.1415926535897932384626433832795)

uniform int nVertices = 4;

uniform bool fade_tails = false;
uniform int fade_start;
uniform int fade_length;
uniform int fade_stop;

vec2 CalculateFade()
{
    if (!fade_tails)
        return vec2(1.0);

    int i = (nVertices -4) - gl_PrimitiveIDIn;

    i -= fade_stop;

    if (i > 0)
    return vec2(
        1.0 - clamp(((i+1) - fade_start) / float(fade_length), 0.0, 1.0),
        1.0 - clamp((  i   - fade_start) / float(fade_length), 0.0, 1.0)
    );
    else
        return vec2(0);
}
