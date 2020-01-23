precision mediump float;

varying vec2 v_texCoord;
const int textureSize = 3;
uniform float progress;
uniform float speed;
uniform sampler2D s_texture[textureSize];

void main() {
    vec4 firstColor = texture2D(s_texture[0], v_texCoord);
    vec4 secondColor = texture2D(s_texture[1], v_texCoord);

    float time = progress * speed > 1.0 ? 1.0 : pow(progress * speed, 5.0);
    gl_FragColor = mix(firstColor, secondColor, time);
}
