#version 460
out vec4 fragColor;
in vec2 coords;
in vec2 outVector;

uniform float radius;

void main() {
    float dist = distance(gl_FragCoord.xy, coords);
    if(dist >= radius){
        discard;
    }
    float color = sqrt(pow(outVector[0], 2) + pow(outVector[1],2)) * 2;

    fragColor = vec4(color, 0, 1 - color, 1);
}