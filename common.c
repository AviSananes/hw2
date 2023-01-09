//
// Created by Miki Segall on 28/12/2022.
//

#include "common.h"


void separate_strings(char *str, char *str1, char *str2, char *str3)
{
    int i, j, k;
    int len = strlen(str);
    for (i = 0; i < len; i++)
    {
        if (str[i] == ' ')
        {
            break;
        }
        str1[i] = str[i];
    }
    str1[i] = '\0';
    for (j = 0; i < len; i++, j++)
    {
        if (str[i] == ' ')
        {
            break;
        }
        str2[j] = str[i];
    }
    str2[j] = '\0';
    for (k = 0; i < len; i++, k++)
    {
        str3[k] = str[i];
    }
    str3[k] = '\0';
}
