#include "strings.h"
#include "assert.h"

void *memset(void *s, int c, size_t n){
    /*The least significant byte of the second argument will be stored at the first argument*/
    for (int i = 0; i < n; i++){  
        *((unsigned char*)s + i) = (unsigned char)c; //takes the least significant byte of c
    }
    return s;
}

void *memcpy(void *dst, const void *src, size_t n)
{
    if(n == 0){
        return dst;
    }
    for (int i = 0; i < n; i++){
        *((unsigned char*)dst + i) = *((unsigned char*)src + i);
    }
    return dst;
}

void clearString(char *s, int length){
    for(int i = 0; i < length; i++){
        s[i] = 0;
    }
}

int strlen(const char *s)
{
    /*Implementation a gift to you from lab3*//*WOW I am so grateful for this teaching team :,) */
    int i;
    for (i = 0; s[i] != '\0'; i++) ;
    return i;
}

int strcmp(const char *s1, const char *s2)
{
    int i = 0;
    while(s1[i] == s2[i]){
        if(!s1[i]){
            return 0;
        }
        i++;
    }
    if (s1[i] < s2[i]){
        return -1;
    } 
    if (s1[i] > s2[i]){
        return 1;
    }
    return 0;
}

int strlcat(char *dst, const char *src, int maxsize)
{   
    int size = strlen(dst);
    int i = size;
    if(maxsize < size){
        dst[maxsize] = 0;
    }
    while(i < maxsize && src[i-size]){
        dst[i] = src[i-size];
        i++;
    }
    dst[i] = 0;
    return i;
}

unsigned int strtonum(const char *str, const char **endptr) {
    //assert(0 == 0);
    unsigned int index = 0;
    unsigned char currChar;
    unsigned int value = 0;
    unsigned int hex = 0;
    unsigned int base = 10;
    if (str[0] == '0' && str[1] == 'x'){
        hex = 1;
        base = 16;
        str = &str[2];
    }
    while(str[index]){
        currChar = str[index];
            if (currChar >= 'a' && currChar <= 'f'){
                if(hex){
                    currChar -= 'a';
                    currChar += 10;
                } else {
                    *endptr = &(str[index]);
                    return value;
                }
            } else if (currChar >= '0' && currChar <= '9'){
                currChar -= '0';
            } else {
                *endptr = &(str[index]);
                return value;
            }
        value = value * base + (unsigned int)currChar;
        index++;
    }
    
    *endptr = &(str[index]);
    return value;
}