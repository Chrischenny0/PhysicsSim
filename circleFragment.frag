#version 460
out vec4 fragColor;
in vec2 coords;

uniform float radius;

void main() {
    float dist = distance(gl_FragCoord.xy, coords);
    if(dist >= radius){
        discard;
    }
    fragColor = vec4(1, 0, 0, 1);
}