/* Copyright 2018 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "micro_features/micro_model_settings.h"

#if WORDCOUNT == 1 // SmartMirror hotword: marvin
const char* kCategoryLabels[kCategoryCount] = {
    "silence",
    "unknown",
    "marvin",
};
#endif

#if WORDCOUNT == 9 // 9words (8 demo words + marvin hotword): yes,no,up,down,left,right,on,off,marvin
const char* kCategoryLabels[kCategoryCount] = {
    "silence",
    "unknown",
    "yes",
    "no",
    "up",
    "down",
    "left",
    "right",
    "on",
    "off",
    "marvin",
};
#endif

#if WORDCOUNT == 2
const char* kCategoryLabels[kCategoryCount] = {
    "silence",
    "unknown",
    "yes",
    "no",
};
#endif

#if WORDCOUNT == 8 // Demo hotwords: yes,no,up,down,left,right,on,off
const char* kCategoryLabels[kCategoryCount] = {
    "silence",
    "unknown",
    "yes",
    "no",
    "up",
    "down",
    "left",
    "right",
    "on",
    "off"
};
#endif

#if WORDCOUNT == 10
const char* kCategoryLabels[kCategoryCount] = {
    "silence",
    "unknown",
    "yes",
    "no",
    "up",
    "down",
    "left",
    "right",
    "on",
    "off",
    "stop",
    "go"
};
#endif
