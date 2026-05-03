#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;

void main()
{
    vec4 texel = texture(texture0, fragTexCoord) * fragColor;

    vec2 screenRatio = gl_FragCoord.xy / vec2(600.0, 600.0);

    float fadeSize = 0.15;

    float fadeLeft   = smoothstep(0.0, fadeSize, screenRatio.x);
    float fadeRight  = smoothstep(0.0, fadeSize, 1.0 - screenRatio.x);
    float fadeBottom = smoothstep(0.0, fadeSize, screenRatio.y);
    float fadeTop    = smoothstep(0.0, fadeSize, 1.0 - screenRatio.y);

    float alphaMask = fadeLeft * fadeRight * fadeBottom * fadeTop;

    texel.a *= alphaMask;
//    texel.rgb *= alphaMask; // Premultiplied alpha

    finalColor = texel;
}