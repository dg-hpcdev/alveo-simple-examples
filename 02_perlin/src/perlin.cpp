/**
*
* Copyright 2022 Design Gateway Co., Ltd.
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*     http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
* 
*/

#include <stdio.h>
#include <math.h>
#include "hls_stream.h"

extern "C" {

const int p[] = {
    151,160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

struct Coord
{
    float x;
    float y;
};

void gen_coord(hls::stream<Coord>& coord_stream, const int nx, const int ny, const float freq)
{
    GEN_COORD_OUTER: for(int y=0; y<ny; ++y){
       GEN_COORD_INNER: for(int x=0; x<nx; ++x){
            coord_stream.write({.x=x*freq, .y=y*freq});
       }
	}
}

float dot(int dir, float x, float y)
{
    switch (dir&0x3)
    {
    case 0:
        return x+y;
        break;
    case 1:
        return -x+y;
        break;
    case 2:
        return -x-y;
        break;
    default:
        return x-y;
        break;
    }
}

float interpolate(float x0, float x1, float t)
{
    return (x1-x0)*(3.0 - t * 2.0) * t * t  + x0;
}

void perlin_calc(hls::stream<Coord>& coord_stream, hls::stream<float>& noise_stream, const int N)
{
    for(int i=0; i<N; ++i){
        Coord c = coord_stream.read();
        const float x = c.x;
        const float y = c.y;

        float fX = floor(x);
        float fY = floor(y);

        int X = ((int)fX) & 0xff;
        int Y = ((int)fY) & 0xff;

        float diffX = x - fX;
        float diffY = y - fY;

        float topright = dot(p[p[X+1]+Y+1], diffX-1, diffY-1);
        float topleft = dot(p[p[X]+Y+1], diffX, diffY-1);
        float bottomright = dot(p[p[X+1]+Y], diffX-1, diffY);
        float bottomleft = dot(p[p[X]+Y], diffX, diffY);

        float top = interpolate(topleft, topright, diffX);
        float bottom = interpolate(bottomleft, bottomright, diffX);
        float val = interpolate(bottom, top, diffY);
        noise_stream.write(val);
    }
}

void write_mem(hls::stream<float>& noise_stream, float* out, const int N)
{
    WRITE_MEM_LOOP: for(int i=0; i<N; ++i){
        out[i] = noise_stream.read();
    }
}

void perlin(int nx, int ny, float* result, const float freq)
{
#pragma HLS INTERFACE m_axi port=result
#pragma HLS DATAFLOW
    
    hls::stream<Coord> coord_stream;
    hls::stream<float> noise_stream;

    const int N = nx*ny;

    gen_coord(coord_stream, nx, ny, freq);
    perlin_calc(coord_stream, noise_stream, N);
    write_mem(noise_stream, result, N);
}

}
