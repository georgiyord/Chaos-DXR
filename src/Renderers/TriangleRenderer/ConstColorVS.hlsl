struct VSInput {
  float2 position : POSITION;
  // float angle : ANGLE;
};

struct PSInput {
  float4 position : SV_POSITION;
};

PSInput main(VSInput inputVertex) {
  // float s, c;
  // sincos(inputVertex.angle, s, c);
  // float2x2 rotationMatrix = float2x2(c, -s, s, c);
  float2x2 rotationMatrix = float2x2(1, 0, 0, 1);
  return PSInput(float4(mul(inputVertex.position, rotationMatrix), 0.0, 1.0));
};