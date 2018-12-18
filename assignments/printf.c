#include "printf.h"
#include "assert.h"
#include "strings.h"
#include <stdarg.h>
#include "uart.h"

#define UART_BAUD_RATE	115200
#define MAX_OUTPUT_LEN 1024



int unsigned_to_base(char *buf, int bufsize, unsigned int val, int base, int min_width) 
{
    int digits = 0;
    int tmpVal = val;
    int leading0s = 0;
    while(tmpVal){
        tmpVal /= base;
        digits++; 
    }
    if (min_width > digits){
        leading0s = min_width - digits;
    }
    unsigned char tmpBuf[digits + leading0s];
    for(int i = 0; i < leading0s; i++){
        tmpBuf[i] = '0';
    }
    tmpBuf[digits + leading0s] = '\0';
    int tmpDigits = digits;
    while(tmpDigits){
        int tmpChar = (val%base);
        if (tmpChar > 9){
            tmpChar -=10;
            tmpChar += 'a';
        } else {
            tmpChar += '0';
        }
        tmpBuf[(tmpDigits + leading0s) - 1] = (tmpChar);
        val /= base;
        tmpDigits--;
    }
    for(int i = 0; i < (bufsize - 1); i++){
        if (tmpBuf[i] == 0){
            buf[i] = 0;
            break;
        }
        buf[i] = tmpBuf[i];
    }
    buf[bufsize - 1] = '\0';
    return leading0s + digits;
}


int signed_to_base(char *buf, int bufsize, int val, int base, int min_width) 
{
    int negative = 0;
    if (val < 0){
        buf[0] = '-';
        negative = 1;
        val = 0 - val;
    }
    int value = unsigned_to_base(&buf[negative], bufsize - negative, val, base, min_width - negative) + negative;
    return value;
}

int vsnprintf(char *buf, int bufsize, const char *format, va_list args) 
{
clearString(buf, 100);
    int totalLen = 0; 
    const int origLen = strlen(format);
    char *locations[100];
    int totalArguments = argLocations(format, locations);
    char strSection[100];
    memcpy(strSection, format, origLen);
    strSection[origLen] = 0;
    if(totalArguments == 0){
        memcpy(buf, strSection, strlen(strSection));
        totalLen += strlen(strSection);
        return strlen(buf);
    }
    int i = 0;
    int offset = locations[i] - format; //how many bytes of strSection should be copied over before the '%'
    memcpy(buf, strSection, offset);
    buf[offset] = 0;

    while(i < totalArguments){
        int hexWidth = 0;
        char *nextLoc = locations[i]; //memory address of '%'
        char nextArg = *(nextLoc + 1); //d, p, x, ...
        char nextArgString[100];
        clearString(nextArgString, 100);
        int spacing;
        if(nextArg == 'd'){
            int val = va_arg(args, int);
            spacing = signed_to_base(nextArgString, 100, val, 10, 0);
        } else if (nextArg == 'p'){
            void *p = va_arg(args, void*);
            strlcat(nextArgString, "0x", 100);
            char address[100];
            clearString(address, 100);
            spacing = unsigned_to_base(address, 100, (int)p, 16, 8);
            strlcat(nextArgString, address, 100);
        } else if (nextArg == 'x') {
            int val = va_arg(args, int);
            spacing = signed_to_base(nextArgString, 100, val, 16, 0);
        } else if (nextArg == '0'){//for the hex one, use the **endptr
            char **endptr = NULL;
            int width = strtonum(nextLoc + 1, endptr);
            int val = va_arg(args, int);
            spacing = signed_to_base(nextArgString, 100, val, 16, width);
            hexWidth = 1;
            while(width){
                width /= 10;
                hexWidth++;
            }
            nextArgString[spacing] = 0;
        } else if (nextArg == 's'){
            char *s = va_arg(args, char*);
            memcpy(nextArgString, s, strlen(s));
        } else if (nextArg == 'c'){
            char nextChar = va_arg(args, int);
            nextArgString[0] = nextChar;
        }
        strlcat(buf, nextArgString, MAX_OUTPUT_LEN);

        clearString(strSection, 100);
        memcpy(strSection, locations[i] + 2 + hexWidth, strlen(locations[i] + 2 + hexWidth)); //strSection now holds the rest of format, including '%' symbols
        if((i + 1) < totalArguments){ //if there are more arguments to come
            offset = ((locations[i + 1]) - locations[i] + 2); //offset is set to the number of chars before the next '%' symbol or string
        } else {
            offset = 0;
        }
        char checkStr[100];
        clearString(checkStr, 100);
        memcpy(checkStr, &strSection[offset], strlen(&strSection[offset]));
        if(strcmp(nextArgString, checkStr) != 0){
            clearString(nextArgString, 100);
            memcpy(nextArgString, checkStr, strlen(checkStr));
            if(nextArgString != '%'){
                strlcat(buf, nextArgString, MAX_OUTPUT_LEN);
            }
        }
        i++;
    }

    return 0;
}


/*Returns an array of pointers. Pointers point to the memory address within the string where
** '%' is housed. Array is null-pointer-terminated. When the user implements this function
** the memory location (+ 1) will return the char defining the type of argument to expect. **/

int argLocations(char *buf, char **locations){
    int i = 0;
    int args = 0;
    while(buf[i]){
        if(buf[i] == '%'){
            locations[args] = &buf[i];
            i++;
            args++;
        } else {
            i++;
        }
    }
    *locations[args] = NULL;
    return args;
}

void clearString(char *s, int length);

int snprintf(char *buf, int bufsize, const char *format, ...) 
{
    va_list args;
    va_start(args, format);
    return vsnprintf(buf, bufsize, format, args);
}

int printf(const char *format, ...) 
{   
    char buf[MAX_OUTPUT_LEN];
    clearString(buf, strlen(buf));
    va_list args;
    va_start(args, format);
    vsnprintf(buf, MAX_OUTPUT_LEN, format, args);
    return strlen(buf);
    for(int i = 0; i < strlen(format); i++){
        uart_putchar(format[i]);
    }
}
