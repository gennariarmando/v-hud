#include "VHud.h"
#include "TextNew.h"
#include "Utility.h"
#include "MenuNew.h"

#include "CText.h"
#include "CFileMgr.h"

using namespace plugin;

CTextNew TextNew;

CTextNew::CTextNew() {
    ReadLanguagesFromFile();

    CdeclEvent<AddressList<0x6A03E3, H_CALL>, PRIORITY_AFTER, ArgPickNone, void(const char*)> OnTextLoad;

    OnTextLoad += [] {
        TextNew.Load();
    };

    auto openFile = [](const char* a, const char* b) {
        char file[32];
        sprintf(file, "%s.gxt", TextNew.TextSwitch.text[MenuNew.TempSettings.language].gxt);

        if (!FileCheck(file)) {
            sprintf(file, "%s.gxt", "AMERICAN.GXT");
        }

        return CFileMgr::OpenFile(file, b);
    };
    patch::RedirectJump(0x6A0228, (int(__cdecl*)(const char*, const char*))openFile);
    patch::RedirectJump(0x69FD5A, (int(__cdecl*)(const char*, const char*))openFile);
}

void CTextNew::ReadLanguagesFromFile() {
    std::ifstream file(PLUGIN_PATH("VHud\\data\\languages.dat"));

    if (file.is_open()) {
        int id = 0;
        for (std::string line; getline(file, line);) {
            char name[64];
            char gxt[64];
            char ini[64];

            if (!line[0] || line[0] == '\t' || line[0] == ' ' || line[0] == '#' || line[0] == '[')
                continue;

            sscanf(line.c_str(), "%s %s %s", &name, &ini, &gxt);

            strcpy(TextSwitch.text[id].name, name);
            strcpy(TextSwitch.text[id].ini, ini);
            strcpy(TextSwitch.text[id].gxt, gxt);

            TextSwitch.count = id;
            id++;
        }
        file.close();
    }
}

void CTextNew::Load() {
    std::ifstream file;
    char textFile[512];
    char* filePtr;
    sprintf(textFile, "VHud\\text\\%s.ini", TextNew.TextSwitch.text[MenuNew.TempSettings.language].ini);

    if (!FileCheck(PLUGIN_PATH(textFile)))
        filePtr = "VHud\\text\\english.ini";
    else
        filePtr = textFile;

    for (int i = 0; i < 512; i++) {
        strcpy(TextList[i].text, "\0");
    }

    file.open(PLUGIN_PATH(filePtr));
    if (file.is_open()) {
        int id = 0;

        for (std::string line; getline(file, line);) {
            char str[16];
            char* text;
            int r, g, b, a;

            if (!line[0] || line[0] == '#' || line[0] == '[' || line[0] == ';')
                continue;

            text = new char[16000];
            sscanf(line.c_str(), "%s = %[^\n]", &str, text);

            strcpy(TextList[id].str, str);
            strcpy(TextList[id].text, text);
            id++;

            delete[] text;
        }
        file.close();
    }
}

CTextRead CTextNew::GetText(int s) {
    return TextList[s];
}

CTextRead CTextNew::GetText(const char* str) {
    CTextRead result = CTextRead(str);

    if (*str == '\0')
        return result;

    for (int i = 0; i < 512; i++) {
        if (TextList[i].str[0] == str[0]
            && TextList[i].str[1] == str[1]
            && TextList[i].str[2] == str[2]
            && TextList[i].str[3] == str[3]
            && !faststrcmp(str, TextList[i].str, 4)) {
            result = GetText(i);
            break;
        }
    }
    return result;
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
