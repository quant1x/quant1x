#include <iconv.h>
#include <iostream>
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <windows.h>
#include <quant1x/ta/simd.h>

int code_convert(const char *from_charset, const char *to_charset, char *inbuf, size_t inlen,
                 char *outbuf, size_t outlen) {
    iconv_t cd;
    char **pin = &inbuf;
    char **pout = &outbuf;

    cd = iconv_open(to_charset, from_charset);
    if (cd == 0) {
        return -1;
    }
    memset(outbuf, 0, outlen);

    if ((int) iconv(cd, pin, &inlen, pout, &outlen) == -1) {
        iconv_close(cd);
        return -1;
    }
    iconv_close(cd);
    //*pout = '\0';
    *(outbuf + outlen) = '\0';

    return 0;
}

int u2g(char *inbuf, size_t inlen, char *outbuf, size_t outlen) {
    return code_convert("utf-8", "gbk", inbuf, inlen, outbuf, outlen);
}

int g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen) {
    return code_convert("gbk", "utf-8", inbuf, inlen, outbuf, outlen);
}

std::string GBKToUTF8(const std::string &strGBK) {
    int length = strGBK.size() * 2 + 1;

    char *temp = (char *) malloc(sizeof(char) * length);

    if (g2u((char *) strGBK.c_str(), strGBK.size(), temp, length) >= 0) {
        std::string str_result;
        str_result.append(temp);
        free(temp);
        return str_result;
    } else {
        free(temp);
        return "";
    }
}

std::string UTFtoGBK(const char *utf8) {
    int length = strlen(utf8);

    char *temp = (char *) malloc(sizeof(char) * length);

    if (u2g((char *) utf8, length, temp, length) >= 0) {
        std::string str_result;
        str_result.append(temp);
        free(temp);

        return str_result;
    } else {
        free(temp);
        return "";
    }
}

// Function to convert UTF-8 string to GBK
char* utf8_to_gbk(const char* utf8_str) {
    int len_utf8 = MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, NULL, 0);
    wchar_t* wide_str = (wchar_t*)malloc(len_utf8 * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, wide_str, len_utf8);

    int len_gbk = WideCharToMultiByte(CP_ACP, 0, wide_str, -1, NULL, 0, NULL, NULL);
    char* gbk_str = (char*)malloc(len_gbk);
    WideCharToMultiByte(CP_ACP, 0, wide_str, -1, gbk_str, len_gbk, NULL, NULL);

    free(wide_str);
    return gbk_str;
}

int main() {
    // Set the console code page to GBK
    //SetConsoleOutputCP(936);
    SetConsoleOutputCP(CP_UTF8);
    std::string teststr = "测试字符串";
    printf("%s\n", teststr.c_str());
    // Convert UTF-8 string to GBK
    char* gbk_str = utf8_to_gbk(teststr.c_str());

    // Print the GBK encoded string
    printf("%s\n", gbk_str);

    // Free allocated memory
    free(gbk_str);

    printf("origin string: %s\n", teststr.c_str());
    std::cout << "origin string: " << teststr.c_str() << std::endl;
    std::cout << "UTF8 => GBK ：" << UTFtoGBK(teststr.c_str()).c_str() << std::endl;
    std::cout << "UTF8 => GBK ：" << GBKToUTF8(teststr.c_str()).c_str() << std::endl;
    std::cout << " GBK => UTF8：" << GBKToUTF8(UTFtoGBK(teststr.c_str()).c_str()).c_str() << std::endl;
    //std::cout << " GBK => UTF8：" << UTFtoGBK(GBKToUTF8(teststr.c_str()).c_str()).c_str() << std::endl;
    //getchar();
    std::cout << "simd\n";
    vector_type a,b,c;
    int count = 1000;
    for(int i = 0; i < count; i++) {
        a.push_back(i);
        b.push_back(i);
    }
    c.resize(count);
    //vector_type  *c = new vector_type(count)
    mean(a,b,c);
    // 使用范围基 for 循环遍历 vector 并输出每个元素
    int k = 0;
    const int max_line = 10;
    for (const auto& element : c) {
        if (k % max_line == 0 ) {
            std::cout << "\t";
        } else {
            std::cout << " ";
        }
        std::cout << "\t"<< element;
        ++k;
        if (k != 0 && k % max_line == 0 ) {
            std::cout << "\n";
        }
    }
    std::cout << std::endl;

    int n = 8; // 小数位数为 8
    int result = (3 << n) >> 1; // 左移 8 位 → 右移 1 位
    float final = (float)result / (1 << n); // 解释为浮点数（仅用于验证）
    printf("%f\n", final); // 输出 1.5
    return 0;
}
