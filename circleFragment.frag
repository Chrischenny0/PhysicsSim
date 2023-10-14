#version 460
out vec4 fragColor;
void main() {
    float radius = 0.3;
    vec2 val = vec2(gl_FragCoord.x, gl_FragCoord.y);
    float dist = sqrt(dot(val, val));
    if(dist >= radius){
        discard;
    }
    fragColor = vec4(1, 0, 0, 1);
}