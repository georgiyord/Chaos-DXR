struct PSInput {
  float4 position : SV_POSITION;
};

float4 main(PSInput inputPixel) : SV_TARGET {
  return float4(1.0, 0.0, 0.0, 1.0);
};