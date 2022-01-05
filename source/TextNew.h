#pragma once

class CTextRead {
public:
    char str[16];
    char text[16000];
    
public:
    CTextRead() { }

    CTextRead(const char* s) {
        strcpy(str, s);
        strcpy(text, s);
    }
};

class CTextSwitch {
public:
    int count;

    struct {
        char name[64];
        char ini[64];
        char gxt[64];
    } text[16];

public:
    CTextSwitch() {
        for (int i = 0; i < 16; i++) {
            strcpy(text[i].name, "NONE");
            strcpy(text[i].ini, "NONE");
            strcpy(text[i].gxt, "NONE");
        }
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
    CTextSwitch TextSwitch;

public:
    CTextNew();

    void ReadLanguagesFromFile();

    void Load();
    void LoadGameText();
    CTextRead GetText(int s);
    CTextRead GetText(const char* str);
    void UpperCase(char* s);

private:
    char GetUpperCase(char c);
};

extern CTextNew TextNew;