#version 460
out vec4 fragColor;
in vec2 coords;
in vec2 outVector;

uniform float radius;

void main() {
    float dist = distance(gl_FragCoord.xy, coords);
    float color = (sqrt(pow(outVector[0], 2) + pow(outVector[1],2)) - 0.35) * 2;

    if(dist >= radius){
        discard;
    } else{
        fragColor = vec4(color, color, 1, 1);
    }
}