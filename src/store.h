#ifndef __STORE_H__
#define __STORE_H__

#include <Arduino.h>

class Store {
public:
    bool newScene(char *sceneName);
    bool sceneSave(uint16_t sceneNum);
    bool sceneDelete(uint16_t sceneNum);
    bool sceneLoad(uint16_t sceneNum);
    void scenesClear();
};
extern Store store;

#endif
