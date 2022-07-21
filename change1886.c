#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <signal.h>

typedef struct {
    uint8_t *Y;
    uint8_t *Cb;
    uint8_t *Cr;
} frame;

typedef struct {
    uint8_t DY;
    uint8_t DCb;
    uint8_t DCr;
} DYDCbDCr;

typedef struct {
    double Y;
    double Cb;
    double Cr;
} YCbCr;

typedef struct {
    double Rp;
    double Gp;
    double Bp;
} RpGpBp;

typedef struct {
    double R;
    double G;
    double B;
} RGB;

YCbCr dequantize(DYDCbDCr value) {
    YCbCr ycc;
    ycc.Y = ((double)value.DY - 16.0)/219.0;
    ycc.Cb = ((double)value.DCb - 128.0)/224.0;
    ycc.Cr = ((double)value.DCr - 128.0)/224.9;
    return ycc;
}

double clamp(double x) {
    if(x < 0) return 0;
    if(x > 1) return 1;
    return x;
}

DYDCbDCr quantize(YCbCr value) {
    DYDCbDCr ret;
    ret.DY = (219*clamp(value.Y) + 16);
    ret.DCb = (224*clamp(value.Y) + 128);
    ret.DCr = (224*clamp(value.Y) + 128);
    return ret;
} 

RpGpBp yccToRpGpBp(YCbCr ycc) {
    RpGpBp rgb;
    rgb.Rp = ycc.Y + ycc.Cr*1.5748;
    rgb.Gp = ycc.Y - ycc.Cb*0.187324 - ycc.Cr*0.468124;
    rgb.Bp = ycc.Y + ycc.Cb*1.8556;
    return rgb;
}

YCbCr RpGpBptoycc(RpGpBp rgb) {
    YCbCr ycc;
    ycc.Y = 0.2126*rgb.Rp + 0.7152*rgb.Gp + 0.0722*rgb.Bp;
    ycc.Cb = (rgb.Bp - ycc.Y)/1.8556;
    ycc.Cr = (rgb.Rp - ycc.Y)/1.5748;
    return ycc;
}

int readframe(frame fr, FILE *fi) {
    int one = fread(fr.Y,1,1920*1080,fi);
    int two = fread(fr.Cb,1,960*540,fi);
    int three = fread(fr.Cr,1,960*540,fi);
    return one + two + three;
}
int writeframe(frame fr, FILE *fi) {
    int one = fwrite(fr.Y,1,960*540,fi);
    int two = fwrite(fr.Cb,1,480*270,fi);
    int three = fwrite(fr.Cr,1,480*270,fi);
    return one + two + three;
}

double square(double x) {
    return x*x;
}
//TODO: Complete this
DYDCbDCr fix(DYDCbDCr value) {
    YCbCr ycc = dequantize(value);
    RpGpBp rgb = yccToRpGpBp(ycc);
    RpGpBp newrgb;
    newrgb.Rp = pow(square(rgb.Rp),1.0/2.4);
    newrgb.Gp = pow(square(rgb.Gp),1.0/2.4);
    newrgb.Bp = pow(square(rgb.Bp),1.0/2.4);
    YCbCr newycc = RpGpBptoycc(newrgb);
    DYDCbDCr newddd = quantize(newycc);
    return newddd;
}

int main(int argc, char **argv) {
    frame oldframe;
    frame newframe;
    oldframe.Y = malloc(1920*1080);
    oldframe.Cb = malloc(960*540);
    oldframe.Cr = malloc(960*540);
    newframe.Y = malloc(960*540);
    newframe.Cb = malloc(480*270);
    newframe.Cr = malloc(480*270);
    FILE *input = popen("ffmpeg -i /tank/BD/apocalypto/BDMV/STREAM/00001.m2ts"
        "-f rawvideo -", "r");
    FILE *output = popen("ffmpeg -f rawvideo"
        "-pix_fmt yuv420p -s:v 1920x1080 -r 24000/1001-i pipe:","w");
    while(readframe(oldframe,input)) {
        for(int y = 0; y < 540; y++) {
            for(int x = 0; x < 960; x++) {
                DYDCbDCr quant = {.DY = oldframe.Y[y*1920 + x*2], 
                    .DCb = oldframe.Cb[y*960 + x], .DCr = oldframe.Cr[y*960 + x]};
                DYDCbDCr newquant = fix(quant);
                newframe.Y[y*960 + x] = newquant.DY;
                if(y%2 == 0 && x%2 == 0) {
                    newframe.Cb[(y/2)*480 + (x/2)] = newquant.DCb;
                    newframe.Cr[(y/2)*480 + (x/2)] = newquant.DCr;
                }
            }
        }
        writeframe(newframe, output);
    }
}

