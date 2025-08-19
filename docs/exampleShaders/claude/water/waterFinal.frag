#version 460 core
out vec4 FragColor;
in vec2 TexCoord;

uniform float u_delta;
uniform float u_time;
uniform vec2 u_mouse;
uniform vec2 u_resolution;

// Better hash function
vec2 hash22(vec2 p) {
    p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
    return -1.0 + 2.0 * fract(sin(p) * 43758.5453123);
}

// Improved noise with better interpolation
float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * f * (f * (f * 6.0 - 15.0) + 10.0);

    return mix(mix(dot(hash22(i + vec2(0.0, 0.0)), f - vec2(0.0, 0.0)),
            dot(hash22(i + vec2(1.0, 0.0)), f - vec2(1.0, 0.0)), u.x),
        mix(dot(hash22(i + vec2(0.0, 1.0)), f - vec2(0.0, 1.0)),
            dot(hash22(i + vec2(1.0, 1.0)), f - vec2(1.0, 1.0)), u.x), u.y);
}

// Fractal noise for complex detail
float fbm(vec2 p, int octaves) {
    float value = 0.0;
    float amplitude = 0.5;

    for (int i = 0; i < octaves; i++) {
        value += amplitude * noise(p);
        p *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

// Advanced wave function with multiple systems
float waves(vec2 pos, float time) {
    // Primary wave directions
    vec2 dir1 = vec2(cos(0.3), sin(0.3));
    vec2 dir2 = vec2(cos(2.1), sin(2.1));

    float wave1 = sin(dot(pos, dir1) * 1.2 + time * 1.1) * 0.35;
    float wave2 = sin(dot(pos, dir2) * 0.8 + time * 0.7) * 0.25;

    // Secondary interference patterns
    float inter1 = sin(pos.x * 2.1 + pos.y * 1.3 + time * 1.6) * 0.12;

    // High frequency detail
    float detail = fbm(pos * 2.5 + time * 0.3, 3) * 0.06;

    // Wind ripples
    float ripples = sin(pos.x * 8.0 + pos.y * 6.0 + time * 4.0) * 0.025;

    return wave1 + wave2 + inter1 + detail + ripples;
}

// Enhanced normal calculation
vec3 getNormal(vec2 pos, float time) {
    float eps = 0.008;
    float hL = waves(pos - vec2(eps, 0.0), time);
    float hR = waves(pos + vec2(eps, 0.0), time);
    float hD = waves(pos - vec2(0.0, eps), time);
    float hU = waves(pos + vec2(0.0, eps), time);

    vec3 normal = normalize(vec3((hL - hR) / (2.0 * eps),
                (hD - hU) / (2.0 * eps),
                1.0));
    return normal;
}

void main() {
    vec2 uv = (TexCoord - 0.5) * 5.0;
    uv.x *= u_resolution.x / u_resolution.y;

    // Enhanced mouse interaction
    vec2 mousePos = ((u_mouse / u_resolution) - 0.5) * 5.0;
    mousePos.x *= u_resolution.x / u_resolution.y;
    float mouseDist = length(uv - mousePos);
    float mouseRipple = 0.0;
    if (mouseDist < 3.0) {
        mouseRipple = sin(mouseDist * 8.0 - u_time * 6.0) * exp(-mouseDist * 1.2) * 0.2;
        mouseRipple += sin(mouseDist * 12.0 - u_time * 8.0) * exp(-mouseDist * 2.0) * 0.1;
    }

    float height = waves(uv, u_time) + mouseRipple;
    vec3 normal = getNormal(uv, u_time);

    // Multiple light sources for realism
    vec3 sunDir = normalize(vec3(0.6, 0.8, 0.5));
    vec3 skyDir = normalize(vec3(-0.2, 0.3, 0.8));

    float sunDiffuse = max(dot(normal, sunDir), 0.0);
    float skyDiffuse = max(dot(normal, skyDir), 0.0) * 0.3;
    float totalDiffuse = sunDiffuse + skyDiffuse + 0.5;

    // Advanced specular with multiple highlights
    vec3 viewDir = vec3(0.0, 0.0, 1.0);
    vec3 sunReflect = reflect(-sunDir, normal);
    vec3 skyReflect = reflect(-skyDir, normal);

    float sunSpec = pow(max(dot(viewDir, sunReflect), 0.0), 64.0);
    float skySpec = pow(max(dot(viewDir, skyReflect), 0.0), 16.0) * 0.3;

    // Realistic water colors with depth variation
    vec3 deepWater = vec3(0.04, 0.12, 0.18);
    vec3 mediumWater = vec3(0.07, 0.18, 0.28);
    vec3 shallowWater = vec3(0.1, 0.25, 0.35);

    // Color mixing based on height and lighting
    float depthMix = smoothstep(-0.4, 0.3, height);
    vec3 waterBase = mix(deepWater, mediumWater, totalDiffuse * 0.6 + 0.4);
    waterBase = mix(waterBase, shallowWater, depthMix * 0.7);

    // Dynamic foam with better conditions
    float foamThreshold = 0.3 + sin(u_time * 0.5) * 0.1;
    float foam = smoothstep(foamThreshold, foamThreshold + 0.2, height);
    foam *= smoothstep(0.3, 1.0, sunDiffuse + skyDiffuse);
    vec3 foamColor = vec3(0.35, 0.4, 0.42);

    vec3 finalColor = mix(waterBase, foamColor, foam * 0.8);

    // Apply lighting
    finalColor *= totalDiffuse;

    // Enhanced specular highlights
    finalColor += sunSpec * vec3(1.0, 0.95, 0.8) * 0.7;
    finalColor += skySpec * vec3(0.8, 0.9, 1.0) * 0.4;

    // Fresnel with better calculation
    float fresnel = pow(1.0 - max(dot(normal, viewDir), 0.0), 4.0);
    vec3 reflectionColor = vec3(0.15, 0.25, 0.35);
    finalColor = mix(finalColor, reflectionColor, fresnel * 0.25);

    // Subsurface scattering approximation
    float backlight = max(dot(-sunDir, normal), 0.0);
    finalColor += vec3(0.1, 0.15, 0.2) * backlight * 0.15;

    // Subtle color temperature variation
    float temp = noise(uv * 0.5 + u_time * 0.1) * 0.05;
    finalColor += vec3(temp * 0.5, temp * 0.2, -temp * 0.3);

    FragColor = vec4(finalColor, 1.0);
}
