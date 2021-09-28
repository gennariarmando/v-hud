#pragma once

class CTextRead {
public:
    char str[16];
    char text[16000];

    CTextRead::CTextRead() {
        strcpy(str, "NONE");
        strcpy(text, "NONE");
    }
};

class CTextNew {
public:
    enum eLangList {
        LANGUAGE_AMERICAN,
        LANGUAGE_FRENCH,
        LANGUAGE_GERMAN,
        LANGUAGE_ITALIAN,
        LANGUAGE_SPANISH,
    };
    static CTextRead TextList[256];

public:
    CTextNew();
    static void ReadTextFile();
    static CTextRead GetText(int s);
    static CTextRead GetText(char* str);
    static void UpperCase(char* s);

private:
    static char GetUpperCase(char c);

};
