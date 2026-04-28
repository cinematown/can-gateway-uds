#ifndef AP_MAIN_H
#define AP_MAIN_H

#include "hw_def.h"
// 이 선언이 있어야 freertos.c에서 오버라이딩이 성공합니다.
void StartDefaultTask(void *argument); 

#endif