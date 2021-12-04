#include "VHud.h"
#include "TextNew.h"
#include "Utility.h"
#include "CText.h"
#include "CMenuManager.h"

using namespace plugin;

CTextNew TextNew;

CTextNew::CTextNew() {
    CdeclEvent<AddressList<0x6A03E3, H_CALL>, PRIORITY_AFTER, ArgPickNone, void(const char*)> OnTextLoad;  

    OnTextLoad += [] {
        TextNew.ReadTextFile();
    };
}

CTextNew::~CTextNew() {
    for (int i = 0; i < 512; i++) {
        delete TextList[i].text;
    }
}

void CTextNew::ReadTextFile() {
    std::ifstream file;

    char* textFile = NULL;
    switch (FrontEndMenuManager.m_nLanguage) {
    case LANGUAGE_AMERICAN:
        textFile = "VHud\\text\\english.ini";
        break;
    case LANGUAGE_FRENCH:
        textFile = "VHud\\text\\french.ini";
        break;
    case LANGUAGE_GERMAN:
        textFile = "VHud\\text\\german.ini";
        break;
    case LANGUAGE_ITALIAN:
        textFile = "VHud\\text\\italian.ini";
        break;
    case LANGUAGE_SPANISH:
        textFile = "VHud\\text\\spanish.ini";
        break;
    default:
        textFile = "VHud\\text\\custom.ini";
        break;
    }

    if (!FileCheck(PLUGIN_PATH(textFile)))
        textFile = "VHud\\text\\english.ini";

    file.open(PLUGIN_PATH(textFile));

    if (file.is_open()) {
        int id = 0;

        for (std::string line; getline(file, line);) {
            char str[16];
            char text[16000];
            int r, g, b, a;

            if (!line[0] || line[0] == '#' || line[0] == '[' || line[0] == ';')
                continue;

            sscanf(line.c_str(), "%s = %[^\n]", &str, &text);

            strcpy(TextList[id].str, str);

            int length = strlen(text);
            TextList[id].text = new char [length];
            strncpy(TextList[id].text, text, length);
            TextList[id].text[length] = '\0';
            id++;
        }
        file.close();
    }

    TextResult.Clear();
}

CTextRead CTextNew::GetText(int s) {
    return TextList[s];
}

CTextRead CTextNew::GetText(char* str) {
    if (str[0] == '\0')
        return TextResult;

    for (int i = 0; i < 512; i++) {
        if (!faststrcmp(str, TextList[i].str)) {
            TextResult = GetText(i);
            break;
        }
    }
    return TextResult;
}

char CTextNew::GetUpperCase(char c) {
    if (c >= 'a' && c <= 'z') {
       c = c - ('a' - 'A');
    }
    return c;
}

void CTextNew::UpperCase(char* s) {
    while (*s) {
        *s = GetUpperCase(*s);
        s++;
    }
}
