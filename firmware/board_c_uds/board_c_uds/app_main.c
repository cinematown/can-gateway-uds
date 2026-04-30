#include "app_main.h"

/*
 * 현재 Board C는 freertos.c의 weak UDSMainTask()를 ap_main.c에서 오버라이딩합니다.
 * App_Init/App_Run은 다른 보드 구조와 맞추기 위한 빈 함수입니다.
 */
void App_Init(void)
{
}

void App_Run(void)
{
}
