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
extern "C"{
    const unsigned int K[] = {
        0XD76AA478,
        0XE8C7B756,
        0X242070DB,
        0XC1BDCEEE,
        0XF57C0FAF,
        0X4787C62A,
        0XA8304613,
        0XFD469501,
        0X698098D8,
        0X8B44F7AF,
        0XFFFF5BB1,
        0X895CD7BE,
        0X6B901122,
        0XFD987193,
        0XA679438E,
        0X49B40821,
        0XF61E2562,
        0XC040B340,
        0X265E5A51,
        0XE9B6C7AA,
        0XD62F105D,
        0X02441453,
        0XD8A1E681,
        0XE7D3FBC8,
        0X21E1CDE6,
        0XC33707D6,
        0XF4D50D87,
        0X455A14ED,
        0XA9E3E905,
        0XFCEFA3F8,
        0X676F02D9,
        0X8D2A4C8A,
        0XFFFA3942,
        0X8771F681,
        0X6D9D6122,
        0XFDE5380C,
        0XA4BEEA44,
        0X4BDECFA9,
        0XF6BB4B60,
        0XBEBFBC70,
        0X289B7EC6,
        0XEAA127FA,
        0XD4EF3085,
        0X04881D05,
        0XD9D4D039,
        0XE6DB99E5,
        0X1FA27CF8,
        0XC4AC5665,
        0XF4292244,
        0X432AFF97,
        0XAB9423A7,
        0XFC93A039,
        0X655B59C3,
        0X8F0CCC92,
        0XFFEFF47D,
        0X85845DD1,
        0X6FA87E4F,
        0XFE2CE6E0,
        0XA3014314,
        0X4E0811A1,
        0XF7537E82,
        0XBD3AF235,
        0X2AD7D2BB,
        0XEB86D391
    };

    const unsigned char S[][16] = {
        {7,12,17,22,7,12,17,22,7,12,17,22,7,12,17,22},
        {5,9,14,20,5,9,14,20,5,9,14,20,5,9,14,20},
        {4,11,16,23,4,11,16,23,4,11,16,23,4,11,16,23},
        {6,10,15,21,6,10,15,21,6,10,15,21,6,10,15,21}
    };
    void md5_round1(const unsigned int *M, const unsigned int *in, unsigned int *out)
    {
        unsigned int A = in[0];
        unsigned int B = in[1];
        unsigned int C = in[2];
        unsigned int D = in[3];
        
        
        for(int i=0; i<16; ++i){
            unsigned int tmp0 = (B&C)|(~B&D);
            unsigned int tmp1 = A + tmp0;
            unsigned int tmp2 = M[i] + K[i] + tmp1;
            unsigned int tmp3 = (tmp2 << S[0][i]) | (tmp2 >> (32-S[0][i]));
            A = D;
            D = C;
            C = B;
            B = tmp3 + B;
            
        }
        out[0] = A;
        out[1] = B;
        out[2] = C;
        out[3] = D;
    }
    void md5_round2(const unsigned int *M, const unsigned int *in, unsigned int *out)
    {
        unsigned int A = in[0];
        unsigned int B = in[1];
        unsigned int C = in[2];
        unsigned int D = in[3];
        for(int i=0; i<16; ++i){
            unsigned int tmp0 = (B&D)|(C&~D);
            unsigned int tmp1 = A + tmp0;
            // unsigned int tmp2 = M[16+i] + K[16+i] + tmp1;
            unsigned int tmp2 = M[(((i+16) * 5) + 1) % 16] + K[16+i] + A + tmp0;
            unsigned int tmp3 = (tmp2 << S[1][i]) | (tmp2 >> (32-S[1][i]));
            A = D;
            D = C;
            C = B;
            B = tmp3 + B;
            
        }
        out[0] = A;
        out[1] = B;
        out[2] = C;
        out[3] = D;
    }
    void md5_round3(const unsigned int *M, const unsigned int *in, unsigned int *out)
    {
        unsigned int A = in[0];
        unsigned int B = in[1];
        unsigned int C = in[2];
        unsigned int D = in[3];
        for(int i=0; i<16; ++i){
            unsigned int tmp0 = B^C^D;
            unsigned int tmp1 = A + tmp0;
            unsigned int tmp2 = M[(((i+32) * 3) + 5) % 16] + K[32+i] + tmp1;
            unsigned int tmp3 = (tmp2 << S[2][i]) | (tmp2 >> (32-S[2][i]));
            A = D;
            D = C;
            C = B;
            B = tmp3 + B;
            
        }
        out[0] = A;
        out[1] = B;
        out[2] = C;
        out[3] = D;
    }
    void md5_round4(const unsigned int *M, const unsigned int *in, unsigned int *out)
    {
        unsigned int A = in[0];
        unsigned int B = in[1];
        unsigned int C = in[2];
        unsigned int D = in[3];
        for(int i=0; i<16; ++i){
            unsigned int tmp0 = C^(B|~D);
            unsigned int tmp1 = A + tmp0;
            unsigned int tmp2 = M[((i+48) * 7) % 16] + K[48+i] + tmp1;
            unsigned int tmp3 = (tmp2 << S[3][i]) | (tmp2 >> (32-S[3][i]));
            A = D;
            D = C;
            C = B;
            B = tmp3 + B;
            
        }
        out[0] = A;
        out[1] = B;
        out[2] = C;
        out[3] = D;
    }
    void md5(const unsigned int *in, unsigned int *out, const size_t size){
#pragma HLS INTERFACE m_axi port=in
#pragma HLS INTERFACE m_axi port=out
        
        const unsigned int *M = in;
        unsigned int v0[] = {0x67452301,0xEFCDAB89,0x98BADCFE,0x10325476};
        unsigned int v1[4];
        unsigned int v2[4];
        unsigned int v3[4];
        unsigned int v4[4];

        for(size_t i=0; i<size; i+=64)
        {
            md5_round1(M, v0, v1);       
            md5_round2(M, v1, v2);        
            md5_round3(M, v2, v3);
            md5_round4(M, v3, v4);
            
            M += 16;
            v0[0] += v4[0];
            v0[1] += v4[1];
            v0[2] += v4[2];
            v0[3] += v4[3];
        }
        out[0] = v0[0];
        out[1] = v0[1];
        out[2] = v0[2];
        out[3] = v0[3];
    }
}