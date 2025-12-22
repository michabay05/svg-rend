#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;

void main()
{
    // The Loop-Blinn Curve Equation: f(u, v) = u^2 - v
    vec2 uv = fragTexCoord;
    float d = uv.x * uv.x - uv.y;

#if 0
    // 1. Simple Hard-Edge Clipping:
    if (d > 0.0) discard;
    finalColor = fragColor;
#else
    // 2. High-Quality Anti-Aliasing:
    // We use fwidth to determine how fast 'd' changes per pixel
    float alpha = 0.5 - (d / fwidth(d));

    // Clamp alpha to 0-1 range
    alpha = clamp(alpha, 0.0, 1.0);

    // If transparent, discard to save fill rate (optional)
    if (alpha <= 0.0) discard;

    finalColor = vec4(fragColor.rgb, fragColor.a * alpha);
#endif
}
