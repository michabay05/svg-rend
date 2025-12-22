#version 330 core

in vec2 fragTexCoord; // The UVs we passed
in vec4 fragColor;
out vec4 finalColor;

void main()
{
    // The curve is defined by u^2 - v = 0
    // Gradients for anti-aliasing (smoothing the edge)
    vec2 p = fragTexCoord;
    float d = p.x * p.x - p.y;

    // Calculate screen-space derivates for smooth AA
    float aaf = fwidth(d);

    // If d > 0, we are outside the curve (near P1).
    // We use smoothstep to fade out the alpha for a nice anti-aliased edge.
    float alpha = 1.0 - smoothstep(-aaf, aaf, d);

    if (alpha <= 0.0) discard;

    finalColor = vec4(fragColor.rgb, fragColor.a * alpha);
}
