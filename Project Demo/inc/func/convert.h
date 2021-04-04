#include<stdlib.h>
#include<string.h>
#include<wchar.h>

void ctob(unsigned char n, char *binary){ /* return binary value as string */
    binary[8] = '\0';
    for (int i = 7; i >= 0; i--)
    {
        if (n <= 0)
            binary[i] = '0';
        else
        {
            binary[i] = n % 2 + '0';
            n = n / 2;
        }
    }
}

int mod(char sqr){ /* calculate 2^sqr */
    int res = 1;
    for (int i = 1; i <= sqr; i++)
        res *= 2;
    return res;
}

int btoi(char *binary){ /* convert binary string to integer */
    char a = strlen(binary) - 1;
    int res = 0;
    for (int i = a; i >= 0; i--)
    {
        if (binary[a - i] == '1')
            res += mod(i);
    }
    return res;
}

void UTF8_to_UTF32(unsigned char *word, wchar_t *tempw){
    int count = 0, uni = 0;
    unsigned char temp[10] = "";
    unsigned char bit[60] = "";
    for (int i = 0; i <= strlen(word); i++) /* convert UTF-8 string to UTF-32 long string */
    {
        if (word[i] < 128 || word[i] > 191)
        {
            if (uni)
            {
                uni = 0;
                tempw[count] = btoi(bit);
                bit[0] = '\0';
                count++;
            }
        }
        if (word[i] < 128)
        {
            tempw[count] = word[i];
            count++;
        }
        else
        {
            ctob(word[i], temp);
            uni++;
            if (!strncmp(temp, "110", 3))
                strcat(bit, temp + 3);
            else if (!strncmp(temp, "1110", 4))
                strcat(bit, temp + 4);
            else if (!strncmp(temp, "11110", 5))
                strcat(bit, temp + 5);
            else
                strcat(bit, temp + 2);
        }
    }
    if (uni)
    {
        tempw[count] = btoi(bit);
        bit[0] = '\0';
        count++;
    }
}