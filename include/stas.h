#ifndef _STAS_H_
#define _STAS_H_

/* STT Task Allocation Server : S-TAS
 * 
 * HISTORY
 * 
 * V 0.3 : 솔루션용 개발 버전, 신규 DB 적용 및 Notifier, Schd4DB 모듈 추가 및 개발 중 - 2018/05/16
 * V 0.2 : 솔루션용 개발 버전 - 2018/03/28
 * V 0.1 : 데모용 버전 개발 완료, 정식 이름 결정
 */
#include "configuration.h"


#define STAS_VERSION_MAJ 0
#define STAS_VERSION_MIN 3
//#define STAS_VERSION_BLD 180307

using namespace itfact::common;


extern Configuration *config;

#endif // _STAS_H_