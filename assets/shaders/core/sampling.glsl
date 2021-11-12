float getHaltonSample(int index, int base){
    float f = 1.0f;
    float r = 0.0f;
    float i = index;
    while(i > 0){
        f = f / base;
        r = r + f * fmod(i, base);
        i = floor(i / base);
    }
    return r;
}