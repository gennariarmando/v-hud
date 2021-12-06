#pragma once

class CTextRead {
public:
    char str[16];
    char* text;

    CTextRead::CTextRead() {
        Clear();
    }

    void Clear() {
        strcpy(str, "NONE");
        text = NULL;
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
    CTextRead TextList[512];

public:
    CTextNew();
    ~CTextNew();

    void ReadTextFile();
    CTextRead GetText(int s);
    CTextRead GetText(char* str);
    void UpperCase(char* s);

private:
    char GetUpperCase(char c);
};

extern CTextNew TextNew;